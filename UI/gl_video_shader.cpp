/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#include "gl_video_shader.h"
#include <algorithm>
#include <array>
#include <memory>
#include "logger/logger.h"

// Vertex shader doesn't do anything except pass coordinates through.
const char kRTCVertexShaderSource[] =
SHADER_VERSION
VERTEX_SHADER_IN " vec2 position;\n"
VERTEX_SHADER_IN " vec2 texcoord;\n"
VERTEX_SHADER_OUT " vec2 v_texcoord;\n"
"void main() {\n"
"    gl_Position = vec4(position.x, position.y, 0.0, 1.0);\n"
"    v_texcoord = texcoord;\n"
"}\n";

static const int kYTextureUnit = 0;
static const int kUTextureUnit = 1;
static const int kVTextureUnit = 2;
static const int kUvTextureUnit = 1;

// Fragment shader converts YUV values from input textures into a final RGB
// pixel. The conversion formula is from http://www.fourcc.org/fccyvrgb.php.
static const char kI420FragmentShaderSource[] =
SHADER_VERSION
"precision highp float;"
FRAGMENT_SHADER_IN " vec2 v_texcoord;\n"
"uniform lowp sampler2D s_textureY;\n"
"uniform lowp sampler2D s_textureU;\n"
"uniform lowp sampler2D s_textureV;\n"
FRAGMENT_SHADER_OUT
"void main() {\n"
"    float y, u, v, r, g, b;\n"
"    y = " FRAGMENT_SHADER_TEXTURE "(s_textureY, v_texcoord).r;\n"
"    u = " FRAGMENT_SHADER_TEXTURE "(s_textureU, v_texcoord).r;\n"
"    v = " FRAGMENT_SHADER_TEXTURE "(s_textureV, v_texcoord).r;\n"
"    u = u - 0.5;\n"
"    v = v - 0.5;\n"
"    r = y + 1.403 * v;\n"
"    g = y - 0.344 * u - 0.714 * v;\n"
"    b = y + 1.770 * u;\n"
"    " FRAGMENT_SHADER_COLOR " = vec4(r, g, b, 1.0);\n"
"  }\n";

static const char kNV12FragmentShaderSource[] =
SHADER_VERSION
"precision mediump float;"
FRAGMENT_SHADER_IN " vec2 v_texcoord;\n"
"uniform lowp sampler2D s_textureY;\n"
"uniform lowp sampler2D s_textureUV;\n"
FRAGMENT_SHADER_OUT
"void main() {\n"
"    mediump float y;\n"
"    mediump vec2 uv;\n"
"    y = " FRAGMENT_SHADER_TEXTURE "(s_textureY, v_texcoord).r;\n"
"    uv = " FRAGMENT_SHADER_TEXTURE "(s_textureUV, v_texcoord).ra -\n"
"        vec2(0.5, 0.5);\n"
"    " FRAGMENT_SHADER_COLOR " = vec4(y + 1.403 * uv.y,\n"
"                                     y - 0.344 * uv.x - 0.714 * uv.y,\n"
"                                     y + 1.770 * uv.x,\n"
"                                     1.0);\n"
"  }\n";


GLVideoShader::GLVideoShader()
{

}

GLVideoShader::~GLVideoShader()
{
	glDeleteProgram(_i420Program);
	glDeleteProgram(_nv12Program);
	glDeleteBuffers(1, &_vertexBuffer);
	glDeleteVertexArrays(1, &_vertexArray);
}

// Compiles a shader of the given |type| with GLSL source |source| and returns
// the shader handle or 0 on error.
GLuint GLVideoShader::createShader(GLenum type, const GLchar *source) {
	GLuint shader = glCreateShader(type);
	if (!shader) {
		return 0;
	}
	glShaderSource(shader, 1, &source, NULL);
	glCompileShader(shader);
	GLint compileStatus = GL_FALSE;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);
	if (compileStatus == GL_FALSE) {
		GLint logLength = 0;
		// The null termination character is included in the returned log length.
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
		if (logLength > 0) {
			std::unique_ptr<char[]> compileLog(new char[logLength]);
			// The returned string is null terminated.
			glGetShaderInfoLog(shader, logLength, NULL, compileLog.get());
			DLOG("Shader compile error: {}", compileLog.get());
		}
		glDeleteShader(shader);
		shader = 0;
	}
	return shader;
}

