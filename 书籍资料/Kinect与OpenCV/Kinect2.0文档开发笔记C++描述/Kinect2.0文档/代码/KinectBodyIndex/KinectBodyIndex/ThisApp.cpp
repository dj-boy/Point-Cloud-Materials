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
    if (m_hBodyIndexFrameArrived && m_pBodyIndexFrameReader){
        m_pBodyIndexFrameReader->UnsubscribeFrameArrived(m_hBodyIndexFrameArrived);
        m_hBodyIndexFrameArrived = 0;
    }
    // �ͷ�BodyIndexFrameReader
    SafeRelease(m_pBodyIndexFrameReader);
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
    HANDLE events[] = { reinterpret_cast<HANDLE>(m_hBodyIndexFrameArrived) };
    while (true){
        // ��Ϣ����
        if (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)){
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
        // �����¼�
        // �¼�0: ���������֡�¼�
        events[0] = reinterpret_cast<HANDLE>(m_hBodyIndexFrameArrived);
        // ����¼�
        switch (MsgWaitForMultipleObjects(lengthof(events), events, FALSE, INFINITE, QS_ALLINPUT))
        {
            // events[0]
        case WAIT_OBJECT_0 + 0:
            this->check_body_index_frame();
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
    IBodyIndexFrameSource* pBodyIndexFrameSource = nullptr;
    // ���ҵ�ǰĬ��Kinect
    HRESULT hr = ::GetDefaultKinectSensor(&m_pKinect);
    // ��ʿ�ش�Kinect
    if (SUCCEEDED(hr)){
        hr = m_pKinect->Open();
    }
    // ��ȡ�������֡Դ(BodyIndexFrameSource)
    if (SUCCEEDED(hr)){
        hr = m_pKinect->get_BodyIndexFrameSource(&pBodyIndexFrameSource);
    }
    // �ٻ�ȡ�������֡��ȡ��
    if (SUCCEEDED(hr)){
#ifdef _DEBUG
        // ����Ƿ��Ѵ���
        if (m_pBodyIndexFrameReader)
            ::MessageBoxW(m_hwnd, L"��Ա����:m_pBodyIndexFrameReader ֵ�Ѵ���", L"<ThisApp::init_kinect>", MB_ICONERROR);
#endif
        hr = pBodyIndexFrameSource->OpenReader(&m_pBodyIndexFrameReader);
    }
    // ע����֡�¼�
    if (SUCCEEDED(hr)){
#ifdef _DEBUG
        // ����Ƿ��Ѵ���
        if (m_hBodyIndexFrameArrived)
            ::MessageBoxW(m_hwnd, L"��Ա����:m_hBodyIndexFrameArrived ֵ�Ѵ���", L"<ThisApp::init_kinect>", MB_ICONERROR);
#endif
        m_pBodyIndexFrameReader->SubscribeFrameArrived(&m_hBodyIndexFrameArrived);
    }
    SafeRelease(pBodyIndexFrameSource);
    return hr;
}


// ����������֡
void ThisApp::check_body_index_frame(){
    if (!m_pBodyIndexFrameReader) return;
#ifdef _DEBUG
    static int frame_num = 0;
    ++frame_num; 
    _cwprintf(L"<ThisApp::check_body_index_frame>Frame@%8d ", frame_num);
#endif 
    // ���������֡�¼�����
    IBodyIndexFrameArrivedEventArgs* pArgs = nullptr;
    // �������֡����
    IBodyIndexFrameReference* pCFrameRef = nullptr;
    // �������֡
    IBodyIndexFrame* pBodyIndexFrame = nullptr;
    // ֡����
    IFrameDescription* pFrameDescription = nullptr;
    // �������֡�������
    int width = 0;
    // �������֡�߶�����
    int height = 0;
    // ���������С
    UINT nBufferSize = 0;
    // ��������
    BYTE *pBuffer = nullptr;

    // ��ȡ����
    HRESULT hr = m_pBodyIndexFrameReader->GetFrameArrivedEventData(m_hBodyIndexFrameArrived, &pArgs);
    // ��ȡ����
    if (SUCCEEDED(hr)) {
        hr = pArgs->get_FrameReference(&pCFrameRef);
    }
    // ��ȡ�������֡
    if (SUCCEEDED(hr)) {
        hr = pCFrameRef->AcquireFrame(&pBodyIndexFrame);
    }
    // ��ȡ֡����
    if (SUCCEEDED(hr)) {
        hr = pBodyIndexFrame->get_FrameDescription(&pFrameDescription);
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
        hr = pBodyIndexFrame->AccessUnderlyingBuffer(&nBufferSize, &pBuffer);
    }
    // ��ȡ���ݴ���
    if (SUCCEEDED(hr)) {
        // ���ӻ���ʾ������Ϣ
        auto* pBGRXBuffer = m_ImagaRenderer.GetBuffer();
        for (UINT i = 0; i < nBufferSize; ++i){
            pBGRXBuffer[i].rgbBlue = pBuffer[i] & 0x01 ? 0x00 : 0xFF;
            pBGRXBuffer[i].rgbGreen = pBuffer[i] & 0x02 ? 0x00 : 0xFF;
            pBGRXBuffer[i].rgbRed = pBuffer[i] & 0x04 ? 0x00 : 0xFF;
            pBGRXBuffer[i].rgbReserved = 0xFF;
        }
        // ��ʾ
        m_ImagaRenderer.WriteBitmapData(width, height);
    }
    // ��ȫ�ͷ�
    SafeRelease(pFrameDescription);
    SafeRelease(pBodyIndexFrame);
    SafeRelease(pCFrameRef);
    SafeRelease(pArgs);
#ifdef _DEBUG
    if (SUCCEEDED(hr))
        _cwprintf(L" �ɹ�\n");
    else
        _cwprintf(L" ʧ��\n");
#endif
}