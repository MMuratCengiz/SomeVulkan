#version 450

layout(set = 0, binding = 0) uniform ViewProjection {
    mat4 view;
    mat4 proj;
} vp;

layout( location = 0 ) in vec3 position;

layout (location = 0) out vec3 transitPosition;

void main() {
    transitPosition = position;
    vec4 pos = vp.proj * mat4(mat3(vp.view)) * vec4( position, 1.0f );
    gl_Position = pos.xyww;
}