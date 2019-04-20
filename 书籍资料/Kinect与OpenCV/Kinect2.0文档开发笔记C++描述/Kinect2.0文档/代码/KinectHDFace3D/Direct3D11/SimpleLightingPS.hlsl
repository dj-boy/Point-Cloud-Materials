// C Buffer 0 : ����ƽ�й���Ϣ
cbuffer LightMaterialBuffer : register(b0) {
    // ƽ�йⷽ��
    float3 lightDirection   : packoffset(c0.x);
    // �߹�ָ��
    float shininess : packoffset(c0.w);
    // ��Դλ��
    float4 lightPosition: packoffset(c1);
    // ��Դ��ɫ
    float4 lightColor: packoffset(c2);
    // ��Դ�Ļ����ⷴ��ϵ��
    float4 globalAmbient: packoffset(c3);
    // �������λ��
    float4 cameraPosition: packoffset(c4);
    // ���ʵ��Է���
    float4 Ke: packoffset(c5);
    // ���ʵĻ�����ϵ��
    float4 Ka: packoffset(c6);
    // ���ʵ�������ϵ��
    float4 Kd: packoffset(c7);
    // ���ʵĸ߹�ϵ��
    float4 Ks: packoffset(c8);
}

// PS ���� <-> GS ���
struct PSInput
{
    float4 position : SV_POSITION;
    float4 worldposition: POSITION;
    float3 worldnormal: NORMAL;
};

// Shader ���
float4 main(PSInput input) : SV_TARGET{

    float3 P = input.worldposition.xyz;

    float3 N = normalize(input.worldnormal);

    // �Է�����ɫ
    float4 emissive = Ke;

    // ������
    float4 ambient = Ka * globalAmbient;

    // �����������
    // ��LightDirection���Ǵ�ƽ�й�
    // ��Դλ�ü�����λ�ã��ǲ�����˥���ĵ��Դ
    float3 L = normalize(lightPosition.xyz - P);
    float diffuseLight = max(dot(N, L), 0);
    float4 diffuse = Kd * lightColor * diffuseLight;;
    // ����߹�
    float3 V = normalize(cameraPosition.xyz - P);
    float3 H = normalize(L + V);
    float specularLight = pow(max(dot(N, H), 0), shininess);

    if (diffuseLight <= 0){
        specularLight = 0;
    }
    float4 specular = Ks * lightColor * specularLight;
    // �ϳ�
    float4 finalcolor = emissive + ambient + diffuse + specular;

    return finalcolor;
}
