#ifndef RENDER_HPP_
#define RENDER_HPP_

#include <vector>

#include <GL/glew.h>
#include <SDL2/SDL_opengl.h>

#include <shader_utils.hpp>
#include <vector2.hpp>

/**
 * @class Shader
 * @brief Handles a texture shader, allowing it's use in the program.
 */
class Shader {
public:
    GLuint shader;
    GLint  coord;
    GLint  tex;
    std::vector<GLint> uniform;

    void create(const char *vert, const char *frag) {
        shader = create_program(vert, frag);
     	coord  = get_attrib(shader, "coord2d");
     	tex    = get_attrib(shader, "tex_coord");
    }

    inline void addUniform(const char *name) {
         uniform.push_back(get_uniform(shader, name));
    }

    inline void use(void) {
        glUseProgram(shader);
    }

    inline void unuse(void) {
        glUseProgram(0);
    }

    inline void enable(void) {
        glEnableVertexAttribArray(coord);
        glEnableVertexAttribArray(tex);
    }

    inline void disable(void) {
        glDisableVertexAttribArray(coord);
        glDisableVertexAttribArray(tex);
    }

    ~Shader(void) {
        uniform.clear();
    }
};

typedef enum {
    WU_texture = 0,
    WU_ortho,
    WU_tex_color,
    WU_transform,
    WU_ambient,
    WU_light_impact,
    WU_light,
    WU_light_color,
    WU_light_size
} WorldUniform;

namespace Render {
	namespace ZRange {
		constexpr float World     =  9.9f;
		constexpr float RenderSys =  5.0f;
		constexpr float Ground    = -4.5f;
		constexpr float Attack    = -5.0f;
		constexpr float Particle  = -8.0f;
		constexpr float Important = -8.5f;
		constexpr float Inventory = -9.0f;
		constexpr float Dialog    = -9.3f;
		constexpr float Quest     = -9.5f;
		constexpr float Fade      = -9.7f;
		constexpr float Menu      = -9.8f;
		constexpr float Mouse     = -9.89f;
		constexpr float Debug     = -9.9f;
	}

    extern Shader worldShader;
    extern Shader textShader;

    void useShader(Shader *s);

    void drawRect(vec2 ll, vec2 ur, float z);

	void init(void);
	void render(const int& fps);
}

#endif // RENDER_HPP_
