#include "Program.h"
#include "Graphics.h"

namespace aurora::modules::graphics::win::glew {
	Program::Program(Graphics& graphics) : aurora::modules::graphics::Program(graphics),
		_handle(glCreateProgram()) {

		const char* vert = "";
		const char* frag = "";

		auto vertexShader = _compileShader(vert, GL_VERTEX_SHADER);
		auto fragmentShader = _compileShader(frag, GL_FRAGMENT_SHADER);

		if (vertexShader && fragmentShader) {
			glAttachShader(_handle, vertexShader);
			glAttachShader(_handle, fragmentShader);
			glLinkProgram(_handle);
		}

		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
	}

	Program::~Program() {
		glDeleteProgram(_handle);
	}

	GLuint Program::_compileShader(const GLchar* source, GLenum type) {
		GLuint shader = glCreateShader(type);
		glShaderSource(shader, 1, &source, nullptr);
		glCompileShader(shader);

		auto compileStatus = GL_TRUE;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);

		if (compileStatus == GL_FALSE) {
			glDeleteShader(shader);
			return 0;
		}
		return shader;
	}
}