// ThisApp�� ������ĳ���

#pragma once


// ThisApp��
class ThisApp
{
public:
    // ���캯��
    ThisApp();
    // ��������
    ~ThisApp();
    // �˳�
    void Exit(){ SetEvent(m_hExit); m_threadAudio.join(); }
private:
    // ���ڹ��̺���
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    // ��ʼ��Kinect
    HRESULT init_kinect();
    // ��ʼ������ʶ��
    HRESULT init_speech_recognizer();
    // ��Ƶ�߳�
    static void AudioThread(ThisApp* pointer);
    // ������Ϊ
    void speech_behavior(const SPPHRASEPROPERTY* tag);
    // ��������
    void speech_process();
private:
    // Kinect v2 ������
    IKinectSensor*              m_pKinect = nullptr;
    // ��װ��
    KinectAudioStreamWrapper*   m_p16BitPCMAudioStream = nullptr;
    // ��Ƶ
    IAudioBeam*                 m_pAudioBeam = nullptr;
    // ����ʶ��������
    ISpStream*                  m_pSpeechStream = nullptr;
    // ����ʶ����
    ISpRecognizer*              m_pSpeechRecognizer = nullptr;
    // ����ʶ��������
    ISpRecoContext*             m_pSpeechContext = nullptr;
    // ����ʶ���﷨
    ISpRecoGrammar*             m_pSpeechGrammar = nullptr;
    // ����ʶ�𴥷��¼�
    HANDLE                      m_hSpeechEvent = nullptr;
    // �˳��¼�
    HANDLE                      m_hExit = ::CreateEventW(nullptr, FALSE, FALSE, nullptr);
    // ��Ƶ�����߳�
    std::thread                 m_threadAudio;
    // �﷨�ļ�
    static WCHAR*               s_GrammarFileName;
};