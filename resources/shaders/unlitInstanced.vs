#version 330
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;
in mat4 instanceTransform;

uniform mat4 mvp;

out vec2 fragTexCoord;
out vec3 fragPosition;

#define LIGHT_FALLOFF_DISTANCE 18.0

void main() {
    fragPosition = vec3(instanceTransform * vec4(vertexPosition, 1.0));
    fragTexCoord = vertexTexCoord;
    gl_Position = mvp * instanceTransform * vec4(vertexPosition, 1.0);
}