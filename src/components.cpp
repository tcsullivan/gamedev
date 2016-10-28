#include <components.hpp>

#include <entityx/entityx.h>
#include <events.hpp>

#include <render.hpp>
#include <ui.hpp>
#include <engine.hpp>
#include <world.hpp>

void MovementSystem::update(entityx::EntityManager &en, entityx::EventManager &ev, entityx::TimeDelta dt)
{
	(void)ev;
	en.each<Position, Direction>([dt](entityx::Entity entity, Position &position, Direction &direction) {
		(void)entity;
		position.x += direction.x * dt;
		position.y += direction.y * dt;
	});
}

void PhysicsSystem::update(entityx::EntityManager &en, entityx::EventManager &ev, entityx::TimeDelta dt)
{
	(void)ev;
	en.each<Direction, Physics>([dt](entityx::Entity entity, Direction &direction, Physics &physics) {
		(void)entity;
		// TODO GET GRAVITY FROM WOLRD
		direction.y += physics.g * dt;
	});
}

void RenderSystem::update(entityx::EntityManager &en, entityx::EventManager &ev, entityx::TimeDelta dt)
{
	(void)ev;
	Render::worldShader.use();

	en.each<Visible, Sprite, Position>([dt](entityx::Entity entity, Visible &visible, Sprite &sprite, Position &pos) {
		(void)entity;
		// Verticies and shit
		GLfloat tex_coord[] = {0.0, 0.0,
							   1.0, 0.0,
							   1.0, 1.0,

							   1.0, 1.0,
							   0.0, 1.0,
							   0.0, 0.0};

		GLfloat tex_coordL[] = {1.0, 0.0,
								0.0, 0.0,
								0.0, 1.0,

								0.0, 1.0,
								1.0, 1.0,
								1.0, 0.0};

		for (auto &S : sprite.sprite) {
			float width = S.first.size.x;
			float height = S.first.size.y;

			vec2 loc = vec2(pos.x + S.first.offset.x, pos.y + S.first.offset.y);

			GLfloat coords[] = {loc.x, 			loc.y, 			visible.z,
								loc.x + width, 	loc.y, 			visible.z,
								loc.x + width, 	loc.y + height, visible.z,

								loc.x + width, 	loc.y + height, visible.z,
								loc.x, 			loc.y + height, visible.z,
								loc.x, 			loc.y, 			visible.z};


			// make the entity hit flash red
			// TODO
			/*if (maxHitDuration-hitDuration) {
				float flashAmt = 1-(hitDuration/maxHitDuration);
				glUniform4f(Render::worldShader.uniform[WU_tex_color], 1.0, flashAmt, flashAmt, 1.0);
			}*/
			glBindTexture(GL_TEXTURE_2D, S.first.pic);
			glUniform1i(Render::worldShader.uniform[WU_texture], 0);
			Render::worldShader.enable();

			glVertexAttribPointer(Render::worldShader.coord, 3, GL_FLOAT, GL_FALSE, 0, coords);
			if (sprite.faceLeft)
				glVertexAttribPointer(Render::worldShader.tex, 2, GL_FLOAT, GL_FALSE, 0 ,tex_coordL);
			else
				glVertexAttribPointer(Render::worldShader.tex, 2, GL_FLOAT, GL_FALSE, 0 ,tex_coord);
			glDrawArrays(GL_TRIANGLES, 0, 6);

			glUniform4f(Render::worldShader.uniform[WU_tex_color], 1.0, 1.0, 1.0, 1.0);
		}
	});

	Render::worldShader.disable();
	Render::worldShader.unuse();

	en.each<Visible, Position, Solid, Name>([](entityx::Entity e, Visible &v, Position &pos, Solid& dim, Name &name) {
		(void)e;
		(void)v;
		ui::putStringCentered(pos.x + dim.width / 2, pos.y - ui::fontSize - HLINES(0.5), name.name);
	});
}

