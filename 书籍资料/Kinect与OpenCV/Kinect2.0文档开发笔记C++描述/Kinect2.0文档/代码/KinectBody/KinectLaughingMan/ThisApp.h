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
	// ��ʼ��
	HRESULT Initialize(HINSTANCE hInstance, int nCmdShow);
    // ��Ⱦ����
    static void Render(ThisApp* pThis);
	// ��Ϣѭ��
    void RunMessageLoop();
    // ˢ��
    void Update();
private:
	// ���ڹ��̺���
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    // ��ʼ��Kinect
    HRESULT init_kinect();
    // ����ɫ֡
    void check_color_frame();
    // ������֡
    void check_body_frame();
private:
	// ���ھ��
	HWND			            m_hwnd = nullptr;
    // Kinect v2 ������
    IKinectSensor*              m_pKinect = nullptr;
    // ����ӳ����
    ICoordinateMapper*          m_pCoordinateMapper = nullptr;
    // ��ɫ֡��ȡ��
    IColorFrameReader*          m_pColorFrameReader = nullptr;
    // ����֡��ȡ��
    IBodyFrameReader*           m_pBodyFrameReader = nullptr;
    // ��ɫ֡����
    RGBQUAD*                    m_pColorFrameBuffer = nullptr;
    // �˳�
    BOOL                        m_bExit = FALSE;
	// ��Ⱦ��
	ImageRenderer	            m_ImagaRenderer;
    // ��Ϣ�����߳�
    std::thread                 m_threadMSG;
};