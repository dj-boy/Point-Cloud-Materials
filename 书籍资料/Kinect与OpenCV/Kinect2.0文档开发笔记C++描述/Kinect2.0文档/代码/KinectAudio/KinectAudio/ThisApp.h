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
    // �����Ƶ֡
    void check_audio_frame();
    // ��Ƶ�߳�
    static void AudioThread(ThisApp* pointer);
private:
    // Kinect v2 ������
    IKinectSensor*          m_pKinect = nullptr;
    // ��Ƶ֡��ȡ��
    IAudioBeamFrameReader*  m_pAudioBeamFrameReader = nullptr;
    // ��Ƶ��֡�¼� ������nullptr��ʼ�� ����
    WAITABLE_HANDLE         m_hAudioBeamFrameArrived = 0;
    // �˳��¼�
    HANDLE                  m_hExit = ::CreateEventW(nullptr, FALSE, FALSE, nullptr);
    // ��Ƶ������
    BYTE*                   m_pAudioBuffer = nullptr;
    // ��ǰд�����
    UINT32                  m_uBufferUsed = 0;
    // ��Ƶ�����߳�
    std::thread             m_threadAudio;
};