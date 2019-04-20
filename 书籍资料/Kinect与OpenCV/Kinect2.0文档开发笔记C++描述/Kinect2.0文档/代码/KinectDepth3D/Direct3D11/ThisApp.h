// ThisApp�� ������ĳ���

#pragma once
#define TITLE L"Title"

#define WNDWIDTH 1024
#define WNDHEIGHT 768



// ThisApp��
class ThisApp {
public:
    // ���캯��
    ThisApp();
    // ��������
    ~ThisApp();
    // ��Ⱦ����
    static void Render(ThisApp* pThis);
    // ��ʼ��
    HRESULT Initialize(HINSTANCE hInstance, int nCmdShow);
    // ��Ϣѭ��
    void RunMessageLoop();
    // ˢ��
    void Update();
private:
    // ��ʼ��Kinect
    HRESULT init_kinect();
    // ������֡
    void check_depth_frame();
    // ��ʼ����Ϸ����
    void initialize_game_objects();
    // ����ʼ����Ϸ����
    void finalize_game_objects();
    // ���ڹ��̺���
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
private:
    // ��Ⱦ��
    SceneRenderer                       m_SceneRenderer;
    // ��Ⱦ�߳�
    std::thread                         m_threadRender;
    // ���ھ��
    HWND                                m_hwnd = nullptr;
    // Kinect v2 ������
    IKinectSensor*                      m_pKinect = nullptr;
    // ���֡��ȡ��
    IDepthFrameReader*                  m_pDepthFrameReader = nullptr;
    // ����ӳ����
    ICoordinateMapper*                  m_pMapper = nullptr;
    // �˳�
    std::atomic<BOOL>                   m_bExit = FALSE;
    // ��֡���Xλ��
    int                                 m_lastFrameX = 0;
    // ��֡���Yλ��
    int                                 m_lastFrameY = 0;
};
