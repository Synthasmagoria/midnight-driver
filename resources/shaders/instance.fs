in vec3 fragPosition;
in vec2 fragTexCoord;
in float fragLightVal;

uniform sampler2D texture0;
uniform vec4 colDiffuse;

out vec4 finalColor;

void main() {
    finalColor = texture2D(texture0, fragTexCoord) * colDiffuse * vec4(vec3(fragLightVal), 1.0);
}