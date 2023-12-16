#version 300 es

precision highp float;

in vec4 vs_color;
out vec4 fragColor;

void main(void)
{
    vec4 color = vs_color;
    color.rgb *= color.a; // premultiply alpha
    fragColor = color;
}
