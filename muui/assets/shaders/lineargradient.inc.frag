uniform sampler2D gradientTexture;

in vec2 vs_gradientFrom;
in vec2 vs_gradientTo;

vec4 gradientColor(vec2 position)
{
    vec2 u = position - vs_gradientFrom;
    vec2 v = vs_gradientTo - vs_gradientFrom;
    float t = clamp(dot(u, v) / dot(v, v), 0.0, 1.0);
    return texture(gradientTexture, vec2(t, 0.0));
}
