#version 310 es
layout(location = 0) in vec4 Position;
layout(location = 1) in vec2 TexCoord;
layout(location = 2) in vec4 Color;
layout(location = 0) out vec2 vTexCoord;
layout(location = 1) out vec4 vColor;

layout(set = 0, binding = 0, std140) uniform UBO
{
   vec2 center;
   vec2 size;
   float angle;
}global;

//layout(push_constant) uniform UBO
//{
//   vec2 center;
//   vec2 size;
//   float angle;
//}global;
//layout(set = 0, binding = 0, std140) uniform UBO
//{
//   mat4 MVP;
//} global;
//x' = x cos θ − y sin θ
//y' = x sin θ + y cos θ
//|cos θ   −sin θ| |x| = |x cos θ − y sin θ| = |x'|
//|sin θ    cos θ| |y|   |x sin θ + y cos θ|   |y'|
void main()
{
//   gl_Position = vec4(gl_VertexIndex, gl_VertexIndex, 0.0f, 1.0f);
   gl_Position = Position;
   gl_Position.xy *= global.size;
   mat2 rot = mat2(cos(global.angle), -sin(global.angle), sin(global.angle), cos(global.angle));
   gl_Position.xy = rot * gl_Position.xy;
   gl_Position.xy += global.center;

   vTexCoord = TexCoord;
   vColor = Color;
}
