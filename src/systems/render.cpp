#include <systems/render.hpp>

#include <components/visible.hpp>
#include <components/sprite.hpp>
#include <components/position.hpp>
#include <components/solid.hpp>
#include <components/flash.hpp>

#include <thread>
#include <chrono>
using namespace std::literals::chrono_literals;

#include <glm.hpp>
#include <render.hpp>
#include <engine.hpp>
#include <font.hpp>
#include <ui.hpp>

std::string RenderSystem::loadTexString;
Texture     RenderSystem::loadTexResult;

Texture RenderSystem::loadTexture(const std::string& file)
{
	loadTexString = file;
	loadTexResult = Texture();
	while (loadTexResult.isEmpty())
		std::this_thread::sleep_for(1ms);
	auto t = loadTexResult;
	loadTexResult = Texture();
	return t;
}

void RenderSystem::render(void)
{	
	if (!loadTexString.empty()) {
		loadTexResult = Texture(loadTexString, false);
		loadTexString.clear();
	}

	if (!loadTexResult.isEmpty())
		return;

	Render::worldShader.use();
	Render::worldShader.enable();

	game::entities.each<Visible, Sprite, Position>([](entityx::Entity entity, Visible &visible, Sprite &sprite, Position &pos) {
		// Verticies and shit
		float its = 0;
		
		float sz;
		if (entity.has_component<Solid>())
			sz = entity.component<Solid>()->width;
		else 
			sz = sprite.getSpriteSize().x;

		if (sprite.faceLeft) {
			glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(-1.0f,1.0f,1.0f));
			glm::mat4 translate = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f - sz - pos.x * 2.0f, 0.0f, 0.0f));

			glm::mat4 mov = scale * translate;
			glUniformMatrix4fv(Render::worldShader.uniform[WU_transform], 1, GL_FALSE, glm::value_ptr(mov));
		}

		for (auto &S : sprite.sprite) {
			auto sp = S.first;
			auto size = sp.size * game::HLINE;
			vec2 drawOffset (HLINES(S.second.x), HLINES(S.second.y));
			vec2 loc (pos.x + drawOffset.x, pos.y + drawOffset.y);

			GLfloat verts[] = {
				loc.x,          loc.y,          visible.z + its, sp.offset_tex.x,                 sp.offset_tex.y,
				loc.x + size.x,	loc.y,          visible.z + its, sp.offset_tex.x + sp.size_tex.x, sp.offset_tex.y,
				loc.x + size.x, loc.y + size.y, visible.z + its, sp.offset_tex.x + sp.size_tex.x, sp.offset_tex.y + sp.size_tex.y,
				loc.x,          loc.y,          visible.z + its, sp.offset_tex.x,                 sp.offset_tex.y,
				loc.x + size.x, loc.y + size.y, visible.z + its, sp.offset_tex.x + sp.size_tex.x, sp.offset_tex.y + sp.size_tex.y,
				loc.x,          loc.y + size.y, visible.z + its, sp.offset_tex.x,                 sp.offset_tex.y + sp.size_tex.y
			};

			sp.tex.use();

			glUniform1i(Render::worldShader.uniform[WU_texture], 0);

			glVertexAttribPointer(Render::worldShader.coord, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), verts);
			glVertexAttribPointer(Render::worldShader.tex, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), verts + 3);
			glDrawArrays(GL_TRIANGLES, 0, 6);

			if (entity.has_component<Flash>()) {
				auto& f = *entity.component<Flash>();
				if (f.ms > 0) {
					verts[2] = verts[7] = verts[12] = verts[17] = verts[22] = verts[27] = visible.z + its - 0.001f;
					float alpha = static_cast<float>(f.ms) / static_cast<float>(f.totalMs);
					glUniform4f(Render::worldShader.uniform[WU_tex_color], f.color.red, f.color.green, f.color.blue, alpha);
					glVertexAttribPointer(Render::worldShader.coord, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), verts);
					glVertexAttribPointer(Render::worldShader.tex, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), verts + 3);
					glDrawArrays(GL_TRIANGLES, 0, 6);
					glUniform4f(Render::worldShader.uniform[WU_tex_color], 1.0, 1.0, 1.0, 1.0);
				}
			}

			its-=.01;
		}
		glUniformMatrix4fv(Render::worldShader.uniform[WU_transform], 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));

		if (entity.has_component<Health>()) {
			float width = entity.component<Solid>()->width;
			auto& health = *entity.component<Health>();
			width *= health.health / static_cast<float>(health.maxHealth);

			GLfloat health_coord[] = {
				pos.x, pos.y, -9, 0, 0,
				pos.x + width, pos.y, -9, 0, 0,
				pos.x + width, pos.y - 5, -9, 0, 0,
				pos.x + width, pos.y - 5, -9, 0, 0,
				pos.x, pos.y - 5, -9, 0, 0,
				pos.x, pos.y, -9, 0, 0,
			};

			Colors::red.use();
			glVertexAttribPointer(Render::worldShader.coord, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), health_coord);
			glVertexAttribPointer(Render::worldShader.tex, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), health_coord + 3);
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}
	});

	Render::worldShader.disable();
	Render::worldShader.unuse();

	game::entities.each<Visible, Position, Solid, Name>([](entityx::Entity e, Visible &v, Position &pos, Solid& dim, Name &name) {
		(void)e;
		(void)v;
		FontSystem::setFontZ(-5.0f);
		UISystem::putStringCentered(vec2(pos.x + dim.width / 2, pos.y - FontSystem::getSize() - HLINES(0.5)), name.name);
	});
}

void RenderSystem::update(entityx::EntityManager &en, entityx::EventManager &ev, entityx::TimeDelta dt)
{
	(void)ev;
	(void)dt;

	en.each<Flash>([](entityx::Entity e, Flash& flash) {
		if (--flash.ms <= 0) // TODO delta time?
			e.remove<Flash>();
	});
}

