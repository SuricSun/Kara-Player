cbuffer cb0 : register(b0)
{
    float4x4 mvp;
    float4 screen;
    float4 time;
    float4 dataf;
    int4 datai;
};

RWTexture2D<unorm float4> pre : register(u0);
RWTexture2D<unorm float4> cur : register(u1);

[numthreads(32, 32, 1)]
void main(uint3 dtid : SV_DispatchThreadID)
{
    // * calc the corresponding tex this thread need to handle
    float dt = dataf.x;
    float diff = dataf.y;
    float visc = dataf.z;
    float itCnt = datai.x;
    float scrx = screen.x;
    float scry = screen.y;
    
    float dx = 1.0f / scrx;
    float dy = 1.0f / scry;
    
    pre[dtid.xy] = float4(0,
    -0.5f * dy *
        (cur[float2(dtid.x + 1, dtid.y)].x - cur[float2(dtid.x - 1, dtid.y)].x 
        + cur[float2(dtid.x, dtid.y + 1)].y - cur[float2(dtid.x, dtid.y - 1)].y)
    , pre[dtid.xy].zw);
    
    for (int k = 0; k < itCnt; k++)
    {
        pre[dtid.xy] = float4((pre[dtid.xy].x + cur[float2(dtid.x + 1, dtid.y)].y + cur[float2(dtid.x - 1, dtid.y)].y 
            + cur[float2(dtid.x, dtid.y + 1)].y + cur[float2(dtid.x, dtid.y - 1)].y)
                / 4,
        pre[dtid.xy].xyw);
    }

    cur[dtid.xy] -=
    float4(
    0.5f * (pre[float2(dtid.x + 1, dtid.y)].x - pre[float2(dtid.x - 1, dtid.y)].x) / dx,
    0.5f * (pre[float2(dtid.x, dtid.y + 1)].x - pre[float2(dtid.x, dtid.y - 1)].x) / dy,
    0, 0);

}