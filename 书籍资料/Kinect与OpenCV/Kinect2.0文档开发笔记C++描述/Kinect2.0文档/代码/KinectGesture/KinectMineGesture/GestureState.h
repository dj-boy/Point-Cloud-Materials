// GestureStateBase�� ״̬������ ��ʾ״̬��һ������

#pragma once

// GestureStateBase ��
class GestureStateBase : public GestureBase{
public:
    // ��ȡ����
    virtual GestureType GetType(){ return GestureType::TYPE_STATE; };
    // ��ȡ��������
    virtual wchar_t* GetName(){ return L"GestureState"; };
};


// GestureStateData
struct GestureStateData{
    // �ṹ���С
    UINT    size = sizeof(GestureStateData);
    // �ؽ�����
    Joint   joint[JointType_Count];
    // ������Χ
    FLOAT   float_range[JointType_Count];
    // Ȩ��
    FLOAT   weight[JointType_Count];
    // ��������
    WCHAR   name[64];
};


// GestureState ��
class GestureState : public GestureStateBase {
public:
    // ״̬�ļ�����
    GestureState(wchar_t* file_name);
    // �ڴ湹��
    GestureState(GestureStateData* address);
    // ɾ��Ĭ�Ϲ��캯��
    GestureState() = delete;
    // ˢ��
    virtual bool Update(GestureDataRequired*);
    // ��ȡ��������
    virtual wchar_t* GetName(){ return m_data.name; };
private:
    // ��������
    GestureStateData        m_data;
};