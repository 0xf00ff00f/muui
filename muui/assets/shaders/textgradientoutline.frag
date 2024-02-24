precision highp float;

uniform sampler2D baseColorTexture;

in vec2 vs_texCoord;
in vec2 vs_position;
out vec4 fragColor;

#include "lineargradient.inc.frag"

void main(void)
{
    vec4 color = gradientColor(vs_position);
    float alpha = texture(baseColorTexture, vs_texCoord).a;
    color.a *= alpha;
    color.rgb *= color.a; // premultiply alpha
    fragColor = color;
}
