#include "stdafx.h"
#include "included.h"


// GestureGroup�๹�캯��
GestureGroup::GestureGroup(std::initializer_list<GestureGroup::Group>& list, wchar_t* name){
    m_cMemberCount = list.size();
    m_pGroup = new GestureGroup::Group[m_cMemberCount];
    auto pGroup = m_pGroup;
    for (auto& i : list){
        *pGroup = i;
        ++pGroup;
    }
    wcscpy_s(m_name, name);
    GESTURE_GET_TIME(m_dwTime);
}



// GestureGroup�� ��������
GestureGroup::~GestureGroup(){
    if (m_pGroup){
        delete[] m_pGroup;
        m_pGroup = nullptr;
        m_cMemberCount = 0;
        m_index = 0;
    }
}


// ˢ��
bool GestureGroup::Updata(GestureDataRequired* data){
#ifdef _DEBUG
    if (!m_pGroup) 
        ::MessageBoxW(nullptr, L"���ڶ���", L"<GestureGroup::Updata> called.", MB_ICONERROR);
#endif
    Group group = m_pGroup[m_cMemberCount];
    // ˢ������
    if (group.pointer->Update(data)){
        // ��ȡʱ��
        DWORD dwNowTime;
        GESTURE_GET_TIME(dwNowTime);
        // �ڶ�����֮��ʱ����
        if (dwNowTime - m_dwTime > group.most && !m_cMemberCount){
            m_cMemberCount = 0;
            m_dwTime = dwNowTime;
            return false;
        }
        // �ڱ�׼��
        if (dwNowTime - m_dwTime > group.least ){
            m_dwTime = dwNowTime;
            if (++m_cMemberCount > m_cMemberCount)
                return true;
        }
    }
    return false;
}