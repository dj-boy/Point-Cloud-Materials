#include "stdafx.h"
#include "included.h"

#define TITLE L"Title"
#define WNDWIDTH 1024
#define WNDHEIGHT 768

#define lengthof(a) (sizeof(a)/sizeof(*a))

// ThisApp���캯��
ThisApp::ThisApp(){
    m_pAudioBuffer = new BYTE[64 * 1024 *1024];
    if (SUCCEEDED(init_kinect())){
        m_threadAudio.std::thread::~thread();
        m_threadAudio.std::thread::thread(AudioThread, this);
    }
}

// ThisApp��������
ThisApp::~ThisApp(){
    // �����¼�
    if (m_hAudioBeamFrameArrived && m_pAudioBeamFrameReader){
        m_pAudioBeamFrameReader->UnsubscribeFrameArrived(m_hAudioBeamFrameArrived);
        m_hAudioBeamFrameArrived = 0;
    }
    // �ͷ�AudioBeamFrameReader
    SafeRelease(m_pAudioBeamFrameReader);
    // ���ŵعر�Kinect
    if (m_pKinect){
        m_pKinect->Close();
    }
    SafeRelease(m_pKinect);
    if (m_hExit){
        ::CloseHandle(m_hExit);
        m_hExit = nullptr;
    }
    if (m_pAudioBuffer){
        // д�����
        FILE* file = nullptr;
        if (!fopen_s(&file, "temp.raw", "wb")){
            fwrite(m_pAudioBuffer, m_uBufferUsed, 1, file);
            fclose(file);
        }
        delete[] m_pAudioBuffer;
        m_pAudioBuffer = nullptr;
    }
}


// ��Ƶ�߳�
void ThisApp::AudioThread(ThisApp* pointer){
    // ������
    HANDLE events[] = { pointer->m_hExit, reinterpret_cast<HANDLE>(pointer->m_hAudioBeamFrameArrived)};
    bool exit = false;
    while (!exit) {
        switch (::WaitForMultipleObjects(lengthof(events), events, FALSE, INFINITE)){
        case WAIT_OBJECT_0 + 0:
            // �˳�����
            exit = true;
            break;
        case WAIT_OBJECT_0 + 1:
            // ��Ƶ֡����
            pointer->check_audio_frame();
            break;
        }
    }
}

// ��ʼ��Kinect
HRESULT ThisApp::init_kinect(){
    IAudioSource* pAudioSource = nullptr;
    // ���ҵ�ǰĬ��Kinect
    HRESULT hr = ::GetDefaultKinectSensor(&m_pKinect);
    // ��ʿ�ش�Kinect
    if (SUCCEEDED(hr)){
        hr = m_pKinect->Open();
    }
    // ��ȡ��ƵԴ(AudioSource)
    if (SUCCEEDED(hr)){
        hr = m_pKinect->get_AudioSource(&pAudioSource);
    }
    // �ٻ�ȡ��Ƶ֡��ȡ��
    if (SUCCEEDED(hr)){
        hr = pAudioSource->OpenReader(&m_pAudioBeamFrameReader);
    }
    // ע����֡�¼�
    if (SUCCEEDED(hr)){
        m_pAudioBeamFrameReader->SubscribeFrameArrived(&m_hAudioBeamFrameArrived);
    }
    SafeRelease(pAudioSource);
    return hr;
}


// �����Ƶ֡
void ThisApp::check_audio_frame(){
    if (!m_pAudioBeamFrameReader) return;
    // ��Ƶ��֡�¼�����
    IAudioBeamFrameArrivedEventArgs* pArgs = nullptr;
    // ��Ƶ֡����
    IAudioBeamFrameReference* pABFrameRef = nullptr;
    // ��Ƶ֡����
    IAudioBeamFrameList* pAudioBeamFrameList = nullptr;
    // ��Ƶ֡
    IAudioBeamFrame* pAudioBeamFrame = nullptr;
    // ����Ƶ֡����
    UINT32 subframe_count = 0;

    // �����µ���Ƶ֡
    HRESULT hr = m_pAudioBeamFrameReader->GetFrameArrivedEventData(m_hAudioBeamFrameArrived, &pArgs);

    // ��ȡ֡����
    if (SUCCEEDED(hr)) {
        hr = pArgs->get_FrameReference(&pABFrameRef);
    }
    // ��ȡ֡����
    if (SUCCEEDED(hr)) {
        hr = pABFrameRef->AcquireBeamFrames(&pAudioBeamFrameList);
    }
    // ��ȡ��Ƶ֡ Ŀǰ��Ƶ֡����ֻ֧��һ����Ƶ֡�� ֱ�ӻ�ȡ����
    if (SUCCEEDED(hr))  {
        hr = pAudioBeamFrameList->OpenAudioBeamFrame(0, &pAudioBeamFrame);
    }
    // ����Ƶ֡����
    if (SUCCEEDED(hr))  {
        hr = pAudioBeamFrame->get_SubFrameCount(&subframe_count);
    }
    // ������Ƶ
    if (SUCCEEDED(hr) && subframe_count){
        for (UINT i = 0U; i < subframe_count; ++i){
            UINT count = 0; BYTE* data = nullptr;
            IAudioBeamSubFrame* pAudioBeamSubFrame = nullptr;
            // ��ȡ����Ƶ��
            hr = pAudioBeamFrame->GetSubFrame(i, &pAudioBeamSubFrame);
            // ��ȡ��Ƶ��
            if (SUCCEEDED(hr)){
                hr = pAudioBeamSubFrame->AccessUnderlyingBuffer(&count, &data);
            }
            // ��������
            if (SUCCEEDED(hr)){
                memcpy(m_pAudioBuffer + m_uBufferUsed, data, count);
                m_uBufferUsed += count;
            }
            SafeRelease(pAudioBeamSubFrame);
        }
    }
    SafeRelease(pAudioBeamFrame);
    SafeRelease(pAudioBeamFrameList);
    SafeRelease(pABFrameRef);
    SafeRelease(pArgs);
}