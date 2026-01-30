#version 460
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_shader_16bit_storage : require
#extension GL_EXT_shader_explicit_arithmetic_types : require

struct ColorBoxData
{
	u16vec2 lo_v;
	u16vec2 hi_v;
	f16vec4 color_rgba;
	uint16_t stack_position;
} ColorBoxData_t;

layout(buffer_reference) readonly buffer RenderDataBuffer
{
	ColorBoxData boxes_data[];
};
layout(std430, push_constant) uniform Registers
{
	f16vec2 dimension_multiplier;
	RenderDataBuffer boxes;
} registers;

flat layout(location = 0) out f16vec4 color_rgba;

f16vec2 getFrameBufferPosition(const u16vec2 lo, const u16vec2 hi)
{
	const uint16_t v_id = uint16_t(gl_VertexIndex);
	const u16vec2 v_data_array[2] =
	{
		lo,
		hi
	};
	const uint16_t y_id = uint16_t(1) - (v_id % uint16_t(2));
	const uint16_t x_id = uint16_t(step(2.0f, float(v_id)));

	return f16vec2(v_data_array[x_id].x, v_data_array[y_id].y) * registers.dimension_multiplier * f16vec2(2.0f) - f16vec2(1.0f);
}

void main()
{
	const ColorBoxData box_data = registers.boxes.boxes_data[gl_InstanceIndex];
	color_rgba = f16vec4(smoothstep(0.0f, 255.0f, vec4(box_data.color_rgba)));
	
	gl_Position = vec4(vec2(getFrameBufferPosition(box_data.lo_v, box_data.hi_v)), float(box_data.stack_position) / float(255.0f), 1.0f);
}