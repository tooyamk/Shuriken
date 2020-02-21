#include "lib.hlsl"

struct VS_INPUT {
    float3 position : POSITION0;
    float3 ccc : ATTRIBUTE1;
    float2 uv : UV0;
};

struct VS_OUTPUT {
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

float4x4 _matrix_lp;

cbuffer MyBuffer : register(b3) {
float4 eee1 : packoffset(c0);
float1 ddd2 : packoffset(c1);
float1 fff3 : packoffset(c1.y);
}

float4x4 aabbcc11;

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT)0;
    //output.position = float4(input.position, aabbcc11);
    //output.position = mul(_m44_l2p, float4(input.position, 1.0f));
    output.position = mul(float4(input.position, 1.0f), _matrix_lp);
    output.uv = input.uv;
    //output.position = mul(float4(input.position.x / 10.0f, input.position.y / 10.0f, 0.0f, 1.0f * aabbcc * fff3 * aabbcc11), matWorld);

    return output;
}