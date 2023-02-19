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


#define VERTEX_BUFFER_SIZE 1024 * 1024 * 16 //16mb
#define ELEMENT_BUFFER_SIZE 1024 * 1024 * 16 //16mb

struct Geometry
{
	GLsizei elementCount;
	GLuint firstIndex;
	GLint baseVertex;

};

#define SIZE_OF_VERTEX 12 // in bytes

typedef uint32_t GeoID;
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
		geometry.baseVertex = (GLint)m_VertexBufferTop / SIZE_OF_VERTEX;


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

	size_t GetGeoCount()
	{
		return m_NextID - 1;
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

#define INSTANCE_BUFFER_DATA_SIZE 1024 * 1024 * 128 // 64mb

class Renderer
{
private:
	struct InstanceData
	{
		glm::mat4 modelTransform = glm::mat4(1.f);
	};

	struct DrawData
	{
		GeoID geoID;
		uint32_t instanceCount;
		std::vector<InstanceData> instanceData{};
	};

	struct DrawCommand
	{
		GLuint elementCount;
		GLuint instanceCount;
		GLuint firstIndex;
		GLint baseVertex;
		GLuint baseInstance;
	};


public:
	Renderer()
		: m_VertexArray(0)
		, m_InstanceDataBuffer{0,0}
		, m_PersistentInstanceDataBuffer(0)
		, m_DrawIndirectBuffer(0)
		, m_GeoManagerGeoCount(0)
		, m_InstanceDataBufferTop(0)
	{
		glCreateVertexArrays(1, &m_VertexArray);

		glCreateBuffers(2, m_InstanceDataBuffer);

		glNamedBufferData(m_InstanceDataBuffer[0], INSTANCE_BUFFER_DATA_SIZE, nullptr, GL_STATIC_DRAW);
		glNamedBufferData(m_InstanceDataBuffer[1], INSTANCE_BUFFER_DATA_SIZE, nullptr, GL_STATIC_DRAW);


		m_SyncObject = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

		GLbitfield flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
		glCreateBuffers(1, &m_PersistentInstanceDataBuffer);
		glNamedBufferStorage(m_PersistentInstanceDataBuffer, INSTANCE_BUFFER_DATA_SIZE, nullptr, flags);

		m_InstanceDataPtr = (char*)glMapNamedBufferRange(m_PersistentInstanceDataBuffer, 0, INSTANCE_BUFFER_DATA_SIZE, flags);

		// Bind buffer to binding point 1
		//glVertexArrayVertexBuffer(m_VertexArray, 1, m_InstanceDataBuffer[0], 0, 64);
		glVertexArrayVertexBuffer(m_VertexArray, 1, m_PersistentInstanceDataBuffer, 0, 64);

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

	void SetGeoCount(size_t count)
	{
		m_GeoManagerGeoCount = count;

		glCreateBuffers(1, &m_DrawIndirectBuffer);
		glNamedBufferData(m_DrawIndirectBuffer, m_GeoManagerGeoCount * sizeof(DrawCommand), nullptr, GL_STREAM_DRAW);
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
			drawData.instanceData.reserve(30000);
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



		// Wait buffer
		GLenum waitReturn = GL_UNSIGNALED;
		while (waitReturn != GL_ALREADY_SIGNALED && waitReturn != GL_CONDITION_SATISFIED)
		{
			waitReturn = glClientWaitSync(m_SyncObject, GL_SYNC_FLUSH_COMMANDS_BIT, 1);
		}
		glDeleteSync(m_SyncObject);


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


			memcpy(m_InstanceDataPtr + m_InstanceDataBufferTop,
				drawData.instanceData.data(), 
				instanceDataSize
			);
			m_InstanceDataBufferTop += instanceDataSize;
			LOG_INFO("Copied %d bytes into instance data buffer", instanceDataSize)


			baseInstance += drawData.instanceCount;
		}
		// Lock buffer
		m_SyncObject = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

		glNamedBufferSubData(m_DrawIndirectBuffer, 0, drawCommands.size() * sizeof(DrawCommand), drawCommands.data());

		glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_DrawIndirectBuffer);
		glBindVertexArray(m_VertexArray);

		glMultiDrawElementsIndirect(
			GL_TRIANGLES, 
			GL_UNSIGNED_INT, 
			nullptr, 
			drawCommands.size(),
			0
		);

		glBindVertexArray(0);
		glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);

		m_DrawData.clear();
	}

	void DrawIndexed(GLFWwindow* window, const Renderable& renderable)
	{

		auto context = static_cast<SharedContext*>(glfwGetWindowUserPointer(window));
		GeometryManager* geometryManager = context->geometryManager;
		Geometry& geometry = geometryManager->GetGeometry(renderable.geoID);


		GLenum waitReturn = GL_UNSIGNALED;
		while (waitReturn != GL_ALREADY_SIGNALED && waitReturn != GL_CONDITION_SATISFIED)
		{
			waitReturn = glClientWaitSync(m_SyncObject, GL_SYNC_FLUSH_COMMANDS_BIT, 1);
		}
		glDeleteSync(m_SyncObject);

		memcpy(m_InstanceDataPtr + m_InstanceDataBufferTop, glm::value_ptr(renderable.modelTransform), sizeof(InstanceData));
		m_SyncObject = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
		glPointSize(10.f);
		glBindVertexArray(m_VertexArray);
		
		glDrawElementsBaseVertex(
			GL_POINTS, // todo
			geometry.elementCount,
			GL_UNSIGNED_INT,
			(const void*) (4 * geometry.firstIndex),
			geometry.baseVertex
		);

		glBindVertexArray(0);
	}

