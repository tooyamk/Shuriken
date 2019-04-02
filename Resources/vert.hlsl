struct VS_INPUT
{
    float2 position : POSITION;
    float3 ccc : COLOR0;
    float2 cdddcc : COLOR1;
};

struct VS_OUTPUT
{
    float4 position : SV_POSITION;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT)0;
    output.position = float4(input.position, 0.0f, 1.0f);

    return output;
}