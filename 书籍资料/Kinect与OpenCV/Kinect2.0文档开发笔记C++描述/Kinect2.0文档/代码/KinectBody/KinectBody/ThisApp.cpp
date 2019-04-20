#include "stdafx.h"
#include "included.h"

#define TITLE L"Title"
#define WNDWIDTH 1024
#define WNDHEIGHT 768

#define lengthof(a) sizeof(a)/sizeof(*a)

// ThisApp���캯��
ThisApp::ThisApp(){

}

// ThisApp��������
ThisApp::~ThisApp(){
    SafeRelease(m_pCoordinateMapper);
    // �����¼�
    if (m_hBodyFrameArrived && m_pBodyFrameReader){
        m_pBodyFrameReader->UnsubscribeFrameArrived(m_hBodyFrameArrived);
        m_hBodyFrameArrived = 0;
    }
    // �ͷ�BodyFrameReader
    SafeRelease(m_pBodyFrameReader);
    // ���ŵعر�Kinect
    if (m_pKinect){
        m_pKinect->Close();
    }
    SafeRelease(m_pKinect);
}

// ��ʼ��
HRESULT ThisApp::Initialize(HINSTANCE hInstance, int nCmdShow){
	HRESULT hr = E_FAIL;
	if (SUCCEEDED(static_cast<HRESULT>(m_ImagaRenderer)))
	{
		//register window class
		WNDCLASSEX wcex = { sizeof(WNDCLASSEX) };
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = ThisApp::WndProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = sizeof(LONG_PTR);
		wcex.hInstance = hInstance;
		wcex.hCursor = LoadCursorW(nullptr, IDC_ARROW);
		wcex.hbrBackground = nullptr;
		wcex.lpszMenuName = nullptr;
		wcex.lpszClassName = L"Direct2DTemplate";
		wcex.hIcon = nullptr;
		// ע�ᴰ��
		RegisterClassEx(&wcex);
		// ��������
		RECT window_rect = { 0, 0, WNDWIDTH, WNDHEIGHT };
		DWORD window_style = WS_CAPTION | WS_VISIBLE | WS_POPUP | WS_SYSMENU |	WS_MINIMIZEBOX;
		AdjustWindowRect(&window_rect, window_style, FALSE);
		AdjustWindowRect(&window_rect, window_style, FALSE);
		window_rect.right -= window_rect.left;
		window_rect.bottom -= window_rect.top;
		window_rect.left = (GetSystemMetrics(SM_CXFULLSCREEN) - window_rect.right) / 2;
		window_rect.top = (GetSystemMetrics(SM_CYFULLSCREEN) - window_rect.bottom) / 2;

		m_hwnd = CreateWindowExW(0, wcex.lpszClassName, TITLE, window_style,
			window_rect.left, window_rect.top, window_rect.right, window_rect.bottom, 0, 0, hInstance, this);
		hr = m_hwnd ? S_OK : E_FAIL;
		if (SUCCEEDED(hr))
		{	
			// ���ô��ھ��
			m_ImagaRenderer.SetHwnd(m_hwnd);
			// ��ʾ����
			ShowWindow(m_hwnd, nCmdShow);
			UpdateWindow(m_hwnd);
		}
	}

	return hr;
}



// ��Ϣѭ��
void ThisApp::RunMessageLoop()
{
	MSG msg;
    HANDLE events[] = { reinterpret_cast<HANDLE>(m_hBodyFrameArrived) };
    while (true){
        // ��Ϣ����
        if (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)){
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
        // �����¼�
        // �¼�0: ��ɫ��֡�¼�
        events[0] = reinterpret_cast<HANDLE>(m_hBodyFrameArrived);
        // ����¼�
        switch (MsgWaitForMultipleObjects(lengthof(events), events, FALSE, INFINITE, QS_ALLINPUT))
        {
            // events[0]
        case WAIT_OBJECT_0 + 0:
            this->check_color_frame();
            break;
        default:
            break;
        }
        // �˳�
        if (msg.message == WM_QUIT){
            break;
        }
    }
}


// ���ڹ��̺���
LRESULT CALLBACK ThisApp::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = 0;

	if (message == WM_CREATE)
	{
		LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
		ThisApp *pOurApp = (ThisApp *)pcs->lpCreateParams;

		::SetWindowLongPtrW(
			hwnd,
			GWLP_USERDATA,
			PtrToUlong(pOurApp)
			);

        // ����ʼ��Kinect
        if (FAILED(pOurApp->init_kinect())){
            ::MessageBoxW(hwnd, L"��ʼ��Kinect v2ʧ��", L"��ĺ��ź�", MB_ICONERROR);
        }
		result = 1;
	}
	else
	{
		ThisApp *pOurApp = reinterpret_cast<ThisApp *>(static_cast<LONG_PTR>(
			::GetWindowLongPtrW(
			hwnd,
			GWLP_USERDATA
			)));

		bool wasHandled = false;
		if (pOurApp)
		{
			switch (message)
			{
			case WM_DISPLAYCHANGE:
				InvalidateRect(hwnd, NULL, FALSE);
				result = 0;
				wasHandled = true;
				break;
            case WM_MOUSEWHEEL:
            
                pOurApp->m_ImagaRenderer.matrix._11 += 0.05f * static_cast<float>(static_cast<short>(HIWORD(wParam))) 
                    / static_cast<float>(WHEEL_DELTA);
                    pOurApp->m_ImagaRenderer.matrix._22 = pOurApp->m_ImagaRenderer.matrix._11;
                pOurApp->m_ImagaRenderer.OnRender();
            
                break;
			case WM_PAINT:
				pOurApp->m_ImagaRenderer.OnRender();
				break;
			case WM_CLOSE:
				// ����β����(�����ȫ�����߳�)��������
				DestroyWindow(hwnd);
				result = 1;
				wasHandled = true;
				break;
			case WM_DESTROY:
				PostQuitMessage(0);
				result = 1;
				wasHandled = true;
				break;
			}
		}

		if (!wasHandled)
		{
			result = DefWindowProc(hwnd, message, wParam, lParam);
		}
	}

	return result;
}


