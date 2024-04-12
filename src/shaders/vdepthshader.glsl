#version 330 core

// layout (location = 0) in vec3 aPos;
/*
in vec3 aPos;

out VS_OUT {
    float depth;
} vs_out;

uniform mat4 Projection;
uniform mat4 View;
uniform mat4 Model;

void main()
{
    vs_out.depth = (Projection * View * Model * vec4(aPos, 1.0)).z;
    gl_Position = Projection * View * Model * vec4(aPos, 1.0);
}
*/
layout (location = 0) in vec3 aPos;

out DEPTH_OUT {
    float depth;
} depth_out;

uniform mat4 Projection;
uniform mat4 View;
uniform mat4 Model;

void main()
{
    depth_out.depth = (Projection * View * Model * vec4(aPos, 1.0)).z;
    gl_Position = Projection * View * Model * vec4(aPos, 1.0);
}
