struct PS_INPUT {
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

//float2 red[2];
float3 green;

Texture2D texDiffuse : register(t3);
SamplerState samLiner;

struct aabbcc {
    float val1;
    float4 val2[1];
    float val3;
};

cbuffer buf1 {
    float red;
aabbcc blue;
//Texture2D texDiffuse;
//SamplerState samLiner;
}

float4 main(PS_INPUT input) : SV_TARGET {
    float4 c = texDiffuse.Sample(samLiner, input.uv);
    c.x = red;
    return c;
    //c.x = red;
    //return c;
    //return float4(blue.val1, red, 0.0, 1.0);
    //return float4(1.0, 0.0, 0.0, 1.0);
}