private:
	std::vector<DrawData> m_DrawData;
	GLuint m_VertexArray;
	GLuint m_InstanceDataBuffer[2];
	GLuint m_PersistentInstanceDataBuffer;
	GLuint m_DrawIndirectBuffer;

	GLsync m_SyncObject;

	char* m_InstanceDataPtr;

	size_t m_GeoManagerGeoCount;

	GLintptr m_InstanceDataBufferTop;
};

class EntityManager
{
public:

private:
};

struct Sphere
{
	Sphere(const int prec)
	{
		Init(prec);
	}

	void Init(const int prec)
	{
		const int numVertices = (prec + 1) * (prec + 1);
		const int numIndices = prec * prec * 6;

		// Default initialize i indices (NOT PERFORMANT AT ALL)
		for (int i = 0; i < numIndices; i++)
		{
			m_Indices.push_back(0);
		}

		for (int i = 0; i <= prec; i++)
		{
			for (int j = 0; j <= prec; j++)
			{
				float y = (float)cos(glm::radians(180.0f - i * 180.0f / prec));
				// not sure what the minus infront of cos is used for
				// The minus infront of the x or z value influences the winding!
				float x = -(float)cos(glm::radians(j * 360.0f / prec)) * (float)abs(cos(asin(y)));
				float z = (float)sin(glm::radians(j * 360.0f / prec)) * (float)abs(cos(asin(y)));


				m_Vertices.push_back(x);
				m_Vertices.push_back(y);
				m_Vertices.push_back(z);
				//Vertex& vertex = m_Vertices[i * (prec + 1) + j];
				//vertex.position = glm::vec3(x, y, z);
				//vertex.color = glm::vec4(0.2f, 0.3f, 0.8f, 1.f);
				//// No color value specified
				//vertex.texCoord = glm::vec2(((float)j / prec), ((float)i / prec));
				//vertex.normal = glm::vec3(x, y, z);
			}
		}
		for (int i = 0; i < prec; i++)
		{
			for (int j = 0; j < prec; j++)
			{
				m_Indices[6 * (i * prec + j) + 0] = i * (prec + 1) + j;
				m_Indices[6 * (i * prec + j) + 1] = i * (prec + 1) + j + 1;
				m_Indices[6 * (i * prec + j) + 2] = (i + 1) * (prec + 1) + j;
				m_Indices[6 * (i * prec + j) + 3] = i * (prec + 1) + j + 1;
				m_Indices[6 * (i * prec + j) + 4] = (i + 1) * (prec + 1) + j + 1;
				m_Indices[6 * (i * prec + j) + 5] = (i + 1) * (prec + 1) + j;
			}
		}
	}


	std::vector<float> m_Vertices;
	std::vector<uint32_t> m_Indices;
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
	glEnable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	//glEnable(GL_CULL_FACE);


	GLint noExtensions = 0;
	glGetIntegerv(GL_NUM_EXTENSIONS, &noExtensions);

	std::set<std::string> oglExtensions;
	for(int i = 0; i < noExtensions; i++)
	{
		oglExtensions.insert((const char*)glGetStringi(GL_EXTENSIONS, i));
	}
	bool isSupported = oglExtensions.find("GL_ARB_bindless_texture") != oglExtensions.end();
	
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
		"uniform mat4 u_ViewMat;\n"
		"uniform mat4 u_PerspectiveMat;\n"
		"out mat4 gsModelMat;\n"
		"void main()\n"
		"{\n"
		"gsModelMat = a_ModelMat;\n"
		"gl_Position =  vec4(a_Position, 1.0);\n"
		"}";

