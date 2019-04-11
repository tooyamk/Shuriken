struct PS_INPUT
{
};

float3 color;

float4 main(PS_INPUT input) : SV_TARGET
{
    return float4(color, 1.0f);
}