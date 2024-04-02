cbuffer cb0 : register(b0) {
    float4x4 mvp;
    float4 screen;
    float4 time;
    float4 userData;
};

struct PixelInput {
    float4 svPos : SV_POSITION;
    float4 col : COLOR;
};

Texture2D tex : register(t0);
SamplerState state : register(s0);


float4 mainold(PixelInput input) : SV_Target {
    
    float4 rgba = float4(0, 0, 0, 0);
    float2 xyStep = float2(1.0f / screen.x, 1.0f / screen.y);
    float2 uv = float2(input.svPos.x / screen.x, input.svPos.y / screen.y);
    int cnt = 1;
    int stepFactor = 2;
    for (int i = -cnt; i <= cnt; i++) {
        rgba += tex.Sample(state, float2(uv.x + i * xyStep.x * stepFactor, uv.y));
    }
    return rgba / (2.0f * cnt + 1.0f);
}

float NeverReach(float val, float max)
{

    return (-1.0f / (16 * val + 1.0f) + 1.0f) * max;
}

float4 main(PixelInput input) : SV_TARGET {
    
    float4 outRgbTearing = float4(0, 0, 0, 0);
    float4 outBlur = float4(0, 0, 0, 0);
    float curX = input.svPos.x / screen.x;
    float curY = input.svPos.y / screen.y;
    float2 uv = float2(curX, curY);
    float onePixRadius = sqrt(pow(1.0f / screen.x, 2.0f) + pow(1.0f / screen.y, 2.0f));
    float sinT = sin(time.x);
    float cosT = cos(time.x);
    float4 oriSample = tex.Sample(state, uv);
    float4 ret = float4(0,0,0,0);
    [unroll]
    for (float i = 0; i < 10; i+=0.5)
    {
        ret += tex.SampleLevel(state, uv, i);
    }
    return ret / (float(i));
    
    //
   // float2 newuv = uv * float2(1920 / 8.0, 1080 / 8.0);
   // newuv = frac(newuv);
   // if (distance(newuv, float2(0.5, 0.5)) <= 0.2)
   // {
     //   outBlur = float4(uv, 1, 1) * oriSample.a;
    //}
   // return outBlur;
    
    //float2 c0 = float2(0.5 + 0.5 * sinT, 0.5 + 0.5 * sinT);
    //float2 c1 = float2(0.5 + 0.5 * cosT, 0.5 + 0.5 * cosT);
    //float radius = 0.01;
    
    //float den = EnergyFieldDensity(c0, radius, uv) + EnergyFieldDensity(c1, radius, uv);
    //float col = step(0.5, den);
    //return float4(col, col, col, col); // * step(0.3, den);
    // * sample rgb
    //rgba.a = 0.0f;
    float tearingOffset = pow(userData.x - 1, 2) * 8;
    outRgbTearing.r = tex.Sample(state, float2(uv.x + onePixRadius * sinT * tearingOffset, uv.y + onePixRadius * cosT * tearingOffset)).r;
    outRgbTearing.g = tex.Sample(state, uv).g;
    outRgbTearing.b = tex.Sample(state, float2(uv.x + onePixRadius * cosT * tearingOffset, uv.y + onePixRadius * sinT * tearingOffset)).b;
    // * sample 3x3 core
    int cnt = 8;
    float2 center = float2(0.5 * sinT + 0.5, 0.5 * cosT + 0.5);
    center = float2(0.5, 0.5);
    float2 outBlurVec = (center - uv) * 0.05; // * 4 * (-1.0f * pow(abs(center.x - uv.x) * 2.0f, 2.0f) + 1.0f);
    //uv -= (cnt / 2) * outBlurVec;
    for (int i = 0; i < cnt; i++) {
        outBlur += tex.Sample(state, uv + outBlurVec);
        uv += outBlurVec / float(cnt);
    }
    
    outBlur /= cnt;
    outRgbTearing /= cnt;
    
    return outBlur + outRgbTearing + oriSample * 0;
}