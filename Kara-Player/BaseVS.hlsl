cbuffer cb0 : register(b0) {
    float4x4 mvp;
    float4 screen;
    float4 time;
    float4 userData;
};

struct VertexInput {
    float3 pos : POSITION;
    float4 col : COLOR;
};

struct PixelInput {
    float4 svPos : SV_POSITION;
    float3 posObj : POSITION;
    float4 col : COLOR;
};

PixelInput main(VertexInput input) {
    
    PixelInput output;
    if (userData.x > 0)
    {
        output.svPos = mul(mvp, float4(input.pos.x, input.pos.y * (0.5 / (input.pos.x + 1)), input.pos.z, 1));
    }
    else
    {
        output.svPos = mul(mvp, float4(input.pos, 1));
    }
    output.posObj = input.pos;
    output.col = input.col;
    return output;
}