#version 400

uniform vec4 u_lightPos;
uniform vec4 u_colour;

// Fragment Shader Inputs
in VertexData
{
	vec3 normal;
	vec3 texCoord;
	vec4 colour;
	vec3 posEye;
} vIn;

layout(location = 0) out vec4 FragColor; // corresponds to the FBO attachments

void main()
{
	FragColor = vec4(vIn.normal * 0.5 + 0.5, 1.0f);
	
	vec3 L = normalize(u_lightPos.xyz - vIn.posEye);
	vec3 N = normalize(vIn.normal);

	float diffuse = max(0.0, dot(N, L));
	FragColor = vec4(vec3(0.5, 0.5, 0.5) * (diffuse * 0.8f) + u_colour.rgb, 1.0f);
}