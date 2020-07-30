struct PS_INPUT {
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

//Texture2D ppTex;
Texture2DMS<float4> ppTex;
SamplerState ppTexSampler;

float4 main(PS_INPUT input) : SV_TARGET {
    float4 c1 = ppTex.Load(int2(input.uv.x * 800, input.uv.y * 600), 0);
    float4 c2 = ppTex.Load(int2(input.uv.x * 800, input.uv.y * 600), 1);
    float4 c3 = ppTex.Load(int2(input.uv.x * 800, input.uv.y * 600), 2);
    float4 c4 = ppTex.Load(int2(input.uv.x * 800, input.uv.y * 600), 3);
    return (c1 + c2 + c3 + c4) / 4.0f;
    //return ppTex.Sample(ppTexSampler, input.uv);
}