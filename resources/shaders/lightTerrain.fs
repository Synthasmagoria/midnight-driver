#version 330
in vec2 fragTexCoord;
in vec3 fragPosition;
in float fragLight;

uniform sampler2D texture0;
uniform sampler2D texture1;
uniform vec4 colDiffuse;
uniform vec3 lightPosition;
uniform float lightFalloff;

out vec4 finalColor;

#define TERRAIN_MAP texture0
#define TERRAIN_TEXTURE texture1
#define FOG_COLOR vec3(0.08, 0.08, 0.12)

void main() {
    // TODO: Add some sort of ambient light color
    vec4 terrain = texture(TERRAIN_MAP, fragTexCoord);
    vec2 uv_gravel = vec2(fragTexCoord.x / 0.5, fragTexCoord.y);
    vec2 uv_forestfloor = vec2(fragTexCoord.x / 0.5 + 0.5, fragTexCoord.y);
    vec4 gravel = texture(TERRAIN_TEXTURE, uv_gravel);
    vec4 forestfloor = texture(TERRAIN_TEXTURE, uv_forestfloor);
    vec4 img = mix(gravel, forestfloor, 0.5 + terrain.g / 0.5 - terrain.r / 0.5);
    vec4 col = img * colDiffuse * vec4(vec3(fragLight), 1.0);
    col.rgb = mix(FOG_COLOR, col.rgb, pow(fragLight, 2.0));
    finalColor = col;

    finalColor = texture(TERRAIN_MAP, fragTexCoord);
}