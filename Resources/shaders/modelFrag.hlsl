struct PS_INPUT {
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

//float2 red[2];
float3 green;

Texture2D _diffuseTex : register(t3);
SamplerState _diffuseTexSampler;

//struct aabbcc {
//    float val1;
 //   float val2[3];
//    float val3;
//};

cbuffer buf1 {
    float red;
//aabbcc blue;
//Texture2D texDiffuse;
//SamplerState samLiner;
}

float4 main(PS_INPUT input) : SV_TARGET {
    float4 c = _diffuseTex.Sample(_diffuseTexSampler, input.uv);
    c.x = green.x;
    //c.x = blue.val2[1];
    return c;
    //c.x = red;
    //return c;
    //return float4(blue.val1, red, 0.0, 1.0);
    //return float4(1.0, 0.0, 0.0, 1.0);
}