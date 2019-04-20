#include "stdafx.h"
#include "included.h"


// FileDataLoader ��ȡ�ļ�
bool FileDataLoader::ReadFile(WCHAR* file_name){
    bool re = false;
    // ���ļ�
    FILE* file = _wfopen(file_name, L"rb");
    re = reinterpret_cast<size_t>(file) > 0;
    // ��ȡ�ļ���С
    if (re){
        fseek(file, 0L, SEEK_END);
        m_cLength = ftell(file);
        fseek(file, 0L, SEEK_SET);
        if (m_cLength > m_cLengthReal){
            m_cLengthReal = m_cLength;
            if (m_pData)
                free(m_pData);
            m_pData = malloc(m_cLength);
        }
        re = reinterpret_cast<size_t>(m_pData) > 0;
    }
    // ��ȡ�ļ�
    if (re){
        fread(m_pData, 1, m_cLength, file);
    }
    // �ر��ļ�
    if (file){
        fclose(file);
        file = nullptr;
    }
    return re;
}