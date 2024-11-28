#version 330
in vec2 fragTexCoord;
in vec3 fragPosition;
in float fragLight;

uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform vec3 lightPosition;
uniform float lightFalloff;

out vec4 finalColor;

void main() {
    // TODO: Add some sort of ambient light color
    finalColor = texture(texture0, fragTexCoord) * colDiffuse * vec4(vec3(fragLight), 1.0);
}