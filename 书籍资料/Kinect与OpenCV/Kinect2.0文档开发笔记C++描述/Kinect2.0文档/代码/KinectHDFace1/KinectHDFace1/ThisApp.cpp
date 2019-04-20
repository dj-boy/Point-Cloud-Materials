#include "stdafx.h"
#include "included.h"



// ThisApp���캯��
ThisApp::ThisApp(){
}

// ThisApp��������
ThisApp::~ThisApp(){
    // �����¼�
    if (m_hColorFrameArrived && m_pColorFrameReader){
        m_pColorFrameReader->UnsubscribeFrameArrived(m_hColorFrameArrived);
        m_hColorFrameArrived = 0;
    }
    // �ͷ�HighDefinitionFaceFrameReader
    SafeRelease(m_pHDFaceFrameReader);
    // �ͷ�HighDefinitionFaceFrameSource
    SafeRelease(m_pHDFaceFrameSource);
    // �ͷ�ColorFrameReader
    SafeRelease(m_pColorFrameReader);
    // �ͷ�BodyFrameReader
    SafeRelease(m_pBodyFrameReader);
    // �ͷ�FaceAlignment
    SafeRelease(m_pFaceAlignment);
    // �ͷ�Mapper
    SafeRelease(m_pMapper);
    // �ͷ�Face Model
    SafeRelease(m_pFaceModel);
    // ���ŵعر�Kinect
    if (m_pKinect){
        m_pKinect->Close();
        m_pKinect->Release();
        m_pKinect = nullptr;
    }
    // �ͷŻ���
    if (m_pFaceVertices){
        free(m_pFaceVertices);
        m_pFaceVertices = nullptr;
        const_cast<const ColorSpacePoint*>(m_ImagaRenderer.data.face_points) = nullptr;
        const_cast<UINT&>(m_ImagaRenderer.data.face_points_count) = 0;
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
        reinterpret_cast<HANDLE>(m_hColorFrameArrived),
        reinterpret_cast<HANDLE>(m_hBodyFrameArrived),
        reinterpret_cast<HANDLE>(m_hHDFFrameArrived),
        
    };
    while (true){
        // ��Ϣ����
        if (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)){
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
        // �����¼�
        // �¼�0: ��ɫ��֡�¼�
        events[0] = reinterpret_cast<HANDLE>(m_hColorFrameArrived);
        // �¼�1: ������֡�¼�
        events[1] = reinterpret_cast<HANDLE>(m_hBodyFrameArrived);
        // �¼�2: �����沿��֡�¼�
        events[2] = reinterpret_cast<HANDLE>(m_hHDFFrameArrived);
        // ����¼�
        switch (MsgWaitForMultipleObjects(lengthof(events), events, FALSE, INFINITE, QS_ALLINPUT))
        {
            // events[0]
        case WAIT_OBJECT_0 + 0:
            this->check_color_frame();
            break;
            // events[1]
        case WAIT_OBJECT_0 + 1:
            this->check_body_frame();
            break;
            // events[2]
        case WAIT_OBJECT_0 + 2:
            this->check_hd_face_frame();
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
    IColorFrameSource* pColorFrameSource = nullptr;
    // ���ҵ�ǰĬ��Kinect
    HRESULT hr = ::GetDefaultKinectSensor(&m_pKinect);
    // ��ʿ�ش�Kinect
    if (SUCCEEDED(hr)){
        hr = m_pKinect->Open();
    }
    // ��ȡ��ɫ֡Դ(ColorFrameSource)
    if (SUCCEEDED(hr)){
        hr = m_pKinect->get_ColorFrameSource(&pColorFrameSource);
    }
    // �ٻ�ȡ��ɫ֡��ȡ��
    if (SUCCEEDED(hr)){
        hr = pColorFrameSource->OpenReader(&m_pColorFrameReader);
    }
    // ע����֡�¼�
    if (SUCCEEDED(hr)){
        m_pColorFrameReader->SubscribeFrameArrived(&m_hColorFrameArrived);
    }
    // ��ȡ����֡Դ
    if (SUCCEEDED(hr)) {
        hr = m_pKinect->get_BodyFrameSource(&pBodyFrameSource);
    }
    // ��ȡ����֡��ȡ��
    if (SUCCEEDED(hr)) {
        hr = pBodyFrameSource->OpenReader(&m_pBodyFrameReader);
    }
    // ע����֡�¼�
    if (SUCCEEDED(hr)){
        m_pBodyFrameReader->SubscribeFrameArrived(&m_hBodyFrameArrived);
    }
    // ���������沿֡Դ
    if (SUCCEEDED(hr)){
        hr = CreateHighDefinitionFaceFrameSource(m_pKinect, &m_pHDFaceFrameSource);
    }
    // ���������沿֡��ȡ��
    if (SUCCEEDED(hr)){
        hr = m_pHDFaceFrameSource->OpenReader(&m_pHDFaceFrameReader);
    }
    // ע����֡�¼�
    if (SUCCEEDED(hr)){
        hr = m_pHDFaceFrameReader->SubscribeFrameArrived(&m_hHDFFrameArrived);
    }
    // �����沿��������
    if (SUCCEEDED(hr)){
        hr = CreateFaceAlignment(&m_pFaceAlignment);
    }
    // �����沿ģ��
    if (SUCCEEDED(hr)){
        hr = CreateFaceModel(1.f, FaceShapeDeformations::FaceShapeDeformations_Count, m_ImagaRenderer.data.sd, &m_pFaceModel);
    }
    // ��ȡӳ����
    if (SUCCEEDED(hr)){
        hr = m_pKinect->get_CoordinateMapper(&m_pMapper);
    }
    // ��ȡ�沿������
    if (SUCCEEDED(hr)){
        hr = GetFaceModelVertexCount(&m_cFaceVerticeCount);
    }
    // �������㻺��
    if (SUCCEEDED(hr)){
        m_pFaceVertices = reinterpret_cast<CameraSpacePoint*>(malloc(
            (sizeof(CameraSpacePoint) + sizeof(ColorSpacePoint)) * m_cFaceVerticeCount)
            );
        if (!m_pFaceVertices) hr = E_OUTOFMEMORY;
    }
    // �޸�����
    if (SUCCEEDED(hr)){
        const_cast<const ColorSpacePoint*>(m_ImagaRenderer.data.face_points) =
            reinterpret_cast<const ColorSpacePoint*>(m_pFaceVertices + m_cFaceVerticeCount);
        const_cast<UINT&>(m_ImagaRenderer.data.face_points_count) = m_cFaceVerticeCount;
    }
    SafeRelease(pColorFrameSource);
    SafeRelease(pBodyFrameSource);
    return hr;
}


// ����ɫ֡
void ThisApp::check_color_frame(){
    if (!m_pColorFrameReader) return;
    // ��ɫ��֡�¼�����
    IColorFrameArrivedEventArgs* pArgs = nullptr;
    // ��ɫ֡����
    IColorFrameReference* pCFrameRef = nullptr;
    // ��ɫ֡
    IColorFrame* pColorFrame = nullptr;
    // ֡����
    IFrameDescription* pFrameDescription = nullptr;
    // ��ɫ֡�������
    int width = 0;
    // ��ɫ֡�߶�����
    int height = 0;
    // ֡��ʽ
    ColorImageFormat imageFormat = ColorImageFormat_None;
    // ֡�����С
    UINT nBufferSize = 0;
    // ֡����
    RGBQUAD *pBuffer = nullptr;

    // ��ȡ����
    HRESULT hr = m_pColorFrameReader->GetFrameArrivedEventData(m_hColorFrameArrived, &pArgs);
    // ��ȡ����
    if (SUCCEEDED(hr)) {
        hr = pArgs->get_FrameReference(&pCFrameRef);
    }
    // ��ȡ��ɫ֡
    if (SUCCEEDED(hr)) {
        hr = pCFrameRef->AcquireFrame(&pColorFrame);
    }
    // ��ȡ֡����
    if (SUCCEEDED(hr)) {
        hr = pColorFrame->get_FrameDescription(&pFrameDescription);
    }
    // ��ȡ֡���
    if (SUCCEEDED(hr)) {
        hr = pFrameDescription->get_Width(&width);
    }
    // ��ȡ֡�߶�
    if (SUCCEEDED(hr)) {
        hr = pFrameDescription->get_Height(&height);
    }
    // ��ȡ֡��ʽ
    if (SUCCEEDED(hr)) {
        hr = pColorFrame->get_RawColorImageFormat(&imageFormat);
    }
    // ��ȡ֡������
    if (SUCCEEDED(hr)) {
        // ���Ѿ���BGRA������� ֱ�ӻ�ȡԴ����
        if (imageFormat == ColorImageFormat_Bgra) {
            hr = pColorFrame->AccessRawUnderlyingBuffer(&nBufferSize, reinterpret_cast<BYTE**>(&pBuffer));
        }
        // �������Դ��ķ���ת��
        else{
            pBuffer = m_ImagaRenderer.GetBuffer();
            nBufferSize = IMAGE_WIDTH * IMAGE_HEIGHT * sizeof(RGBQUAD);
            hr = pColorFrame->CopyConvertedFrameDataToArray(nBufferSize, reinterpret_cast<BYTE*>(pBuffer), ColorImageFormat_Bgra);
        }
    }
    // ��������
    if (SUCCEEDED(hr)){
        m_ImagaRenderer.WriteBitmapData(pBuffer, width, height);
    }
    // ��ȫ�ͷ�
    SafeRelease(pFrameDescription);
    SafeRelease(pColorFrame);
    SafeRelease(pCFrameRef);
    SafeRelease(pArgs);
}



// ������֡
void ThisApp::check_body_frame(){
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
    // �����沿֡Դδ������ʱ���Ը���ID
    BOOLEAN tracked = FALSE;
    // ����Ƿ�δ������
    if (SUCCEEDED(hr)){
        hr = m_pHDFaceFrameSource->get_IsTrackingIdValid(&tracked);
    }
    // �����沿֡Դδ������ʱ���Ը���ID
    if (SUCCEEDED(hr) && !tracked){
        for (int i = 0; i < BODY_COUNT; ++i) {
            hr = ppBody[i]->get_IsTracked(&tracked);
            if (SUCCEEDED(hr) && tracked){
                UINT64 id = 0;
                if (FAILED(ppBody[i]->get_TrackingId(&id))) continue;
#ifdef _DEBUG
                _cwprintf(L"����ID: %08X-%08X\n", HIDWORD(id), LODWORD(id));
#endif
                m_pHDFaceFrameSource->put_TrackingId(id);
                break;
            }
        }

    }
    // ��ȫ�ͷ�
    for (int i = 0; i < BODY_COUNT; ++i) SafeRelease(ppBody[i]);
    SafeRelease(pBodyFrame);
    SafeRelease(pBFrameRef);
    SafeRelease(pArgs);
}


// �������沿֡
void ThisApp::check_hd_face_frame(){
    m_ImagaRenderer.data.tracked = FALSE;
#ifdef _DEBUG
    static int frame_num = 0;
    ++frame_num;
    _cwprintf(L"<ThisApp::check_hd_face_frame>Frame@%8d ", frame_num);
#endif 
    // �����沿��֡�¼�����
    IHighDefinitionFaceFrameArrivedEventArgs* pArgs = nullptr;
    // �����沿֡����
    IHighDefinitionFaceFrameReference* pHDFFrameRef = nullptr;
    // �����沿֡
    IHighDefinitionFaceFrame* pHDFaceFrame = nullptr;

    // ��ȡ����
    HRESULT hr = m_pHDFaceFrameReader->GetFrameArrivedEventData(m_hHDFFrameArrived, &pArgs);
    // ��ȡ����
    if (SUCCEEDED(hr)) {
        hr = pArgs->get_FrameReference(&pHDFFrameRef);
    }
    // ��ȡ����֡
    if (SUCCEEDED(hr)) {
        hr = pHDFFrameRef->AcquireFrame(&pHDFaceFrame);
    }
    // �����沿��������
    if (SUCCEEDED(hr)){
        hr = pHDFaceFrame->GetAndRefreshFaceAlignmentResult(m_pFaceAlignment);
    }
    // ��ȡ�沿����
    if (SUCCEEDED(hr)){
        hr = m_pFaceModel->CalculateVerticesForAlignment(m_pFaceAlignment, m_cFaceVerticeCount, m_pFaceVertices);
    }
    /*/ ��ȡFace Shape Deformations
    if (SUCCEEDED(hr)){
       hr = m_pFaceModel->GetFaceShapeDeformations(FaceShapeDeformations_Count, m_ImagaRenderer.data.sd);
    }*/
    // �ɹ�
    if (SUCCEEDED(hr)){
        for (UINT i = 0U; i < m_cFaceVerticeCount; ++i){
            m_pMapper->MapCameraPointsToColorSpace(m_cFaceVerticeCount, m_pFaceVertices,
                m_ImagaRenderer.data.face_points_count,
                const_cast<ColorSpacePoint*>(m_ImagaRenderer.data.face_points)
                );
        }
        m_ImagaRenderer.data.tracked = TRUE;
    }
    // ��ȫ�ͷ�
    SafeRelease(pHDFaceFrame);
    SafeRelease(pHDFFrameRef);
    SafeRelease(pArgs);

#ifdef _DEBUG
    if (SUCCEEDED(hr))
        _cwprintf(L" �ɹ�\n");
    else
        _cwprintf(L" ʧ��\n");
#endif
}