#define GLEW_STATIC
#define GLFW_INCLUDE_NONE




#include <Gl/glew.h>
#include <GLFW/glfw3.h>


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

void DebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH:
		LOG_ERROR("GL_SEVERITY_HIGH")
			break;

	case GL_DEBUG_SEVERITY_MEDIUM:
		LOG_WARN("GL_SEVERITY_MEDIUM")
			break;

	case GL_DEBUG_SEVERITY_LOW:
		LOG_INFO("GL_SEVERITY_LOW")
			break;

	case GL_DEBUG_SEVERITY_NOTIFICATION:
		LOG_DEBUG("GL_SEVERITY_NOTIFICATION")
			break;
	}

	LOG_DEBUG("GL_SOURCE: %u", source)
		LOG_DEBUG("GL_TYPE: %u", type)
		LOG_DEBUG("GL_ID: %u", id)
		LOG_DEBUG("GL_MESSAGE: %s", message)


}


class Camera;
class GeometryManager;

struct SharedContext
{
	Camera* worldCamera;
	GeometryManager* geometryManager;
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

		LOG_INFO("%f, %f, %f", m_Rotation.x,m_Rotation.y, m_Rotation.z)
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

typedef uint32_t GeoID;

#define VERTEX_BUFFER_SIZE 1024 * 1024 * 16 //16mb
#define ELEMENT_BUFFER_SIZE 1024 * 1024 * 16 //16mb

struct Geometry
{
	GLsizei elementCount;
	GLuint firstIndex;
	GLint baseVertex;

};

class GeometryManager
{
public:
	GeometryManager()
		: m_VertexBuffer(0)
		, m_ElementBuffer(0)
		, m_VertexBufferTop(0)
		, m_ElementBufferTop(0)
		, m_Indices(0)
		, m_NextID(1)
	{
		glCreateBuffers(1, &m_VertexBuffer);
		glNamedBufferData(m_VertexBuffer, VERTEX_BUFFER_SIZE, nullptr, GL_STATIC_DRAW);

		glCreateBuffers(1, &m_ElementBuffer);
		glNamedBufferData(m_ElementBuffer, ELEMENT_BUFFER_SIZE, nullptr, GL_STATIC_DRAW);

	}

	~GeometryManager()
	{
		// delete buffers
	}

	// TODO Rausfinden was baseVertex und firstIndex sind

	void AddGeometry(const std::string& name, void* vertexData, GLsizeiptr bytes, uint32_t* elementData, uint32_t elementCount)
	{
		assert(m_VertexBufferTop + bytes < VERTEX_BUFFER_SIZE);
		assert(m_ElementBufferTop + elementCount * 4 < ELEMENT_BUFFER_SIZE);

		Geometry& geometry = m_Geometry[m_NextID];
		geometry.elementCount = elementCount;
		geometry.firstIndex = m_Indices;
		geometry.baseVertex = (GLint)m_VertexBufferTop / 4;


		m_Indices += elementCount;

		glNamedBufferSubData(m_VertexBuffer, m_VertexBufferTop, bytes, vertexData);
		m_VertexBufferTop += bytes;

		glNamedBufferSubData(m_ElementBuffer, m_ElementBufferTop, elementCount * sizeof(uint32_t), (void*)elementData);
		m_ElementBufferTop += elementCount * sizeof(uint32_t);

		assert(m_NameToGeoID.find(name) == m_NameToGeoID.end());
		m_NameToGeoID[name] = m_NextID++;
	}

	GLuint GetVertexBufferID()
	{
		return m_VertexBuffer;
	}

	GLuint GetElementBufferID()
	{
		return m_ElementBuffer;
	}

	GeoID GetID(const std::string name)
	{
		auto it = m_NameToGeoID.find(name);
		if(it == m_NameToGeoID.end())
		{
			return 0;
		}

		return it->second;
	}

	Geometry& GetGeometry(GeoID geoID)
	{
		assert(m_Geometry.find(geoID) != m_Geometry.end());
		return m_Geometry[geoID];
	}

private:
	GLuint m_VertexBuffer;
	GLuint m_ElementBuffer;

	GLintptr m_VertexBufferTop;
	GLintptr m_ElementBufferTop;
	GLuint m_Indices;

	GeoID m_NextID;

