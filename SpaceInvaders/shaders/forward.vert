#version 460
// #extension GL_EXT_buffer_reference : require

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec2 in_uv;
layout (location = 2) in vec3 in_normal;
layout (location = 3) in vec4 in_color;

layout (location = 0) out vec3 out_color;
layout (location = 1) out vec2 out_uv;

layout (push_constant) uniform constants
{
    mat4 model;
    mat4 view_projection;
} push_constants;

void main()
{
    mat4 model = push_constants.model;
    mat4 view_projection = push_constants.view_projection;

    gl_Position = view_projection * model * vec4(in_position, 1.0f);
    out_color = in_color.xyz;
    out_uv = in_uv;
}