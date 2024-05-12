struct VertexInput
{
    float4 pos : POSITION;
    float4 col : COLOR;
};

struct PixelInput
{
    float4 svPos : SV_POSITION;
    float4 col : COLOR;
};

PixelInput main(VertexInput input)
{
    
    PixelInput output;
    output.svPos = input.pos;
    output.col = input.col;
    return output;
}