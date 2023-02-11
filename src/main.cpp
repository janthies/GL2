#define GLEW_STATIC
#define GLFW_INCLUDE_NONE

#include <Gl/glew.h>
#include <GLFW/glfw3.h>

struct WindowAttributes
{
	uint16_t width;
	uint16_t height;
	const char* name;
} windowAttributes;

void windowCloseFun(GLFWwindow* window)
{
	std::cout << "Destroying window\n";
	glfwDestroyWindow(window);
}



void WindowSizeCallback(GLFWwindow* window, int width, int height)
{
	glViewport(100, 0, width, height);
}

int main()
{
	if (glfwInit() != GLFW_TRUE)
	{
		std::cout << "Couldn't initialize GLFW\n";
		abort();
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(800, 600, "Init", NULL, NULL);

	if (!window)
	{

		std::cout << "Couln't initialize window\n";
		glfwTerminate();
		abort();
	}

	glfwMakeContextCurrent(window);
	glfwSetWindowCloseCallback(window, windowCloseFun);
	glfwSetWindowSizeCallback(window, WindowSizeCallback);


	if (GLEW_OK != glewInit())
	{
		std::cout << "Couldn't initialize GLEW\n";
		glfwTerminate();
		abort();
	}


	const char* vertexShaderText =
		"#version 460\n"
		"layout(location = 0) in vec3 a_Position;\n"
		"void main()\n"
		"{\n"
		"gl_Position = vec4(a_Position, 1.0);\n"
		"}";

	const char* fragmentShaderText =
		"#version 460\n"
		"out vec4 color;"
		"\n"
		"void main()\n"
		"{\n"
		"color = vec4(0.6, 0.5, 0.0, 1.0);\n"
		"}\n";

	


	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderText, NULL);
	glCompileShader(vertexShader);

	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderText, NULL);
	glCompileShader(fragmentShader);

	GLuint program = glCreateProgram();
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	glLinkProgram(program);


	float trianglePositions[] = {
		-0.5f, -0.5f, 0.0f,
		 0.5f, -0.5f, 0.0f,
		 0.5f,  0.5f, 0.0f,
		-0.5f,  0.5f, 0.0f
	};

	GLuint triangleIndices[] = {
		0,1,2,
		0,2,3
	};


	GLuint vertexBuffer;
	glCreateBuffers(1, &vertexBuffer);
	glNamedBufferData(vertexBuffer, sizeof(trianglePositions), trianglePositions, GL_STATIC_DRAW);


	GLuint indexBuffer;
	glCreateBuffers(1, &indexBuffer);
	glNamedBufferData(indexBuffer, sizeof(triangleIndices), triangleIndices, GL_STATIC_DRAW);


	// create a vao
	GLuint VAOID;
	glCreateVertexArrays(1, &VAOID);

	// enable vao binding so that vertices get passed along to shader
	glEnableVertexArrayAttrib(VAOID, 0);

	// associate VAO binding with vertex attribute in buffer
	glVertexArrayVertexBuffer(VAOID, 0, vertexBuffer, 0, 3 * sizeof(float));


	// define format of vertex attrib
	glVertexArrayAttribFormat(VAOID, 0, 3, GL_FLOAT, GL_FALSE, 0);


	glVertexArrayElementBuffer(VAOID, indexBuffer);


	glBindVertexArray(VAOID);
	glUseProgram(program);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glClearColor(0.16f, 0.2f, 0.35f, 1.f);
	while (!glfwWindowShouldClose(window))
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();



	return 0;
}