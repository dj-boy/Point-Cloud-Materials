#include "stdafx.h"
#include "included.h"

//#define TITLE L"Title"


static const char* header = R"(
#
# Created via Kinect v2 by dustpg. 2014
# Contact dustpg@gmail.com
#

g face_model



)";

// ThisApp���캯��
ThisApp::ThisApp(){

}

// ThisApp��������
ThisApp::~ThisApp(){
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
    // �ͷ�Face Model Builder
    SafeRelease(m_pFaceModelBuilder);
    // �ͷ� ����
    for (int i = 0; i < BODY_COUNT; ++i){
        SafeRelease(m_apBody[i]);
    }
    // ���ŵعر�Kinect
    if (m_pKinect){
        m_pKinect->Close();
        m_pKinect->Release();
        m_pKinect = nullptr;
    }
    // �ͷŻ���
    if (m_pFaceVertices){
        delete[] m_pFaceVertices;
        m_pFaceVertices = nullptr;
    }
}

// ��Ⱦ����
void ThisApp::Render(ThisApp* pThis){
    // ��Ⱦ
    while (true){
        pThis->Update();
        pThis->m_SceneRenderer.OnRender();
        if (pThis->m_bExit) break;
    }
}

// ��ʼ��
HRESULT ThisApp::Initialize(HINSTANCE hInstance, int nCmdShow){
    HRESULT hr = E_FAIL;
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
    wcex.lpszClassName = L"D3D11 Template";
    wcex.hIcon = nullptr;
    // ע�ᴰ��
    RegisterClassEx(&wcex);
    // ��������
    RECT window_rect = { 0, 0, WNDWIDTH, WNDHEIGHT };
    DWORD window_style = WS_CAPTION | WS_VISIBLE | WS_POPUP | WS_SYSMENU | WS_MINIMIZEBOX;
    AdjustWindowRect(&window_rect, window_style, FALSE);
    window_rect.right -= window_rect.left;
    window_rect.bottom -= window_rect.top;
    window_rect.left = (GetSystemMetrics(SM_CXFULLSCREEN) - window_rect.right) / 2;
    window_rect.top = (GetSystemMetrics(SM_CYFULLSCREEN) - window_rect.bottom) / 2;

    m_hwnd = CreateWindowExW(0, wcex.lpszClassName, TITLE, window_style,
        window_rect.left, window_rect.top, window_rect.right, window_rect.bottom, 0, 0, hInstance, this);
    hr = m_hwnd ? S_OK : E_FAIL;
    // ���ô��ھ��
    if (SUCCEEDED(hr)) {
        hr = m_SceneRenderer.SetHwnd(m_hwnd);
    }
#ifdef USING_DIRECTINPUT
    // DInput...
    if (SUCCEEDED(hr)){
        hr = KMInput.Init(hInstance, m_hwnd);
    }
#endif
    // ��ʼ����Ϸ����
    if (SUCCEEDED(hr)){
        this->initialize_game_objects();
    }
    // ��ʾ����
    if (SUCCEEDED(hr)) {
        ShowWindow(m_hwnd, nCmdShow);
        UpdateWindow(m_hwnd);
        m_threadRender.std::thread::~thread();
        m_threadRender.std::thread::thread(Render, this);
    }
#ifdef USING_GAMEIME
    // IME...
    if (SUCCEEDED(hr)){
        m_gameIME.SetHwnd(m_hwnd);
    }
#endif
    return hr;
}



// ��Ϣѭ��
void ThisApp::RunMessageLoop(){
    MSG msg;

    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}


// ���ڹ��̺���
LRESULT CALLBACK ThisApp::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;
#ifdef USING_GAMEIME
    uint32_t temp_input;
