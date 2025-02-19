#version 330
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;
in mat4 instanceTransform;

uniform mat4 mvp;
uniform vec3 lightPosition;
uniform float lightFalloffDistance;
uniform float lightLuminocity;

out vec2 fragTexCoord;
out vec3 fragPosition;
out float fragLight;

void main() {
    fragPosition = vec3(instanceTransform * vec4(vertexPosition, 1.0));
    fragLight = clamp(1.0 - distance(fragPosition, lightPosition) / lightFalloffDistance, 0.0, 1.0) * lightLuminocity;
    fragTexCoord = vertexTexCoord;
    gl_Position = mvp * instanceTransform * vec4(vertexPosition, 1.0);
}