	std::unordered_map<std::string, GeoID> m_NameToGeoID;
	std::unordered_map<GeoID, Geometry> m_Geometry;

};

struct Renderable
{
	GeoID geoID;
	glm::mat4 modelTransform;
};


struct InstanceData
{
	glm::mat4 modelTransform = glm::mat4(1.f);
};

struct DrawData
{
	GeoID geoID;
	uint32_t instanceCount;
	std::vector<InstanceData> instanceData;
};

struct DrawCommand
{
	GLuint elementCount;
	GLuint instanceCount;
	GLuint firstIndex;
	GLint baseVertex;
	GLuint baseInstance;
};

#define INSTANCE_BUFFER_DATA_SIZE 1024 * 1024 // 1mb

class Renderer
{
public:
	Renderer()
		: m_VertexArray(0)
		, m_InstanceDataBuffer(0)
		, m_InstanceDataBufferTop(0)
	{
		glCreateVertexArrays(1, &m_VertexArray);
		glCreateBuffers(1, &m_InstanceDataBuffer);
		glNamedBufferData(m_InstanceDataBuffer, INSTANCE_BUFFER_DATA_SIZE, nullptr, GL_DYNAMIC_DRAW);


		// Bind buffer to binding point 1
		glVertexArrayVertexBuffer(m_VertexArray, 1, m_InstanceDataBuffer, 0, 64);

		// Enable attribute indices
		glEnableVertexArrayAttrib(m_VertexArray, 1);
		glEnableVertexArrayAttrib(m_VertexArray, 2);
		glEnableVertexArrayAttrib(m_VertexArray, 3);
		glEnableVertexArrayAttrib(m_VertexArray, 4);

		// Associate Binding Index with attribute indices
		glVertexArrayAttribBinding(m_VertexArray, 1, 1);
		glVertexArrayAttribBinding(m_VertexArray, 2, 1);
		glVertexArrayAttribBinding(m_VertexArray, 3, 1);
		glVertexArrayAttribBinding(m_VertexArray, 4, 1);

		// Specify layout of data for attribute indices
		glVertexArrayAttribFormat(m_VertexArray, 1, 4, GL_FLOAT, GL_FALSE, 0);
		glVertexArrayAttribFormat(m_VertexArray, 2, 4, GL_FLOAT, GL_FALSE, 16);
		glVertexArrayAttribFormat(m_VertexArray, 3, 4, GL_FLOAT, GL_FALSE, 32);
		glVertexArrayAttribFormat(m_VertexArray, 4, 4, GL_FLOAT, GL_FALSE, 48);

		// Specify Divisor for Binding Index
		glVertexArrayBindingDivisor(m_VertexArray, 1, 1);

		LOG_INFO("Renderer initialized InstanceDataBuffer")

		glCreateBuffers(1, &m_DrawIndirectBuffer);
		glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_DrawIndirectBuffer);

	}

	~Renderer()
	{

	}

	void SetVertexBuffer(GLuint vertexBufferID)
	{
		LOG_INFO("Set vertex buffer for renderer")
		glEnableVertexArrayAttrib(m_VertexArray, 0);
		glVertexArrayVertexBuffer(m_VertexArray, 0, vertexBufferID, 0, sizeof(float) * 3);
		glVertexArrayAttribFormat(m_VertexArray, 0, 3, GL_FLOAT, GL_FALSE, 0);


	}

	void SetElementBuffer(GLuint elementBufferID)
	{
		LOG_INFO("Set element buffer for renderer")
		glVertexArrayElementBuffer(m_VertexArray, elementBufferID);
	}

	void BeginScene()
	{
	}

	void Submit(const Renderable& renderable)
	{
		// Find DrawData entry for current geoID
		uint32_t drawDataPos = 0;
		while(drawDataPos < m_DrawData.size())
		{
			if(m_DrawData[drawDataPos].geoID == renderable.geoID)
			{
				break;
			}

			drawDataPos++;
		}

		// Create new entry if no DrawData entry exists for current geoID
		if(drawDataPos >= m_DrawData.size())
		{
			DrawData drawData{};
			drawData.geoID = renderable.geoID;
			m_DrawData.push_back(drawData);
		}

		// Update DrawData entry
		DrawData& drawData = m_DrawData[drawDataPos];
		drawData.instanceCount++;
		InstanceData instanceData;
		instanceData.modelTransform = renderable.modelTransform;
		drawData.instanceData.push_back(instanceData);
	}

	void EndScene(GLFWwindow* window)
	{
		std::vector<DrawCommand> drawCommands;
		uint32_t baseInstance = 0;
		m_InstanceDataBufferTop = 0;


		auto context = static_cast<SharedContext*>(glfwGetWindowUserPointer(window));
		GeometryManager* geometryManager = context->geometryManager;

		char* instanceData = (char*)glMapNamedBuffer(m_InstanceDataBuffer, GL_WRITE_ONLY);
		for(auto drawData: m_DrawData)
		{
			Geometry& geometry = geometryManager->GetGeometry(drawData.geoID);
			DrawCommand drawCommand{};

			// unsure about this mapping
			drawCommand.elementCount = geometry.elementCount;
			drawCommand.instanceCount = drawData.instanceCount;
			drawCommand.baseVertex = geometry.baseVertex;
			drawCommand.firstIndex = geometry.firstIndex;
			drawCommand.baseInstance = baseInstance;

			drawCommands.push_back(drawCommand);

			const size_t instanceDataSize = drawData.instanceData.size() * sizeof(InstanceData);
			assert(m_InstanceDataBufferTop + instanceDataSize < INSTANCE_BUFFER_DATA_SIZE);
			memcpy(instanceData + m_InstanceDataBufferTop,
				drawData.instanceData.data(), 
				instanceDataSize
			);
			m_InstanceDataBufferTop += instanceDataSize;

			baseInstance += drawData.instanceCount;
		}
		glUnmapNamedBuffer(m_InstanceDataBuffer);



		glBindVertexArray(m_VertexArray);
		

		glMultiDrawElementsIndirect(
			GL_TRIANGLES, 
			GL_UNSIGNED_INT, 
			(const void*)drawCommands.data(), 
			drawCommands.size(),
			sizeof(DrawCommand)
		);

		glBindVertexArray(0);

		m_DrawData.clear();
	}

private:
	std::vector<DrawData> m_DrawData;
	GLuint m_VertexArray;
	GLuint m_InstanceDataBuffer;
	GLuint m_DrawIndirectBuffer;


