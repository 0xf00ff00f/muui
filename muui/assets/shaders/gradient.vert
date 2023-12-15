#version 300 es

layout(location=0) in vec2 position;
layout(location=2) in vec4 gradientFromTo;

uniform mat4 mvp;

out vec2 vs_position;
out vec2 vs_gradientFrom;
out vec2 vs_gradientTo;

void main(void)
{
    vs_position = position;
    vs_gradientFrom = gradientFromTo.xy;
    vs_gradientTo = gradientFromTo.zw;
    gl_Position = mvp * vec4(position, 0.0, 1.0);
}
