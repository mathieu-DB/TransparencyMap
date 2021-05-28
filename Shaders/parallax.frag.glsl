/*
* 
* https://github.com/JoeyDeVries/LearnOpenGL/blob/master/src/5.advanced_lighting/5.2.steep_parallax_mapping/5.2.parallax_mapping.fs
* https://learnopengl.com/Advanced-Lighting/Parallax-Mapping
* 
* Copyright c 2021 Joey de Vries
* Licensed under the Creative Commons Attribution-NonCommercial 4.0 Unported License (the “License”).
* You may not use this file except in compliance with the License. You may obtain a copy of the License at
* http://creativecommons.org/licenses/by-nc/4.0. Unless required by applicable law or
* agreed to in writing, software distributed under the License is distributed on an “AS IS” BASIS, WITHOUT
* WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific
* language governing permissions and limitations under the License.
* Twitter : https://twitter.com/JoeyDeVriez
* 
* Modifications :
* May 2021
* Mathieu David-Babin
* mathieu.david-babin@umontreal.ca
*/
#version 330 core
out vec4 FragColor;

in VS_OUT{
    vec3 FragPos;
    vec2 TexCoords;
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
} fs_in;

uniform sampler2D diffuseMap;
uniform sampler2D normalMap;
uniform sampler2D depthMap;
uniform sampler2D transparency;


uniform float heightScale;
uniform bool doDiscard;

vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir)
{

        // number of depth layers
        const float minLayers = 8;
        const float maxLayers = 32;
        float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0), viewDir)));
        // calculate the size of each layer
        float layerDepth = 1.0 / numLayers;
        // depth of current layer
        float currentLayerDepth = 0.0;
        // the amount to shift the texture coordinates per layer (from vector P)
        vec2 P = viewDir.xy / viewDir.z * heightScale;
        vec2 deltaTexCoords = P / numLayers;

        // get initial values
        vec2  currentTexCoords = texCoords;
        float currentDepthMapValue = texture(depthMap, currentTexCoords).r;

        while (currentLayerDepth < currentDepthMapValue)
        {
            // shift texture coordinates along direction of P
            currentTexCoords -= deltaTexCoords;
            // get depthmap value at current texture coordinates
            currentDepthMapValue = texture(depthMap, currentTexCoords).r;
            // get depth of next layer
            currentLayerDepth += layerDepth;
        }

        return currentTexCoords;
   
}

void main()
{
    if (gl_FragCoord.x < 270) {

        // offset texture coordinates with Parallax Mapping
        vec3 viewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);
        vec2 texCoords = fs_in.TexCoords;

        texCoords = ParallaxMapping(fs_in.TexCoords, viewDir);
        //if (texCoords.x > 1.0 || texCoords.y > 1.0 || texCoords.x < 0.0 || texCoords.y < 0.0)
        //    discard;

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
        vec4 trans = texture(transparency, texCoords);
        if (doDiscard)
            if (trans.a < 0.2)
                discard;
        FragColor = vec4(ambient + diffuse + specular, 1.0);
    }
    else {

        // offset texture coordinates with Parallax Mapping
        vec3 viewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);
        vec2 texCoords = fs_in.TexCoords;

        texCoords = ParallaxMapping(fs_in.TexCoords, viewDir);
        //if (texCoords.x > 1.0 || texCoords.y > 1.0 || texCoords.x < 0.0 || texCoords.y < 0.0)
        //    discard;

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
        vec4 trans = texture(transparency, texCoords);
       
        FragColor = vec4(ambient + diffuse + specular, 1.0);
    }
     
  
}