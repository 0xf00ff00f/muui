layout(location=0) in vec2 position;
layout(location=1) in vec2 texCoord;
layout(location=2) in vec4 parameters;

uniform mat4 mvp;

out vec2 vs_texCoord;
out float vs_spacing;
out float vs_slope;
out float vs_progress;

void main(void)
{
    vs_texCoord = texCoord;
    vs_spacing = parameters.x;
    vs_slope = parameters.y;
    vs_progress = parameters.z;
    gl_Position = mvp * vec4(position, 0.0, 1.0);
}
