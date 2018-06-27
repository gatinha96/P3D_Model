#pragma comment(lib, "glew32s.lib")
#pragma comment(lib, "glfw3.lib")
#pragma comment(lib, "opengl32.lib")

#include <stdio.h>
#include <stdlib.h>

#define GLEW_STATIC
#include <GL\glew.h>
#define GLFW_USE_DWM_SWAP_INTERVAL
#include <GLFW\glfw3.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <string>
#include <vector>
using namespace std;

#include <glm\glm.hpp>
#include <glm\gtc\type_ptr.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\matrix_inverse.hpp> // glm::inverseTranspose()
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "LoadShaders.h"

#define WIDTH 800
#define HEIGHT 600

//functions...
void print_error(int error, const char *description);
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
bool LoadModelVertex(FILE *f);
void display();
void Load_texture(char *namepath);

GLuint VAO;
GLuint normalbuffer;
GLuint uvbuffer;
GLuint vertexbuffer;

GLuint programa;

glm::mat4 Model, View, Projection;
glm::mat3 NormalMatrix;
GLfloat angle = 0.0f;
float FoV = 45;
// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units

GLFWwindow *window;

//load model variables
std::vector< unsigned int > vertexIndices, uvIndices, normalIndices;
std::vector< glm::vec3 > temp_vertices;
std::vector< glm::vec2 > temp_uvs;
std::vector< glm::vec3 > temp_normals;
std::vector< glm::vec3 > out_vertices;
std::vector< glm::vec2 > out_uvs;
std::vector< glm::vec3 > out_normals;

