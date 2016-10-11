#include <render.hpp>

static Shader *currentShader = nullptr;

namespace Render {

Shader worldShader;
Shader textShader;

void initShaders(void)
{
    // create the world shader
    worldShader.create("shaders/world.vert", "shaders/world.frag");
    worldShader.addUniform("texture");
    worldShader.addUniform("ortho");
    worldShader.addUniform("tex_color");
    worldShader.addUniform("transform");
    worldShader.addUniform("ambientLight");
    worldShader.addUniform("lightImpact");
    worldShader.addUniform("light");
    worldShader.addUniform("lightColor");
    worldShader.addUniform("lightSize");

    // create the text shader
    textShader.create("shaders/new.vert", "shaders/new.frag");
    textShader.addUniform("sampler");
    textShader.addUniform("ortho"); // actually not used, ortho in new.vert is mislabeled (actually transform)
    textShader.addUniform("tex_color");
    textShader.addUniform("ortho"); // this is transform
}

void useShader(Shader *s)
{
    currentShader = s;
}

void drawRect(vec2 ll, vec2 ur, float z)
{
    GLfloat verts[] = {ll.x, ll.y, z,
                       ur.x, ll.y, z,
                       ur.x, ur.y, z,

                       ur.x, ur.y, z,
                       ll.x, ur.y, z,
                       ll.x, ll.y, z};

    GLfloat tex[] = {0.0, 1.0,
                     1.0, 1.0,
                     1.0, 0.0,

                     1.0, 0.0,
                     0.0, 0.0,
                     0.0, 1.0};

    glUniform1i(currentShader->uniform[WU_texture], 0);
    currentShader->enable();

    glVertexAttribPointer(currentShader->coord, 3, GL_FLOAT, GL_FALSE, 0, verts);
    glVertexAttribPointer(currentShader->tex, 2, GL_FLOAT, GL_FALSE, 0, tex);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    currentShader->disable();
}


}
