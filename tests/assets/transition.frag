precision highp float;

uniform sampler2D baseColorTexture;

in vec2 vs_texCoord;
in vec2 vs_viewportSize;
in float vs_spacing;
in float vs_slope;
in float vs_progress;

out vec4 fragColor;

const float Spread = 0.25;

void main(void)
{
    vec2 size = vec2(textureSize(baseColorTexture, 0));
    vec2 pos = vs_texCoord * size;

    float totalWidth = size.x + (size.y / vs_slope);
    float tileCount = ceil(totalWidth / vs_spacing);
    float baseX = pos.x + (size.y - pos.y) / vs_slope;
    float tile = floor(baseX / vs_spacing);

    float t0 = (tile / (tileCount - 1.0)) * (1.0 - Spread);
    float t1 = t0 + Spread;
    float t = 1.0 - vs_progress;
    t = clamp((t - t0) / (t1 - t0), 0.0, 1.0);
    t *= t;

    float direction = 2.0 * mod(tile, 2.0) - 1.0; // -1 or 1
    vec2 v = vec2(size.y / vs_slope, size.y);
    vec2 samplePos = pos + direction * t * v;
    vec4 baseColor = texture(baseColorTexture, samplePos / size);

    fragColor = baseColor;
}
