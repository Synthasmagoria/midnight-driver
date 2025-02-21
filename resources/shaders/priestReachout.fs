#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;
uniform sampler2D noiseTexture;
uniform vec4 colDiffuse;
uniform float time;
uniform vec4 color_a;
uniform vec4 color_b;
uniform float frequency;

out vec4 finalColor;

void main() {
    vec2 st = fragTexCoord;
    float t = time * 0.02;
    vec2 distortion = vec2(
        texture(noiseTexture, st * 1.125 + t).r,
        texture(noiseTexture, st * 1.1 + vec2(0.5) + t).r) * 2.0 - 1.0;
    float fbm = texture(noiseTexture, fract(st + distortion * 0.1)).r;
    finalColor = texture(texture0, st) * vec4(vec3(fbm), 1.0);
}

// void main() {
//     float t = time * 0.01;
//     vec2 st = fract(fragTexCoord * vec2(0.8316, 0.5839) * frequency + vec2(t, 0.0));
//     vec2 st2 = fract(fragTexCoord * vec2(1.2316, 0.9839) * frequency + vec2(t, t * 0.1 + 0.555));
//     vec2 st3 = vec2(
//         texture(noiseTexture, st).r,
//         texture(noiseTexture, st2).r) * 2.0 - 1.0;
//     float n1 = texture(noiseTexture, fract(fragTexCoord + st3)).r;
//     vec4 color = mix(color_a, color_b, n1);
//     finalColor = texture(texture0, fragTexCoord).a * color;
// }