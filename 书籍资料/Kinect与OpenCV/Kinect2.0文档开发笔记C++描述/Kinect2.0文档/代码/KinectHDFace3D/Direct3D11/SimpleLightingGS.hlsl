// VS ��� <-> GS ����
struct GSInput
{
    float4 position : SV_POSITION;
    float4 worldposition: POSITION;
};

// PS ���� <-> GS ���
struct GSOutput
{
    float4 position : SV_POSITION;
    float4 worldposition: POSITION;
    float3 worldnormal: NORMAL;
};


// ���
[maxvertexcount(3)]
void main(triangle GSInput input[3], inout TriangleStream<GSOutput> OutputStream)
{
    GSOutput output;
    // ����A
    float3 faceEdgeA = input[1].position.xyz - input[0].position.xyz;
    // ����B
    float3 faceEdgeB = input[2].position.xyz - input[0].position.xyz;
    // ���㷨�� 
    output.worldnormal = normalize(cross(faceEdgeA, faceEdgeB));
    for (int i = 0; i < 3; i++){
        output.position = input[i].position;
        output.worldposition = input[i].worldposition;
        OutputStream.Append(output);
    }
    // ����������
    //OutputStream.RestartStrip();
}