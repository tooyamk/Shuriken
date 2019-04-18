struct PS_INPUT
{
};

float red;
float3 green;

cbuffer buf1 {
float blue;
}

float4 main(PS_INPUT input) : SV_TARGET
{
    return float4(red, green.x, blue, 1.0);
}