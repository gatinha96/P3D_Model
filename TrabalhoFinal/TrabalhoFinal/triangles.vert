#version 440 core

layout( location = 0 ) in vec2 vPosition;
layout( location = 1 ) in vec3 vColors;

uniform mat4 Model;
uniform mat4 View;
uniform mat4 ModelView;		// View * Model
uniform mat4 Projection;
uniform mat3 NormalMatrix;

in vec3 vPosition;			// Coordenadas locais do vértice
in vec3 vNormal;			// Normal do vértice

out vec3 vPositionEyeSpace;
out vec3 vNormalEyeSpace;
out vec3 textureVector;

out vec3 color;

void main()
{
   // Posição do vértice em coordenadas do olho.
	vPositionEyeSpace = (ModelView * vec4(vPosition, 1.0)).xyz;

	// Transformar a normal do vértice.
	vNormalEyeSpace = normalize(NormalMatrix * vNormal);

	// Coordenada de textura para o CubeMap
	textureVector = vPosition;

	// Posição final do vértice (em coordenadas de clip)
	gl_Position = Projection * ModelView * vec4(vPosition, 1.0f);
	color = vColors;
}
