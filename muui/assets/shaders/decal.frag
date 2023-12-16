#version 300 es

precision highp float;

uniform sampler2D baseColorTexture;

in vec2 vs_texCoord;
in vec4 vs_color;
out vec4 fragColor;

void main(void)
{
    vec4 baseColor = texture(baseColorTexture, vs_texCoord);
    vec4 color = baseColor * vs_color;
    color.rgb *= color.a; // premultiply alpha
    fragColor = color;
}
