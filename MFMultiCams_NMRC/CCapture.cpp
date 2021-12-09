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


#include "CCapture.h"


//-------------------------------------------------------------------
//  constructor
//-------------------------------------------------------------------

CCapture::CCapture(HWND hwnd) :
    m_pReader(NULL),
    m_pWriter(NULL),
    m_hwndEvent(hwnd),
    m_nRefCount(1),
    m_bFirstSample(FALSE),
    m_llBaseTime(0),
    m_pwszSymbolicLink(NULL)
{
    InitializeCriticalSection(&m_critsec);
}

//-------------------------------------------------------------------
//  destructor
//-------------------------------------------------------------------

CCapture::~CCapture()
{
    assert(m_pReader == NULL);
    assert(m_pWriter == NULL);
    DeleteCriticalSection(&m_critsec);
}



//-------------------------------------------------------------------
// EndCaptureSession
//
// Stop the capture session. 
//
// NOTE: This method resets the object's state to State_NotReady.
// To start another capture session, call SetCaptureFile.
//-------------------------------------------------------------------

HRESULT CCapture::EndCaptureSession()
{
    EnterCriticalSection(&m_critsec);

    HRESULT hr = S_OK;

    if (m_pWriter)
    {
        hr = m_pWriter->Finalize();
    }

    SafeRelease(&m_pWriter);
    SafeRelease(&m_pReader);

    LeaveCriticalSection(&m_critsec);

    return hr;
}

HRESULT CCapture::OpenMediaSource(IMFMediaSource* pSource)
{
    return E_NOTIMPL;
}

HRESULT CCapture::ConfigureCapture(const EncodingParameters& param)
{
    return E_NOTIMPL;
}

HRESULT CCapture::EndCaptureInternal()
{
    return E_NOTIMPL;
}
