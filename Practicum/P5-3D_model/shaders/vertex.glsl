#version 430

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;
layout (location = 2) in vec2 VertexTexCoord;

struct LightInfo {
  vec4 Position;  // Light position in eye coords.
  vec3 Intensity; // A,D,S intensity
};
uniform LightInfo Light;

out vec3 LightDir;
out vec2 TexCoord;
out vec3 ViewDir;
out vec3 Normal;

uniform mat4 ModelViewMatrix;
uniform mat3 NormalMatrix;
uniform mat4 ProjectionMatrix;
uniform mat4 MVP;

void main()
{
    vec3 pos = vec3( ModelViewMatrix * vec4(VertexPosition,1.0) );
    Normal = normalize( NormalMatrix * VertexNormal );
    LightDir = normalize( Light.Position.xyz - pos );
    ViewDir = normalize( -pos );

    TexCoord = VertexTexCoord;

    gl_Position =  vec4(VertexPosition,1.0) * transpose(MVP);
}
