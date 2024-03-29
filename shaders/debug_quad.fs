#version 330
// Output fragment color
out vec4 finalColor;

in vec2 fragTexCoord;

uniform sampler2D depthMap;
const float near_plane = 0.1;
const float far_plane = 1000.0;

// required when using a perspective projection matrix
float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // Back to NDC 
    return (2.0 * near_plane * far_plane) / (far_plane + near_plane - z * (far_plane - near_plane));	
}

void main()
{             
    float depthValue = texture(depthMap, fragTexCoord).r;
    // FragColor = vec4(vec3(LinearizeDepth(depthValue) / far_plane), 1.0); // perspective
    finalColor = vec4(vec3(depthValue), 1.0); // orthographic
}