#endif

    if (message == WM_CREATE)
    {
        LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
        ThisApp *pOurApp = (ThisApp *)pcs->lpCreateParams;

        ::SetWindowLongPtrW(
            hwnd,
            GWLP_USERDATA,
            PtrToUlong(pOurApp)
            );

        if (SUCCEEDED(pOurApp->init_kinect())){
            result = 1;
        }
        else{
            result = 0;
        }
    }
    else
    {
        ThisApp *pOurApp = reinterpret_cast<ThisApp *>(static_cast<LONG_PTR>(
            ::GetWindowLongPtrW(
            hwnd,
            GWLP_USERDATA
            )));
        bool wasHandled = false;

        if (pOurApp){
#ifdef USING_GAMEIME
            uint32_t flags = pOurApp->m_imeFlag;
#endif
            switch (message)
            {
#ifdef USING_GAMEIME
            case WM_INPUTLANGCHANGE:
                // ���뷨�ı�
                flags |= GameIME_LangChange;
                break;
            case WM_IME_STARTCOMPOSITION:
                pOurApp->m_cInputCount = 0;
                // �̾����뿪ʼ
                flags |= GameIME_StartComposition;
                break;
            case WM_IME_ENDCOMPOSITION:
                // �̾��������
                flags |= GameIME_EndComposition;
                break;
            case WM_IME_NOTIFY:
                // �������ϲ㴦��
                result = 1;
                wasHandled = true;
                flags |= 1 << wParam;
                break;
            case WM_CHAR:
                break;
            case WM_IME_CHAR:
                // ����
                temp_input = pOurApp->m_cInputCount;
                pOurApp->m_mux4Input.lock();
                pOurApp->m_scBuffer4Input[temp_input] = static_cast<WCHAR>(wParam);
                ++temp_input;
                pOurApp->m_scBuffer4Input[temp_input] = 0;
                // ����
                pOurApp->m_mux4Input.unlock();
                pOurApp->m_cInputCount = temp_input;
                break;
#endif
            case WM_KEYDOWN:
                if (wParam == 'S'){
                    // ��������
                    FILE* file = nullptr;
                    size_t now_length = 0;
                    if (!_wfopen_s(&file, L"FaceModel.obj", L"w")){
                        // ����
                        pOurApp->m_muxFaceVertice.lock();
                        // д��ͷ
                        fprintf(file, header);
                        // ��д��������
                        for (UINT i = 0u; i < pOurApp->m_cFaceVerticeCount; ++i){
                            fprintf(file, 
                                "v %f %f %f\n",
                                pOurApp->m_pFaceVertices[i].X,
                                pOurApp->m_pFaceVertices[i].Y,
                                pOurApp->m_pFaceVertices[i].Z
                                );
                        }
                        // ����
                        pOurApp->m_muxFaceVertice.unlock();
                        // ��д������Ϣ
                        fprintf(file, "\n\n\n\n");
                        for (UINT32 i = 0u; i < pOurApp->m_SceneRenderer.face_model.index_count; i += 3){
                            fprintf(file,
                                "f %d %d %d\n",
                                pOurApp->m_SceneRenderer.face_model.index[i + 0] + 1,
                                pOurApp->m_SceneRenderer.face_model.index[i + 1] + 1,
                                pOurApp->m_SceneRenderer.face_model.index[i + 2] + 1
                                );
                        }
                    }
                    // �رճ���
                    if (file){
                        fclose(file);
                        file = nullptr;
                        ::SendMessageW(hwnd, WM_CLOSE, 0, 0);
                    }
                    else{
                        ::MessageBoxW(hwnd, L"����ʧ��", L"Error", MB_ICONERROR);
                    }
                }
                break;
            case WM_CLOSE:
                // ����β����(�����ȫ�����߳�)��������
                pOurApp->m_bExit = TRUE;
                // join
                pOurApp->m_threadRender.join();
                // ����ʼ��
                pOurApp->finalize_game_objects();
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
#ifdef USING_GAMEIME
            pOurApp->m_imeFlag = flags;
#endif
        }
        if (!wasHandled)
        {
            result = DefWindowProc(hwnd, message, wParam, lParam);
        }
    }

    return result;
}


// ˢ�¶���
void ThisApp::Update(){

    check_color_frame();
    check_body_frame();
    check_hd_face_frame();

}


// ��ʼ����Ϸ����
void ThisApp::initialize_game_objects(){
    /*ID3D11Device* device = m_SceneRenderer.Get3DDevice();
    
    SafeRelease(device);*/
}

// ����ʼ����Ϸ����
void ThisApp::finalize_game_objects(){


}




