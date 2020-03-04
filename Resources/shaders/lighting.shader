shader {
    define {
        static {
        }

        dynamic {

        }
    }

    program {
        vs {
            struct VS_INPUT {
                float3 position : POSITION0;
                float3 normal : NORMAL0;
                float2 uv : UV0;
            };

            struct VS_OUTPUT {
                float4 position : SV_POSITION;
                float3 normal : NORMAL0;
                float2 uv : UV0;
            };

            float4x4 _matrix_lp;

            VS_OUTPUT main(VS_INPUT input)
            {
                VS_OUTPUT output;
                output.position = mul(float4(input.position, 1.0f), _matrix_lp);
                output.normal = input.normal;
                output.uv = input.uv;

                return output;
            }
        }

        ps {
            struct PS_INPUT {
                float4 position : SV_POSITION;
                float3 normal : NORMAL0;
                float2 uv : UV0;
            };

            float3 _diffuseColor;

            Texture2D _diffuseTex;
            SamplerState _diffuseTexSampler;

            float4 main(PS_INPUT input) : SV_TARGET {
                float4 c = _diffuseTex.Sample(_diffuseTexSampler, input.uv);
                c.xyz *= _diffuseColor;
                return c;
            }
        }
    }
}