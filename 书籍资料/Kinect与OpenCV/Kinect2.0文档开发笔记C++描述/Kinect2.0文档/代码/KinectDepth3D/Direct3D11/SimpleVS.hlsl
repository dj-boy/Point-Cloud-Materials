// C Buffer 0 : ����任����
cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};


// VS ���� <-> ��������
struct VSInput
{
    // λ�� �Զ����뵽16�ֽ�
    float4 position : POSITION;
};

// VS ��� <-> PS ����
struct VSOutput
{
    float4 position : SV_POSITION;
    float4 raw_position: POSITION;
};

// Shader ���
VSOutput main(VSInput input)
{
    VSOutput output;
    float4 worldPosition;
    // �ı䶥��Ϊ�ĸ������������.
    input.position.w = 1.0f;

    // ����ת��
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);


    // ԭλ��
    output.raw_position = input.position;

    return output;
}

