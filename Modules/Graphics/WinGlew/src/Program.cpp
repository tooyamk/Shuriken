#include "Program.h"
#include "Graphics.h"

namespace aurora::modules::graphics::win_glew {
	Program::Program(Graphics& graphics) : IProgram(graphics),
		_handle(0) {
	}

	Program::~Program() {
		_release();
	}

	bool Program::upload(const ProgramSource& vert, const ProgramSource& frag) {
		_release();

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

		GLint status;
		glGetProgramiv(_handle, GL_LINK_STATUS, &status);
		if (status == GL_FALSE) {
			_release();
			return false;
		}

		GLint atts;
		glGetProgramiv(_handle, GL_ACTIVE_ATTRIBUTES, &atts);

		GLchar buf[1024];
		for (GLint i = 0; i < atts; ++i) {
			GLsizei len;
			GLint size;
			GLenum type;
			glGetActiveAttrib(_handle, i, sizeof(buf), &len, &size, &type, buf);
			int a = 1;
		}

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

	GLuint Program::_compileShader(const ProgramSource& source, GLenum type) {
		if (!source.isValid()) return 0;

		if (source.language != ProgramLanguage::GLSL) {
			auto g = (Graphics*)_graphics;
			auto translator = g->getProgramSourceTranslator();
			if (translator) {
				return _compileShader(g->getProgramSourceTranslator()->translate(source, ProgramLanguage::GLSL, g->getStringVersion()), type);
			} else {
				return 0;
			}
		}

		println("%s", source.data.getBytes());

		GLuint shader = glCreateShader(type);
		auto s = source.data.getBytes();
		auto len = (GLint)source.data.getLength();
		glShaderSource(shader, 1, &s, &len);
		glCompileShader(shader);

		auto compileStatus = GL_TRUE;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);

		if (compileStatus == GL_FALSE) {
			GLsizei logLen = 0;
			GLchar log[1024];
			glGetShaderInfoLog(shader, sizeof(log), &logLen, log);
			println("compile shader error(%s) :\n %s", (type == GL_VERTEX_SHADER ? "vert" : "frag"), log);

			glDeleteShader(shader);
			return 0;
		}
		return shader;
	}
}