int main(void)
{
	glfwSetErrorCallback(print_error);

	//-------------Matrizes de transformação----------//
	Projection = glm::perspective(glm::radians(FoV), 4.0f / 3.0f, 0.1f, 100.0f);
	View = glm::lookAt(
		glm::vec3(0.0f, 0.0f, 5.0f),	// eye (posição da câmara).
		glm::vec3(0.0f, 0.0f, 0.0f),	// center (para onde está a "olhar")
		glm::vec3(0.0f, 1.0f, 0.0f)		// up
	);
	Model = glm::rotate(glm::mat4(), angle, glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 ModelView = View * Model;
	NormalMatrix = glm::inverseTranspose(glm::mat3(ModelView));
	
	//-------------INPUT-------------//
	glfwSetKeyCallback(window, keyCallback);
	glfwSetInputMode(window, GLFW_STICKY_KEYS, 1);
	glfwSetScrollCallback(window, scrollCallback);

	//-------------WINDOW-------------//
	if (!glfwInit())
	{
		fprintf(stderr, "MORREU!!!\n");
		getchar();
		return -1;
	}

	window = glfwCreateWindow(WIDTH, HEIGHT, "Plz Work", NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "PIFOU!!!\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glewExperimental = true;

	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "FALECEU!!!\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	// Set the mouse at the center of the screen
	glfwPollEvents();
	glfwSetCursorPos(window, WIDTH / 2, HEIGHT / 2);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);

	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

	// Show 2 sides of triangle
	glEnable(GL_CULL_FACE);

	glDepthMask(GLU_TRUE);

	//near and far
	glDepthRange(0.01, 100.0);

	glEnable(GL_TEXTURE_2D);

	//-------LOAD MODEL-------
	FILE * file = fopen("Iron_Man.obj", "r");
	if (file == NULL)
	{
		printf("Impossible to open the file !\n");
		return false;
	}
	if (LoadModelVertex(file))
	{
		printf("Read file successfully\n");
	}
	else
	{
		printf("Failed to read file\n");
	}

	//-------LOAD SHADERS------
	ShaderInfo shaders[] = {
		{ GL_VERTEX_SHADER,   "triangles.vert" },
		{ GL_FRAGMENT_SHADER, "triangles.frag" },
		{ GL_NONE, NULL }
	};

	programa = LoadShaders(shaders);
	if (!programa) exit(EXIT_FAILURE);
	glUseProgram(programa);

	//-------------LIGHTING----------//
	// Uniforms

	// Atribui valor ao uniform Model
	GLint modelId = glGetProgramResourceLocation(programa, GL_UNIFORM, "Model");
	glProgramUniformMatrix4fv(programa, modelId, 1, GL_FALSE, glm::value_ptr(Model));
	// Atribui valor ao uniform View
	GLint viewId = glGetProgramResourceLocation(programa, GL_UNIFORM, "View");
	glProgramUniformMatrix4fv(programa, viewId, 1, GL_FALSE, glm::value_ptr(View));
	// Atribui valor ao uniform ModelView
	GLint modelViewId = glGetProgramResourceLocation(programa, GL_UNIFORM, "ModelView");
	glProgramUniformMatrix4fv(programa, modelViewId, 1, GL_FALSE, glm::value_ptr(ModelView));
	// Atribui valor ao uniform Projection
	GLint projectionId = glGetProgramResourceLocation(programa, GL_UNIFORM, "Projection");
	glProgramUniformMatrix4fv(programa, projectionId, 1, GL_FALSE, glm::value_ptr(Projection));
	// Atribui valor ao uniform NormalMatrix
	GLint normalViewId = glGetProgramResourceLocation(programa, GL_UNIFORM, "NormalMatrix");
	glProgramUniformMatrix3fv(programa, normalViewId, 1, GL_FALSE, glm::value_ptr(NormalMatrix));


	//-----LOAD TEXTURE------
	Load_texture("Iron_Man_D.tga");

	//----VAO----
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	//make VBOs
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, out_vertices.size() * sizeof(glm::vec3), &out_uvs[0], GL_STATIC_DRAW);

	glGenBuffers(1, &uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, out_uvs.size() * sizeof(glm::vec2), &out_uvs[0], GL_STATIC_DRAW);

	glGenBuffers(1, &normalbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glBufferData(GL_ARRAY_BUFFER, out_normals.size() * sizeof(glm::vec3), &out_normals[0], GL_STATIC_DRAW);

	//glVertexAttribPointer(uvbuffer, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(0));//3 * sizeof(float)->0 and 8->3
	//glVertexAttribPointer(normalbuffer, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)(0));//(3 + 3) * sizeof(float)->0 and 8->2
	//glVertexAttribPointer(vertexbuffer, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(0));//8->3 (2nd)

	/*glEnableVertexAttribArray(uvbuffer);
	glEnableVertexAttribArray(normalbuffer);
	glEnableVertexAttribArray(vertexbuffer);*/

	glViewport(0, 0, WIDTH, HEIGHT);

	GLint locationTexSampler = glGetProgramResourceLocation(programa, GL_UNIFORM, "TexSampler");
	glProgramUniform1i(programa, locationTexSampler, 0 /* Unidade de Textura #0 */);

	glewExperimental = GL_TRUE;
	glewInit();

	//Draw
	display();

	//End
	glfwTerminate();
}

void print_error(int error, const char *description) {
	cout << description << endl;
}

void display(void) {
	do
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		// Atualiza os dados do Uniform
		Model = glm::rotate(glm::mat4(), angle += 0.0002f, glm::normalize(glm::vec3(1.0f, 1.0f, 0.0f)));
		glm::mat4 ModelView = View * Model;
		NormalMatrix = glm::inverseTranspose(glm::mat3(ModelView));
		// Atribui valor ao uniform Model
		GLint modelId = glGetProgramResourceLocation(programa, GL_UNIFORM, "Model");
		glProgramUniformMatrix4fv(programa, modelId, 1, GL_FALSE, glm::value_ptr(Model));
		GLint viewId = glGetProgramResourceLocation(programa, GL_UNIFORM, "View");
		glProgramUniformMatrix4fv(programa, viewId, 1, GL_FALSE, glm::value_ptr(View));
		GLint modelViewId = glGetProgramResourceLocation(programa, GL_UNIFORM, "ModelView");
		glProgramUniformMatrix4fv(programa, modelViewId, 1, GL_FALSE, glm::value_ptr(ModelView));
		// Atribui valor ao uniform NormalMatrix
		GLint normalViewId = glGetProgramResourceLocation(programa, GL_UNIFORM, "NormalMatrix");
		glProgramUniformMatrix3fv(programa, normalViewId, 1, GL_FALSE, glm::value_ptr(NormalMatrix));


		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		//glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		//glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
		glEnableClientState(GL_VERTEX_ARRAY);
		// Vincula (torna ativo) o VAO
		//glUseProgram(programa);//NEW
		//glBindVertexArray(VAO);
		// Envia comando para desenho de primitivas GL_TRIANGLES, que utilizará os dados do VAO vinculado.
		glDrawArrays(GL_TRIANGLES, 0, out_vertices.size());

		glDisableClientState(GL_VERTEX_ARRAY);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glfwSwapBuffers(window);
		glfwPollEvents();
	} while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0);
}

