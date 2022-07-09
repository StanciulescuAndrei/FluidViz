#include "libraries/shader.h"
#include "libraries/PerlinNoise.h"
#include "libraries/Simulator.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>

#define gridSize 128

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void processInput(GLFWwindow* window);
void updateSurface(float* vertexBuffer);

double xMousePos = 0, yMousePos = 0;
float rotX = 0, rotY = 0;

float heightMap[gridSize][gridSize];
float dhMap[gridSize][gridSize];

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 800;
glm::vec3 cameraPos = glm::vec3(0.0f, 1.5f, 1.5f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

glm::mat4 rotationMatrix = glm::mat4(1.0f);
glm::mat4 view = glm::mat4(1.0f);

int noiseDirX = -1, noiseDirY = -1;

Simulator sim(gridSize);

float degToRad(float degrees) {
	return 3.1415 * degrees / 180;
}
float t = 0.0f;

int main()
{
	// Initializari pentru fereastra si environment OpenGL
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "FluidViz", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glViewport(0.0f, 0.0f, SCR_WIDTH, SCR_HEIGHT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, SCR_WIDTH, 0, SCR_HEIGHT, 0, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetKeyCallback(window, key_callback);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glewInit();

	// Incarcarea celor 3 shadere

	Shader surfaceShader("shaders/surface_vertex_shader.vs", "shaders/surface_fragment_shader.fs");
	Shader wireframeShader("shaders/wireframe_vertex_shader.vs", "shaders/wireframe_fragment_shader.fs");
	Shader cubeShader("shaders/cube_vertex_shader.vs", "shaders/cube_fragment_shader.fs");
#pragma region wireframe
	// Desenare wireframe pentru delimitarea volumului de simulare

	float vertices[] = {-1, -1, -1, 1, 0, 1,
						-1, 1, -1, 1, 1, 0,
						1, 1, -1, 0, 1, 1,
						1, -1, -1, 0, 1, 0,
						-1, -1, 1, 1, 0, 1,
						-1, 1, 1, 1, 1, 0,
						1, 1, 1, 0, 1, 1,
						1, -1, 1, 0, 1, 0,
						-1, -1, -1, 1, 0, 1,
						-1, -1, 1, 1, 0, 1,
						-1, 1, -1, 1, 1, 0,
						-1, 1, 1, 1, 1, 0,
						1, 1, -1, 0, 1, 1, 
						1, 1, 1, 0, 1, 1,
						1, -1, -1, 0, 1, 0,
						1, -1, 1, 0, 1, 0, };

	unsigned int VBO, VAO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, 16 * 6 * sizeof(float), vertices, GL_STATIC_DRAW);

	/* Pozitii */
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 24, (void*)0);
	glEnableVertexAttribArray(0);
	/* Culori */
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 24, (void*)12);
	glEnableVertexAttribArray(1);


	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
#pragma endregion

#pragma region cube
	// Cub in interiorul fluidului ca sa evidentiem transparenta
	float cubepos[] = {-0.2, -0.4, -0.2};
	float cubesize = 0.2;
	float cube_vertices[] = { 
		cubepos[0] - cubesize, cubepos[1] - cubesize, cubepos[2] - cubesize,
		cubepos[0] - cubesize, cubepos[1] - cubesize, cubepos[2] + cubesize,
		cubepos[0] + cubesize, cubepos[1] - cubesize, cubepos[2] + cubesize,
		cubepos[0] + cubesize, cubepos[1] - cubesize, cubepos[2] - cubesize,
		cubepos[0] - cubesize, cubepos[1] + cubesize, cubepos[2] - cubesize,
		cubepos[0] - cubesize, cubepos[1] + cubesize, cubepos[2] + cubesize,
		cubepos[0] + cubesize, cubepos[1] + cubesize, cubepos[2] + cubesize,
		cubepos[0] + cubesize, cubepos[1] + cubesize, cubepos[2] - cubesize,

	};

	GLubyte cubeIndices[] = {
		0, 1, 3,
		3, 1, 2,
		0, 4, 3,
		3, 4, 7,
		3, 7, 2,
		2, 7, 6,
		2, 6, 5,
		2, 5, 1,
		5, 4, 1,
		1, 4, 0,
		4, 5, 7,
		7, 5, 6
	};

	unsigned int VBO_cube, VAO_cube;
	glGenVertexArrays(1, &VAO_cube);
	glGenBuffers(1, &VBO_cube);
	glBindVertexArray(VAO_cube);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_cube);
	glBufferData(GL_ARRAY_BUFFER, 8 * 3 * sizeof(float), cube_vertices, GL_STATIC_DRAW);

	/* Pozitii */
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
#pragma endregion

