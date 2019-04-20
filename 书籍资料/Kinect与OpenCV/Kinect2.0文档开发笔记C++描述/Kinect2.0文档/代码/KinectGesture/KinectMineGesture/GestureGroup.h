// GestureGroup�� ��������� ���Ա�ʾһ��������

#pragma once



// GestureGroup��
class GestureGroup : public GestureBase{
    struct Group{
        // ָ��
        GestureStateBase*   pointer;
        // ����ʱ��
        UINT16              least;
        // ����ʱ��
        UINT16              most;
    };
public:
    // ���캯��
    GestureGroup(std::initializer_list<Group>& list, wchar_t* name);
    // ɾ��Ĭ�Ϲ���
    GestureGroup() = delete;
    // ��������
    virtual ~GestureGroup();
    // ˢ��
    virtual bool Updata(GestureDataRequired* data);
    // ��ȡ����
    virtual GestureType GetType() { return GestureType::TYPE_GROUP; };
    // ��ȡ��������
    virtual wchar_t* GetName(){ return m_name; };
private:
    // ����ָ��
    Group*                  m_pGroup = nullptr;
    // ������
    UINT                    m_cMemberCount = 0;
    // ��ǰ���� Ϊ���Խ���
    UINT                    m_index = 0;
    // ʱ�䵥λ
    DWORD                   m_dwTime = 0;
    // ��������
    wchar_t                 m_name[64];
};
