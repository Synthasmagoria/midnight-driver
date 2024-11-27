in vec3 vertexPosition;
in vec2 vertexTexCoord;
in mat4 instanceTransform;

uniform mat4 mvp;

out vec3 fragPosition;
out vec2 fragTexCoord;
out float fragLightVal;

void main() {
    fragPosition = vec3(instanceTransform * vec4(vertexPosition, 1.0));
    fragTexCoord = vertexTexCoord;
    fragLightVal = 1.0;
    gl_Position = mvp * instanceTransform * vec4(vertexPosition, 1.0);
}