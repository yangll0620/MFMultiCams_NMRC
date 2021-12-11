#pragma once
class CVideo : public IMFSourceReaderCallback
{
public:
    UINT height;
    UINT width;
    GUID videoFormat;
    BYTE* rawData;
    LONG stride;
    int bytesPerPixel;

    CVideo();
    ~CVideo();


    HRESULT SetSourceReader(IMFActivate* device);
    HRESULT IsMediaTypeSupported(IMFMediaType* pType);
    HRESULT GetDefaultStride(IMFMediaType* type, LONG* stride);


    // the class must implement the methods from IUnknown
    STDMETHODIMP QueryInterface(REFIID iid, void** ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();


    //  the class must implement the methods from IMFSourceReaderCallback
    STDMETHODIMP OnReadSample(HRESULT status, DWORD streamIndex, DWORD streamFlags, LONGLONG timeStamp, IMFSample* sample);
    STDMETHODIMP OnEvent(DWORD, IMFMediaEvent*)
    {
        return S_OK;
    }
    STDMETHODIMP OnFlush(DWORD)
    {
        return S_OK;
    }

protected:
    long m_nRefCount; // Reference count.
    CRITICAL_SECTION        m_critsec;

    WCHAR* m_pwszSymbolicLink;

    IMFSourceReader* m_pReader;
};

