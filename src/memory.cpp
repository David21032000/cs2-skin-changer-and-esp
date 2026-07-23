#include "memory.h"
#include "process.h"

ProcessMemory::ProcessMemory()
    : m_handle(nullptr)
    , m_pid(0) {
}

ProcessMemory::~ProcessMemory() {
    Close();
}

ProcessMemory::ProcessMemory(ProcessMemory&& other) noexcept
    : m_handle(other.m_handle)
    , m_pid(other.m_pid) {
    other.m_handle = nullptr;
    other.m_pid = 0;
}

ProcessMemory& ProcessMemory::operator=(ProcessMemory&& other) noexcept {
    if (this != &other) {
        Close();
        m_handle = other.m_handle;
        m_pid = other.m_pid;
        other.m_handle = nullptr;
        other.m_pid = 0;
    }
    return *this;
}

bool ProcessMemory::Open(const char* processName) {
    Close();

    m_pid = Process::FindProcessId(processName);
    if (m_pid == 0) {
        return false;
    }

    m_handle = OpenProcess(
        PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION | PROCESS_QUERY_INFORMATION,
        FALSE,
        m_pid
    );

    return m_handle != nullptr;
}

void ProcessMemory::Close() {
    if (m_handle && m_handle != INVALID_HANDLE_VALUE) {
        CloseHandle(m_handle);
    }
    m_handle = nullptr;
    m_pid = 0;
}
