#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

out VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
} vs_out;

uniform mat4 Projection;
uniform mat4 View;
uniform mat4 Model;

uniform vec3 LightPos;
uniform vec3 ViewPos;

void main()
{
    // vs_out.FragPos = vec3(View * Model * vec4(aPos, 1.0));
    vs_out.FragPos = vec3(Model * vec4(aPos, 1.0));
    vs_out.TexCoords = aTexCoords;

    vec3 T = normalize(mat3(Model) * aTangent);
    vec3 B = normalize(mat3(Model) * aBitangent);
    vec3 N = normalize(mat3(Model) * aNormal);
    mat3 TBN = transpose(mat3(T, B, N));

    vs_out.TangentLightPos = TBN * LightPos;
    vs_out.TangentViewPos  = TBN * ViewPos;
    vs_out.TangentFragPos  = TBN * vs_out.FragPos;

    gl_Position = Projection * View * Model * vec4(aPos, 1.0);
}

/*

#version 330

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

out VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
} vs_out;

uniform mat4 Projection;
uniform mat4 View;
uniform mat4 Model;

uniform vec3 LightPos;
uniform vec3 ViewPos;

void main()
{
    gl_Position      = Projection * View * Model * vec4(aPos, 1.0);
    vs_out.FragPos   = vec3(Model * vec4(aPos, 1.0));
    vs_out.TexCoords = aTexCoords;

    vec3 T   = normalize(mat3(Model) * aTangent);
    vec3 B   = normalize(mat3(Model) * aBitangent);
    vec3 N   = normalize(mat3(Model) * aNormal);
    mat3 TBN = transpose(mat3(T, B, N));

    vs_out.TangentLightPos = TBN * LightPos;
    vs_out.TangentViewPos  = TBN * ViewPos;
    vs_out.TangentFragPos  = TBN * vs_out.FragPos;
}

*/
/*
layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 TexCoord;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

out vec3 fragPos;
out vec3 fViewPos;
out vec2 fTexCoord;
out vec3 fTangentViewPos;
out vec3 fTangentFragPos;

uniform float Time; // in milliseconds
uniform mat4 Model, View, Projection;
uniform vec3 ViewPos;

void main()
{
    vec4 v = vec4(vPosition, 1.0);
    gl_Position = Projection * View * Model * v;

    fragPos = vec3(Model * v);
    fViewPos = vec3(0, 0, 0); // Z might be wrong
    fTexCoord = TexCoord;

    // Paralax mapping
    vec3 T   = normalize(mat3(Model) * aTangent);
    vec3 B   = normalize(mat3(Model) * aBitangent);
    vec3 N   = normalize(mat3(Model) * vNormal);
    mat3 TBN = transpose(mat3(T, B, N));

    fTangentViewPos = TBN * ViewPos; // Might be wrong
    fTangentFragPos = TBN * fragPos;
}

*/