#pragma region surface
	// Suprafata fluidului  
	float dx = 2.0f / (gridSize - 1);
	float dy = 2.0f / (gridSize - 1);
	float* surfVertices = new float[gridSize * gridSize * 9];

	// Initializare ca sa avem ceva valori, vor fi suprascrise oricum
	for (int y = 0; y < gridSize; y++) {
		for (int x = 0; x < gridSize; x++)
		{
			int offset = y * gridSize * 9 + x * 9;
			// Pozitie
			surfVertices[offset + 0] = -1.0f + dx * x;
			surfVertices[offset + 1] = -1.0f;
			surfVertices[offset + 2] = -1.0f + dy * y;
			// Culoare
			surfVertices[offset + 3] = 0.5;
			surfVertices[offset + 4] = 0.5;
			surfVertices[offset + 5] = 0.5;
			// Normala
			surfVertices[offset + 6] = 0.5;
			surfVertices[offset + 7] = 0.5;
			surfVertices[offset + 8] = 0.5;
		}
	}

	GLuint* surfIndices = new GLuint[6 * (gridSize - 1) * (gridSize - 1)];
	int counter = 0;
	for (int y = 0; y < (gridSize - 1); y++)
	{
		for (int x = 0; x < (gridSize - 1); x++)
		{
			surfIndices[counter + 0] = x + y * gridSize;
			surfIndices[counter + 1] = x + 1 + gridSize + y * gridSize;
			surfIndices[counter + 2] = x + 1 + y * gridSize;
			surfIndices[counter + 3] = x + y * gridSize;
			surfIndices[counter + 4] = x + (y + 1) * gridSize;
			surfIndices[counter + 5] = (x + 1) + (y + 1) * gridSize;
			counter += 6;
		}
	}

	unsigned int VBO_surface, VAO_surface;
	glGenVertexArrays(1, &VAO_surface);
	glGenBuffers(1, &VBO_surface);
	glBindVertexArray(VAO_surface);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_surface);
	glBufferData(GL_ARRAY_BUFFER, gridSize * gridSize * 9 * sizeof(float), surfVertices, GL_DYNAMIC_DRAW);

	/* Pozitii */
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 36, (void*)0);
	glEnableVertexAttribArray(0);
	/* Culori */
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 36, (void*)12);
	glEnableVertexAttribArray(1);
	/* Normale */
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 36, (void*)24);
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

