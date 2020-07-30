struct PS_INPUT {
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

Texture2D ppTex;
SamplerState ppTexSampler;

float4 main(PS_INPUT input) : SV_TARGET {
    return ppTex.Sample(ppTexSampler, input.uv);
}