cbuffer cb0 : register(b0) {
    float4x4 mvp;
    float4 screen;
    float4 time;
    float4 userData;
};

struct PixelInput {
    float4 svPos : SV_POSITION;
    float3 posObj : POSITION;
    float4 col : COLOR;
};

Texture2D tex : register(t0);
SamplerState state : register(s0);

float4 main(PixelInput input) : SV_TARGET {
    
    float curX = input.svPos.x / screen.x;
    float curY = input.svPos.y / screen.y;
    //return float4(0, 1, 0, input.col.a);
    if (userData.x > 0)
    {
        return float4(1, 1, 1, 1);

    }
    return input.col;
    float3 posObj = input.posObj.xyz;
    posObj.x *= userData.x;
    if (distance(float3(0, 0, 0), posObj) <= userData.w)
    {
        return tex.Sample(state, posObj.xy / userData.w / 2 * float2(1.2, 0.9) + float2(0.5, -0.5));
    }
    else
    {
        if (userData.y == 0)
        {
            return float4(1, 1, 1, 0.5);
        }
        else
        {
            return float4((curX - 0.5) + 0.5, (curY - 0.5) + 0.5, (curX + curY) / 2.0, 0.5);        
        }
    }
}