bool LoadModelVertex(FILE *file)
{
	//read vertexes

	while (1)//read .obj
	{
		char lineHeader[128];
		// read the first word of the line
		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF)
		{
			break;
		}// EOF = End Of File

		if (strcmp(lineHeader, "v") == 0)
		{
			glm::vec3 vertex;
			fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
			temp_vertices.push_back(vertex);
		}
		else if (strcmp(lineHeader, "vt") == 0)
		{
			glm::vec2 uv;
			fscanf(file, "%f %f\n", &uv.x, &uv.y);
			temp_uvs.push_back(uv);
		}
		else if (strcmp(lineHeader, "vn") == 0)
		{
			glm::vec3 normal;
			fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
			temp_normals.push_back(normal);
		}
		else if (strcmp(lineHeader, "f") == 0)
		{
			std::string vertex1, vertex2, vertex3;
			unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
			int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
			if (matches != 9)
			{
				printf("File can't be read :(\n");
				return false;
			}
			vertexIndices.push_back(vertexIndex[0]);
			vertexIndices.push_back(vertexIndex[1]);
			vertexIndices.push_back(vertexIndex[2]);
			uvIndices.push_back(uvIndex[0]);
			uvIndices.push_back(uvIndex[1]);
			uvIndices.push_back(uvIndex[2]);
			normalIndices.push_back(normalIndex[0]);
			normalIndices.push_back(normalIndex[1]);
			normalIndices.push_back(normalIndex[2]);
		}
	}

	for (unsigned int i = 0; i < vertexIndices.size(); i++)
	{
		// Get the indices of its attributes
		unsigned int vertexIndex = vertexIndices[i];
		unsigned int uvIndex = uvIndices[i];
		unsigned int normalIndex = normalIndices[i];

		// Get the attributes thanks to the index
		glm::vec3 vertex = temp_vertices[vertexIndex - 1];
		glm::vec2 uv = temp_uvs[uvIndex - 1];
		glm::vec3 normal = temp_normals[normalIndex - 1];

		// Put the attributes in buffers
		out_vertices.push_back(vertex);
		out_uvs.push_back(uv);
		out_normals.push_back(normal);
	}
	fclose(file);
	return true;
}