/*
void Entity::draw(void)
{
	GLfloat tex_coord[] = {0.0, 0.0,
						   1.0, 0.0,
						   1.0, 1.0,

						   1.0, 1.0,
						   0.0, 1.0,
						   0.0, 0.0};

	GLfloat tex_coordL[] = {1.0, 0.0,
						   	0.0, 0.0,
						   	0.0, 1.0,

						   	0.0, 1.0,
						   	1.0, 1.0,
						   	1.0, 0.0};

	GLfloat coords[] = {loc.x, loc.y, z,
						loc.x + width, loc.y, z,
						loc.x + width, loc.y + height, z,

						loc.x + width, loc.y + height, z,
						loc.x, loc.y + height, z,
						loc.x, loc.y, z};


	glActiveTexture(GL_TEXTURE0);

	if (!alive)
		return;

	if (type == NPCT) {
		NPCp(this)->drawThingy();

		if (gender == MALE)
			glColor3ub(255, 255, 255);
		else if (gender == FEMALE)
			glColor3ub(255, 105, 180);
	} else if (type == MOBT) {
		if (Mobp(this)->rider != nullptr) {
			Mobp(this)->rider->loc.x = loc.x + width * 0.25f;
	        Mobp(this)->rider->loc.y = loc.y + height - HLINES(5);
	        Mobp(this)->rider->vel.y = .12;
			Mobp(this)->rider->z     = z + 0.01;
	    }
	}
	switch(type) {
	case PLAYERT:
		static int texState = 0;
		if (speed && !(game::time::getTickCount() % ((2.0f/speed) < 1 ? 1 : (int)((float)2.0f/(float)speed)))) {
			if (++texState == 9)
				texState = 1;
			glActiveTexture(GL_TEXTURE0);
			tex(texState);
		}
		if (!ground) {
			glActiveTexture(GL_TEXTURE0);
			tex(0);
		} else if (vel.x) {
			glActiveTexture(GL_TEXTURE0);
			tex(texState);
		} else {
			glActiveTexture(GL_TEXTURE0);
			tex(0);
		}
		break;
	case MOBT:
		if (!Mobp(this)->bindTex())
			goto NOPE;
		break;
	case STRUCTURET:
	default:
		glActiveTexture(GL_TEXTURE0);
		tex(0);
		break;
	}

	Render::worldShader.use();
	// make the entity hit flash red
	if (maxHitDuration-hitDuration) {
		float flashAmt = 1-(hitDuration/maxHitDuration);
		glUniform4f(Render::worldShader.uniform[WU_tex_color], 1.0, flashAmt, flashAmt, 1.0);
	}

	glUniform1i(Render::worldShader.uniform[WU_texture], 0);
	Render::worldShader.enable();

	glVertexAttribPointer(Render::worldShader.coord, 3, GL_FLOAT, GL_FALSE, 0, coords);
	if (left)
		glVertexAttribPointer(Render::worldShader.tex, 2, GL_FLOAT, GL_FALSE, 0 ,tex_coordL);
	else
		glVertexAttribPointer(Render::worldShader.tex, 2, GL_FLOAT, GL_FALSE, 0 ,tex_coord);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glUniform4f(Render::worldShader.uniform[WU_tex_color], 1.0, 1.0, 1.0, 1.0);
NOPE:
if (near && type != MOBT)
	ui::putStringCentered(loc.x+width/2,loc.y-ui::fontSize-game::HLINE/2,name);
if (health != maxHealth) {

	static GLuint frontH = Texture::genColor(Color(255,0,0));
	static GLuint backH =  Texture::genColor(Color(150,0,0));
	glUniform1i(Render::worldShader.uniform[WU_texture], 0);

	GLfloat coord_back[] = {
		loc.x, 			loc.y + height, 			      z + 0.1f,
		loc.x + width,	loc.y + height, 			      z + 0.1f,
		loc.x + width, 	loc.y + height + game::HLINE * 2, z + 0.1f,

		loc.x + width, 	loc.y + height + game::HLINE * 2, z + 0.1f,
		loc.x, 			loc.y + height + game::HLINE * 2, z + 0.1f,
		loc.x, 			loc.y + height, 			      z + 0.1f,
	};

	GLfloat coord_front[] = {
		loc.x, 			                    loc.y + height,                   z,
		loc.x + health / maxHealth * width, loc.y + height,                   z,
		loc.x + health / maxHealth * width, loc.y + height + game::HLINE * 2, z,

		loc.x + health / maxHealth * width, loc.y + height + game::HLINE * 2, z,
		loc.x,                              loc.y + height + game::HLINE * 2, z,
		loc.x,                              loc.y + height,                   z,
	};

	glBindTexture(GL_TEXTURE_2D, backH);
	GLfloat tex[] = { 0.0, 0.0,
					  1.0, 0.0,
					  1.0, 1.0,

					  1.0, 1.0,
					  0.0, 1.0,
					  0.0, 0.0,
	};
	glVertexAttribPointer(Render::worldShader.coord, 3, GL_FLOAT, GL_FALSE, 0, coord_back);
	glVertexAttribPointer(Render::worldShader.tex, 2, GL_FLOAT, GL_FALSE, 0, tex);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindTexture(GL_TEXTURE_2D, frontH);
	glVertexAttribPointer(Render::worldShader.coord, 3, GL_FLOAT, GL_FALSE, 0, coord_front);
	glVertexAttribPointer(Render::worldShader.tex, 2, GL_FLOAT, GL_FALSE, 0, tex);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

Render::worldShader.disable();
Render::worldShader.unuse();
}*/

void DialogSystem::configure(entityx::EventManager &ev)
{
	ev.subscribe<MouseClickEvent>(*this);
}

void DialogSystem::receive(const MouseClickEvent &mce)
{
	game::entities.each<Position, Solid, Dialog, Name>(
		[&](entityx::Entity e, Position &pos, Solid &dim, Dialog &d, Name &name) {
			(void)e;
			(void)d;

			if (((mce.position.x > pos.x) & (mce.position.x < pos.x + dim.width)) &&
			    ((mce.position.y > pos.y) & (mce.position.y < pos.y + dim.height))) {

			std::thread([&] {
				auto exml = game::engine.getSystem<WorldSystem>()->getXML()->FirstChildElement("Dialog");
				int newIndex;

				if (exml != nullptr) {
					while (exml->StrAttribute("name") != name.name)
						exml = exml->NextSiblingElement();

					exml = exml->FirstChildElement("text");
					while (exml->IntAttribute("id") != d.index)
						exml = exml->NextSiblingElement();

					auto cxml = exml->FirstChildElement("content");
					if (cxml == nullptr)
						return;

					auto content = cxml->GetText() - 1;
					while (*++content && isspace(*content));

					ui::dialogBox(name.name, "", false, content);
					ui::waitForDialog();

					if (exml->QueryIntAttribute("nextid", &newIndex) == XML_NO_ERROR)
						d.index = newIndex;
				}
			}).detach();

			}
		}
	);
}

void DialogSystem::update(entityx::EntityManager &en, entityx::EventManager &ev, entityx::TimeDelta dt)
{
	(void)en;
	(void)ev;
	(void)dt;
}

