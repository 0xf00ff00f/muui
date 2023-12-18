layout(location=0) in vec2 position;
layout(location=1) in vec2 texCoord;
layout(location=2) in vec4 parameters;

uniform mat4 mvp;

out vec2 vs_texCoord;
out vec2 vs_viewportSize;
out float vs_spacing;
out float vs_transitionFactor;

void main(void)
{
    vs_texCoord = texCoord;
    vs_viewportSize = vec2(parameters.x, parameters.y);
    vs_spacing = parameters.z;
    vs_transitionFactor = parameters.w;
    gl_Position = mvp * vec4(position, 0.0, 1.0);
}
