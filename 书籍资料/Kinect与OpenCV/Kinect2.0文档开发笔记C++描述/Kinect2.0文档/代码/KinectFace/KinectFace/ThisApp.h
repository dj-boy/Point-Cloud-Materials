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
	// ��Ϣѭ��
	void RunMessageLoop();
private:
	// ���ڹ��̺���
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    // ��ʼ��Kinect
    HRESULT init_kinect();
    // ����ɫ֡
    void check_color_frame();
    // ������֡
    void check_body_frame();
    // ����沿
    void check_faces();
private:
	// ���ھ��
	HWND			        m_hwnd = nullptr;
    // Kinect v2 ������
    IKinectSensor*          m_pKinect = nullptr;
    // ��ɫ֡��ȡ��
    IColorFrameReader*      m_pColorFrameReader = nullptr;
    // ����֡��ȡ��
    IBodyFrameReader*       m_pBodyFrameReader = nullptr;
    // ��ɫ��֡�¼� ������nullptr��ʼ�� ����
    WAITABLE_HANDLE         m_hColorFrameArrived = 0;
    // ������֡�¼� ������nullptr��ʼ�� ����
    WAITABLE_HANDLE         m_hBodyFrameArrived = 0;
    // �沿֡Դ ��Ϊ��Ҫ��ȡ�������� ���������ﱣ��
    IFaceFrameSource*		m_pFaceFrameSources[BODY_COUNT];
    // �沿֡��ȡ��
    IFaceFrameReader*		m_pFaceFrameReaders[BODY_COUNT];
    // ��������
    IBody*                  m_pBodies[BODY_COUNT];
	// ��Ⱦ��
	ImageRenderer	        m_ImagaRenderer;
};