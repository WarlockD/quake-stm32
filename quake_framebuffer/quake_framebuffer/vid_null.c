// vid_null.c -- null video driver to aid porting efforts

#include "quakedef.h"
#include "d_local.h"
#include <glad\glad.h>
#include <GLFW\glfw3.h>
#include "linmath.h"
#include <assert.h>


static const struct
{
	float x, y;
} vertices[6] =
{
	{ -1.0f, -1.0f},
	{ 1.0f, -1.0f },
	{ -1.0f, 1.0f},
	{ -1.0f, 1.0f},
	{ 1.0f, -1.0f },
	{ 1.0f, 1.0f },
};

static const struct
{
	float u, v;
} uv_positions[6] = {
	{ 0.0f, 0.0f },
	{ 0.0f, 1.0f },
	{ 1.0f, 0.0f },
	{ 1.0f, 0.0f },
	{ 0.0f, 1.0f },
	{ 1.0f, 1.0f },
};

static const char* vertex_shader_text =
"#version 330 core\n"
"// Input vertex data, different for all executions of this shader.\n"
"layout(location = 0) in vec2 vertexPosition_modelspace;\n"
"layout(location = 1) in vec2 vertexUV;\n"
"// Output data ; will be interpolated for each fragment.\n"
"out vec2 UV;\n"
// Values that stay constant for the whole mesh.\n"
"uniform mat4 MVP;\n"
"void main() {\n"
"	// Output position of the vertex, in clip space : MVP * position\n"
"	vec3 tmp = vec3(vertexPosition_modelspace,0);\n"
"	gl_Position = MVP * vec4(tmp, 1);\n"
"	// UV of the vertex. No special space for this one.\n"
"	UV = vertexUV;\n"
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
GLuint vertex_buffer, uv_buffer, vertex_shader, fragment_shader, program;
GLint mvp_location, vpos_location, vcol_location, texture_location;
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
		Sys_DebugLog(NULL, "Shader Error(%s): %s\n", msg, &VertexShaderErrorMessage[0]);
	}
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




void	VID_SetPalette (unsigned char *palette)
{
	for (size_t i = 0; i < 256; i++) {
		byte* vid_pal = &vid_palate[i];
		*vid_pal++ = *palette++;
		*vid_pal++ = *palette++;
		*vid_pal++ = *palette++;
		*vid_pal++ = 0xFF;
	}
}

void	VID_ShiftPalette (unsigned char *palette)
{
	VID_SetPalette(palette);
}
void VID_LockBuffer() {

}
void VID_UnlockBuffer() {

}
void VID_SetWindowTitle(const char* title) {
	glfwSetWindowTitle(glfw_window, title);
}
void VID_FocusGameWindow() {

}
static void glad_post_error(const char *name, void *funcptr, int len_args, ...) {
	GLenum err;
	while ((err = glad_glGetError()) != GL_NO_ERROR)
	{
		Sys_DebugLog(NULL, "ERROR %d in %s\n", err, name);
		//fprintf(stderr, "ERROR %d in %s\n", error_code, name);
		//Process/log the error.
	}
}
void	VID_Init (unsigned char *palette)
{
	vid.maxwarpwidth = vid.width = vid.conwidth = BASEWIDTH;
	vid.maxwarpheight = vid.height = vid.conheight = BASEHEIGHT;
	vid.aspect = 1.0;
	vid.numpages = 1;
	vid.colormap = host_colormap;
	vid.fullbright = 256 - LittleLong (*((int *)vid.colormap + 2048));
	vid.buffer = vid.conbuffer = vid_buffer;
	vid.rowbytes = vid.conrowbytes = BASEWIDTH;
	
	d_pzbuffer = zbuffer;
	D_InitCaches (surfcache, sizeof(surfcache));

	glfwMakeContextCurrent(glfw_window);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	glfwSwapInterval(1);

	glad_set_post_callback(glad_post_error);
	LoadShaderProgram();


	glGenBuffers(1, &vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);


	glGenBuffers(1, &uv_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, uv_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(uv_positions), uv_positions, GL_STATIC_DRAW);

	glGenTextures(1, &textureID);

	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,BASEWIDTH,BASEHEIGHT,0,GL_RGBA,GL_UNSIGNED_BYTE,vid_texture);


	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	// NOTE: OpenGL error checks have been omitted for brevity
	//glGenBuffers(1, &vertex_buffer);
	//glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);



	mvp_location = glGetUniformLocation(program, "MVP");
	texture_location = glGetUniformLocation(program, "myTextureSampler");

	//vpos_location = glGetAttribLocation(program, "vPos");
	//vcol_location = glGetAttribLocation(program, "vCol");
	glEnableVertexAttribArray(vpos_location);
	glVertexAttribPointer(vpos_location, 2, GL_FLOAT, GL_FALSE,
		sizeof(float) * 5, (void*)0);
	glEnableVertexAttribArray(vcol_location);
	glVertexAttribPointer(vcol_location, 3, GL_FLOAT, GL_FALSE,
		sizeof(float) * 5, (void*)(sizeof(float) * 2));
}

void	VID_Shutdown (void)
{
}

void	VID_Update (vrect_t *rects)
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
	//mat4x4_rotate_Z(m, m, (float)glfwGetTime());
	mat4x4_ortho(p, -ratio, ratio, -1.f, 1.f, 1.f, -1.f);
	//mat4x4_mul(mvp, p, m);
	glUseProgram(program);
	glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*)m);
	// Bind our texture in Texture Unit 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, BASEWIDTH, BASEHEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, vid_texture);

	// Set our "myTextureSampler" sampler to use Texture Unit 0
	glUniform1i(texture_location, 0);
	// 1rst attribute buffer : vertices
	glEnableVertexAttribArray(0);
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
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	glfwSwapBuffers(glfw_window);
}

/*
================
D_BeginDirectRect
================
*/
void D_BeginDirectRect (int x, int y, byte *pbitmap, int width, int height)
{
}


/*
================
D_EndDirectRect
================
*/
void D_EndDirectRect (int x, int y, int width, int height)
{
}