// ��ʼ��Kinect
HRESULT ThisApp::init_kinect(){

    IBodyFrameSource* pBodyFrameSource = nullptr;
    IColorFrameSource* pColorFrameSource = nullptr;
    IFrameDescription* pColorFrameDescription = nullptr;
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
    // ��ȡ��ɫ֡֡����
    if(SUCCEEDED(hr)){
        hr = pColorFrameSource->get_FrameDescription(&pColorFrameDescription);
        // ����D3D �����Ұ ��Kinect ��ɫ�����һ��
        if (pColorFrameDescription){
            int width = 0, height = 0;
            float angle_y=0.f;
            pColorFrameDescription->get_Height(&height);
            pColorFrameDescription->get_Width(&width);
            pColorFrameDescription->get_VerticalFieldOfView(&angle_y);
            // ����͸��ת��
            auto projection = DirectX::XMMatrixPerspectiveFovLH(
                DirectX::XM_PI * angle_y / 180.f,
                static_cast<float>(WNDWIDTH) / (static_cast<float>(WNDHEIGHT)),
                SCREEN_NEAR_Z,
                SCREEN_FAR_Z
                );
            // ͸��
            DirectX::XMStoreFloat4x4(
                &m_SceneRenderer.face_model.vscb.projection,
                projection
                );
        }
    }
    // ��ȡ����֡Դ
    if (SUCCEEDED(hr)) {
        hr = m_pKinect->get_BodyFrameSource(&pBodyFrameSource);
    }
    // ��ȡ����֡��ȡ��
    if (SUCCEEDED(hr)) {
        hr = pBodyFrameSource->OpenReader(&m_pBodyFrameReader);
    }
    // ���������沿֡Դ
    if (SUCCEEDED(hr)){
        hr = CreateHighDefinitionFaceFrameSource(m_pKinect, &m_pHDFaceFrameSource);
    }
    // �����沿ģ�͹����� ֮ǰ���Դ��ļ��ж�ȡ����
    if (SUCCEEDED(hr)){
        hr = m_pHDFaceFrameSource->OpenModelBuilder(FaceModelBuilderAttributes_None, &m_pFaceModelBuilder);
    }
    // ��ʼ�����ռ�
    if (SUCCEEDED(hr)){
        hr = m_pFaceModelBuilder->BeginFaceDataCollection();
    }
    // ���������沿֡��ȡ��
    if (SUCCEEDED(hr)){
        hr = m_pHDFaceFrameSource->OpenReader(&m_pHDFaceFrameReader);
    }
    // �����沿��������
    if (SUCCEEDED(hr)){
        hr = CreateFaceAlignment(&m_pFaceAlignment);
    }
    // �����沿ģ��
    if (SUCCEEDED(hr)){
        hr = CreateFaceModel(1.f, FaceShapeDeformations::FaceShapeDeformations_Count, m_afFSD, &m_pFaceModel);
    }
    // ��ȡӳ����
    if (SUCCEEDED(hr)){
        hr = m_pKinect->get_CoordinateMapper(&m_pMapper);
    }
    // ��ȡ�沿������
    if (SUCCEEDED(hr)){
        hr = GetFaceModelVertexCount(&m_cFaceVerticeCount);
    }
    // �������㻺�� ����������
    if (SUCCEEDED(hr)){
        UINT32 index_size = 0;
        // ��ȡ����������
        GetFaceModelTriangleCount(&index_size);
        // ������������(x 3)
        index_size *= 3;
        m_SceneRenderer.face_model.index_count = index_size;
        // һ���Է���3������
        m_pFaceVertices = reinterpret_cast<CameraSpacePoint*>(malloc(
            (sizeof(CameraSpacePoint) + sizeof(SceneRenderer::VertexNormal))* m_cFaceVerticeCount +
            sizeof(UINT32) * index_size
            ));
        if (!m_pFaceVertices) hr = E_OUTOFMEMORY;
    }
    // 
    if (SUCCEEDED(hr)){
        // �ֽ�1: ����ģ�Ͷ��㻺���ַ
        m_SceneRenderer.face_model.vertex_count = m_cFaceVerticeCount;
        m_SceneRenderer.face_model.face_mash_vertexs = reinterpret_cast<SceneRenderer::VertexNormal*>(
            m_pFaceVertices + m_cFaceVerticeCount
            );
        // �ֽ�2: ����ģ�����������ַ
        m_SceneRenderer.face_model.index = reinterpret_cast<UINT32*>(
            m_SceneRenderer.face_model.face_mash_vertexs + m_cFaceVerticeCount
            );
        // ��ȡ����
        GetFaceModelTriangles(
            m_SceneRenderer.face_model.index_count,
            m_SceneRenderer.face_model.index
            );
        ZeroMemory(
            m_SceneRenderer.face_model.face_mash_vertexs, 
            sizeof(SceneRenderer::VertexNormal)* m_cFaceVerticeCount
            );
    }
    SafeRelease(pColorFrameDescription);
    SafeRelease(pColorFrameSource);
    SafeRelease(pBodyFrameSource);
    return hr;
}


