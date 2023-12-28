precision highp float;

uniform sampler2D baseColorTexture;

in vec2 vs_texCoord;
out vec4 fragColor;

void main(void)
{
    fragColor = texture(baseColorTexture, vs_texCoord);
}
