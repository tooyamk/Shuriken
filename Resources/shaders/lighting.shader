shader {
    define {
        static {
        }

        dynamic {
            _LIGHT_TYPE
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

               #if _LIGHT_TYPE >= 1 && _LIGHT_TYPE <= 3
                float3 pos : POSITION0;
                #endif
            };

            float3x4 _matrix_lw;
            float4x4 _matrix_lp;

            VS_OUTPUT main(VS_INPUT input)
            {
                VS_OUTPUT output;
                output.position = mul(float4(input.position, 1.0f), _matrix_lp);
                output.normal = mul(input.normal, _matrix_lw);
                output.uv = input.uv;

                #if _LIGHT_TYPE >= 1 && _LIGHT_TYPE <= 3
                output.pos = mul(input.position, _matrix_lw);
                #endif

                return output;
            }
        }

        ps {
            struct PS_INPUT {
                float4 position : SV_POSITION;
                float3 normal : NORMAL0;
                float2 uv : UV0;

                #if _LIGHT_TYPE >= 1 && _LIGHT_TYPE <= 3
                float3 pos : POSITION0;
                #endif
            };

            #if _LIGHT_TYPE >= 1 && _LIGHT_TYPE <= 3
                #if _LIGHT_TYPE == 1
            struct Light {
                float3 color;
                float3 dir;
            };
                #elif _LIGHT_TYPE == 2
            struct Light {
                float3 color;
                float3 pos;
                float3 attenuation;
            };
                #else
            struct Light {
                float3 color;
                float3 dir;
                float3 pos;
                float4 attenuation;
            };
                #endif

            Light _light;
            float3 _cameraPos;
            #endif

            float3 _ambientColor;

            Texture2D _diffuseTex;
            SamplerState _diffuseTexSampler;

            float BlinnPhoneFactor(float3 normal, float3 lightingDir, float3 viewDir, float shininess) {
	            float3 h = normalize(lightingDir + viewDir);
	            return pow(max(dot(normal, h), 0.0), shininess);
            }

            float4 main(PS_INPUT input) : SV_TARGET {
                float4 c = _diffuseTex.Sample(_diffuseTexSampler, input.uv);

                #if _LIGHT_TYPE >= 1 && _LIGHT_TYPE <= 3
                    #if _LIGHT_TYPE == 1

                float luminosity = 1.0;
                float3 lightingDir = -_light.dir;

                    #else

                float3 dis3 = _light.pos - input.pos;
                float3 lightingDir = normalize(dis3);
                float dis = length(dis3);
                float luminosity = 1.0 / (_light.attenuation.x + (_light.attenuation.y + _light.attenuation.z * dis) * dis);
                
                        #if _LIGHT_TYPE == 3

                float theta = max(0.0, dot(-lightingDir, _light.dir));
                if (theta <= _light.attenuation.w) {
                    luminosity = 0.0;
                } else {
                    luminosity *= pow(theta, 1200);
                }

                        #endif
                    #endif

                float3 diffuseColor = _light.color * max(dot(input.normal, lightingDir), 0.0);

                float3 viewDir = normalize(_cameraPos - input.pos);
                float3 specularColor = BlinnPhoneFactor(input.normal, lightingDir, viewDir, 100);

                c.xyz = (c.xyz * _ambientColor) + (c.xyz * diffuseColor + specularColor) * luminosity;

                #else
                
                c.xyz = c.xyz * _ambientColor;

                #endif

                return c;
            }
        }
    }
}