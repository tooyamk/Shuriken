struct VS_INPUT {
    float2 position : POSITION0;
    float2 uv : TEXCOORD0;
};

struct VS_OUTPUT {
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT)0;
    output.position = float4(input.position.x, input.position.y, 0.0f, 1.0f);
    output.uv = input.uv;
    return output;
}