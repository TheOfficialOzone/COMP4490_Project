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
uniform sampler2D depthMap;

uniform float heightScale;

// Working
// vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir)
// { 
//     float height =  texture(depthMap, texCoords).r;     
//     // float height = 1 - texture(depthMap, texCoords).r;     
//     return texCoords - viewDir.xy * (height * heightScale);        
// }

vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir)
{ 
    // number of depth layers
    const float numLayers = 10;
    // calculate the size of each layer
    float layerDepth = 1.0 / numLayers;
    // depth of current layer
    float currentLayerDepth = 0.0;

    // the amount to shift the texture coordinates per layer (from vector P)
    vec2 P = viewDir.xy * heightScale; 
    vec2 deltaTexCoords = P / numLayers;

    // get initial values
    vec2  currentTexCoords     = texCoords;
    float currentDepthMapValue = texture(depthMap, currentTexCoords).r;
    
    while(currentLayerDepth < currentDepthMapValue)
    {
        // shift texture coordinates along direction of P
        currentTexCoords -= deltaTexCoords;
        // get depthmap value at current texture coordinates
        currentDepthMapValue = texture(depthMap, currentTexCoords).r;  
        // get depth of next layer
        currentLayerDepth += layerDepth;  
    }
    // get texture coordinates before collision (reverse operations)
    vec2 prevTexCoords = currentTexCoords + deltaTexCoords;

    // get depth after and before collision for linear interpolation
    float afterDepth  = currentDepthMapValue - currentLayerDepth;
    float beforeDepth = texture(depthMap, prevTexCoords).r - currentLayerDepth + layerDepth;
    
    // interpolation of texture coordinates
    float weight = afterDepth / (afterDepth - beforeDepth);
    vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);

    return finalTexCoords;  
    // return currentTexCoords;
    // float height =  texture(depthMap, texCoords).r;     
    // // float height = 1 - texture(depthMap, texCoords).r;     
    // return texCoords - viewDir.xy * (height * heightScale);        
}



void main()
{           
    // offset texture coordinates with Parallax Mapping
    vec3 viewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);
    vec2 texCoords = ParallaxMapping(fs_in.TexCoords, viewDir);       
    // if(texCoords.x > 1.0 || texCoords.y > 1.0 || texCoords.x < 0.0 || texCoords.y < 0.0)
    //     discard;

    // obtain normal from normal map
    vec3 normal = texture(normalMap, texCoords).rgb;
    normal = normalize(normal * 2.0 - 1.0);   
   
    // get diffuse color
    vec3 color = texture(diffuseMap, texCoords).rgb;

    // ambient
    vec3 ambient = 0.1 * color;
    // diffuse
    vec3 lightDir = normalize(fs_in.TangentLightPos - fs_in.TangentFragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * color;
    // specular    
    vec3 reflectDir = reflect(-lightDir, normal);
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);

    vec3 specular = vec3(0.2) * spec;
    FragColor = vec4(ambient + diffuse + specular, 1.0);
}
/*
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
uniform sampler2D depthMap;
  
uniform float height_scale;



vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir)
{ 
    float height = texture(depthMap, texCoords).r;
    return texCoords - viewDir.xy * (height * 1.f);
}

void main()
{           
   // offset texture coordinates with Parallax Mapping
   vec3 viewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);
   vec2 texCoords = fs_in.TexCoords;
    
   texCoords = ParallaxMapping(fs_in.TexCoords, viewDir);       
//    if(texCoords.x > 1.0 || texCoords.y > 1.0 || texCoords.x < 0.0 || texCoords.y < 0.0)
//       discard;

   // obtain normal from normal map
   vec3 normal = texture(normalMap, texCoords).rgb;
   normal = normalize(normal * 2.0 - 1.0);   
   
    // get diffuse color
   vec3 color = texture(diffuseMap, texCoords).rgb;
    // ambient
   vec3 ambient = color;
    // diffuse
    vec3 lightDir = normalize(fs_in.TangentLightPos - fs_in.TangentFragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * color;
    // specular    
    vec3 reflectDir = reflect(-lightDir, normal);
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);

    vec3 specular = vec3(0.2) * spec;
    FragColor = vec4(ambient + diffuse + specular, 1.0);
}
*/
// void main()
// {           
//    // offset texture coordinates with Parallax Mapping
//    vec3 viewDir   = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);
//    vec2 texCoords = ParallaxMapping(fs_in.TexCoords,  viewDir);

//    if(texCoords.x > 1.0 || texCoords.y > 1.0 || texCoords.x < 0.0 || texCoords.y < 0.0)
//       discard;
//     // then sample textures with new texture coords
//    vec3 diffuse = vec3(texture(diffuseMap, texCoords));
//    vec3 normal = vec3(texture(normalMap, texCoords));
//    normal = normalize(normal * 2.0 - 1.0);

//    //  FragColor = mix(vec4(1.f, 1.f, 0.5f, 1.f), texture(diffuseMap, texCoords), 0.5);
//    //  FragColor = mix(vec4(1.f, 1.f, 0.5f, 1.f), texture(diffuseMap, fs_in.TexCoords), 0.5);
//    // FragColor = texture(diffuseMap, fs_in.TexCoords);

//    float Kd = max(dot(normal, ))
//    float lighting_effect = 0;

//    FragColor = vec4(diffuse, 1.f);
//    // FragColor = vec4(1, 1, 0, 1);
//     // proceed with lighting code
//    //  [...]    
// }

/*
// Needs normal
in vec3 fragPos;
in vec3 fViewPos;
in vec2 fTexCoord;
in vec3 fTangentViewPos;
in vec3 fTangentFragPos;

out vec4 color;

uniform sampler2D diffuseMap;
uniform sampler2D normalMap;
uniform sampler2D depthMap;

vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir)
{ 
    float height =  texture(depthMap, texCoords).r;    
   //  vec2 p = viewDir.xy / viewDir.z * (height * height_scale);
   vec2 p = viewDir.xy / viewDir.z * (height * 1.f);
    return texCoords - p;    
} 

void main() 
{ 
   // offset texture coordinates with Parallax Mapping
   vec3 viewDir   = normalize(fTangentViewPos - fTangentFragPos);
   vec2 texCoords = ParallaxMapping(fTexCoord,  viewDir);

   // then sample textures with new texture coords
   // vec3 diffuse = texture(diffuseMap, texCoords);
   vec3 normal  = vec3(texture(normalMap, texCoords));
   normal = normalize(normal * 2.0 - 1.0);
   // Discard the result if it's off the edge
   // discard; Can use discard!!!!
   // color = vec4(0.2,1.0,0.5,1);

   color = mix(vec4(1, 1, 0.5, 1), texture(depthMap, texCoords), 0.5); 

   // color = mix(vec4(1, 1, 0.5, 1), texture(snow_texture, texCoords), 0.5); 

   // color.rgb = color.rrr;
}
*/