// Links a shader program with the given vertex and fragment shaders and
// returns the program handle or 0 on error.
GLuint GLVideoShader::createProgram(GLuint vertexShader, GLuint fragmentShader) {
	if (vertexShader == 0 || fragmentShader == 0) {
		return 0;
	}
	GLuint program = glCreateProgram();
	if (!program) {
		return 0;
	}
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	glLinkProgram(program);
	GLint linkStatus = GL_FALSE;
	glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
	if (linkStatus == GL_FALSE) {
		glDeleteProgram(program);
		program = 0;
	}
	return program;
}

// Creates and links a shader program with the given fragment shader source and
// a plain vertex shader. Returns the program handle or 0 on error.
GLuint GLVideoShader::createProgramFromFragmentSource(const char fragmentShaderSource[]) {
	GLuint vertexShader = createShader(GL_VERTEX_SHADER, kRTCVertexShaderSource);
	//RTC_CHECK(vertexShader) << "failed to create vertex shader";
	if (vertexShader == 0) {
		DLOG("failed to create vertex shader");
	}
	GLuint fragmentShader = createShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
	//RTC_CHECK(fragmentShader) << "failed to create fragment shader"; 
	if (fragmentShader == 0) {
		DLOG("failed to create fragment shader");
	}
	GLuint program = createProgram(vertexShader, fragmentShader);
	// Shaders are created only to generate program.
	if (vertexShader) {
		glDeleteShader(vertexShader);
	}
	if (fragmentShader) {
		glDeleteShader(fragmentShader);
	}

	// Set vertex shader variables 'position' and 'texcoord' in program.
	GLint position = glGetAttribLocation(program, "position");
	GLint texcoord = glGetAttribLocation(program, "texcoord");
	if (position < 0 || texcoord < 0) {
		glDeleteProgram(program);
		return 0;
	}

	// Read position attribute with size of 2 and stride of 4 beginning at the start of the array. The
	// last argument indicates offset of data within the vertex buffer.
	glVertexAttribPointer(position, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void *)0);
	glEnableVertexAttribArray(position);

	// Read texcoord attribute  with size of 2 and stride of 4 beginning at the first texcoord in the
	// array. The last argument indicates offset of data within the vertex buffer.
	glVertexAttribPointer(texcoord, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void *)(2 * sizeof(GLfloat)));
	glEnableVertexAttribArray(texcoord);

	return program;
}

bool GLVideoShader::createVertexBuffer(GLuint *vertexBuffer, GLuint *vertexArray) {
	glGenVertexArrays(1, vertexArray);
	if (*vertexArray == 0) {
		return false;
	}
	glBindVertexArray(*vertexArray);

	glGenBuffers(1, vertexBuffer);
	if (*vertexBuffer == 0) {
		glDeleteVertexArrays(1, vertexArray);
		return false;
	}
	glBindBuffer(GL_ARRAY_BUFFER, *vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, 4 * 4 * sizeof(GLfloat), NULL, GL_DYNAMIC_DRAW);
	return true;
}

// Set vertex data to the currently bound vertex buffer.
void GLVideoShader::setVertexData(webrtc::VideoRotation rotation) {
	// When modelview and projection matrices are identity (default) the world is
	// contained in the square around origin with unit size 2. Drawing to these
	// coordinates is equivalent to drawing to the entire screen. The texture is
	// stretched over that square using texture coordinates (u, v) that range
	// from (0, 0) to (1, 1) inclusive. Texture coordinates are flipped vertically
	// here because the incoming frame has origin in upper left hand corner but
	// OpenGL expects origin in bottom left corner.
	std::array<std::array<GLfloat, 2>, 4> UVCoords = { {
		{{0, 1}},  // Lower left.
		{{1, 1}},  // Lower right.
		{{1, 0}},  // Upper right.
		{{0, 0}},  // Upper left.
	} };

	// Rotate the UV coordinates.
	int rotation_offset;
	switch (rotation) {
	case webrtc::kVideoRotation_0:
		rotation_offset = 0;
		break;
	case webrtc::kVideoRotation_90:
		rotation_offset = 1;
		break;
	case webrtc::kVideoRotation_180:
		rotation_offset = 2;
		break;
	case webrtc::kVideoRotation_270:
		rotation_offset = 3;
		break;
	}
	std::rotate(UVCoords.begin(), UVCoords.begin() + rotation_offset,
		UVCoords.end());

	const GLfloat gVertices[] = {
		// X, Y, U, V.
		-1, -1, UVCoords[0][0], UVCoords[0][1],
		 1, -1, UVCoords[1][0], UVCoords[1][1],
		 1,  1, UVCoords[2][0], UVCoords[2][1],
		-1,  1, UVCoords[3][0], UVCoords[3][1],
	};

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(gVertices), gVertices);
}

