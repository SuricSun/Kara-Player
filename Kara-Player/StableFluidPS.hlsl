cbuffer cb0 : register(b0)
{
    float4x4 mvp;
    float4 screen;
    float4 time;
    float4 userData;
};

SamplerState state : register(s0);

RWTexture2D<unorm float4> Pre : register(u0);
RWTexture2D<unorm float4> Cur : register(u1);

void diffuse(RWTexture2D<unorm float4> pre, RWTexture2D<unorm float4> cur, float diff, float dt)
{

}

void affect()
{
    
}

void project()
{
    
}

void velStep(float visc, float dt)
{
    
}

void denStep(float diff, float dt)
{

}

float4 main() : SV_TARGET
{
    float dt = userData.x;
    float diff = userData.y;
    float visc = userData.z;
    RWTexture2D<unorm float4> pre = Pre; 
    RWTexture2D<unorm float4> cur = Cur;
	return float4(1.0f, 1.0f, 1.0f, 1.0f);
}