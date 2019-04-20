#include "stdafx.h"
#include "included.h"


// KinectAudioStreamWrapper ���캯��
KinectAudioStreamWrapper::KinectAudioStreamWrapper(IStream *p32BitAudio) :m_p32BitAudio(p32BitAudio){
    // ���Ӽ���
    if (m_p32BitAudio){
        m_p32BitAudio->AddRef();
    }
}


// ��������
KinectAudioStreamWrapper::~KinectAudioStreamWrapper(){
    SafeRelease(m_p32BitAudio);
    if (m_pFloatBuffer){
        delete[] m_pFloatBuffer;
        m_pFloatBuffer = nullptr;
    }
}




// IStream Read������ʵ��
STDMETHODIMP KinectAudioStreamWrapper::Read(void *pBuffer, ULONG cbBuffer, ULONG *pcbRead){
    // �������
    if (!pBuffer || !pcbRead) return E_INVALIDARG;
    // �ڶ�ȡǰδʹ�� m_SpeechActive ����S_OK
    if (!m_SpeechActive){
        *pcbRead = cbBuffer;
        return S_OK;
    }
    HRESULT hr = S_OK;
    // Ŀ���ǽ��������ת����16λPCM����
    INT16* const p16Buffer = reinterpret_cast<INT16*>(pBuffer);
    // ���ȱ���
    const int multiple = sizeof(float) / sizeof(INT16);
    // ��黺�����ͷ��㹻
    auto float_buffer_size = cbBuffer / multiple;
    if (float_buffer_size > m_uFloatBuferSize){
        // ���������������ڴ�
        m_uFloatBuferSize = float_buffer_size;
        if (m_pFloatBuffer) delete[]m_pFloatBuffer;
        m_pFloatBuffer = new float[m_uFloatBuferSize];
    }
    // ������д����� �ֽ�Ϊ��λ
    BYTE* pWriteProgress = reinterpret_cast<BYTE*>(m_pFloatBuffer);
    // Ŀǰ��ȡ��
    ULONG bytesRead = 0;
    // ��Ҫ��ȡ��
    ULONG bytesNeed = cbBuffer * multiple;
    // ѭ����ȡ
    while (true){
        // �Ѿ�����Ҫ�����������
        if (!m_SpeechActive){
            *pcbRead = cbBuffer;
            hr = S_OK;
            break;
        }
        // �Ӱ�װ�����ȡ����
        hr = m_p32BitAudio->Read(pWriteProgress, bytesNeed, &bytesRead);
        bytesNeed -= bytesRead;
        pWriteProgress += bytesRead;
        // ����Ƿ��㹻
        if (!bytesNeed){
            *pcbRead = cbBuffer;
            break;
        }
        // ��Ȼ��˯һ��ʱ��Ƭ��ʱ��
        Sleep(20);
    }
    // ���ݴ��� float -> 16bit PCM
    if (!bytesNeed){
        for (UINT i = 0; i < cbBuffer / multiple; i++) {
            float sample = m_pFloatBuffer[i];
            // ���䱣֤
            //sample = max(min(sample, 1.f), -1.f);
            if (sample > 1.f) sample = 1.f;
            if (sample < -1.f) sample = -1.f;
            // ����ת��
            float sampleScaled = sample * (float)SHRT_MAX;
            p16Buffer[i] = (sampleScaled > 0.f) ? (INT16)(sampleScaled + 0.5f) : (INT16)(sampleScaled - 0.5f);
        }
    }
    return hr;
}

// ��������Ҫ֧�ֵķ���ʵ��

STDMETHODIMP KinectAudioStreamWrapper::Write(const void *, ULONG, ULONG *)
{
    return E_NOTIMPL;
}

STDMETHODIMP KinectAudioStreamWrapper::Seek(LARGE_INTEGER /* dlibMove */, DWORD /* dwOrigin */, ULARGE_INTEGER * /* plibNewPosition */)
{
    // Seek������ʶ�����Ǹ��ȽϹؼ��ĺ��� KinectĿǰ��֧�� ���Ƿ�ֹʧ�ܷ���S_OK
    return S_OK;
}

STDMETHODIMP KinectAudioStreamWrapper::SetSize(ULARGE_INTEGER)
{
    return E_NOTIMPL;
}

STDMETHODIMP KinectAudioStreamWrapper::CopyTo(IStream *, ULARGE_INTEGER, ULARGE_INTEGER *, ULARGE_INTEGER *)
{
    return E_NOTIMPL;
}

STDMETHODIMP KinectAudioStreamWrapper::Commit(DWORD)
{
    return E_NOTIMPL;
}

STDMETHODIMP KinectAudioStreamWrapper::Revert()
{
    return E_NOTIMPL;
}

STDMETHODIMP KinectAudioStreamWrapper::LockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD)
{
    return E_NOTIMPL;
}

STDMETHODIMP KinectAudioStreamWrapper::UnlockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD)
{
    return E_NOTIMPL;
}

STDMETHODIMP KinectAudioStreamWrapper::Stat(STATSTG *, DWORD)
{
    return E_NOTIMPL;
}

STDMETHODIMP KinectAudioStreamWrapper::Clone(IStream **)
{
    return E_NOTIMPL;
}
