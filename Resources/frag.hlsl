struct PS_INPUT {
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

float2 red[2];
float3 green;

Texture2DArray texDiffuse;
SamplerState samLiner {
    Filter = MIN_MAG_MIP_LINEAR;
};

cbuffer buf1 {
float blue;
}

float4 main(PS_INPUT input) : SV_TARGET {
    float4 c = texDiffuse.Sample(samLiner, float3(input.uv, 3.5));
    return c;
    //c.x = red;
    //return c;
    //return float4(red[1].x, green.x, 0.0, 1.0);
    //return float4(1.0, 0.0, 0.0, 1.0);
}