#include <new>
#include <windows.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <Wmcodecdsp.h>
#include <assert.h>
#include <Dbt.h>
#include <shlwapi.h>

template <class T> void SafeRelease(T** ppT)
{
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = NULL;
    }
}

#include "CVideo.h"


CVideo::CVideo(HWND hwnd)
{
    InitializeCriticalSection(&m_critsec);
    m_nRefCount = 1;
    m_pReader = NULL;
    m_pwszSymbolicLink = NULL;
    width = 0;
    height = 0;
    rawData = NULL;
    m_bFirstSample = FALSE;
    m_llBaseTime = 0;
    m_hwndEvent = hwnd;
}


CVideo::~CVideo()
{
    assert(m_pReader == NULL);
    DeleteCriticalSection(&m_critsec);
}


HRESULT CVideo::SetSourceReader(IMFActivate* device) {
    HRESULT hr = S_OK;

    IMFMediaSource* pSource = NULL;
    IMFAttributes* pAttributes = NULL;
    IMFMediaType* mediaType = NULL;

    EnterCriticalSection(&m_critsec);

    // Create the media source for the device.
    hr = device->ActivateObject(__uuidof(IMFMediaSource), (void**)&pSource);

    
    // Get the symbolic link for the device. This is needed to handle device-loss notifications. (See CheckDeviceLost.)
    if (SUCCEEDED(hr))
    {
        hr = device->GetAllocatedString(
            MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK,
            &m_pwszSymbolicLink,
            NULL);
    }


    //Allocate attributes
    if (SUCCEEDED(hr))
        hr = MFCreateAttributes(&pAttributes, 2);


    //ser attributes
    if (SUCCEEDED(hr))
        hr = pAttributes->SetUINT32(MF_READWRITE_DISABLE_CONVERTERS, TRUE);

    // Set the callback pointer, operate in asynchronous mode
    if (SUCCEEDED(hr))
        hr = pAttributes->SetUnknown(MF_SOURCE_READER_ASYNC_CALLBACK, this);


    if (SUCCEEDED(hr))
        hr = EnumerateCaptureFormats(pSource);

    //Create the source reader
    if (SUCCEEDED(hr))
        hr = MFCreateSourceReaderFromMediaSource(pSource, pAttributes, &m_pReader);




    // Try to find a suitable output type.
    if (SUCCEEDED(hr))
    {
        for (DWORD i = 0; ; i++)
        {
            hr = m_pReader->GetNativeMediaType((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, i, &mediaType);
            if (FAILED(hr)) { break; }

            hr = IsMediaTypeSupported(mediaType);
            if (FAILED(hr)) { break; }
            //Get width and height
            MFGetAttributeSize(mediaType, MF_MT_FRAME_SIZE, &width, &height);
            if (mediaType)
            {
                mediaType->Release(); 
                mediaType = NULL;
            }

            if (SUCCEEDED(hr))// Found an output type.
                break;
        }
    }


    if (SUCCEEDED(hr))
    {
        m_bFirstSample = TRUE;
        m_llBaseTime = 0;

        // Ask for the first sample.
        hr = m_pReader->ReadSample((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, 0, NULL, NULL, NULL, NULL);
    }

    SafeRelease(&pSource);
    SafeRelease(&pAttributes);
    SafeRelease(&mediaType);
    LeaveCriticalSection(&m_critsec);

    return hr;
}


HRESULT CVideo::IsMediaTypeSupported(IMFMediaType* pType)
{
    HRESULT hr = S_OK;

    BOOL bFound = FALSE;
    GUID subtype = { 0 };

    //Get the stride for this format so we can calculate the number of bytes per pixel
    GetDefaultStride(pType, &stride);

    if (FAILED(hr)) { return hr; }
    hr = pType->GetGUID(MF_MT_SUBTYPE, &subtype);

    videoFormat = subtype;

    if (FAILED(hr)) { return hr; }

    if (subtype == MFVideoFormat_RGB32 || subtype == MFVideoFormat_RGB24 || subtype == MFVideoFormat_YUY2 || subtype == MFVideoFormat_NV12)
        return S_OK;
    else
        return S_FALSE;

    return hr;
}


HRESULT CVideo::GetDefaultStride(IMFMediaType* type, LONG* stride)
{
    LONG tempStride = 0;

    // Try to get the default stride from the media type.
    HRESULT hr = type->GetUINT32(MF_MT_DEFAULT_STRIDE, (UINT32*)&tempStride);
    if (FAILED(hr))
    {
        //Setting this atribute to NULL we can obtain the default stride
        GUID subtype = GUID_NULL;

        UINT32 width = 0;
        UINT32 height = 0;

        // Obtain the subtype
        hr = type->GetGUID(MF_MT_SUBTYPE, &subtype);
        //obtain the width and height
        if (SUCCEEDED(hr))
            hr = MFGetAttributeSize(type, MF_MT_FRAME_SIZE, &width, &height);
        //Calculate the stride based on the subtype and width
        if (SUCCEEDED(hr))
            hr = MFGetStrideForBitmapInfoHeader(subtype.Data1, width, &tempStride);
        // set the attribute so it can be read
        if (SUCCEEDED(hr))
            (void)type ->SetUINT32(MF_MT_DEFAULT_STRIDE, UINT32(tempStride));
    }

    if (SUCCEEDED(hr))
        *stride = tempStride;
    return hr;
}

HRESULT EnumerateCaptureFormats(IMFMediaSource* pSource)
{
    IMFPresentationDescriptor* pPD = NULL;
    IMFStreamDescriptor* pSD = NULL;
    IMFMediaTypeHandler* pHandler = NULL;
    IMFMediaType* pType = NULL;

    DWORD cTypes = 0;

    HRESULT hr = pSource->CreatePresentationDescriptor(&pPD);
    if (FAILED(hr))
    {
        goto done;
    }

    BOOL fSelected;
    hr = pPD->GetStreamDescriptorByIndex(0, &fSelected, &pSD);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = pSD->GetMediaTypeHandler(&pHandler);
    if (FAILED(hr))
    {
        goto done;
    } 
    hr = pHandler->GetMediaTypeCount(&cTypes);
    if (FAILED(hr))
    {
        goto done;
    }

    for (DWORD i = 0; i < cTypes; i++)
    {
        hr = pHandler->GetMediaTypeByIndex(i, &pType);
        if (FAILED(hr))
        {
            goto done;
        }

        //LogMediaType(pType);
        OutputDebugString(L"\n");

        SafeRelease(&pType);
    }

done:
    SafeRelease(&pPD);
    SafeRelease(&pSD);
    SafeRelease(&pHandler);
    SafeRelease(&pType);
    return hr;
}


/////////////// IMFSourceReaderCallback methods ///////////////

//Method from IMFSourceReaderCallback
HRESULT CVideo::OnReadSample(HRESULT status, DWORD streamIndex, DWORD streamFlags, LONGLONG timeStamp, IMFSample* sample)
{
    HRESULT hr = S_OK;

    EnterCriticalSection(&m_critsec);

    if (FAILED(status))
    {
        hr = status;
        goto done;
    }
        

    if (sample)
    {
        if (m_bFirstSample)
        {
            m_llBaseTime = timeStamp;
            m_bFirstSample = FALSE;
        }

        // rebase the time stamp
        timeStamp -= m_llBaseTime;

        hr = sample->SetSampleTime(timeStamp);

        if (FAILED(hr)) { goto done; }
    }

    // Request the next frame.
    if (SUCCEEDED(hr))
        hr = m_pReader ->ReadSample((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, 0, NULL, NULL, NULL, NULL);

   

done:
    if (FAILED(hr))
    {
        NotifyError(hr);
    }
    LeaveCriticalSection(&m_critsec);
    return hr;
}





/////////////// IUnknown methods ///////////////

//-------------------------------------------------------------------
//  AddRef
//-------------------------------------------------------------------

ULONG CVideo::AddRef()
{
    return InterlockedIncrement(&m_nRefCount);
}


//-------------------------------------------------------------------
//  Release
//-------------------------------------------------------------------

ULONG CVideo::Release()
{
    ULONG uCount = InterlockedDecrement(&m_nRefCount);
    if (uCount == 0)
    {
        delete this;
    }
    return uCount;
}


//-------------------------------------------------------------------
//  QueryInterface
//-------------------------------------------------------------------

HRESULT CVideo::QueryInterface(REFIID riid, void** ppv)
{
    static const QITAB qit[] =
    {
        QITABENT(CVideo, IMFSourceReaderCallback),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}