	//u_PerspectiveMat * u_ViewMat * a_ModelMat *


	// TODO ModelMat wieder einfügen


	const char* fragmentShaderText =
		"#version 460\n"
		"out vec4 color;"
		"\n"
		"void main()\n"
		"{\n"
		"color = vec4(1,1,0, 1.0);\n"
		"}\n";



	const char* geoShaderText =
		"#version 460\n"
		"layout(points) in;\n"
		"layout(triangle_strip, max_vertices=4) out;\n"
		"in mat4 gsModelMat[];\n"
		"uniform mat4 u_ViewMat;\n"
		"uniform mat4 u_PerspectiveMat;\n"
		"void main()\n"
		"{\n"
		"vec4 offset = vec4(-0.25, 0.25, 0.0, 0.0);\n" // oben links
		"vec4 vertexPos = offset + gl_in[0].gl_Position;\n"
		"gl_Position = u_PerspectiveMat * u_ViewMat * gsModelMat[0] * vertexPos;\n"
		"EmitVertex();\n"
		"\n"
		"offset = vec4(0.25, 0.25, 0.0, 0.0);\n" // oben rechts
		"vertexPos = offset + gl_in[0].gl_Position;\n"
		"gl_Position = u_PerspectiveMat * u_ViewMat * gsModelMat[0] * vertexPos;\n"
		"EmitVertex();\n"
		"\n"
		"offset = vec4(-0.25, -0.25, 0.0, 0.0);\n" // unten links
		"vertexPos = offset + gl_in[0].gl_Position;\n"
		"gl_Position = u_PerspectiveMat * u_ViewMat * gsModelMat[0] * vertexPos;\n"
		"EmitVertex();\n"
		"\n"
		"offset = vec4(0.25, -0.25, 0.0, 0.0);\n" // unten rechts
		"vertexPos = offset + gl_in[0].gl_Position;\n"
		"gl_Position = u_PerspectiveMat * u_ViewMat * gsModelMat[0] * vertexPos;\n"
		"EmitVertex();\n"
		"EndPrimitive();\n"
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

	GLuint geoShader = glCreateShader(GL_GEOMETRY_SHADER);
	glShaderSource(geoShader, 1, &geoShaderText, NULL);
	glCompileShader(geoShader);

	GLuint geoProgram = glCreateProgram();
	glAttachShader(geoProgram, vertexShader);
	glAttachShader(geoProgram, geoShader);
	glAttachShader(geoProgram, fragmentShader);
	glLinkProgram(geoProgram);


	float Geo1[] = {
		0.f, 0.f, 0.f,
		1.f, 0.f, 0.f,
		1.f, 1.f, 0.f
	};

	uint32_t Geo1Indices[] = { 0,1,2 };

	float trianglePositions[] = {
		-0.5f, -0.5f, 0.0f,
		 0.5f, -0.5f, 0.0f,
		 0.5f,  0.5f, 0.0f,
		-0.5f,  0.5f, 0.0f
	};

	uint32_t triangleIndices[] = {
		0,1,2,
		0,2,3
	};


	float fuenfeck[] = {
		 0.0f,  0.5f, 0.0f,
		-0.5f,  0.3f, 0.0f,
		-0.3f, -0.4f, 0.0f,
		 0.3f, -0.4f, 0.0f,
		 0.5f,  0.3f, 0.0f,
	};

	uint32_t fuenfeckIndices[] = {
		0,1,2,
		0,2,3,
		0,3,4
	};

	float simpleCube[] = { 
		 0.5f,  0.5f,  0.5f, //or   0
		-0.5f,  0.5f,  0.5f, //ol	1
		-0.5f, -0.5f,  0.5f, //ul	2
		 0.5f, -0.5f,  0.5f, //ur	3
		 0.5f,  0.5f, -0.5f, //or	4
		-0.5f,  0.5f, -0.5f, //ol	5
		-0.5f, -0.5f, -0.5f, //ul	6
		 0.5f, -0.5f, -0.5f, //ur	7
	};

	uint32_t simpleCubeIndices[] = {
		0,1,2,0,2,3, // vorne
		4,0,3,4,3,7, // rechts
		5,4,7,5,7,6, // hinten
		1,5,6,1,6,2, // links
		4,5,1,4,1,0, // oben
		3,2,6,3,6,7

	};

	Sphere sphere30(10);

