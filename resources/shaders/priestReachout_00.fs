#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform int time;

#include "lygia/generative/snoise.glsl"

out vec4 finalColor;

void main() {
    float distortion = snoise(vec3(fragTexCoord * 10.0, float(time) / 60.0)) * 0.05;
    finalColor = texture(texture0, fragTexCoord + distortion) * colDiffuse;
}