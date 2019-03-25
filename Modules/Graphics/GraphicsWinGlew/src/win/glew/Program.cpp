#include "Program.h"
#include "Graphics.h"

namespace aurora::modules::graphics::win::glew {
	Program::Program(Graphics& graphics) : aurora::modules::graphics::Program(graphics),
		_handle(glCreateProgram()) {

		const char* vert = "#version 420 core\n"
			"attribute vec2 position;\n"
			"void main(void)\n"
			"{\n"
			"  gl_Position = vec4(position, 0.0, 1.0);\n"
			"}\n";
		const char* frag = "#version 420 core\n"
			"void main(void)\n"
			"{\n"
			"  gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
			"}\n";

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

	void Program::use() {
		if (_handle) glUseProgram(_handle);
		
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glDisable(GL_DEPTH_TEST);
		glDrawArrays(GL_TRIANGLES, 0, 3);
	}

	GLuint Program::_compileShader(const GLchar* source, GLenum type) {
		GLuint shader = glCreateShader(type);
		glShaderSource(shader, 1, &source, nullptr);
		glCompileShader(shader);

		auto compileStatus = GL_TRUE;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);

		if (compileStatus == GL_FALSE) {
			int logLen = 0;
			char log[1024];
			glGetShaderInfoLog(shader, 1024, &logLen, log);
			println("compile shader error(%s) :\n %s", (type == GL_VERTEX_SHADER ? "vert" : "frag"), log);

			glDeleteShader(shader);
			return 0;
		}
		return shader;
	}
}