#version 330

out vec4 FragColor;

in DEPTH_OUT {
    float depth;
} fs_in;

uniform float heightScale;

void main()
{
    // FragColor = vec4(1 - fs_in.depth / heightScale, 0, 0, 1.0);
    FragColor = vec4(1 - fs_in.depth, 0, 0, 1.0);
}
