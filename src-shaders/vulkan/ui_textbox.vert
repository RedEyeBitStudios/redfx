#version 460
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_shader_16bit_storage : require
#extension GL_EXT_shader_explicit_arithmetic_types : require

layout(location = 0) in f16vec2 input_vertex_pos;

struct TextBoxData
{
	f16vec4 color_rgba;
	f16vec2 offset_px;
	float16_t size_px;
} TextBoxData_t;

layout(buffer_reference) readonly buffer RenderDataBuffer
{
	TextBoxData boxes_data[];
};
layout(std430, push_constant) uniform Registers
{
	f16vec2 dimension_multiplier;
	f16vec2 aspect_corrector;
	RenderDataBuffer boxes;
} registers;

flat layout(location = 0) out f16vec4 color_rgba;

void main()
{
	const TextBoxData box_data = registers.boxes.boxes_data[gl_InstanceIndex];
	const f16vec2 raw_v_pos = (f16vec2(input_vertex_pos.x, -input_vertex_pos.y) * f16vec2(0.5f) + f16vec2(0.5f)) * f16vec2(box_data.size_px) * registers.dimension_multiplier;
	const f16vec2 v_pos = ((raw_v_pos + (box_data.offset_px * registers.dimension_multiplier)) * f16vec2(2.0f) - f16vec2(1.0f));

	gl_Position = vec4(v_pos, 0, 1.0f);
	color_rgba = box_data.color_rgba;
}