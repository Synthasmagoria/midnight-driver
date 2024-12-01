#version 330
in vec2 fragTexCoord;
in vec3 fragPosition;
in float fragLight;

uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform vec3 lightPosition;
uniform float lightFalloff;

out vec4 finalColor;

#define FOG_COLOR vec3(0.08, 0.08, 0.12)

void main() {
    // TODO: Add some sort of ambient light color
    vec4 img = texture(texture0, fragTexCoord);
    vec4 col = img * colDiffuse * vec4(vec3(fragLight), 1.0);
    col.rgb = mix(FOG_COLOR, col.rgb, pow(fragLight, 2.0));
    finalColor = col;
}