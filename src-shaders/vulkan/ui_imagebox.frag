#version 460
#extension GL_EXT_shader_16bit_storage : require
#extension GL_EXT_shader_explicit_arithmetic_types : require

layout(location = 0) in f16vec2 texture_coords;
flat layout(location = 1) in uint16_t texture_id;

layout(set = 0, binding = 0) uniform sampler2D images[64];

layout(location = 0) out vec4 color_attachment;

void main()
{
	color_attachment = texture(images[0], vec2(texture_coords));
}