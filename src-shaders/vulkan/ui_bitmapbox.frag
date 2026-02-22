#version 460
#extension GL_EXT_shader_16bit_storage : require
#extension GL_EXT_shader_explicit_arithmetic_types : require

layout(location = 0) in f16vec4 color_rgba;

layout(location = 0) out vec4 color_attachment;

void main()
{
	color_attachment = vec4(color_rgba);
}