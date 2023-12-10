#version 300 es

precision highp float;

in vec2 vs_texCoord;
in vec4 vs_color;
in vec2 vs_size;
in float vs_cornerRadius;

out vec4 fragColor;

// distance function taken from https://www.shadertoy.com/view/WtdSDs
// position relative to rectangle center
float sdRoundedRect(vec2 position, vec2 halfSize, float cornerRadius)
{
    return length(max(abs(position) - halfSize + cornerRadius, 0.0)) - cornerRadius;
}

void main(void)
{
    float dist = sdRoundedRect(vs_texCoord, 0.5 * vs_size - vec2(1.0), vs_cornerRadius);
    float alpha = smoothstep(2.0, 0.0, dist);
    fragColor = vec4(vs_color.xyz, alpha * vs_color.w);
}
