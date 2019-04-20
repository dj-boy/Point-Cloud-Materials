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
    // ������֡
    void check_infrared_frame();
private:
	// ���ھ��
	HWND			        m_hwnd = nullptr;
    // Kinect v2 ������
    IKinectSensor*          m_pKinect = nullptr;
    // ����֡��ȡ��
    IInfraredFrameReader*   m_pInfraredFrameReader = nullptr;
    // ������֡�¼� ������nullptr��ʼ�� ����
    WAITABLE_HANDLE         m_hInfraredFrameArrived = 0;
	// ��Ⱦ��
	ImageRenderer	        m_ImagaRenderer;
};