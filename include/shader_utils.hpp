/**
 * @file shader_utils.hpp
 * @brief Utilities to use to handle GLSL shaders.
 * From the OpenGL Programming wikibook: http://en.wikibooks.org/wiki/OpenGL_Programming
 * This file is in the public domain.
 * Contributors: Sylvain Beucler, Guus Sliepen
 */

#ifndef _CREATE_SHADER_H
#define _CREATE_SHADER_H

#define GLEW_STATIC
#include <GL/glew.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

char* file_read(const char* filename);
void print_log(GLuint object);
GLuint create_shader(const char* filename, GLenum type);
GLuint create_program(const char* vertexfile, const char *fragmentfile);
GLint get_attrib(GLuint program, const char *name);
GLint get_uniform(GLuint program, const char *name);
void print_opengl_info();

#endif