// ��ʼ��Kinect
HRESULT ThisApp::init_kinect(){
    IBodyFrameSource* pBodyFrameSource = nullptr;
    // ���ҵ�ǰĬ��Kinect
    HRESULT hr = ::GetDefaultKinectSensor(&m_pKinect);
    // ��ʿ�ش�Kinect
    if (SUCCEEDED(hr)){
        hr = m_pKinect->Open();
    }
    // ��ȡ��ɫ֡Դ(BodyFrameSource)
    if (SUCCEEDED(hr)){
        hr = m_pKinect->get_BodyFrameSource(&pBodyFrameSource);
    }
    // �ٻ�ȡ��ɫ֡��ȡ��
    if (SUCCEEDED(hr)){
#ifdef _DEBUG
        // ����Ƿ��Ѵ���
        if (m_pBodyFrameReader)
            ::MessageBoxW(m_hwnd, L"��Ա����:m_pBodyFrameReader ֵ�Ѵ���", L"<ThisApp::init_kinect>", MB_ICONERROR);
#endif
        hr = pBodyFrameSource->OpenReader(&m_pBodyFrameReader);
    }
    // ע����֡�¼�
    if (SUCCEEDED(hr)){
#ifdef _DEBUG
        // ����Ƿ��Ѵ���
        if (m_hBodyFrameArrived)
            ::MessageBoxW(m_hwnd, L"��Ա����:m_hBodyFrameArrived ֵ�Ѵ���", L"<ThisApp::init_kinect>", MB_ICONERROR);
#endif
        m_pBodyFrameReader->SubscribeFrameArrived(&m_hBodyFrameArrived);
    }
    // ��ȡ����ӳ����
    if (SUCCEEDED(hr)) {
        hr = m_pKinect->get_CoordinateMapper(&m_pCoordinateMapper);
    }
    SafeRelease(pBodyFrameSource);
    return hr;
}


// ������֡
void ThisApp::check_color_frame(){
    if (!m_pBodyFrameReader) return;
#ifdef _DEBUG
    static int frame_num = 0;
    ++frame_num; 
    _cwprintf(L"<ThisApp::check_color_frame>Frame@%8d ", frame_num);
#endif 
    // ������֡�¼�����
    IBodyFrameArrivedEventArgs* pArgs = nullptr;
    // ����֡����
    IBodyFrameReference* pBFrameRef = nullptr;
    // ����֡
    IBodyFrame* pBodyFrame = nullptr;
    // ����
    IBody*  ppBody[BODY_COUNT] = { 0 };

    // ��ȡ����
    HRESULT hr = m_pBodyFrameReader->GetFrameArrivedEventData(m_hBodyFrameArrived, &pArgs);
    // ��ȡ����
    if (SUCCEEDED(hr)) {
        hr = pArgs->get_FrameReference(&pBFrameRef);
    }
    // ��ȡ����֡
    if (SUCCEEDED(hr)) {
        hr = pBFrameRef->AcquireFrame(&pBodyFrame);
    }
    // ��ȡ��������
    if (SUCCEEDED(hr)) {
        hr = pBodyFrame->GetAndRefreshBodyData(BODY_COUNT, ppBody);
    }
    // ��������
    if (SUCCEEDED(hr)){
        for (int i = 0; i < BODY_COUNT; ++i){
            // ��֤����
            BOOLEAN bTracked = false;
            hr = ppBody[i]->get_IsTracked(&bTracked);
            if (!ppBody[i] || FAILED(hr) || !bTracked) continue;
            IBody* pNowBody = ppBody[i];
            // ��������
            BodyInfo info;
            // ��ȡ������״̬
            pNowBody->get_HandLeftState(&info.leftHandState);
            pNowBody->get_HandRightState(&info.rightHandState);
            // ��ȡ����λ����Ϣ
            hr = pNowBody->GetJoints(JointType_Count, info.joints);
            if (SUCCEEDED(hr)) {
                // ����ת��
                for (int j = 0; j < JointType_Count; ++j)
                    info.jointPoints[j] = BodyToScreen(info.joints[j].Position, WNDWIDTH, WNDHEIGHT);
                // ��������
                m_ImagaRenderer.SetBodyInfo(i, &info);
            }

        }
    }
    // ��Ⱦ
    if (SUCCEEDED(hr)) m_ImagaRenderer.OnRender();
    // ��ȫ�ͷ�
    for (int i = 0; i < BODY_COUNT; ++i) SafeRelease(ppBody[i]);
    SafeRelease(pBodyFrame);
    SafeRelease(pBFrameRef);
    SafeRelease(pArgs);
#ifdef _DEBUG
    if (SUCCEEDED(hr))
        _cwprintf(L" �ɹ�\n");
    else
        _cwprintf(L" ʧ��\n");
#endif
}

// ��������ӳ����Ļ����
D2D1_POINT_2F ThisApp::BodyToScreen(const CameraSpacePoint& bodyPoint, int width, int height){

    DepthSpacePoint depthPoint = { 0 };
    m_pCoordinateMapper->MapCameraPointToDepthSpace(bodyPoint, &depthPoint);

    float screenPointX = static_cast<float>(depthPoint.X * width) / DEPTH_WIDTH;
    float screenPointY = static_cast<float>(depthPoint.Y * height) / DEPTH_HEIGHT;

    return D2D1::Point2F(screenPointX, screenPointY);
}



