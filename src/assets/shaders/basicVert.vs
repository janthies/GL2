		#version 460
		layout(location = 0) in vec3 a_Position;
		layout(location = 1) in mat4 a_ModelMat;

		uniform mat4 u_ViewMat;
		uniform mat4 u_PerspectiveMat;
		out mat4 gsModelMat;

		void main()
		{
		gsModelMat = a_ModelMat;
		//gl_Position = u_PerspectiveMat * u_ViewMat * a_ModelMat * vec4(a_Position, 1.0);
		gl_Position = vec4(a_Position, 1.0);
		}