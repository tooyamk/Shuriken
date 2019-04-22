struct PS_INPUT {
};

float2 red[2];
float3 green;

Texture2D texDiffuse;
SamplerState samLiner {
    Filter = MIN_MAG_MIP_LINEAR;
};

cbuffer buf1 {
float blue;
}

float4 main(PS_INPUT input) : SV_TARGET {
    float4 c = texDiffuse.Sample(samLiner, float2(0.0, 0.0));
    return c;
    //c.x = red;
    //return c;
    //return float4(red[1].x, green.x, 0.0, 1.0);
}