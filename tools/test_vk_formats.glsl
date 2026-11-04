#version 450
// Shaders of tools/test_vk_formats.c, one per define; tools/test_vk_formats.ps1 compiles each with glslc.

#if defined(VS_FULL)
void main() { vec2 p = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2); gl_Position = vec4(p * 2.0 - 1.0, 0, 1); }

#elif defined(VS_TEX)
layout(push_constant) uniform P { vec4 a; vec4 b; } pc;
layout(set=0, binding=0) uniform sampler2D t;
layout(location=0) out vec4 v;
void main() { vec2 p = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2); gl_Position = vec4(p * 2.0 - 1.0, 0, 1); v = textureLod(t, pc.a.xy, 0.0); }

#elif defined(VS_CMP)
layout(push_constant) uniform P { vec4 a; vec4 b; } pc;
layout(set=0, binding=0) uniform sampler2DShadow t;
layout(location=0) out vec4 v;
void main() { vec2 p = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2); gl_Position = vec4(p * 2.0 - 1.0, 0, 1); v = vec4(textureLod(t, pc.a.xyz, 0.0)); }

#elif defined(VS_POINT)
void main() { gl_Position = vec4(1.0/32.0, 1.0/32.0, 0, 1); }

#elif defined(VS_POINT1)
void main() { gl_Position = vec4(1.0/32.0, 1.0/32.0, 0, 1); gl_PointSize = 1.0; }

#elif defined(VS_ATOMIC)
layout(std430, set=0, binding=1) buffer S { uint w[]; } s;
void main() { vec2 p = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2); gl_Position = vec4(p * 2.0 - 1.0, 0, 1); uint k = atomicAdd(s.w[0], 1u); s.w[1] = 0xABCD0000u + k; }

#elif defined(FS_TEX)
layout(push_constant) uniform P { vec4 a; vec4 b; } pc;
layout(set=0, binding=0) uniform sampler2D t;
layout(location=0) out vec4 c;
void main() { c = textureLod(t, pc.a.xy, 0.0); }

#elif defined(FS_CMP)
layout(push_constant) uniform P { vec4 a; vec4 b; } pc;
layout(set=0, binding=0) uniform sampler2DShadow t;
layout(location=0) out vec4 c;
void main() { c = vec4(textureLod(t, pc.a.xyz, 0.0)); }

#elif defined(FS_PASS)
layout(location=0) in vec4 v;
layout(location=0) out vec4 c;
void main() { c = v; }

#elif defined(FS_CONST)
layout(push_constant) uniform P { vec4 a; vec4 b; } pc;
layout(location=0) out vec4 c;
void main() { c = pc.a; }

#elif defined(CS_CMP)
layout(local_size_x=1) in;
layout(push_constant) uniform P { vec4 a; vec4 b; } pc;
layout(set=0, binding=0) uniform sampler2DShadow t;
layout(std430, set=0, binding=1) buffer S { float w[]; } s;
void main() { s.w[0] = textureLod(t, pc.a.xyz, 0.0); }
#endif
