cbuffer cb0 : register(b0) {
    float4x4 mvp;
    float4 screen;
    float4 time;
};

struct PixelInput {
    float4 svPos : SV_POSITION;
    float4 col : COLOR;
};

Texture2D tex : register(t0);
SamplerState state : register(s0);

float4 main(PixelInput input) : SV_Target {
    
    float curX = input.svPos.x / screen.x;
    float curY = input.svPos.y / screen.y;
    float2 uv = float2(curX, curY);
    return tex.Sample(state, uv);
}