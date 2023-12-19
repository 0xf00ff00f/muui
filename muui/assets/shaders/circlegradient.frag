#extension GL_OES_standard_derivatives : enable

precision highp float;

in vec2 vs_position;
in vec2 vs_texCoord;
out vec4 fragColor;

#include "lineargradient.inc.frag"

void main(void)
{
    const float Radius = 0.5;
    float dist = distance(vs_texCoord, vec2(0.5, 0.5));
    float feather = fwidth(dist);
    float alpha = smoothstep(Radius, Radius - feather, dist);
    vec4 color = gradientColor(vs_position);
    color.a *= alpha;
    color.rgb *= color.a; // premultiply alpha
    fragColor = color;
}