	GLintptr m_InstanceDataBufferTop;
};


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

	glDebugMessageCallback(DebugCallback, 0);
	
	SharedContext sharedContext{};
	glfwSetWindowUserPointer(window, &sharedContext);
	
	int w, h;
	glfwGetWindowSize(window, &w, &h);
	Camera camera((float)w/h);
	sharedContext.worldCamera = &camera;

	GeometryManager geometryManager;
	sharedContext.geometryManager = &geometryManager;


	const char* vertexShaderText =
		"#version 460\n"
		"layout(location = 0) in vec3 a_Position;\n"
		"layout(location = 1) in mat4 a_ModelMat;\n"
		"uniform mat4 u_ModelMat;\n"
		"uniform mat4 u_ViewMat;\n"
		"uniform mat4 u_PerspectiveMat;\n"
		"void main()\n"
		"{\n"
		"gl_Position = u_PerspectiveMat * u_ViewMat * a_ModelMat * vec4(a_Position, 1.0);\n"
		"}";


	// TODO ModelMat wieder einfügen


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

	float Geo1[] = {
		-1.f, -1.f, 1.f,
		0.f, -1.f, 1.f,
		0.f, 0.f, 1.f
	};

	uint32_t Geo1Indices[] = { 0,1,2 };

	float trianglePositions[] = {
		-0.5f, -0.5f, 1.0f,
		 0.5f, -0.5f, 1.0f,
		 0.5f,  0.5f, 1.0f,
		-0.5f,  0.5f, 1.0f
	};

	uint32_t triangleIndices[] = {
		0,1,2,
		0,2,3
	};


	geometryManager.AddGeometry("triangle", Geo1, sizeof(Geo1), Geo1Indices, sizeof(Geo1Indices) / sizeof(GLuint));
	geometryManager.AddGeometry("square", trianglePositions, sizeof(trianglePositions), triangleIndices, sizeof(triangleIndices) / sizeof(GLuint));


	/*
	 
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
	glBindVertexArray(VAOID);

	 */


	GLint modelMatLoc = glGetUniformLocation(program, "u_ModelMat");
	GLint viewMatLoc = glGetUniformLocation(program, "u_ViewMat");
	GLint perspectiveMatLoc = glGetUniformLocation(program, "u_PerspectiveMat");

	glm::mat4 identity = glm::mat4(1.f);
	
	
	glUseProgram(program);
	//glUniformMatrix4fv(modelMatLoc, 1, GL_FALSE, glm::value_ptr(identity));
	glUniformMatrix4fv(viewMatLoc, 1, GL_FALSE, glm::value_ptr(camera.GetViewMatrix()));
	glUniformMatrix4fv(perspectiveMatLoc, 1, GL_FALSE, glm::value_ptr(camera.GetPerspectiveMatrix()));
	glUseProgram(0);



	glm::mat4 mat = { 0.f, 1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f, 9.f, 10.f, 11.f, 12.f, 13.f, 14.f, 15.f};


	Renderable a;
	a.geoID = sharedContext.geometryManager->GetID("triangle");
	a.modelTransform = glm::mat4(1.f);
	//a.modelTransform = mat;
	

	Renderable c;
	c.geoID = sharedContext.geometryManager->GetID("triangle");
	c.modelTransform = glm::translate(glm::mat4(1.f), { 2.f, 0, 2.f });

	Renderable b;
	b.geoID = sharedContext.geometryManager->GetID("square");
	b.modelTransform = glm::mat4(1.f);


	Renderer renderer;
	renderer.SetVertexBuffer(sharedContext.geometryManager->GetVertexBufferID());
	renderer.SetElementBuffer(sharedContext.geometryManager->GetElementBufferID());

	glClearColor(0.16f, 0.2f, 0.35f, 1.f);
	while (!glfwWindowShouldClose(window))
	{
		static double lastTime = glfwGetTime();

		const double timeNow = glfwGetTime();
		const double dt = timeNow - lastTime;
		lastTime = timeNow;

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		camera.Update(dt);

		glUseProgram(program);
		glUniformMatrix4fv(viewMatLoc, 1, GL_FALSE, glm::value_ptr(camera.GetViewMatrix()));
		glUniformMatrix4fv(perspectiveMatLoc, 1, GL_FALSE, glm::value_ptr(camera.GetPerspectiveMatrix()));


		renderer.Submit(a);
		renderer.Submit(a);
		renderer.Submit(b);
		//renderer.Submit(c);




		renderer.EndScene(window);
		glUseProgram(0);


		//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();



	return 0;
}