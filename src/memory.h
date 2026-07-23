#pragma once
#include <cstdint>
#include <cstddef>
#include <windows.h>

class ProcessMemory {
public:
    ProcessMemory();
    ~ProcessMemory();

    ProcessMemory(const ProcessMemory&) = delete;
    ProcessMemory& operator=(const ProcessMemory&) = delete;
    ProcessMemory(ProcessMemory&& other) noexcept;
    ProcessMemory& operator=(ProcessMemory&& other) noexcept;

    bool Open(const char* processName);
    void Close();

    template<typename T>
    T Read(uintptr_t address) const {
        T value{};
        if (!IsOpen()) {
            return value;
        }
        SIZE_T bytesRead = 0;
        ReadProcessMemory(m_handle, reinterpret_cast<LPCVOID>(address), &value, sizeof(T), &bytesRead);
        return value;
    }

    template<typename T>
    bool ReadArray(uintptr_t address, T* buffer, size_t count) const {
        if (!IsOpen()) return false;
        SIZE_T bytesRead = 0;
        return ReadProcessMemory(m_handle, reinterpret_cast<LPCVOID>(address), buffer,
            count * sizeof(T), &bytesRead) == TRUE
            && bytesRead == count * sizeof(T);
    }

    template<typename T>
    bool Write(uintptr_t address, const T& value) const {
        if (!IsOpen()) return false;
        SIZE_T bytesWritten = 0;
        return WriteProcessMemory(m_handle, reinterpret_cast<LPVOID>(address),
            &value, sizeof(T), &bytesWritten) == TRUE
            && bytesWritten == sizeof(T);
    }

    template<typename T>
    bool WriteArray(uintptr_t address, const T* buffer, size_t count) const {
        if (!IsOpen()) return false;
        SIZE_T bytesWritten = 0;
        return WriteProcessMemory(m_handle, reinterpret_cast<LPVOID>(address),
            buffer, count * sizeof(T), &bytesWritten) == TRUE
            && bytesWritten == count * sizeof(T);
    }

    bool IsOpen() const { return m_handle != nullptr && m_handle != INVALID_HANDLE_VALUE; }
    DWORD GetProcessId() const { return m_pid; }
    HANDLE GetHandle() const { return m_handle; }

private:
    HANDLE m_handle = nullptr;
    DWORD m_pid = 0;
};
