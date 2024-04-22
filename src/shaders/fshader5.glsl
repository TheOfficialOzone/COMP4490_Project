#version 330
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
} fs_in;

uniform sampler2D diffuseMap;
uniform sampler2D normalMap;
uniform sampler2D staticDepthMap;

uniform sampler2D depthMap;

uniform sampler2D iceDiffuseMap;
uniform sampler2D iceNormalMap;
uniform sampler2D iceDisplacementMap;


uniform float heightScale;
uniform float pushedScale;

//
vec4 getDiffuseValue(vec2 texCoord, float depth) {
  return mix(texture(diffuseMap, texCoord), texture(iceDiffuseMap, texCoord), depth);
}

vec4 getNormalValue(vec2 texCoord, float depth) {
  return mix(texture(normalMap, texCoord), texture(iceNormalMap, texCoord), depth);
}

float getPushedDepthValue(vec2 texCoord) {
  return clamp(texture(depthMap, texCoord).r, 0.0, 1.0);
}

float getTextureDepthValue(vec2 texCoord, float depth) {
  return mix(texture(staticDepthMap, texCoord).r, texture(iceDisplacementMap, texCoord).r, depth);
}

vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir)
{
    // number of depth layers
    const float numLayers = 32;
    // calculate the size of each layer
    float layerDepth = 1.0 / numLayers;
    // depth of current layer
    float currentLayerDepth = 0.0;

    // the amount to shift the texture coordinates per layer (from vector P)
    vec2 P = viewDir.xy * heightScale;
    vec2 deltaTexCoords = P / numLayers;

    // get initial values
    vec2  currentTexCoords = texCoords;

    float pushedDepth = getPushedDepthValue(texCoords) * pushedScale;
    float currentDepthMapValue = pushedDepth + getTextureDepthValue(texCoords, pushedDepth);

    while(currentLayerDepth < currentDepthMapValue)
    {
        // shift texture coordinates along direction of P
        currentTexCoords -= deltaTexCoords;

        // get depthmap value at current texture coordinates
        pushedDepth = getPushedDepthValue(currentTexCoords) * pushedScale;
        currentDepthMapValue = pushedDepth + getTextureDepthValue(currentTexCoords, pushedDepth);

        // get depth of next layer
        currentLayerDepth += layerDepth;
    }
    // get texture coordinates before collision (reverse operations)
    vec2 prevTexCoords = currentTexCoords + deltaTexCoords;

    // get depth after and before collision for linear interpolation
    float afterDepth  = currentDepthMapValue - currentLayerDepth;
    float prev_pushed_depth = getPushedDepthValue(prevTexCoords) * pushedScale;
    float beforeDepth =
      getTextureDepthValue(prevTexCoords, prev_pushed_depth) +
      prev_pushed_depth -
      (currentLayerDepth - layerDepth);

    // interpolation of texture coordinates
    float weight = afterDepth / (afterDepth - beforeDepth);
    vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);
    return finalTexCoords;
}

float ShadowCalc(vec2 texCoord, vec3 lightDir)
{
    if ( lightDir.z <= 0.0 )
        return 0.5;

    const float numLayers = 10;

    vec2 currentTexCoords = texCoord;

    // Uses only the pushed depth for the shadows
    float pushedDepth = getPushedDepthValue(texCoord) * pushedScale;
    float currentDepthMapValue = pushedDepth + getTextureDepthValue(texCoord, pushedDepth);

    float currentLayerDepth = currentDepthMapValue;

    if (currentLayerDepth < 0.01) {
      return 1.0;
    }

    float layerDepth = 1.0 / numLayers;
    // vec2 P = lightDir.xy * heightScale;
    vec2 P = lightDir.xy / lightDir.z * heightScale;
    vec2 deltaTexCoords = P / numLayers;

    while (currentLayerDepth <= currentDepthMapValue && currentLayerDepth > 0.0)
    {
        currentTexCoords += deltaTexCoords;
        pushedDepth = getPushedDepthValue(currentTexCoords) * pushedScale;
        currentDepthMapValue = pushedDepth + getTextureDepthValue(currentTexCoords, pushedDepth);

        currentLayerDepth -= layerDepth;
    }

    float r = currentLayerDepth >= currentDepthMapValue ? 0.5 : 1.0;
    return r;
}


void main()
{
    // offset texture coordinates with Parallax Mapping

    vec3 viewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);
    vec2 texCoords = ParallaxMapping(fs_in.TexCoords, viewDir);
    if(texCoords.x > 0.99 || texCoords.y > 0.99 || texCoords.x < 0.01 || texCoords.y < 0.01)
        discard;

    // Check if in shadow
    vec3 lightDir = normalize(fs_in.TangentLightPos - fs_in.TangentFragPos);
    // float in_shadow = ShadowCalc(texCoords, lightDir);
    float in_shadow = 1.0;

    float displaced_depth = getPushedDepthValue(texCoords);
    float depth = displaced_depth + getTextureDepthValue(texCoords, displaced_depth);
    // if (displaced_depth > 0.8) { discard; }

    // get diffuse color
    vec3 color = getDiffuseValue(texCoords, displaced_depth).rgb;

    // obtain normal from normal map
    vec3 normal = getNormalValue(texCoords, displaced_depth).rgb;//texture(normalMap, texCoords).rgb;
    normal = normalize(normal * 2.0 - 1.0);

    // ambient
    vec3 ambient = 0.2 * color;
    // diffuse
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * color;
    // specular
    vec3 reflectDir = reflect(-lightDir, normal);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);

    vec3 specular = vec3(0.2) * spec;
    // Disabling specular for now
    specular = vec3(0);
    FragColor = vec4(ambient + diffuse + specular, 1.0) * in_shadow;
}
