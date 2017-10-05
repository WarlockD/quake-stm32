/*
Copyright (C) 1996-1997 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
// vid_null.c -- null video driver to aid porting efforts

#include "quakedef.h"
#include "d_local.h"
#include <glad\glad.h>
#include <GLFW\glfw3.h>
#include "linmath.h"
#include <assert.h>

#define	BASEWIDTH	320
#define	BASEHEIGHT	200
typedef struct vec2f_s {
	float x, y;
} vec2f_t;
typedef struct vertf_s {
	vec2f_t xy;
	vec2f_t uv;
} vertf;
static const struct
{
	float x, y;
} vertices[6] =
{
	{ -1.0f, -1.0f },
	{ 1.0f, -1.0f },
	{ -1.0f, 1.0f },

	{ -1.0f, 1.0f },
	{ 1.0f, -1.0f },
	{ 1.0f, 1.0f },
};
static const vertf square[] = {
{ { -1.0f, -1.0f }, { 0.0f, 1.0f } },
{ { -1.0f, 1.0f }, { 0.0f, 0.0f } },
{ { 1.0f, 1.0f },{ 1.0f, 0.0f } },
{ { 1.0f, -1.0f },{ 1.0f, 1.0f } },
};

const unsigned short square_index[] = {
	0,1,2,0,2,3
};

static const struct
{
	float u, v;
} uv_positions[6] = {
	{ 1.0f, 0.0f },
	{ 0.0f, 1.0f },
	{ 0.0f, 0.0f },

	{ 0.0f, 0.0f },
	{ 0.0f, 1.0f },
	{ 1.0f, 1.0f },
};


static const char* vertex_shader_text =
"#version 330 core\n"
"// Input vertex data, different for all executions of this shader.\n"
"layout(location = 0) in vec2 vertex_pos;\n"
"layout(location = 1) in vec2 vertex_uv;\n"
"// Output data ; will be interpolated for each fragment.\n"
"out vec2 UV;\n"
// Values that stay constant for the whole mesh.\n"
"uniform mat4 MVP;\n"
"void main() {\n"
"	// Output position of the vertex, in clip space : MVP * position\n"
"	vec3 tmp = vec3(vertex_pos,0);\n"
"	gl_Position = MVP * vec4(tmp, 1);\n"
"	// UV of the vertex. No special space for this one.\n"
"	UV = vertex_uv;\n"
"}\n";

static const char* fragment_shader_text =
"#version 330 core\n"
"// Interpolated values from the vertex shaders\n"
"in vec2 UV;\n"
"// Ouput data\n"
"out vec3 color;\n"
"// Values that stay constant for the whole mesh.\n"
"uniform sampler2D myTextureSampler;\n"
"void main() {\n"
"	// Output color = color of the texture at the specified UV\n"
"	color = texture(myTextureSampler, UV).rgb;\n"
"}\n";
GLuint vertex_buffer, uv_buffer, index_buffer, vertex_shader, fragment_shader, program;
GLint mvp_location, rot_location, vpos_location, vcol_location, texture_location;
GLint vertex_pos_location, vertex_uv_location;
GLuint textureID;

viddef_t	vid;				// global video state
extern GLFWwindow* glfw_window;

#define	BASEWIDTH	320
#define	BASEHEIGHT	200

byte	vid_buffer[BASEWIDTH*BASEHEIGHT];
short	zbuffer[BASEWIDTH*BASEHEIGHT];
byte	surfcache[256 * 1024];
uint32_t vid_texture[BASEWIDTH*BASEHEIGHT];
uint32_t vid_palate[256];

unsigned short	d_8to16table[256];
unsigned	d_8to24table[256];
void CheckLog(GLuint shader, const char* msg) {

	int Result = 0;
	int InfoLogLength = 0;
	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		char VertexShaderErrorMessage[1024];
		assert(InfoLogLength < (1024 - 1));
		glGetShaderInfoLog(shader, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		Sys_Printf(NULL, "Shader Error(%s): %s\n", msg, &VertexShaderErrorMessage[0]);
	}
}

void VID_HandlePause(qboolean value) {
	(void)value;
}
void VID_UnlockBuffer() {

}
void VID_LockBuffer() {

}

void LoadShaderProgram() {
	vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
	glCompileShader(vertex_shader);
	CheckLog(vertex_shader, "Vertex");

	fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
	glCompileShader(fragment_shader);
	CheckLog(fragment_shader, "Fragment");

	program = glCreateProgram();
	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);
	glLinkProgram(program);
}




void	VID_SetPalette(unsigned char *palette)
{
	for (size_t i = 0; i < 256; i++) {
		byte* vid_pal = (byte* )&vid_palate[i];
		*vid_pal++ = *palette++;
		*vid_pal++ = *palette++;
		*vid_pal++ = *palette++;
		*vid_pal++ = 0xFF;
	}
}

void	VID_ShiftPalette(unsigned char *palette)
{
	VID_SetPalette(palette);
}

static void glad_post_error(const char *name, void *funcptr, int len_args, ...) {
	GLenum err;
	while ((err = glad_glGetError()) != GL_NO_ERROR)
	{
		Sys_Printf(NULL, "GLAD ERROR %d in %s\n", err, name);
		//fprintf(stderr, "ERROR %d in %s\n", error_code, name);
		//Process/log the error.
	}
}
void	VID_Init(unsigned char *palette)
{
	vid.maxwarpwidth = vid.width = vid.conwidth = BASEWIDTH;
	vid.maxwarpheight = vid.height = vid.conheight = BASEHEIGHT;
	vid.aspect = 1.0;
	vid.numpages = 1;
	vid.colormap = host_colormap;
	vid.fullbright = 256 - LittleLong(*((int *)vid.colormap + 2048));
	vid.buffer = vid.conbuffer = vid_buffer;
	vid.rowbytes = vid.conrowbytes = BASEWIDTH;

	d_pzbuffer = zbuffer;
	D_InitCaches(surfcache, sizeof(surfcache));

	glfwMakeContextCurrent(glfw_window);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	glfwSwapInterval(1);

	glad_set_post_callback(glad_post_error);
	LoadShaderProgram();

	mvp_location = glGetUniformLocation(program, "MVP");
	texture_location = glGetUniformLocation(program, "myTextureSampler");
	vertex_pos_location = glGetAttribLocation(program, "vertex_pos");
	vertex_uv_location = glGetAttribLocation(program, "vertex_uv");
	glEnableVertexAttribArray(vertex_pos_location);
	glEnableVertexAttribArray(vertex_uv_location);


	//glGenBuffers(1, &vertex_buffer);
	//glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);


	//glGenBuffers(1, &uv_buffer);
	//glBindBuffer(GL_ARRAY_BUFFER, uv_buffer);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(uv_positions), uv_positions, GL_STATIC_DRAW);

	glGenTextures(1, &textureID);

	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, BASEWIDTH, BASEHEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, vid_texture);


	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	// NOTE: OpenGL error checks have been omitted for brevity

}

void	VID_Shutdown(void)
{
}

void	VID_Update(vrect_t *rects)
{
	for (size_t y = 0; y < BASEHEIGHT; y++) {
		for (size_t x = 0; x < BASEWIDTH; x++) {
			vid_texture[x + y * BASEWIDTH] = vid_palate[vid.buffer[x + y * BASEWIDTH]];
		}
		//vid.buffer = vid.conbuffer = vid_buffer;
		//vid.rowbytes = vid.conrowbytes = BASEWIDTH;
	}


	float ratio;
	int width, height;
	mat4x4 m, p, mvp;
	glfwGetFramebufferSize(glfw_window, &width, &height);
	ratio = width / (float)height;
	glViewport(0, 0, width, height);
	glClear(GL_COLOR_BUFFER_BIT);
	mat4x4_identity(m);

	mat4x4_ortho(m, -1.0f, ratio, -1.f, 1.f, 1.f, -1.f);
	
	glUseProgram(program);
	glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*)m);

	// Bind our texture in Texture Unit 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, BASEWIDTH, BASEHEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, vid_texture);

	// Set our "myTextureSampler" sampler to use Texture Unit 0
	glUniform1i(texture_location, 0);
	// 1rst attribute buffer : vertices
	glEnableVertexAttribArray(vertex_pos_location);
	glEnableVertexAttribArray(vertex_uv_location);

	//glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

	GLsizei stride = sizeof(vertf);
	const GLvoid* pCoords = &square[0].xy;
	const GLvoid* pTexCoords = &square[0].uv;

	glVertexAttribPointer(vertex_pos_location, 2, GL_FLOAT, GL_FALSE, stride, pCoords);
	glVertexAttribPointer(vertex_uv_location, 2, GL_FLOAT, GL_FALSE, stride, pTexCoords);

	const GLsizei vertexCount = sizeof(square) / sizeof(vertf);
	const GLsizei indexCount = sizeof(square_index) / sizeof(unsigned short);
	glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_SHORT, square_index);
	//glDrawArrays(GL_TRIANGLES, 0, vertexCount);

	glDisableVertexAttribArray(vertex_pos_location);
	glDisableVertexAttribArray(vertex_uv_location);

#if 0
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glVertexAttribPointer(
		0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
		2,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
	);

	// 2nd attribute buffer : UVs
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, uv_buffer);
	glVertexAttribPointer(
		1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
		2,                                // size : U+V => 2
		GL_FLOAT,                         // type
		GL_FALSE,                         // normalized?
		0,                                // stride
		(void*)0                          // array buffer offset
	);

	// Draw the triangle !
	glDrawArrays(GL_TRIANGLES, 0, 2 * 3); // 12*3 indices starting at 0 -> 12 triangles
	glDrawArrays(GL_TRIANGLES, 0, 2 * 3); // 12*3 indices starting at 0 -> 12 triangles
	GLsizei vertexCount = sizeof(Vertices) / sizeof(Vertex);
	glDrawArrays(GL_TRIANGLES, 0, vertexCount);

	glDisableVertexAttribArray(0);
	//glDisableVertexAttribArray(1);
#endif
	glfwSwapBuffers(glfw_window);
}

/*
================
D_BeginDirectRect
================
*/
void D_BeginDirectRect(int x, int y, byte *pbitmap, int width, int height)
{
}


/*
================
D_EndDirectRect
================
*/
void D_EndDirectRect(int x, int y, int width, int height)
{
}

// hack here just to include the glad librayrs

#ifndef _DEBUG
#include "../glad/release/src/glad.c"
#else
#include "../glad/debug/src/glad.c"
#endif


