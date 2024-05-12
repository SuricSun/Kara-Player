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
void main( uint3 dtid : SV_DispatchThreadID )
{
    // * calc the corresponding tex this thread need to handle
    float dt = dataf.x;
    float diff = dataf.y;
    float itCnt = datai.x;
    float scrx = screen.x;
    float scry = screen.y;
    
    float a = dt * diff * scrx * scry;
    
    cur[dtid.xy] = (pre[dtid.xy] + a * (
            cur[float2(dtid.x - 1, dtid.y)] + cur[float2(dtid.x + 1, dtid.y)]
            + cur[float2(dtid.x, dtid.y - 1)] + cur[float2(dtid.x, dtid.y + 1)]
    )) / (1.0f + 4.0f * a);
}