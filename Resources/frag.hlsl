struct PS_INPUT
{
};

int red;
float3 green;

cbuffer buf1 {
float blue;
}

float4 main(PS_INPUT input) : SV_TARGET
{
    return float4(red, 0.0, 0.0, 1.0);
}