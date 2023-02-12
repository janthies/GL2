#define GLEW_STATIC
#define GLFW_INCLUDE_NONE




#include <Gl/glew.h>
#include <GLFW/glfw3.h>


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Camera;

struct SharedContext
{
	Camera* worldCamera;
};

class Camera
{
public:
	Camera(float aspectRatio)
		:m_AspectRatio(aspectRatio)
	{
		GLFWwindow* context = glfwGetCurrentContext();
		glfwGetCursorPos(context, &m_CursorLastX, &m_CursorLastY);
	}

	glm::mat4& GetViewMatrix()
	{
		if (m_ViewIsDirty)
		{
			m_ViewIsDirty = false;

			m_ViewMatrix = glm::lookAt(
				m_Position,
				m_Position + CalculateForwardVector(),
				m_Up
			);
		}
		return m_ViewMatrix;
	}

	glm::mat4& GetPerspectiveMatrix()
	{
		if (m_PerspectiveIsDirty)
		{
			m_PerspectiveIsDirty = false;

			m_PerspectiveProjection = glm::perspective(
				glm::radians(m_Fov),
				m_AspectRatio,
				m_NearZ,
				m_FarZ
			);
		}

		return m_PerspectiveProjection;
	}

	void Update(const float dt)
	{
		GLFWwindow* context = glfwGetCurrentContext();

		// Preven't the camera from losing horizontal speed when looking up or down
		const glm::vec3 forward = CalculateForwardVector();
		const glm::vec3 horizontalDir = { forward.x, 0, forward.z };
		const glm::vec3 direction = glm::normalize(horizontalDir);


		float moveSpeed = m_BaseSpeed;
		if(glfwGetKey(context, GLFW_KEY_E))
		{
			moveSpeed *= m_SpeedModifier;
		}

		if(glfwGetKey(context, GLFW_KEY_W))
		{
			Translate(glm::vec3(direction.x, 0, direction.z) * moveSpeed * dt);
		}

		if(glfwGetKey(context, GLFW_KEY_S))
		{
			Translate(glm::vec3(-direction.x, 0, -direction.z) * moveSpeed * dt);
		}

		if (glfwGetKey(context, GLFW_KEY_A))
		{
			Translate(glm::vec3(direction.z, 0, -direction.x) * moveSpeed * dt);
		}

		if (glfwGetKey(context, GLFW_KEY_D))
		{
			Translate(glm::vec3(-direction.z, 0, direction.x) * moveSpeed * dt);
		}

		if (glfwGetKey(context, GLFW_KEY_LEFT_CONTROL))
		{
			Translate(glm::vec3(0.f, -1, 0.f) * moveSpeed * dt);
		}

		if (glfwGetKey(context, GLFW_KEY_SPACE))
		{
			Translate(glm::vec3(0.f, 1, 0.f) * moveSpeed * dt);
		}

		double currentCursorX, currentCursorY;
		glfwGetCursorPos(context, &currentCursorX, &currentCursorY);

		const double mouseDeltaX = currentCursorX - m_CursorLastX;
		const double mouseDeltaY = currentCursorY - m_CursorLastY;

		m_CursorLastX = currentCursorX;
		m_CursorLastY = currentCursorY;

		if(abs(mouseDeltaX) > m_MouseThreshhold || abs(mouseDeltaY) > m_MouseThreshhold)
			Rotate(glm::vec3(-mouseDeltaY, mouseDeltaX, 0) * dt * m_RotationSpeed);
	}

	void SetAspectRatio(float aspectRatio)
	{
		m_PerspectiveIsDirty = true;
		m_AspectRatio = aspectRatio;
	}

	void SetFov(float fov)
	{
		m_PerspectiveIsDirty = true;
		m_Fov = fov;
	}

	void SetNearFarPlane(float near, float far)
	{
		m_PerspectiveIsDirty = true;
		m_NearZ = near;
		m_FarZ = far;
	}

	void SetPosition(const glm::vec3& position)
	{
		m_Position = position;
		m_ViewIsDirty = true;
	}

	void Translate(glm::vec3&& step)
	{
		m_Position += step;
		m_ViewIsDirty = true;
	}

	void SetRotation(const glm::vec3& rotation)
	{
		m_Rotation = rotation;
		m_Rotation.x = std::clamp(m_Rotation.x, -89.f, 89.f);

		m_ViewIsDirty = true;
	}

	void Rotate(const glm::vec3& angles)
	{
		m_Rotation += angles;
		m_Rotation.x = std::clamp(m_Rotation.x, -89.f, 89.f);
		
		m_ViewIsDirty = true;
	}

private:

