#pragma once
class DeviceList
{
    UINT32      m_cDevices;
    IMFActivate** m_ppDevices;

public:
    DeviceList() : m_ppDevices(NULL), m_cDevices(0)
    {

    }
    ~DeviceList()
    {
        Clear();
    }

    UINT32  Count() const { return m_cDevices; }

    void    Clear();
    HRESULT EnumerateDevices();
    HRESULT GetDevice(UINT32 index, IMFActivate** ppActivate);
    HRESULT GetDeviceName(UINT32 index, WCHAR** ppszName);
};

