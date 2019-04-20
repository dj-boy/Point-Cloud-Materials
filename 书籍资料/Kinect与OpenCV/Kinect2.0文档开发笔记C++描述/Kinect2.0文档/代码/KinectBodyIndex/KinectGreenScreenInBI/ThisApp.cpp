#include "stdafx.h"
#include "included.h"

#define TITLE L"Title"
#define WNDWIDTH 1024
#define WNDHEIGHT 768

#define lengthof(a) sizeof(a)/sizeof(*a)

// ThisApp���캯��
ThisApp::ThisApp(){
    m_pColorCoordinates = new ColorSpacePoint[IMAGE_D_WIDTH*IMAGE_D_HEIGHT];
    m_pColorFrameBuffer = new RGBQUAD[IMAGE_C_WIDTH*IMAGE_C_HEIGHT];
}

// ThisApp��������
ThisApp::~ThisApp(){
    // ������֡�¼�
    if (m_hMultiSourceFrameArrived && m_pMultiSourceFrameReader){
        m_pMultiSourceFrameReader->UnsubscribeMultiSourceFrameArrived(m_hMultiSourceFrameArrived);
        m_hMultiSourceFrameArrived = 0;
    }
    // �ͷ�MultiSourceFrameReader
    SafeRelease(m_pMultiSourceFrameReader);
    // ����ӳ�����ı��¼�
    if (m_hCoordinateMapperChanged && m_pCoordinateMapper){
        m_pCoordinateMapper->UnsubscribeCoordinateMappingChanged(m_hCoordinateMapperChanged);
        m_hCoordinateMapperChanged = 0;
    }
    // �ͷ�ӳ����
    SafeRelease(m_pCoordinateMapper);
    // ���ŵعر�Kinect
    if (m_pKinect){
        m_pKinect->Close();
    }
    SafeRelease(m_pKinect);
    // �ͷ�����
    if (m_pColorCoordinates){
        delete[] m_pColorCoordinates;
        m_pColorCoordinates = nullptr;
    }
    if (m_pColorFrameBuffer){
        delete[] m_pColorFrameBuffer;
        m_pColorFrameBuffer = nullptr;
    }
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
    HANDLE events[] = { 
        reinterpret_cast<HANDLE>(m_hMultiSourceFrameArrived) ,
       // reinterpret_cast<HANDLE>(m_hCoordinateMapperChanged),
    };
    while (true){
        // ��Ϣ����
        if (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)){
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
        // �����¼�
        // �¼�0: ��Դ��֡�¼�
        events[0] = reinterpret_cast<HANDLE>(m_hMultiSourceFrameArrived);
        // �¼�1: ӳ�����ı��¼�
        //events[1] = reinterpret_cast<HANDLE>(m_hCoordinateMapperChanged);
        // ����¼�
        switch (MsgWaitForMultipleObjects(lengthof(events), events, FALSE, INFINITE, QS_ALLINPUT))
        {
            // events[0]
        case WAIT_OBJECT_0 + 0:
            this->check_color_frame();
            break;
#if 0
            // events[1]
        case WAIT_OBJECT_0 + 1:
        {
            ICoordinateMappingChangedEventArgs* pArgs;
            m_pCoordinateMapper->GetCoordinateMappingChangedEventData(m_hCoordinateMapperChanged, &pArgs);
            // ĿǰɶҲ�ɲ���
            
            // ���ͷ���
            SafeRelease(pArgs);
        }
#ifdef _DEBUG
            _cwprintf(L"<ThisApp::RunMessageLoop> Coordinate Mapper Changed.\n");
#endif
            break;
#endif
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
    // ���ҵ�ǰĬ��Kinect
    HRESULT hr = ::GetDefaultKinectSensor(&m_pKinect);
    // ��ʿ�ش�Kinect
    if (SUCCEEDED(hr)){
        hr = m_pKinect->Open();
    }
    // �򿪸�Դ֡��ȡ�� �����ɫ+�������
    if (SUCCEEDED(hr)){
        hr = m_pKinect->OpenMultiSourceFrameReader(
            FrameSourceTypes::FrameSourceTypes_Color | 
            FrameSourceTypes::FrameSourceTypes_BodyIndex |
            FrameSourceTypes::FrameSourceTypes_Depth,
            &m_pMultiSourceFrameReader
            );
    }
    // ע����֡�¼�
    if (SUCCEEDED(hr)){
#ifdef _DEBUG
        // ����Ƿ��Ѵ���
        if (m_hMultiSourceFrameArrived)
            ::MessageBoxW(m_hwnd, L"��Ա����:m_hMultiSourceFrameArrived ֵ�Ѵ���", L"<ThisApp::init_kinect>", MB_ICONERROR);
#endif
        m_pMultiSourceFrameReader->SubscribeMultiSourceFrameArrived(&m_hMultiSourceFrameArrived);
    }
    // ��ȡ����ӳ����
    if (SUCCEEDED(hr)){
        hr = m_pKinect->get_CoordinateMapper(&m_pCoordinateMapper);
    }
    // ע��ӳ�����ı��¼� ��ΪĿǰ���ܸı�ֱ��� ���Ի�����ı�
    if (SUCCEEDED(hr)){
        hr = m_pCoordinateMapper->SubscribeCoordinateMappingChanged(&m_hCoordinateMapperChanged);
    }
    return hr;
}


// ��鸴Դ֡
void ThisApp::check_color_frame(){
    if (!m_pMultiSourceFrameReader) return;
#ifdef _DEBUG
    static int frame_num = 0;
    ++frame_num; 
    _cwprintf(L"<ThisApp::check_color_frame>Frame@%8d ", frame_num);
#endif 
    // ��Դ��֡�¼�����
    IMultiSourceFrameArrivedEventArgs* pArgs = nullptr;
    // ��Դ֡����
    IMultiSourceFrameReference* pMSFrameRef = nullptr;
    // ��Դ֡
    IMultiSourceFrame* pMultiSourceFrame = nullptr;
    // ��ɫ֡
    IColorFrame* pColorFrame = nullptr;
    // ֡����
    IFrameDescription* pColorFrameDescription = nullptr;
    // ��ɫ֡�������
    int width_color = 0;
    // ��ɫ֡�߶�����
    int height_color = 0;
    // ֡��ʽ
    ColorImageFormat imageFormat = ColorImageFormat_None;
    // ��ɫ֡�����С
    UINT nColorBufferSize = 0;
    // ��ɫ֡����
    RGBQUAD *pColorBuffer = nullptr;

    // ��ȡ����
    HRESULT hr = m_pMultiSourceFrameReader->GetMultiSourceFrameArrivedEventData(m_hMultiSourceFrameArrived, &pArgs);
    // ��ȡ����
    if (SUCCEEDED(hr)) {
        hr = pArgs->get_FrameReference(&pMSFrameRef);
    }
    // ��ȡ��Դ֡
    if (SUCCEEDED(hr)) {
        hr = pMSFrameRef->AcquireFrame(&pMultiSourceFrame);
    }
    // ��ȡ��ɫ֡
    if (SUCCEEDED(hr)) {
        IColorFrameReference* pColorFrameReference = nullptr;
        hr = pMultiSourceFrame->get_ColorFrameReference(&pColorFrameReference);
        if (SUCCEEDED(hr)){
            hr = pColorFrameReference->AcquireFrame(&pColorFrame);
        }
        SafeRelease(pColorFrameReference);
    }
    // ��ȡ��ɫ֡����
    if (SUCCEEDED(hr)) {
        hr = pColorFrame->get_FrameDescription(&pColorFrameDescription);
    }
    // ��ȡ��ɫ֡���
    if (SUCCEEDED(hr)) {
        hr = pColorFrameDescription->get_Width(&width_color);
    }
    // ��ȡ��ɫ֡�߶�
    if (SUCCEEDED(hr)) {
        hr = pColorFrameDescription->get_Height(&height_color);
    }
    // ��ȡ֡��ʽ
    if (SUCCEEDED(hr)) {
        hr = pColorFrame->get_RawColorImageFormat(&imageFormat);
    }
    // ��ȡ֡������
    if (SUCCEEDED(hr)) {
        // ���Ѿ���BGRA������� ��ȡ����ָ��
        if (imageFormat == ColorImageFormat_Bgra) {
            hr = pColorFrame->AccessRawUnderlyingBuffer(&nColorBufferSize, reinterpret_cast<BYTE**>(&pColorBuffer));
        }
        // �������Դ��ķ���ת��
        else{
            nColorBufferSize = IMAGE_C_WIDTH * IMAGE_C_HEIGHT * sizeof(RGBQUAD);
            pColorBuffer = m_pColorFrameBuffer;
            hr = pColorFrame->CopyConvertedFrameDataToArray(nColorBufferSize, reinterpret_cast<BYTE*>(pColorBuffer), ColorImageFormat_Bgra);
        }
    }


    // ���֡
    IDepthFrame* pDepthFrame = nullptr;
    // ���֡��������С
    UINT32 nDepthFrameBufferSize = 0;
    // ���֡������λ��
    UINT16* pDepthBuffer = nullptr;
    // ���֡���
    int nDepthFrameWidth = 0;
    // ���֡�߶�
    int nDepthFrameHeight = 0;
    // ���֡����
    IFrameDescription* pDepthFrameDescription = nullptr;
    // ��ȡ�������֡
    if (SUCCEEDED(hr)) {
        IDepthFrameReference* pDepthFrameReference = nullptr;
        hr = pMultiSourceFrame->get_DepthFrameReference(&pDepthFrameReference);
        if (SUCCEEDED(hr)){
            hr = pDepthFrameReference->AcquireFrame(&pDepthFrame);
        }
        SafeRelease(pDepthFrameReference);
    }
    // ��ȡ���֡����
    if (SUCCEEDED(hr)){
        hr = pDepthFrame->get_FrameDescription(&pDepthFrameDescription);
    }
    // ��ȡ���֡���
    if (SUCCEEDED(hr)){
        hr = pDepthFrameDescription->get_Width(&nDepthFrameWidth);
    }
    // ��ȡ���֡�߶�
    if (SUCCEEDED(hr)){
        hr = pDepthFrameDescription->get_Height(&nDepthFrameHeight);
    }
    // ��ȡ��������
    if (SUCCEEDED(hr)){
        hr = pDepthFrame->AccessUnderlyingBuffer(&nDepthFrameBufferSize, &pDepthBuffer);
    }


    // �������֡
    IBodyIndexFrame* pBodyIndexFrame = nullptr;
    // ������������С
    UINT32 nBodyIndexSize = 0;
    // ����������λ��
    BYTE* pIndexBuffer = nullptr;
    // ��ȡ�������֡
    if (SUCCEEDED(hr)) {
        IBodyIndexFrameReference* pBodyIndexFrameReference = nullptr;
        hr = pMultiSourceFrame->get_BodyIndexFrameReference(&pBodyIndexFrameReference);
        if (SUCCEEDED(hr)){
            hr = pBodyIndexFrameReference->AcquireFrame(&pBodyIndexFrame);
        }
        SafeRelease(pBodyIndexFrameReference);
    }
    // ��ȡ��������
    if (SUCCEEDED(hr)){
        pBodyIndexFrame->AccessUnderlyingBuffer(&nBodyIndexSize, &pIndexBuffer);
    }


    // ��������ӳ�� �Բ�ɫ֡Ϊ����
    if (SUCCEEDED(hr)){
       // ����: ��ȡ�ɹ����ٸ�������
#if 0
       static bool called = true;
       if (called){
           hr = m_pCoordinateMapper->MapDepthFrameToColorSpace(
               nDepthFrameWidth * nDepthFrameHeight, (UINT16*)pDepthBuffer, nDepthFrameWidth * nDepthFrameHeight, m_pColorCoordinates
               );
           if (SUCCEEDED(hr))
               called = false;
       }
#else
        hr = m_pCoordinateMapper->MapDepthFrameToColorSpace(
            nDepthFrameWidth * nDepthFrameHeight, (UINT16*)pDepthBuffer, nDepthFrameWidth * nDepthFrameHeight, m_pColorCoordinates
            );
#endif
    }
    // ��������
    if (SUCCEEDED(hr)){
        auto buffer = m_ImagaRenderer.GetBuffer();
        int x, y; RGBQUAD color;
        for (UINT i = 0; i < nDepthFrameBufferSize; ++i){
            ColorSpacePoint colorPoint = m_pColorCoordinates[i];
            x = static_cast<int>(floor(colorPoint.X + .5F));
            y = static_cast<int>(floor(colorPoint.Y + .5F));
            if (pIndexBuffer[i] == 0xFF || x >= width_color || y >= height_color || x < 0 || y < 0){
                color = {0};
            }
            else{
                color = pColorBuffer[y*width_color + x];
            }
            buffer[i] = color;
        }
    }
    // ��������
    if (SUCCEEDED(hr)){
        m_ImagaRenderer.WriteBitmapData(nDepthFrameWidth, nDepthFrameHeight);
    }
    // ��ȫ�ͷ�
    SafeRelease(pDepthFrameDescription);
    SafeRelease(pDepthFrame);


    SafeRelease(pBodyIndexFrame);

    SafeRelease(pColorFrameDescription);
    SafeRelease(pColorFrame);
    SafeRelease(pMultiSourceFrame);
    SafeRelease(pMSFrameRef);
    SafeRelease(pArgs);
#ifdef _DEBUG
    if (SUCCEEDED(hr))
        _cwprintf(L" �ɹ�\n");
    else
        _cwprintf(L" ʧ��\n");
#endif
}