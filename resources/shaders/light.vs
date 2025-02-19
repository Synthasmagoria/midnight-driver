#version 330
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;

uniform mat4 mvp;
uniform mat4 modelMat;
uniform vec3 lightPosition;
uniform float lightFalloffDistance;

out vec2 fragTexCoord;
out vec3 fragPosition;
out float fragLight;

void main() {
    fragTexCoord = vertexTexCoord;
    fragPosition = (modelMat * vec4(vertexPosition, 1.0)).xyz;
    fragLight = clamp(1.0 - distance(fragPosition, lightPosition) / lightFalloffDistance, 0.0, 1.0);
    gl_Position = mvp * vec4(vertexPosition, 1.0);
}