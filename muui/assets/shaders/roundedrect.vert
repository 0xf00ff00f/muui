layout(location=0) in vec2 position;
layout(location=1) in vec2 texCoord;
layout(location=2) in vec4 fgColor;
layout(location=3) in vec4 bgColor;

uniform mat4 mvp;

out vec2 vs_texCoord;
out vec4 vs_color;
out vec2 vs_size;
out float vs_cornerRadius;

void main(void)
{
    vs_texCoord = texCoord;
    vs_color = fgColor;
    vs_size = bgColor.xy;
    vs_cornerRadius = bgColor.z;
    gl_Position = mvp * vec4(position, 0.0, 1.0);
}
