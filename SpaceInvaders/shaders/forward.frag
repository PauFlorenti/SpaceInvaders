#version 460

layout (location = 0) in vec3 in_color;
layout (location = 1) in vec2 in_uv;

layout (location = 0) out vec4 out_color;

layout (set = 0, binding = 0) uniform sampler2D albedo_texture;

void main()
{
    vec4 color_texture = texture(albedo_texture, in_uv);

    if (color_texture.w != 1.0f)
        discard;

    out_color = vec4(in_color, 1.0f) * color_texture;
}