/*
*
* https://github.com/JoeyDeVries/LearnOpenGL/blob/master/src/5.advanced_lighting/5.2.steep_parallax_mapping/5.2.parallax_mapping.vs
* https://learnopengl.com/Advanced-Lighting/Parallax-Mapping
*
* Copyright c 2021 Joey de Vries
* Licensed under the Creative Commons Attribution-NonCommercial 4.0 Unported License (the ?License?).
* You may not use this file except in compliance with the License. You may obtain a copy of the License at
* http://creativecommons.org/licenses/by-nc/4.0. Unless required by applicable law or
* agreed to in writing, software distributed under the License is distributed on an ?AS IS? BASIS, WITHOUT
* WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific
* language governing permissions and limitations under the License.
* Twitter : https://twitter.com/JoeyDeVriez
*/
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;
layout(location = 3) in vec3 aTangent;
layout(location = 4) in vec3 aBitangent;

out VS_OUT{
    vec3 FragPos;
    vec2 TexCoords;
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
} vs_out;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

uniform vec3 lightPos;
uniform vec3 viewPos;

void main()
{
    vs_out.FragPos = vec3(model * vec4(aPos, 1.0));
    vs_out.TexCoords = aTexCoords;

    vec3 T = normalize(mat3(model) * aTangent);
    vec3 B = normalize(mat3(model) * aBitangent);
    vec3 N = normalize(mat3(model) * aNormal);
    mat3 TBN = transpose(mat3(T, B, N));

    vs_out.TangentLightPos = TBN * lightPos;
    vs_out.TangentViewPos = TBN * viewPos;
    vs_out.TangentFragPos = TBN * vs_out.FragPos;

    gl_Position = projection * view * model * vec4(aPos, 1.0);
}