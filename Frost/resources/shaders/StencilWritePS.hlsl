struct VSOutput
{
    float4 position : SV_POSITION;
};

void main(VSOutput input)
{
    // Rien - on veut juste écrire dans le stencil
}