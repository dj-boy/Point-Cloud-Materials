// FileDataLoader�� �ļ���ȡ��

#pragma once


// FileDataLoader��
class FileDataLoader{
private:
    // ˽�л����캯��
    FileDataLoader(){  }
    // ˽�л���������
    ~FileDataLoader(){
        if (m_pData){
            free(m_pData);
            m_pData = nullptr;
        }
    }
public:
    // ��ȡ�ļ�
    bool ReadFile(WCHAR* file_name);
    // ��ȡ����ָ��
    const void* GetData(){ return m_pData; }
    // ��ȡ���ݳ���
    size_t GetLength(){ return m_cLength; }
private:
    // ����ָ��
    void*           m_pData = nullptr;
    // ���ݳ���
    size_t          m_cLength = 0;
    // ʵ�ʳ���
    size_t          m_cLengthReal = 0;
public:
    // ��Ⱦ�̵߳���
    static FileDataLoader s_instanceForRenderThread;
};

#define FileLoader FileDataLoader::s_instanceForRenderThread