	geometryManager.AddGeometry("square", trianglePositions, sizeof(trianglePositions), triangleIndices, sizeof(triangleIndices) / sizeof(GLuint));
	geometryManager.AddGeometry("triangle", Geo1, sizeof(Geo1), Geo1Indices, sizeof(Geo1Indices) / sizeof(GLuint));
	geometryManager.AddGeometry("fuenfeck", fuenfeck, sizeof(fuenfeck), fuenfeckIndices, sizeof(fuenfeckIndices) / sizeof(GLuint));
	geometryManager.AddGeometry("simpleCube", simpleCube, sizeof(simpleCube), simpleCubeIndices, sizeof(simpleCubeIndices) / sizeof(GLuint));
	geometryManager.AddGeometry("sphere30", sphere30.m_Vertices.data(), sphere30.m_Vertices.size() * sizeof(float), sphere30.m_Indices.data(), sphere30.m_Vertices.size() * sizeof(uint32_t));

	GLint viewMatLoc = glGetUniformLocation(geoProgram, "u_ViewMat");
	GLint perspectiveMatLoc = glGetUniformLocation(geoProgram, "u_PerspectiveMat");

	glm::mat4 identity = glm::mat4(1.f);
	
	
	glUseProgram(geoProgram);
	//glUniformMatrix4fv(modelMatLoc, 1, GL_FALSE, glm::value_ptr(identity));
	glUniformMatrix4fv(viewMatLoc, 1, GL_FALSE, glm::value_ptr(camera.GetViewMatrix()));
	glUniformMatrix4fv(perspectiveMatLoc, 1, GL_FALSE, glm::value_ptr(camera.GetPerspectiveMatrix()));
	glUseProgram(0);



	glm::mat4 mat = { 0.f, 1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f, 9.f, 10.f, 11.f, 12.f, 13.f, 14.f, 15.f};


	Renderable a;
	a.geoID = sharedContext.geometryManager->GetID("triangle");
	a.modelTransform = glm::mat4(1.f);
	//a.modelTransform = mat;

	Renderable plane;
	plane.geoID = sharedContext.geometryManager->GetID("square");
	plane.modelTransform = glm::translate(glm::mat4(1.f), { 0.f, 1.5f, 0.f });

	Renderable sphere;
	sphere.geoID = sharedContext.geometryManager->GetID("sphere30");
	sphere.modelTransform = glm::translate(glm::mat4(1.f), { 2.f, 0, 2.f });

	Renderable cube;
	cube.geoID = sharedContext.geometryManager->GetID("simpleCube");
	cube.modelTransform = glm::mat4(1.f);

	Renderer renderer;
	renderer.SetVertexBuffer(sharedContext.geometryManager->GetVertexBufferID());
	renderer.SetElementBuffer(sharedContext.geometryManager->GetElementBufferID());
	renderer.SetGeoCount(sharedContext.geometryManager->GetGeoCount());

	constexpr uint32_t gridsize = 22;
	constexpr uint32_t distance = 80;
	

	std::vector<Renderable> renderables;
	renderables.reserve(gridsize* gridsize* gridsize);


	GeoID geoID = sharedContext.geometryManager->GetID("sphere30");
	for(int x = 0; x < gridsize; x++)
	{
		for(int y = 0; y < gridsize; y++)
		{
			for(int z = 0; z < gridsize; z++)
			{
				Renderable renderable{};
				renderable.geoID = geoID;
				renderable.modelTransform = glm::translate(
					glm::scale(glm::mat4(1.f), {0.1f, 0.1f, 0.1f}),
					{x * distance, y * distance, z * distance }
				);
				renderables.push_back(renderable);
			}
		}
	}

	glClearColor(0.16f, 0.2f, 0.35f, 1.f);
	while (!glfwWindowShouldClose(window))
	{
		static double lastTime = glfwGetTime();

		const double timeNow = glfwGetTime();
		const double dt = timeNow - lastTime;
		lastTime = timeNow;

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		

		camera.Update(dt);

		glUseProgram(geoProgram);
		glUniformMatrix4fv(viewMatLoc, 1, GL_FALSE, glm::value_ptr(camera.GetViewMatrix()));
		glUniformMatrix4fv(perspectiveMatLoc, 1, GL_FALSE, glm::value_ptr(camera.GetPerspectiveMatrix()));

		
		// Indexed Drawing

		//for(auto& r  : renderables)
		//{
		//	renderer.DrawIndexed(window, r);
		//}

		// MDI
		//for(auto& r  : renderables)
		//{
		//	renderer.Submit(r);
		//}
		//
		//renderer.EndScene(window);

		renderer.DrawIndexed(window, plane);


		glUseProgram(0);


		glfwSwapBuffers(window);
		glfwPollEvents();

	}

	glfwTerminate();



	return 0;
}