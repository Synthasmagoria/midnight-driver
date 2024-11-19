#version 330
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragPosition;

uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform vec3 lightPosition;
uniform float lightFalloff;

out vec4 finalColor;

void main() {
    float light = smoothstep(lightFalloff, 0.0, distance(fragPosition, lightPosition));
    vec4 texelColor = texture(texture0, fragTexCoord);
    finalColor = texelColor * colDiffuse * fragColor;
    finalColor.rgb *= light;
}