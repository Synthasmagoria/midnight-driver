#version 330
in vec2 fragTexCoord;
in vec3 fragPosition;
in float fragLight;

uniform sampler2D texture0;
uniform sampler2D texture1;
uniform vec4 colDiffuse;
uniform vec3 lightAmbientColor;

out vec4 finalColor;

#define TERRAIN_MAP texture0
#define TERRAIN_TEXTURE texture1

void main() {
    // TODO: Add some sort of ambient light color
    vec4 terrain = texture(TERRAIN_MAP, fragTexCoord);
    vec2 uv = fract(fragTexCoord * 80.0);
    vec2 uv_gravel = vec2(uv.x * 0.5 + 0.5, uv.y);
    vec2 uv_forest = vec2(uv.x * 0.5, uv.y);
    vec4 gravel = texture(TERRAIN_TEXTURE, uv_gravel);
    vec4 forest = texture(TERRAIN_TEXTURE, uv_forest);
    vec4 img = mix(gravel, forest, 0.5 + terrain.g * 0.5 - terrain.r * 0.5);
    vec4 col = img * colDiffuse * vec4(vec3(fragLight), 1.0);
    col.rgb = mix(lightAmbientColor, col.rgb, pow(fragLight, 2.0));
    finalColor = col;
}