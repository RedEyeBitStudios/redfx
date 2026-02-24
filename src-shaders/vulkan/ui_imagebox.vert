#version 460
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_shader_16bit_storage : require
#extension GL_EXT_shader_explicit_arithmetic_types : require

struct ImageBoxData
{
	u16vec2 lo_v;
	u16vec2 hi_v;
	uint16_t image_index;
} ImageBoxData_t;

layout(buffer_reference) readonly buffer RenderDataBuffer
{
	ImageBoxData boxes_data[];
};
layout(std430, push_constant) uniform Registers
{
	f16vec2 dimension_multiplier;
	RenderDataBuffer boxes;
} registers;

layout(location = 0) out f16vec2 texture_coords;
flat layout(location = 1) out uint16_t texture_id;

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
f16vec2 getTextureCoords()
{
	const uint16_t v_id = uint16_t(gl_VertexIndex);
	const f16vec2 v_data_array[2] =
	{
		f16vec2(0.0f),
		f16vec2(1.0f)
	};
	const uint16_t y_id = uint16_t(1) - (v_id % uint16_t(2));
	const uint16_t x_id = uint16_t(step(2.0f, float(v_id)));

	return f16vec2(v_data_array[x_id].x, v_data_array[y_id].y);
}

void main()
{
	const ImageBoxData box_data = registers.boxes.boxes_data[gl_InstanceIndex];
	texture_id = box_data.image_index;
	texture_coords = getTextureCoords();
	
	gl_Position = vec4(vec2(getFrameBufferPosition(box_data.lo_v, box_data.hi_v)), 0, 1.0f);
}