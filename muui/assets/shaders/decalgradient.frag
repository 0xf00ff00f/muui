precision highp float;

uniform sampler2D baseColorTexture;

in vec2 vs_position;
in vec2 vs_texCoord;
out vec4 fragColor;

#include "lineargradient.inc.frag"

void main(void)
{
    vec4 baseColor = texture(baseColorTexture, vs_texCoord);
    vec4 color = baseColor * gradientColor(vs_position);
    color.rgb *= color.a; // premultiply alpha
    fragColor = color;
}
