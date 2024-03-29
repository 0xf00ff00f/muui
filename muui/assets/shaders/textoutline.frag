precision highp float;

uniform sampler2D baseColorTexture;

in vec2 vs_texCoord;
in vec4 vs_color;

out vec4 fragColor;

void main(void)
{
    float alpha = texture(baseColorTexture, vs_texCoord).a;
    vec4 color = vs_color;
    color.a *= alpha;
    color.rgb *= color.a; // premultiply alpha
    fragColor = color;
}
