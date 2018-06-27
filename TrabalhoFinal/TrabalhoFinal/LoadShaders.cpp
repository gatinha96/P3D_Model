#pragma once

#include <iostream>
#include <fstream>

#define GLEW_STATIC
#include <GL\glew.h>

#include "LoadShaders.h"

static const GLchar* ReadShader(const char* filename) {
	// Abre o ficheiro 'filename' em bin�rio, e coloca-se na �ltima posi��o do ficheiro.
	std::ifstream ficheiro(filename, std::ifstream::ate | std::ifstream::binary);
	// Se o ficheiro foi aberto.
	if (ficheiro.is_open()) {
		// Leitura da pr�xima posi��o de leitura.
		std::streampos tamanhoDoFicheiroEmBytes = ficheiro.tellg();
		// Reposiciona a leitura do ficheiro no seu in�cio.
		ficheiro.seekg(0, std::ios::beg);

		// Aloca��o de espa�o de mem�ria para dados do ficheiro.
		GLchar* source = new GLchar[int(tamanhoDoFicheiroEmBytes) + 1];
		// Leitura do ficheiro para o array 'source'.
		ficheiro.read(source, tamanhoDoFicheiroEmBytes);
		// Fecha a string.
		source[tamanhoDoFicheiroEmBytes] = 0;

		// Fecha o ficheiro.
		ficheiro.close();

		// Retorna o endere�o da string alocada.
		return const_cast<const GLchar*>(source);
	}
	else {
		std::cerr << "Erro ao abrir o ficheiro '" << filename << "'" << std::endl;
	}

	return nullptr;
}

GLuint LoadShaders(ShaderInfo* shaders) {
	if (shaders == nullptr) return 0;

	// Cria um objeto de programa
	GLuint program = glCreateProgram();

	for (GLint i = 0; shaders[i].type != GL_NONE; i++) {
		// Cria um objeto shader
		shaders[i].shader = glCreateShader(shaders[i].type);

		// Efetua a leitura do c�digo do shader
		const GLchar* source = ReadShader(shaders[i].filename);
		// Se n�o conseguir ler o c�digo
		if (source == NULL) {
			// Destr�i os shaders que tinham criados
			for (int j = 0; shaders[j].type != GL_NONE; j++) {
				// Se tem um shader v�lido (i.e., != 0)
				if(shaders[j].shader != 0)
					glDeleteShader(shaders[j].shader);
				shaders[j].shader = 0;
			}

			return 0;
		}

		// Carrega o c�digo do shader
		glShaderSource(shaders[i].shader, 1, &source, NULL);
		delete[] source;

		// Compila o shader
		glCompileShader(shaders[i].shader);

		// Verifica o estado da compila��o
		GLint compiled;
		glGetShaderiv(shaders[i].shader, GL_COMPILE_STATUS, &compiled);
		// Em caso de erro na compila��o
		if (!compiled) {
#ifdef _DEBUG
			GLsizei len;
			glGetShaderiv(shaders[i].shader, GL_INFO_LOG_LENGTH, &len);

			GLchar* log = new GLchar[len + 1];
			glGetShaderInfoLog(shaders[i].shader, len, &len, log);
			std::cerr << "Shader compilation failed: " << log << std::endl;
			delete[] log;
#endif /* DEBUG */

			// Destr�i os shaders que tinham criados
			for (int j = 0; shaders[j].type != GL_NONE; j++) {
				// Se tem um shader v�lido (i.e., != 0)
				if (shaders[j].shader != 0)
					glDeleteShader(shaders[j].shader);
				shaders[j].shader = 0;
			}

			return 0;
		}

		// Anexa o shader ao programa
		glAttachShader(program, shaders[i].shader);
	}

	// Linka o programa
	glLinkProgram(program);

	// Verifica o estado do processo de linkagem
	GLint linked;
	glGetProgramiv(program, GL_LINK_STATUS, &linked);
	// Em caso de erro na linkagem
	if (!linked) {
#ifdef _DEBUG
		GLsizei len;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);

		GLchar* log = new GLchar[len + 1];
		glGetProgramInfoLog(program, len, &len, log);
		std::cerr << "Shader linking failed: " << log << std::endl;
		delete[] log;
#endif /* DEBUG */

		// Destr�i os shaders que tinham criados
		for (int j = 0; shaders[j].type != GL_NONE; j++) {
			// Se tem um shader v�lido (i.e., != 0)
			if (shaders[j].shader != 0)
				glDeleteShader(shaders[j].shader);
			shaders[j].shader = 0;
		}

		return 0;
	}

	return program;
}