#pragma endregion

	// Bucla de randare
	while (!glfwWindowShouldClose(window))
	{
		processInput(window);
		// Punem culoarea albastru inchis de fundal
		glClearColor(0.14, 0.13, 0.23, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		
		glm::mat4 projection = glm::mat4(1.0f);
		projection = glm::perspective(glm::radians(90.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		view =  glm::lookAt(cameraPos, cameraFront, glm::normalize(cameraUp));

		wireframeShader.use();
		wireframeShader.setMat4("view", view);
		wireframeShader.setMat4("projection", projection);
		wireframeShader.setMat4("rotMatrix", rotationMatrix);
		

		// Desenare wireframe
		glBindVertexArray(VAO);
		// Patratul de sus
		glDrawArrays(GL_LINE_LOOP, 0, 4);
		// Patratul de jos
		glDrawArrays(GL_LINE_LOOP, 4, 4);
		// Laturile verticale
		glDrawArrays(GL_LINES, 8, 2);
		glDrawArrays(GL_LINES, 10, 2);
		glDrawArrays(GL_LINES, 12, 2);
		glDrawArrays(GL_LINES, 14, 2);
		glBindVertexArray(0); // no need to unbind it every time 

		// Desenam cubul din interior
		cubeShader.use();
		cubeShader.setMat4("view", view);
		cubeShader.setMat4("projection", projection);
		cubeShader.setMat4("rotMatrix", rotationMatrix);
		cubeShader.setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
		cubeShader.setVec3("lightPos", glm::vec3(0.0f, 30.0f, 0.0f));
		cubeShader.setVec3("viewPos", cameraPos);
		glBindVertexArray(VAO_cube);
		glDrawElements(GL_TRIANGLES, 6 * 6, GL_UNSIGNED_BYTE, cubeIndices);
		glBindVertexArray(0); // no need to unbind it every time 

		// In final desenam si suprafata
		surfaceShader.use();
		surfaceShader.setMat4("view", view);
		surfaceShader.setMat4("projection", projection);
		surfaceShader.setMat4("rotMatrix", rotationMatrix);
		surfaceShader.setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
		surfaceShader.setVec3("lightPos", glm::vec3(0.0f, 30.0f, 0.0f));
		surfaceShader.setVec3("viewPos", cameraPos);
		
		// Reactualizare pozitii, culoare, normale, etc
		updateSurface(surfVertices);
		glBindVertexArray(VAO_surface);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_surface);
		glBufferData(GL_ARRAY_BUFFER, gridSize * gridSize * 9 * sizeof(float), surfVertices, GL_DYNAMIC_DRAW);
		glDrawElements(GL_TRIANGLES, 6 * (gridSize - 1) * (gridSize - 1), GL_UNSIGNED_INT, surfIndices);
		glBindVertexArray(0);

		glfwSwapBuffers(window);
		glfwPollEvents();
		// Mai facem un pas in simulare
		sim.step();
		t += 1.0 / 60.0;
	}
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteVertexArrays(1, &VAO_surface);
	glDeleteBuffers(1, &VBO_surface);
	glfwTerminate();
	return 0;
}

void updateSurface(float* vertexBuffer) {
	float * hm = sim.getHeightMap();
	for (int y = 1; y < gridSize - 1; y++) {
		for (int x = 1; x < gridSize - 1; x++)
		{
			int offset = y * gridSize * 9 + x * 9;

			// Scoatem inaltimea din simulare, coordonatele in plan raman constante pt ca este un grid
			vertexBuffer[offset + 1] = hm[y * gridSize + x];

			// Culori - facem un mix cu inaltimea ca sa fie ceva variatie
			vertexBuffer[offset + 3] = 0.2 + vertexBuffer[offset + 1];
			vertexBuffer[offset + 4] = 0.5 + vertexBuffer[offset + 1];
			vertexBuffer[offset + 5] = 0.8;

			// Normala
			// Normala o calculam ca produsul vectorial al vectorului spre dreapta pe x si a celui pe diagonala dreapta sus
			int offset1 = (y + 1) * gridSize * 9 + (x + 1) * 9;
			int offset2 = y * gridSize * 9 + (x + 1) * 9;

			// Cei 2 vectori care definesc planul caruia ii apartine respectivul vertex:
			glm::vec3 latura1 = glm::vec3(vertexBuffer[offset1 + 0] - vertexBuffer[offset + 0],
				vertexBuffer[offset1 + 1] - vertexBuffer[offset + 1],
				vertexBuffer[offset1 + 2] - vertexBuffer[offset + 2]);
			glm::vec3 latura2 = glm::vec3(vertexBuffer[offset2 + 0] - vertexBuffer[offset + 0],
				vertexBuffer[offset2 + 1] - vertexBuffer[offset + 1],
				vertexBuffer[offset2 + 2] - vertexBuffer[offset + 2]);

			// Normala la suprafata triunghiului curent
			glm::vec3 normalVect = glm::cross(latura1, latura2);

			vertexBuffer[offset + 6] = normalVect.x;
			vertexBuffer[offset + 7] = normalVect.y;
			vertexBuffer[offset + 8] = normalVect.z;
		}
	}
}

void processInput(GLFWwindow* window)
{

	// Procesare evt. mouse
	int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);

	double newMouseX, newMouseY;
	float deltaX, deltaY;
	glfwGetCursorPos(window, &newMouseX, &newMouseY);
	deltaX = newMouseX - xMousePos;
	deltaY = newMouseY - yMousePos;

	if (state == GLFW_PRESS)
	{
		// Trebuie facute inversarile de rotatii, altfel se roteste in spatiul de coordonate propriu si nu pare intuititv
		glm::vec3 relative_x = glm::inverse(rotationMatrix) * glm::inverse(view) * glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
		glm::vec3 relative_y = glm::inverse(rotationMatrix) * glm::inverse(view) * glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
		rotationMatrix = glm::rotate(rotationMatrix, deltaY * 0.005f, glm::vec3(relative_x));
		rotationMatrix = glm::rotate(rotationMatrix, deltaX * 0.005f, glm::vec3(relative_y));
	}

	xMousePos = newMouseX;
	yMousePos = newMouseY;

}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// Cand se apasa space adaugam o picatura random
	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
		int x = rand() % (gridSize - 20) + 10;
		int y = rand() % (gridSize - 20) + 10;
		sim.addDrop(x, y, 8 + rand() % 4);
	}
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