void Load_texture(char *namepath) {
	GLuint textureName = 0;

	// Gera um nome de textura
	glGenTextures(1, &textureName);

	// Ativa a Unidade de Textura #0
	// A Unidade de Textura 0 já está ativa por defeito.
	// Só uma Unidade de Textura está ativa a cada momento.
	glActiveTexture(GL_TEXTURE0);

	// Vincula esse nome de textura ao target GL_TEXTURE_2D da Unidade de Textura ativa.
	glBindTexture(GL_TEXTURE_2D, textureName);

	// Define os parâmetros de filtragem (wrapping e ajuste de tamanho)
	// para a textura que está vinculada ao target GL_TEXTURE_2D da Unidade de Textura ativa.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// Leitura/descompressão do ficheiro com imagem de textura
	int width, height, nChannels;
	// Ativa a inversão vertical da imagem, aquando da sua leitura para memória.
	stbi_set_flip_vertically_on_load(true);
	// Leitura da imagem para memória do CPU
	unsigned char *imageData = stbi_load(namepath, &width, &height, &nChannels, 0);
	if (imageData) {
		// Carrega os dados da imagem para o Objeto de Textura vinculado ao target GL_TEXTURE_2D da Unidade de Textura ativa.
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, nChannels == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, imageData);

		// Gera o Mipmap para essa textura
		glGenerateMipmap(GL_TEXTURE_2D);

		// Liberta a imagem da memória do CPU
		stbi_image_free(imageData);
	}
	else {
		cout << "Error loading texture!" << endl;
	}
	printf("Texture loaded?\n");
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	if (FoV >= 1.0f && FoV <= 45.0f)
		FoV -= yoffset;
	if (FoV <= 1.0f)
		FoV = 1.0f;
	if (FoV >= 45.0f)
		FoV = 45.0f;
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	static bool luz1 = false;
	static bool luz2 = false;
	static bool luz3 = false;
	static bool luz4 = false;
	if (key == GLFW_KEY_1 && action == GLFW_PRESS)
	{
		if (luz1 == false)
		{
			// Fonte de luz ambiente global on
			glProgramUniform3fv(programa, glGetProgramResourceLocation(programa, GL_UNIFORM, "ambientLight.ambient"), 1, glm::value_ptr(glm::vec3(0.1, 0.1, 0.1)));
			luz1 = true;
		}
		else
		{
			// Fonte de luz ambiente global off
			glProgramUniform3fv(programa, glGetProgramResourceLocation(programa, GL_UNIFORM, "ambientLight.ambient"), 1, glm::value_ptr(glm::vec3(0.0, 0.0, 0.0)));
			luz1 = false;
			
		}

	}
	if (key == GLFW_KEY_2 && action == GLFW_PRESS)
	{
		if (luz2 == false)
		{
			// Fonte de luz direcional
			glProgramUniform3fv(programa, glGetProgramResourceLocation(programa, GL_UNIFORM, "directionalLight.direction"), 1, glm::value_ptr(glm::vec3(1.0, 0.0, 0.0)));
			glProgramUniform3fv(programa, glGetProgramResourceLocation(programa, GL_UNIFORM, "directionalLight.ambient"), 1, glm::value_ptr(glm::vec3(0.2, 0.2, 0.2)));
			glProgramUniform3fv(programa, glGetProgramResourceLocation(programa, GL_UNIFORM, "directionalLight.diffuse"), 1, glm::value_ptr(glm::vec3(1.0, 1.0, 1.0)));
			glProgramUniform3fv(programa, glGetProgramResourceLocation(programa, GL_UNIFORM, "directionalLight.specular"), 1, glm::value_ptr(glm::vec3(1.0, 1.0, 1.0)));
			luz2 = true;
		}
		else
		{
			glProgramUniform3fv(programa, glGetProgramResourceLocation(programa, GL_UNIFORM, "directionalLight.direction"), 1, glm::value_ptr(glm::vec3(0.0, 0.0, 0.0)));
			glProgramUniform3fv(programa, glGetProgramResourceLocation(programa, GL_UNIFORM, "directionalLight.ambient"), 1, glm::value_ptr(glm::vec3(0.0, 0.0, 0.0)));
			glProgramUniform3fv(programa, glGetProgramResourceLocation(programa, GL_UNIFORM, "directionalLight.diffuse"), 1, glm::value_ptr(glm::vec3(0.0, 0.0, 0.0)));
			glProgramUniform3fv(programa, glGetProgramResourceLocation(programa, GL_UNIFORM, "directionalLight.specular"), 1, glm::value_ptr(glm::vec3(0.0, 0.0, 0.0)));
			luz2 = false;
		}
	}
	if (key == GLFW_KEY_3 && action == GLFW_PRESS)
	{
		if (luz3 == false)
		{
			// Fonte de luz pontual #1
			glProgramUniform3fv(programa, glGetProgramResourceLocation(programa, GL_UNIFORM, "pointLight[0].position"), 1, glm::value_ptr(glm::vec3(0.0, 0.0, 5.0)));
			glProgramUniform3fv(programa, glGetProgramResourceLocation(programa, GL_UNIFORM, "pointLight[0].ambient"), 1, glm::value_ptr(glm::vec3(0.1, 0.1, 0.1)));
			glProgramUniform3fv(programa, glGetProgramResourceLocation(programa, GL_UNIFORM, "pointLight[0].diffuse"), 1, glm::value_ptr(glm::vec3(1.0, 1.0, 1.0)));
			glProgramUniform3fv(programa, glGetProgramResourceLocation(programa, GL_UNIFORM, "pointLight[0].specular"), 1, glm::value_ptr(glm::vec3(1.0, 1.0, 1.0)));
			glProgramUniform1f(programa, glGetProgramResourceLocation(programa, GL_UNIFORM, "pointLight[0].constant"), 1.0f);
			glProgramUniform1f(programa, glGetProgramResourceLocation(programa, GL_UNIFORM, "pointLight[0].linear"), 0.06f);
			glProgramUniform1f(programa, glGetProgramResourceLocation(programa, GL_UNIFORM, "pointLight[0].quadratic"), 0.02f);
			luz3 = true;
		}
		else
		{
			glProgramUniform3fv(programa, glGetProgramResourceLocation(programa, GL_UNIFORM, "pointLight[0].position"), 1, glm::value_ptr(glm::vec3(0.0, 0.0, 0.0)));
			glProgramUniform3fv(programa, glGetProgramResourceLocation(programa, GL_UNIFORM, "pointLight[0].ambient"), 1, glm::value_ptr(glm::vec3(0.0, 0.0, 0.0)));
			glProgramUniform3fv(programa, glGetProgramResourceLocation(programa, GL_UNIFORM, "pointLight[0].diffuse"), 1, glm::value_ptr(glm::vec3(0.0, 0.0, 0.0)));
			glProgramUniform3fv(programa, glGetProgramResourceLocation(programa, GL_UNIFORM, "pointLight[0].specular"), 1, glm::value_ptr(glm::vec3(0.0, 0.0, 0.0)));
			glProgramUniform1f(programa, glGetProgramResourceLocation(programa, GL_UNIFORM, "pointLight[0].constant"), 0.0f);
			glProgramUniform1f(programa, glGetProgramResourceLocation(programa, GL_UNIFORM, "pointLight[0].linear"), 0.0f);
			glProgramUniform1f(programa, glGetProgramResourceLocation(programa, GL_UNIFORM, "pointLight[0].quadratic"), 0.0f);
			luz3 = false;
		}

	}
	if (key == GLFW_KEY_4 && action == GLFW_PRESS)
	{
		if (luz4 == false)
		{
			// Fonte de luz pontual #2
			glProgramUniform3fv(programa, glGetProgramResourceLocation(programa, GL_UNIFORM, "pointLight[1].position"), 1, glm::value_ptr(glm::vec3(-2.0, 2.0, 5.0)));
			glProgramUniform3fv(programa, glGetProgramResourceLocation(programa, GL_UNIFORM, "pointLight[1].ambient"), 1, glm::value_ptr(glm::vec3(0.1, 0.1, 0.1)));
			glProgramUniform3fv(programa, glGetProgramResourceLocation(programa, GL_UNIFORM, "pointLight[1].diffuse"), 1, glm::value_ptr(glm::vec3(1.0, 1.0, 1.0)));
			glProgramUniform3fv(programa, glGetProgramResourceLocation(programa, GL_UNIFORM, "pointLight[1].specular"), 1, glm::value_ptr(glm::vec3(1.0, 1.0, 1.0)));
			glProgramUniform1f(programa, glGetProgramResourceLocation(programa, GL_UNIFORM, "pointLight[1].constant"), 1.0f);
			glProgramUniform1f(programa, glGetProgramResourceLocation(programa, GL_UNIFORM, "pointLight[1].linear"), 0.06f);
			glProgramUniform1f(programa, glGetProgramResourceLocation(programa, GL_UNIFORM, "pointLight[1].quadratic"), 0.02f);

			glProgramUniform3fv(programa, glGetProgramResourceLocation(programa, GL_UNIFORM, "material.emissive"), 1, glm::value_ptr(glm::vec3(0.0, 0.0, 0.0)));
			glProgramUniform3fv(programa, glGetProgramResourceLocation(programa, GL_UNIFORM, "material.ambient"), 1, glm::value_ptr(glm::vec3(1.0, 1.0, 1.0)));
			glProgramUniform3fv(programa, glGetProgramResourceLocation(programa, GL_UNIFORM, "material.diffuse"), 1, glm::value_ptr(glm::vec3(1.0, 1.0, 1.0)));
			glProgramUniform3fv(programa, glGetProgramResourceLocation(programa, GL_UNIFORM, "material.specular"), 1, glm::value_ptr(glm::vec3(1.0, 1.0, 1.0)));
			glProgramUniform1f(programa, glGetProgramResourceLocation(programa, GL_UNIFORM, "material.shininess"), 12.0f);
			luz4 = true;
		}
		else
		{
			glProgramUniform3fv(programa, glGetProgramResourceLocation(programa, GL_UNIFORM, "pointLight[1].position"), 1, glm::value_ptr(glm::vec3(0.0, 0.0, 0.0)));
			glProgramUniform3fv(programa, glGetProgramResourceLocation(programa, GL_UNIFORM, "pointLight[1].ambient"), 1, glm::value_ptr(glm::vec3(0.0, 0.0, 0.0)));
			glProgramUniform3fv(programa, glGetProgramResourceLocation(programa, GL_UNIFORM, "pointLight[1].diffuse"), 1, glm::value_ptr(glm::vec3(0.0, 0.0, 0.0)));
			glProgramUniform3fv(programa, glGetProgramResourceLocation(programa, GL_UNIFORM, "pointLight[1].specular"), 1, glm::value_ptr(glm::vec3(0.0, 0.0, 0.0)));
			glProgramUniform1f(programa, glGetProgramResourceLocation(programa, GL_UNIFORM, "pointLight[1].constant"), 0.0f);
			glProgramUniform1f(programa, glGetProgramResourceLocation(programa, GL_UNIFORM, "pointLight[1].linear"), 0.0f);
			glProgramUniform1f(programa, glGetProgramResourceLocation(programa, GL_UNIFORM, "pointLight[1].quadratic"), 0.0f);

			glProgramUniform3fv(programa, glGetProgramResourceLocation(programa, GL_UNIFORM, "material.emissive"), 1, glm::value_ptr(glm::vec3(0.0, 0.0, 0.0)));
			glProgramUniform3fv(programa, glGetProgramResourceLocation(programa, GL_UNIFORM, "material.ambient"), 1, glm::value_ptr(glm::vec3(0.0, 0.0, 0.0)));
			glProgramUniform3fv(programa, glGetProgramResourceLocation(programa, GL_UNIFORM, "material.diffuse"), 1, glm::value_ptr(glm::vec3(0.0, 0.0, 0.0)));
			glProgramUniform3fv(programa, glGetProgramResourceLocation(programa, GL_UNIFORM, "material.specular"), 1, glm::value_ptr(glm::vec3(0.0, 0.0, 0.0)));
			glProgramUniform1f(programa, glGetProgramResourceLocation(programa, GL_UNIFORM, "material.shininess"), 0.0f);
			luz4 = false;
		}
	}
}