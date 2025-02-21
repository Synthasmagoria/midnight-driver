#version 330
in vec2 fragTexCoord;
in vec3 fragPosition;
in float fragLight;

uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform vec3 lightAmbientColor;

#include "lygia/generative/snoise.glsl"

out vec4 finalColor;

void main() {
    vec4 img = texture(texture0, fragTexCoord);
    vec4 col = img * colDiffuse * vec4(vec3(fragLight), 1.0);
    col.rgb = mix(lightAmbientColor, col.rgb, pow(fragLight, 2.0));
    col.rgb += snoise(vec3(fragTexCoord, fragPosition));
    finalColor = col;
}