	[[nodiscard]] glm::vec3 CalculateForwardVector() const
	{
		return
		{
			cos(glm::radians(m_Rotation.x)) * cos(glm::radians(m_Rotation.y)),
			sin(glm::radians(m_Rotation.x)),
			sin(glm::radians(m_Rotation.y)) * cos(glm::radians(m_Rotation.x))
		};
	}

private:
	glm::mat4 m_ViewMatrix = glm::mat4(1.f);
	glm::mat4 m_PerspectiveProjection = glm::mat4(1.f);

	glm::vec3 m_Position = {0.f, 0.f, 5.f};
	glm::vec3 m_Rotation = {0.f, -90.f, 0.f};
	glm::vec3 m_Up = {0.f, 1.f, 0.f};

	float m_Fov = 45.f;
	float m_AspectRatio;
	float m_NearZ = 0.1f;
	float m_FarZ = 1000.f;

	bool m_ViewIsDirty = true;
	bool m_PerspectiveIsDirty = true;

	float m_BaseSpeed = 2.f;
	float m_SpeedModifier = 10.f;
	float m_RotationSpeed = 6.f;

	double m_CursorLastX = 0;
	double m_CursorLastY = 0;
	double m_MouseThreshhold = 0.0001;
};

void windowCloseFun(GLFWwindow* window)
{
	glfwDestroyWindow(window);
	LOG_INFO("Destroyed window")
}

void WindowSizeCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	auto sharedContext = static_cast<SharedContext*>(glfwGetWindowUserPointer(window));
	sharedContext->worldCamera->SetAspectRatio((float)width / (float)height);
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

	GLFWwindow* window = glfwCreateWindow(2560, 1440, "Init", NULL, NULL);

	if (!window)
	{

		std::cout << "Couln't initialize window\n";
		glfwTerminate();
		abort();
	}

	glfwMakeContextCurrent(window);
	glfwSetWindowCloseCallback(window, windowCloseFun);
	glfwSetWindowSizeCallback(window, WindowSizeCallback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSwapInterval(1);


	if (GLEW_OK != glewInit())
	{
		std::cout << "Couldn't initialize GLEW\n";
		glfwTerminate();
		abort();
	}

	
	
	int w, h;
	glfwGetWindowSize(window, &w, &h);

	Camera camera((float)w/h);

	SharedContext sharedContext{};
	sharedContext.worldCamera = &camera;
	glfwSetWindowUserPointer(window, &sharedContext);

	


	const char* vertexShaderText =
		"#version 460\n"
		"layout(location = 0) in vec3 a_Position;\n"
		"uniform mat4 u_ModelMat;\n"
		"uniform mat4 u_ViewMat;\n"
		"uniform mat4 u_PerspectiveMat;\n"
		"void main()\n"
		"{\n"
		"gl_Position = u_PerspectiveMat * u_ViewMat * u_ModelMat * vec4(a_Position, 1.0);\n"
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

	// Associate elementbuffer with vertexarray
	glVertexArrayElementBuffer(VAOID, indexBuffer);



	GLint modelMatLoc = glGetUniformLocation(program, "u_ModelMat");
	GLint viewMatLoc = glGetUniformLocation(program, "u_ViewMat");
	GLint perspectiveMatLoc = glGetUniformLocation(program, "u_PerspectiveMat");

	glm::mat4 identity = glm::mat4(1.f);

	glm::mat4 rectModelMat = identity;

	
	glUseProgram(program);
	glUniformMatrix4fv(modelMatLoc, 1, GL_FALSE, glm::value_ptr(identity));
	glUniformMatrix4fv(viewMatLoc, 1, GL_FALSE, glm::value_ptr(camera.GetViewMatrix()));
	glUniformMatrix4fv(perspectiveMatLoc, 1, GL_FALSE, glm::value_ptr(camera.GetPerspectiveMatrix()));
	glUseProgram(0);


	glBindVertexArray(VAOID);
	glUseProgram(program);

	glClearColor(0.16f, 0.2f, 0.35f, 1.f);
	while (!glfwWindowShouldClose(window))
	{
		static double lastTime = glfwGetTime();

		const double timeNow = glfwGetTime();
		const double dt = timeNow - lastTime;
		lastTime = timeNow;

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		camera.Update(dt);

		//rectModelMat = glm::rotate(rectModelMat, (float)dt * glm::radians(90.f), glm::vec3(0.f, 1.f, 0.f));
		glUniformMatrix4fv(modelMatLoc, 1, GL_FALSE, glm::value_ptr(rectModelMat));
		glUniformMatrix4fv(viewMatLoc, 1, GL_FALSE, glm::value_ptr(camera.GetViewMatrix()));
		glUniformMatrix4fv(perspectiveMatLoc, 1, GL_FALSE, glm::value_ptr(camera.GetPerspectiveMatrix()));


		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();



	return 0;
}