#include "Program.h"
#include "Graphics.h"

namespace aurora::modules::graphics_win_glew {
	Program::Program(Graphics& graphics) : IGraphicsProgram(graphics),
		_handle(0) {
	}

	Program::~Program() {
		_release();
	}

	bool Program::upload(const i8* vert, const i8* frag) {
		_release();

		if (!vert || !frag) return false;

		_handle = glCreateProgram();
		if (!_handle) return false;

		auto vertexShader = _compileShader(vert, GL_VERTEX_SHADER);
		if (!vertexShader) return false;

		auto fragmentShader = _compileShader(frag, GL_FRAGMENT_SHADER);
		if (!fragmentShader) {
			glDeleteShader(vertexShader);
			return false;
		}

		glAttachShader(_handle, vertexShader);
		glAttachShader(_handle, fragmentShader);
		glLinkProgram(_handle);

		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);

		return true;
	}

	void Program::use() {
		if (_handle) {
			glUseProgram(_handle);

			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
			glDisable(GL_DEPTH_TEST);
			glDrawArrays(GL_TRIANGLES, 0, 3);
		}
	}

	void Program::_release() {
		if (_handle) {
			glDeleteProgram(_handle);
			_handle = 0;
		}
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