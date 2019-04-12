struct PS_INPUT
{
};

float red;
//float green;
float blue;

float4 main(PS_INPUT input) : SV_TARGET
{
    return float4(red, 0.0, blue, 1.0f);
}