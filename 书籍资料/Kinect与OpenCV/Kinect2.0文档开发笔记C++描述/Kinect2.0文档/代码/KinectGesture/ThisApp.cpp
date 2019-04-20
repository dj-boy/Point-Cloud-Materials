#include "stdafx.h"
#include "included.h"

#define TITLE L"Title"
#define WNDWIDTH 1024
#define WNDHEIGHT 768

#define lengthof(a) (sizeof(a)/sizeof(*a))

#define THRESHOLD (0.1f)


// ThisApp���캯��
ThisApp::ThisApp(){
    ZeroMemory(m_apBodies, sizeof(m_apBodies));
    if (SUCCEEDED(init_kinect())){
        _cwprintf(L"��ʼ���ɹ�\n");
        while (true){
            // ������֡
            if (WaitForSingleObject(reinterpret_cast<HANDLE>(m_hBodyFrameArrived), 20) == WAIT_OBJECT_0){
                if (FAILED(this->check_body_frame()))
                    continue;
                // ������֡û�и���ʱ���Ը���id
                BOOLEAN tracked = TRUE;
                m_pGestureFrameSource->get_IsTrackingIdValid(&tracked);
                if (tracked) continue;
                UINT64 id = 0;
                for (int i = 0; i < BODY_COUNT; ++i){
                    m_apBodies[i]->get_IsTracked(&tracked);
                    if (tracked){
                        m_apBodies[i]->get_TrackingId(&id);
                        m_pGestureFrameSource->put_TrackingId(id);
                        _cwprintf(L"����ID: %l64d\n", id);
                        break;
                    }
                }
            }
            // ��ȡ���һ֡����
            IVisualGestureBuilderFrame* pGestureFrame = nullptr;
            if (SUCCEEDED(m_pGestureFrameReader->CalculateAndAcquireLatestFrame(&pGestureFrame))){
                // �������������
                IContinuousGestureResult* pContinuousGestureResult = nullptr;
                float progress = 0.f;
                // ѭ�����
                for (UINT i = 0U; i < m_cGestureSize; ++i){
                    // ��ȡ���ƽ��
                    pGestureFrame->get_ContinuousGestureResult(
                        m_apGestures[i],
                        &pContinuousGestureResult
                        );
                    if (pContinuousGestureResult){
                        // ��ʾ���� ��������: ����
                        WCHAR buffer[MAX_PATH];
                        HRESULT hr = m_apGestures[i]->get_Name(lengthof(buffer), buffer);
                        pContinuousGestureResult->get_Progress(&progress);
                        wprintf(L"%32s: progress: %3.03f\n", buffer, progress);
                        // �ͷŶ���
                        pContinuousGestureResult->Release();
                        pContinuousGestureResult = nullptr;
                        // ������Χ���˳�
                        if (progress > (1.f - THRESHOLD)){
                            goto end_of_life;
                        }
                    }
                }
            }
            SafeRelease(pGestureFrame);
        }
    }
end_of_life:
    return;
}

// ThisApp��������
ThisApp::~ThisApp(){
    // ���ŵعر�Kinect
    if (m_pKinect){
        m_pKinect->Close();
    }
    // �ͷŶ���
    SafeRelease(m_pGestureFrameReader);
    // �ͷ�����
    for (UINT i = 0; i < m_cGestureSize; ++i) {
        SafeRelease(m_apGestures[i]);
    }
    // �ͷŹ�������
    for (int i = 0; i < BODY_COUNT; ++i){
        SafeRelease(m_apBodies[i]);
    }
    SafeRelease(m_pGestureFrameSource);
    SafeRelease(m_pGestureDatabase);
    SafeRelease(m_pKinect);
    //�ر��¼�
    if (m_hExit){
        ::CloseHandle(m_hExit);
        m_hExit = nullptr;
    }
}


// ��Ƶ�߳�
/*void ThisApp::AudioThread(ThisApp* pointer){
    // ������
    HANDLE events[] = { pointer->m_hExit, pointer->m_hSpeechEvent };
    bool exit = false;
    while (!exit) {
        switch (::WaitForMultipleObjects(lengthof(events), events, FALSE, INFINITE)){
        case WAIT_OBJECT_0 + 0:
            // �˳�����
            exit = true;
            pointer->m_p16BitPCMAudioStream->SetSpeechState(FALSE);
            break;
        case WAIT_OBJECT_0 + 1:
            // ����ʶ��
            pointer->speech_process();
            break;
        }
    }
}*/

// ��ʼ��Kinect
HRESULT ThisApp::init_kinect(){
#ifdef _DEBUG
    static bool first = false;
    if (first){
        ::MessageBoxW(nullptr, L"��γ�ʼ��", L"<ThisApp::init_kinect>", MB_ICONERROR);
    }
    first = true;
#endif
    IBodyFrameSource* pBodyFrameSource = nullptr;
    // ���ҵ�ǰĬ��Kinect
    HRESULT hr = ::GetDefaultKinectSensor(&m_pKinect);
    // ��ʿ�ش�Kinect
    if (SUCCEEDED(hr)){
        hr = m_pKinect->Open();
    }
    // ��������
    if (SUCCEEDED(hr)){
        hr = CreateVisualGestureBuilderDatabaseInstanceFromFile(L"test.gbd", &m_pGestureDatabase);
    }
    // ��ȡ����֡Դ
    if (SUCCEEDED(hr)){
        hr = m_pKinect->get_BodyFrameSource(&pBodyFrameSource);
    }
    // ��ȡ����֡��ȡ��
    if (SUCCEEDED(hr)){
        hr = pBodyFrameSource->OpenReader(&m_pBodyFrameReader);
    }
    // ע����֡�¼�
    if (SUCCEEDED(hr)){
        hr = m_pBodyFrameReader->SubscribeFrameArrived(&m_hBodyFrameArrived);
    }
    // ��������֡Դ
    if (SUCCEEDED(hr)){
        hr = CreateVisualGestureBuilderFrameSource(m_pKinect, 0, &m_pGestureFrameSource);
    }
    // �������
    if (SUCCEEDED(hr)){
        hr = m_pGestureDatabase->get_AvailableGesturesCount(&m_cGestureSize);
    }
#ifdef _DEBUG
    // ������
    if (m_cGestureSize > lengthof(m_apGestures)){
        assert(!"m_cGestureSize > lengthof(m_apGestures)");
        hr = DISP_E_BUFFERTOOSMALL;
    }
#endif
    // ��ȡ����
    if (SUCCEEDED(hr)){
        hr = m_pGestureDatabase->get_AvailableGestures(m_cGestureSize, m_apGestures);
    }
    // �������
    if (SUCCEEDED(hr)){
        hr = m_pGestureFrameSource->AddGestures(m_cGestureSize, m_apGestures);
    }
    // ��ȡ��ȡ��
    if (SUCCEEDED(hr)){
        hr = m_pGestureFrameSource->OpenReader(&m_pGestureFrameReader);
    }
    SafeRelease(pBodyFrameSource);
    return hr;
}


// ������֡
HRESULT ThisApp::check_body_frame(){
    // ������֡�¼�����
    IBodyFrameArrivedEventArgs* pArgs = nullptr;
    // ����֡����
    IBodyFrameReference* pBFrameRef = nullptr;
    // ����֡
    IBodyFrame* pBodyFrame = nullptr;

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
        hr = pBodyFrame->GetAndRefreshBodyData(BODY_COUNT, m_apBodies);
    }
    SafeRelease(pBodyFrame);
    SafeRelease(pBFrameRef);
    SafeRelease(pArgs);
    return hr;
}