struct VS_INPUT {
    uint2 position : POSITION0;
    float3 ccc : ATTRIBUTE1;
    float2 cdddcc : ATTRIBUTE2;
};

struct VS_OUTPUT {
    float4 position : SV_POSITION;
};

float4x4 matWorld;
float aabbcc;

cbuffer MyBuffer : register(b3) {
float4 eee1 : packoffset(c0);
float1 ddd2 : packoffset(c1);
float1 fff3 : packoffset(c1.y);
}

float aabbcc11;

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT)0;
    output.position = float4(input.position.x / 10.0f, input.position.y / 10.0f, 0.0f, 1.0f);
    //output.position = mul(float4(input.position.x / 10.0f, input.position.y / 10.0f, 0.0f, 1.0f * aabbcc * fff3 * aabbcc11), matWorld);

    return output;
}