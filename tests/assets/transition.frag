precision highp float;

uniform sampler2D baseColorTexture;

in vec2 vs_texCoord;
in vec2 vs_viewportSize;
in float vs_spacing;
in float vs_transitionFactor;

out vec4 fragColor;

const float Slope = 3.0;
const float Spread = 0.25;

void main(void)
{
    vec2 pos = vs_texCoord * vs_viewportSize;

    float totalWidth = vs_viewportSize.x + (vs_viewportSize.y / Slope);
    float tileCount = ceil(totalWidth / vs_spacing);
    float baseX = pos.x + (vs_viewportSize.y - pos.y) / Slope;
    float tile = floor(baseX / vs_spacing);

    float t0 = (tile / (tileCount - 1.0)) * (1.0 - Spread);
    float t1 = t0 + Spread;
    float t = 1.0 - vs_transitionFactor;
    t = clamp((t - t0) / (t1 - t0), 0.0, 1.0);
    t *= t;

    float direction = 2.0 * mod(tile, 2.0) - 1.0; // -1 or 1
    vec2 v = vec2(vs_viewportSize.y / Slope, vs_viewportSize.y);
    vec2 samplePos = pos + direction * t * v;
    vec2 texCoord = samplePos / vs_viewportSize;
    vec4 baseColor = texture(baseColorTexture, texCoord);

    fragColor = baseColor;
}
