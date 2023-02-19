#version 460
layout(points) in;
layout(triangle_strip, max_vertices=4) out;
in mat4 gsModelMat[];
uniform mat4 u_ViewMat;
uniform mat4 u_PerspectiveMat;
void main()
{
vec4 offset = vec4(-0.25, 0.25, 0.0, 0.0); // oben links
vec4 vertexPos = offset + gl_in[0].gl_Position;
gl_Position = u_PerspectiveMat * u_ViewMat * gsModelMat[0] * vertexPos;
EmitVertex();

offset = vec4(0.25, 0.25, 0.0, 0.0); // oben rechts
vertexPos = offset + gl_in[0].gl_Position;
gl_Position = u_PerspectiveMat * u_ViewMat * gsModelMat[0] * vertexPos;
EmitVertex();

offset = vec4(-0.25, -0.25, 0.0, 0.0); // unten links
vertexPos = offset + gl_in[0].gl_Position;
gl_Position = u_PerspectiveMat * u_ViewMat * gsModelMat[0] * vertexPos;
EmitVertex();

offset = vec4(0.25, -0.25, 0.0, 0.0); // unten rechts
vertexPos = offset + gl_in[0].gl_Position;
gl_Position = u_PerspectiveMat * u_ViewMat * gsModelMat[0] * vertexPos;
EmitVertex();
EmitVertex();
EndPrimitive();
}