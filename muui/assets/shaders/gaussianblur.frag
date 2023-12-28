precision highp float;

// stolen from https://learnopengl.com/Advanced-Lighting/Bloom

uniform sampler2D baseColorTexture;

uniform bool horizontal;
  
in vec2 vs_texCoord;

out vec4 fragColor;

void main()
{             
    const float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);
    vec2 texCoordOffset = 1.0 / vec2(textureSize(baseColorTexture, 0));
    float result = texture(baseColorTexture, vs_texCoord).a * weight[0];
    if (horizontal)
    {
        for (int i = 1; i < 5; ++i)
        {
            result += texture(baseColorTexture, vs_texCoord + vec2(texCoordOffset.x * float(i), 0.0)).a * weight[i];
            result += texture(baseColorTexture, vs_texCoord - vec2(texCoordOffset.x * float(i), 0.0)).a * weight[i];
        }
    }
    else
    {
        for (int i = 1; i < 5; ++i)
        {
            result += texture(baseColorTexture, vs_texCoord + vec2(0.0, texCoordOffset.y * float(i))).a * weight[i];
            result += texture(baseColorTexture, vs_texCoord - vec2(0.0, texCoordOffset.y * float(i))).a * weight[i];
        }
    }
    fragColor = vec4(1.0, 1.0, 1.0, result);
}