bool GLVideoShader::createAndSetupI420Program() {
	assert(!_i420Program);
	_i420Program = createProgramFromFragmentSource(kI420FragmentShaderSource);
	if (!_i420Program) {
		return false;
	}
	GLint ySampler = glGetUniformLocation(_i420Program, "s_textureY");
	GLint uSampler = glGetUniformLocation(_i420Program, "s_textureU");
	GLint vSampler = glGetUniformLocation(_i420Program, "s_textureV");

	if (ySampler < 0 || uSampler < 0 || vSampler < 0) {
		DLOG("Failed to get uniform variable locations in I420 shader");
		glDeleteProgram(_i420Program);
		_i420Program = 0;
		return false;
	}

	glUseProgram(_i420Program);
	glUniform1i(ySampler, kYTextureUnit);
	glUniform1i(uSampler, kUTextureUnit);
	glUniform1i(vSampler, kVTextureUnit);

	return true;
}

bool GLVideoShader::createAndSetupNV12Program() {
	assert(!_nv12Program);
	_nv12Program = createProgramFromFragmentSource(kNV12FragmentShaderSource);
	if (!_nv12Program) {
		return false;
	}
	GLint ySampler = glGetUniformLocation(_nv12Program, "s_textureY");
	GLint uvSampler = glGetUniformLocation(_nv12Program, "s_textureUV");

	if (ySampler < 0 || uvSampler < 0) {
		DLOG("Failed to get uniform variable locations in NV12 shader");
		glDeleteProgram(_nv12Program);
		_nv12Program = 0;
		return false;
	}

	glUseProgram(_nv12Program);
	glUniform1i(ySampler, kYTextureUnit);
	glUniform1i(uvSampler, kUvTextureUnit);

	return true;
}

bool GLVideoShader::prepareVertexBuffer(webrtc::VideoRotation rotation) {
	if (!_vertexBuffer && !createVertexBuffer(&_vertexBuffer, &_vertexArray)) {
		DLOG("Failed to setup vertex buffer");
		return false;
	}

	glBindVertexArray(_vertexArray);

	glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer); 
	if (!_currentRotation || rotation != *_currentRotation) {
		_currentRotation = absl::optional<webrtc::VideoRotation>(rotation);
		setVertexData(*_currentRotation);
	}
	return true;
}

void GLVideoShader::applyShadingForFrame(int width, 
	int height,
	webrtc::VideoRotation rotation,
	GLuint yPlane,
	GLuint uPlane,
	GLuint vPlane) {
	if (!prepareVertexBuffer(rotation)) {
		return;
	}

	if (!_i420Program && !createAndSetupI420Program()) {
		DLOG("Failed to setup I420 program");
		return;
	}

	glUseProgram(_i420Program);

	glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + kYTextureUnit));
	glBindTexture(GL_TEXTURE_2D, yPlane);

	glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + kUTextureUnit));
	glBindTexture(GL_TEXTURE_2D, uPlane);

	glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + kVTextureUnit));
	glBindTexture(GL_TEXTURE_2D, vPlane);

	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void GLVideoShader::applyShadingForFrame(int width,
	int height,
	webrtc::VideoRotation rotation,
	GLuint yPlane,
	GLuint uvPlane) {
	if (!prepareVertexBuffer(rotation)) {
		return;
	}

	if (!_nv12Program && !createAndSetupNV12Program()) {
		DLOG("Failed to setup NV12 shader");
		return;
	}

	glUseProgram(_nv12Program);

	glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + kYTextureUnit));
	glBindTexture(GL_TEXTURE_2D, yPlane);

	glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + kUvTextureUnit));
	glBindTexture(GL_TEXTURE_2D, uvPlane);

	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}


