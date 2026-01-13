// PixelShader.hlsl
Texture2D tex : register(t0);

SamplerState splr : register(s0);

//要把PSIn VSIn VSOut 以及cpp中的inputlayout中的语义顺序和偏移量设置顺序正确，数值正确
//而且在vs和ps中都要把语义写完整 系统是按照顺序读取的 如果把float4 pos : SV_POSITION省略
//那么color就会读到原来的pos
struct PSIn{
    float4 pos : SV_POSITION;
    float4 color: COLOR;
    float2 uv : TEXCOORD;
};


float4 main(PSIn psin) : SV_TARGET
{
    return tex.Sample(splr, psin.uv);
}
