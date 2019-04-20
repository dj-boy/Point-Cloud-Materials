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
    // �����¼�
    if (m_hInfraredFrameArrived && m_pInfraredFrameReader){
        m_pInfraredFrameReader->UnsubscribeFrameArrived(m_hInfraredFrameArrived);
        m_hInfraredFrameArrived = 0;
    }
    // �ͷ�InfraredFrameReader
    SafeRelease(m_pInfraredFrameReader);
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
    HANDLE events[] = { reinterpret_cast<HANDLE>(m_hInfraredFrameArrived) };
    while (true){
        // ��Ϣ����
        if (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)){
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
        // �����¼�
        // �¼�0: ������֡�¼�
        events[0] = reinterpret_cast<HANDLE>(m_hInfraredFrameArrived);
        // ����¼�
        switch (MsgWaitForMultipleObjects(lengthof(events), events, FALSE, INFINITE, QS_ALLINPUT))
        {
            // events[0]
        case WAIT_OBJECT_0 + 0:
            this->check_infrared_frame();
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
    IInfraredFrameSource* pInfraredFrameSource = nullptr;
    // ���ҵ�ǰĬ��Kinect
    HRESULT hr = ::GetDefaultKinectSensor(&m_pKinect);
    // ��ʿ�ش�Kinect
    if (SUCCEEDED(hr)){
        hr = m_pKinect->Open();
    }
    // ��ȡ����֡Դ(InfraredFrameSource)
    if (SUCCEEDED(hr)){
        hr = m_pKinect->get_InfraredFrameSource(&pInfraredFrameSource);
    }
    // �ٻ�ȡ����֡��ȡ��
    if (SUCCEEDED(hr)){
#ifdef _DEBUG
        // ����Ƿ��Ѵ���
        if (m_pInfraredFrameReader)
            ::MessageBoxW(m_hwnd, L"��Ա����:m_pInfraredFrameReader ֵ�Ѵ���", L"<ThisApp::init_kinect>", MB_ICONERROR);
#endif
        hr = pInfraredFrameSource->OpenReader(&m_pInfraredFrameReader);
    }
    // ע����֡�¼�
    if (SUCCEEDED(hr)){
#ifdef _DEBUG
        // ����Ƿ��Ѵ���
        if (m_hInfraredFrameArrived)
            ::MessageBoxW(m_hwnd, L"��Ա����:m_hInfraredFrameArrived ֵ�Ѵ���", L"<ThisApp::init_kinect>", MB_ICONERROR);
#endif
        m_pInfraredFrameReader->SubscribeFrameArrived(&m_hInfraredFrameArrived);
    }
    SafeRelease(pInfraredFrameSource);
    return hr;
}


// ������֡
void ThisApp::check_infrared_frame(){
    if (!m_pInfraredFrameReader) return;
#ifdef _DEBUG
    static int frame_num = 0;
    ++frame_num; 
    _cwprintf(L"<ThisApp::check_infrared_frame>Frame@%8d ", frame_num);
#endif 
    // ������֡�¼�����
    IInfraredFrameArrivedEventArgs* pArgs = nullptr;
    // ����֡����
    IInfraredFrameReference* pIRFrameRef = nullptr;
    // ����֡
    IInfraredFrame* pInfraredFrame = nullptr;
    // ֡����
    IFrameDescription* pFrameDescription = nullptr;
    // ����֡�������
    int width = 0;
    // ����֡�߶�����
    int height = 0;
    // ֡�����С
    UINT nBufferSize = 0;
    // ֡����
    UINT16 *pBuffer = nullptr;

    // ��ȡ����
    HRESULT hr = m_pInfraredFrameReader->GetFrameArrivedEventData(m_hInfraredFrameArrived, &pArgs);
    // ��ȡ����
    if (SUCCEEDED(hr)) {
        hr = pArgs->get_FrameReference(&pIRFrameRef);
    }
    // ��ȡ����֡
    if (SUCCEEDED(hr)) {
        hr = pIRFrameRef->AcquireFrame(&pInfraredFrame);
    }
    // ��ȡ֡����
    if (SUCCEEDED(hr)) {
        hr = pInfraredFrame->get_FrameDescription(&pFrameDescription);
    }
    // ��ȡ֡���
    if (SUCCEEDED(hr)) {
        hr = pFrameDescription->get_Width(&width);
    }
    // ��ȡ֡�߶�
    if (SUCCEEDED(hr)) {
        hr = pFrameDescription->get_Height(&height);
    }
    // ��ȡ��������
    if (SUCCEEDED(hr)) {
        hr = pInfraredFrame->AccessUnderlyingBuffer(&nBufferSize, &pBuffer);
    }
    // ������������
    if (SUCCEEDED(hr)){
        auto pRGBXBuffer = m_ImagaRenderer.GetBuffer();
        for (UINT i = 0U; i < nBufferSize; ++i){
            pRGBXBuffer[i].rgbBlue = static_cast<BYTE>(pBuffer[i] >> 8);
            pRGBXBuffer[i].rgbGreen = pRGBXBuffer[i].rgbBlue;
            pRGBXBuffer[i].rgbRed = pRGBXBuffer[i].rgbBlue;
            pRGBXBuffer[i].rgbReserved = 0xFF;
        }
        m_ImagaRenderer.WriteBitmapData(width, height);
    }
    // ��ȫ�ͷ�
    SafeRelease(pFrameDescription);
    SafeRelease(pInfraredFrame);
    SafeRelease(pIRFrameRef);
    SafeRelease(pArgs);
#ifdef _DEBUG
    if (SUCCEEDED(hr))
        _cwprintf(L" �ɹ�\n");
    else
        _cwprintf(L" ʧ��\n");
#endif
}