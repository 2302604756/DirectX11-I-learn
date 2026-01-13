// VertexShader.hlsl
struct VSIn
{    
    float3 pos : POSITION;
    float4 color: COLOR;
    float2 uv : TEXCOORD;
};

struct VSOut
{

    float4 pos : SV_POSITION;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
};

cbuffer CBuf : register(b0){
   row_major matrix transform;
    
};

cbuffer CBMVP : register(b1){
    matrix MVPTransform;
};

VSOut main(VSIn vin)
{
    VSOut v;
    v.color = vin.color;
    v.pos = mul(float4(vin.pos, 1.0f),MVPTransform);
    //v.pos = mul(float4(vin.pos, 1.0f),transform);
    v.uv = vin.uv;
    return v;
}
