struct VS_INPUT
{
    uint2 position : POSITION0;
    float3 ccc : ATTRIBUTE1;
    float2 cdddcc : ATTRIBUTE2;
};

struct VS_OUTPUT
{
    float4 position : SV_POSITION;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT)0;
    output.position = float4(input.position.x / 10.0f, input.position.y / 10.0f, 0.0f, 1.0f);

    return output;
}