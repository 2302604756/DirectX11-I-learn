// VertexShader.hlsl
struct VSIn
{
    float4 color: COLOR;
    float2 pos : POSITION;
};

struct VSOut
{
    float4 color : COLOR;
    float4 pos : SV_POSITION;
};

VSOut main(VSIn vin)
{
    VSOut v;
    v.color = vin.color;
    v.pos = float4(vin.pos, 0.0f, 1.0f);
    return v;
}
