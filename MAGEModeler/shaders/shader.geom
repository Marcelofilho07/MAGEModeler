#version 450
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in VS_OUT {
    vec3 fragColor;
} gs_in[];

layout(location = 0) out vec3 fragColor;

vec3 V[3];
vec3 CG;

void ProduceVertex( int v, vec3 cg )
{
    gl_Position = ubo.proj * vec4( cg + 1.0 * ( V[v] - cg ), 1. );
    EmitVertex( );
}


void main()
{
//    V[0] = gl_in[0].gl_Position.xyz;
//    V[1] = gl_in[1].gl_Position.xyz;
//    V[2] = gl_in[2].gl_Position.xyz;
//    CG = ( V[0] + V[1] + V[2] ) / 3.;
//    ProduceVertex(0, CG);
//    ProduceVertex(1, CG);
//    ProduceVertex(2, CG);

    gl_Position = gl_in[0].gl_Position;
    fragColor = gs_in[0].fragColor;
    EmitVertex();
    gl_Position = gl_in[1].gl_Position;
    fragColor = gs_in[1].fragColor;
    EmitVertex();
    gl_Position = gl_in[2].gl_Position;
    fragColor = gs_in[2].fragColor;
    EmitVertex();

    EndPrimitive();
}