// ����ɫ֡
void ThisApp::check_color_frame(){
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
    // ��ȡ��ɫ֡
    HRESULT  hr = m_pColorFrameReader->AcquireLatestFrame(&pColorFrame);
    if (!pColorFrame) {
        return;
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
            pBuffer = m_SceneRenderer.GetBuffer();
            nBufferSize = IMAGE_WIDTH * IMAGE_HEIGHT * sizeof(RGBQUAD);
            hr = pColorFrame->CopyConvertedFrameDataToArray(nBufferSize, reinterpret_cast<BYTE*>(pBuffer), ColorImageFormat_Bgra);
        }
    }
    // ��������
    if (SUCCEEDED(hr)){
        m_SceneRenderer.WriteBitmapData(pBuffer, width, height);
    }
    // ��ȫ�ͷ�
    SafeRelease(pFrameDescription);
    SafeRelease(pColorFrame);
}



// ������֡
void ThisApp::check_body_frame(){
    // ����֡
    IBodyFrame* pBodyFrame = nullptr;
    HRESULT hr = m_pBodyFrameReader->AcquireLatestFrame(&pBodyFrame);
    if (!pBodyFrame) return;
    // ��ȡ��������
    if (SUCCEEDED(hr)) {
        hr = pBodyFrame->GetAndRefreshBodyData(BODY_COUNT, m_apBody);
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
            hr = m_apBody[i]->get_IsTracked(&tracked);
            if (SUCCEEDED(hr) && tracked){
                UINT64 id = 0;
                if (FAILED(m_apBody[i]->get_TrackingId(&id))) continue;
#ifdef _DEBUG
                _cwprintf(L"����ID: %08X-%08X\n", HIDWORD(id), LODWORD(id));
#endif
                m_pHDFaceFrameSource->put_TrackingId(id);
                break;
            }
        }

    }
    SafeRelease(pBodyFrame);
}


// �������沿֡
void ThisApp::check_hd_face_frame(){
    // �����沿֡
    IHighDefinitionFaceFrame* pHDFaceFrame = nullptr;

    // ��ȡ����֡
    HRESULT hr = m_pHDFaceFrameReader->AcquireLatestFrame(&pHDFaceFrame);
    if (!pHDFaceFrame) 
        return;
    // �����沿��������
    if (SUCCEEDED(hr)){
        hr = pHDFaceFrame->GetAndRefreshFaceAlignmentResult(m_pFaceAlignment);
    }
    // ����沿ģ�͹�����
    if (SUCCEEDED(hr) && !m_bProduced){
        IFaceModelData* pFaceModelData = nullptr;
        // ����ռ�״̬
        hr = m_pFaceModelBuilder->get_CollectionStatus(&m_SceneRenderer.face_model.co_status);
        // ���ɼ�״̬
        if (SUCCEEDED(hr)){
            hr = m_pFaceModelBuilder->get_CaptureStatus(&m_SceneRenderer.face_model.ca_status);
        }
        // �ɼ��ɹ� ��ȡ����
        if (SUCCEEDED(hr) && m_SceneRenderer.face_model.co_status == FaceModelBuilderCollectionStatus_Complete){
            hr = m_pFaceModelBuilder->GetFaceData(&pFaceModelData);
        }
        // �����沿ģ��
        if (SUCCEEDED(hr) && pFaceModelData){
            SafeRelease(m_pFaceModel);
            hr = pFaceModelData->ProduceFaceModel(&m_pFaceModel);
        }
        // �����
        if (SUCCEEDED(hr) && pFaceModelData){
            m_bProduced = TRUE;
            // ˳���������
            m_pFaceModel->GetFaceShapeDeformations(lengthof(m_afFSD), m_afFSD);
        }
        // �ͷŵ�
        SafeRelease(pFaceModelData);
    }
    // ��ȡ�沿����
    if (SUCCEEDED(hr)){
        // ����
        m_muxFaceVertice.lock();
        // ��ȡ����
        hr = m_pFaceModel->CalculateVerticesForAlignment(m_pFaceAlignment, m_cFaceVerticeCount, m_pFaceVertices);
        // ����
        m_muxFaceVertice.unlock();
    }
    // ����DX������
    if (SUCCEEDED(hr)){
        memcpy(
            m_SceneRenderer.face_model.face_mash_vertexs,
            m_pFaceVertices,
            sizeof(DirectX::XMFLOAT3) * m_cFaceVerticeCount
            );
        m_SceneRenderer.RefreshFaceData();
    }
    // ��ȫ�ͷ�
    SafeRelease(pHDFaceFrame);

}