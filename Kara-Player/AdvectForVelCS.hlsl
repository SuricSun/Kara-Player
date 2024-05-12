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
    
    int i0, j0, i1, j1;
    float x, y, s0, t0, s1, t1, dtx, dty;
    dtx = dt * scrx;
    dty = dt * scry;
    
    x = dtid.x - dtx * pre[dtid.xy].x;
    y = dtid.y - dty * pre[dtid.xy].y;
    // * x
    if (x < 0.5)
        x = 0.5;
    if (x > scrx + 0.5)
        x = scrx + 0.5;
    i0 = (int) x;
    i1 = i0 + 1;
    // * y
    if (y < 0.5)
        y = 0.5;
    if (y > scry + 0.5)
        y = scry + 0.5;
    j0 = (int) y;
    j1 = j0 + 1;
    // * 
    s1 = x - i0;
    s0 = 1 - s1;
    t1 = y - j0;
    t0 = 1 - t1;
    cur[dtid.xy] = s0 * (t0 * pre[float2(i0, j0)] + t1 * pre[float2(i0, j1)]) +
        s1 * (t0 * pre[float2(i1, j0)] + t1 * pre[float2(i1, j1)]);
}