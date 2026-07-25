#include "auth.h"
#define _WIN32_WINNT 0x0600
#define TEST_BUILD
#include <windows.h>
#include <iphlpapi.h>
#include <winhttp.h>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <cstring>
#include <vector>
#include <iostream>
#include <shlobj.h>
#include <filesystem>

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "winhttp.lib")

namespace fs = std::filesystem;

// ── SHA256 Implementation ─────────────────────────────────────
namespace SHA256 {
    static const uint32_t K[64] = {
        0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
        0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
        0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
        0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
        0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
        0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
        0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
        0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
        0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
        0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
        0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
        0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
        0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
        0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
        0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
        0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
    };

    static inline uint32_t rotr(uint32_t x, uint32_t n) {
        return (x >> n) | (x << (32 - n));
    }

    static inline uint32_t ch(uint32_t x, uint32_t y, uint32_t z) {
        return (x & y) ^ (~x & z);
    }

    static inline uint32_t maj(uint32_t x, uint32_t y, uint32_t z) {
        return (x & y) ^ (x & z) ^ (y & z);
    }

    static inline uint32_t sigma0(uint32_t x) {
        return rotr(x, 2) ^ rotr(x, 13) ^ rotr(x, 22);
    }

    static inline uint32_t sigma1(uint32_t x) {
        return rotr(x, 6) ^ rotr(x, 11) ^ rotr(x, 25);
    }

    static inline uint32_t gamma0(uint32_t x) {
        return rotr(x, 7) ^ rotr(x, 18) ^ (x >> 3);
    }

    static inline uint32_t gamma1(uint32_t x) {
        return rotr(x, 17) ^ rotr(x, 19) ^ (x >> 10);
    }

    struct Context {
        uint32_t state[8];
        uint64_t bitCount;
        uint8_t buffer[64];
        uint32_t bufferLen;
    };

    static void init(Context& ctx) {
        ctx.state[0] = 0x6a09e667;
        ctx.state[1] = 0xbb67ae85;
        ctx.state[2] = 0x3c6ef372;
        ctx.state[3] = 0xa54ff53a;
        ctx.state[4] = 0x510e527f;
        ctx.state[5] = 0x9b05688c;
        ctx.state[6] = 0x1f83d9ab;
        ctx.state[7] = 0x5be0cd19;
        ctx.bitCount = 0;
        ctx.bufferLen = 0;
    }

    static void transform(Context& ctx, const uint8_t* block) {
        uint32_t W[64];
        for (int i = 0; i < 16; i++) {
            W[i] = ((uint32_t)block[i * 4] << 24) |
                   ((uint32_t)block[i * 4 + 1] << 16) |
                   ((uint32_t)block[i * 4 + 2] << 8) |
                   ((uint32_t)block[i * 4 + 3]);
        }
        for (int i = 16; i < 64; i++) {
            W[i] = gamma1(W[i - 2]) + W[i - 7] + gamma0(W[i - 15]) + W[i - 16];
        }

        uint32_t a = ctx.state[0];
        uint32_t b = ctx.state[1];
        uint32_t c = ctx.state[2];
        uint32_t d = ctx.state[3];
        uint32_t e = ctx.state[4];
        uint32_t f = ctx.state[5];
        uint32_t g = ctx.state[6];
        uint32_t h = ctx.state[7];

        for (int i = 0; i < 64; i++) {
            uint32_t T1 = h + sigma1(e) + ch(e, f, g) + K[i] + W[i];
            uint32_t T2 = sigma0(a) + maj(a, b, c);
            h = g;
            g = f;
            f = e;
            e = d + T1;
            d = c;
            c = b;
            b = a;
            a = T1 + T2;
        }

        ctx.state[0] += a;
        ctx.state[1] += b;
        ctx.state[2] += c;
        ctx.state[3] += d;
        ctx.state[4] += e;
        ctx.state[5] += f;
        ctx.state[6] += g;
        ctx.state[7] += h;
    }

    static void update(Context& ctx, const uint8_t* data, size_t len) {
        for (size_t i = 0; i < len; i++) {
            ctx.buffer[ctx.bufferLen++] = data[i];
            ctx.bitCount += 8;
            if (ctx.bufferLen == 64) {
                transform(ctx, ctx.buffer);
                ctx.bufferLen = 0;
            }
        }
    }

    static void finalize(Context& ctx, uint8_t* hash) {
        size_t index = ctx.bufferLen;
        ctx.buffer[index++] = 0x80;
        if (index > 56) {
            while (index < 64) ctx.buffer[index++] = 0;
            transform(ctx, ctx.buffer);
            index = 0;
        }
        while (index < 56) ctx.buffer[index++] = 0;

        uint64_t bits = ctx.bitCount;
        ctx.buffer[56] = (uint8_t)(bits >> 56);
        ctx.buffer[57] = (uint8_t)(bits >> 48);
        ctx.buffer[58] = (uint8_t)(bits >> 40);
        ctx.buffer[59] = (uint8_t)(bits >> 32);
        ctx.buffer[60] = (uint8_t)(bits >> 24);
        ctx.buffer[61] = (uint8_t)(bits >> 16);
        ctx.buffer[62] = (uint8_t)(bits >> 8);
        ctx.buffer[63] = (uint8_t)(bits);
        transform(ctx, ctx.buffer);

        for (int i = 0; i < 8; i++) {
            hash[i * 4] = (uint8_t)(ctx.state[i] >> 24);
            hash[i * 4 + 1] = (uint8_t)(ctx.state[i] >> 16);
            hash[i * 4 + 2] = (uint8_t)(ctx.state[i] >> 8);
            hash[i * 4 + 3] = (uint8_t)(ctx.state[i]);
        }
    }

    static std::string Hash(const std::string& input) {
        Context ctx;
        init(ctx);
        update(ctx, (const uint8_t*)input.data(), input.size());
        uint8_t hash[32];
        finalize(ctx, hash);
        std::stringstream ss;
        for (int i = 0; i < 32; i++) {
            ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
        }
        return ss.str();
    }
}

// ── XOR Encryption ────────────────────────────────────────────
static const char XOR_KEY[] = "C4musCh3at!2024#SecureXorKey";
static const size_t XOR_KEY_LEN = sizeof(XOR_KEY) - 1;

static void XOREncryptDecrypt(std::vector<uint8_t>& data) {
    for (size_t i = 0; i < data.size(); i++) {
        data[i] ^= XOR_KEY[i % XOR_KEY_LEN];
    }
}

static std::string GetAuthDir() {
    char path[MAX_PATH];
    GetModuleFileNameA(GetModuleHandleA(nullptr), path, MAX_PATH);
    std::string full = path;
    size_t pos = full.find_last_of("\\/");
    if (pos != std::string::npos)
        full = full.substr(0, pos);
    return full + "\\camus\\";
}

// ── GetHWID ───────────────────────────────────────────────────
std::string GetHWID() {
    std::string hwid;

    // Volume serial
    char volumeName[MAX_PATH + 1] = { 0 };
    DWORD serial = 0;
    if (GetVolumeInformationA("C:\\", volumeName, MAX_PATH, &serial, nullptr, nullptr, nullptr, 0)) {
        hwid += std::to_string(serial);
    }
    hwid += "-";

    // Username
    char username[UNLEN + 1];
    DWORD userLen = UNLEN + 1;
    if (GetUserNameA(username, &userLen)) {
        hwid += username;
    }

    // Computer name
    char compName[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD compLen = MAX_COMPUTERNAME_LENGTH + 1;
    if (GetComputerNameA(compName, &compLen)) {
        hwid += compName;
    }

    return SHA256::Hash(hwid);
}

// ── VerifyKey (WinHTTP POST) ──────────────────────────────────
bool VerifyKey(const std::string& key) {
#ifdef TEST_BUILD
    return true;
#endif
    std::string hwid = GetHWID();
    std::string postData = "{\"key\":\"" + key + "\",\"hwid\":\"" + hwid + "\"}";

    HINTERNET hSession = WinHttpOpen(L"CAMUS Cheat/1.0",
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, nullptr, nullptr, 0);
    if (!hSession) return false;

    HINTERNET hConnect = WinHttpConnect(hSession, L"127.0.0.1",
        8080, 0);
    if (!hConnect) {
        WinHttpCloseHandle(hSession);
        return false;
    }

    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"POST",
        L"/api/verify", nullptr, nullptr, nullptr,
        0);
    if (!hRequest) {
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return false;
    }

    LPCWSTR headers = L"Content-Type: application/json\r\n";
    BOOL sent = WinHttpSendRequest(hRequest, headers, wcslen(headers),
        (LPVOID)postData.c_str(), (DWORD)postData.size(),
        (DWORD)postData.size(), 0);

    bool result = false;
    if (sent && WinHttpReceiveResponse(hRequest, nullptr)) {
        DWORD statusCode = 0;
        DWORD statusSize = sizeof(statusCode);
        if (WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE |
            WINHTTP_QUERY_FLAG_NUMBER, nullptr, &statusCode, &statusSize, nullptr)) {
            if (statusCode == 200) {
                DWORD bytesRead = 0;
                std::string response;
                char buffer[1024];
                while (WinHttpReadData(hRequest, buffer, sizeof(buffer) - 1, &bytesRead) && bytesRead > 0) {
                    buffer[bytesRead] = '\0';
                    response += buffer;
                }
                if (response.find("\"success\":true") != std::string::npos ||
                    response.find("\"valid\":true") != std::string::npos ||
                    response.find("\"status\":\"ok\"") != std::string::npos) {
                    result = true;
                }
            }
        }
    }

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);
    return result;
}

// ── Auth File Management ──────────────────────────────────────
static bool WriteAuthFile(const std::string& data) {
    std::string dir = GetAuthDir();
    fs::create_directories(dir);
    std::string path = dir + "auth.dat";
    std::vector<uint8_t> bytes(data.begin(), data.end());
    XOREncryptDecrypt(bytes);
    FILE* f = nullptr;
    if (fopen_s(&f, path.c_str(), "wb") != 0 || !f) return false;
    fwrite(bytes.data(), 1, bytes.size(), f);
    fclose(f);
    return true;
}

static bool ReadAuthFile(std::string& outData) {
    std::string path = GetAuthDir() + "auth.dat";
    FILE* f = nullptr;
    if (fopen_s(&f, path.c_str(), "rb") != 0 || !f) return false;
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (size <= 0) { fclose(f); return false; }
    std::vector<uint8_t> bytes(size);
    fread(bytes.data(), 1, size, f);
    fclose(f);
    XOREncryptDecrypt(bytes);
    outData.assign(bytes.begin(), bytes.end());
    return true;
}

// ── CheckAuth ─────────────────────────────────────────────────
bool CheckAuth() {
#ifdef TEST_BUILD
    return true;
#endif
    std::string data;
    if (!ReadAuthFile(data))
        return PromptForKey();

    // Format: "key|expiry_timestamp"
    size_t sep = data.find('|');
    if (sep == std::string::npos) return PromptForKey();

    std::string expiryStr = data.substr(sep + 1);
    try {
        long long expiry = std::stoll(expiryStr);
        long long now = (long long)time(nullptr);
        if (now >= expiry) return PromptForKey();
    } catch (...) {
        return PromptForKey();
    }

    std::string key = data.substr(0, sep);
    return VerifyKey(key);
}

// ── PromptForKey ──────────────────────────────────────────────
bool PromptForKey() {
    std::string key;

    // Try to get key from console
    HWND hwnd = GetConsoleWindow();
    if (hwnd) {
        printf("======================================\n");
        printf("  CAMUS CS2 CHEAT - Authentication\n");
        printf("======================================\n");
        printf("Enter your subscription key: ");
        std::getline(std::cin, key);
        if (!key.empty()) {
            if (VerifyKey(key)) {
                long long expiry = (long long)time(nullptr) + 30LL * 24 * 3600;
                std::string data = key + "|" + std::to_string(expiry);
                WriteAuthFile(data);
                printf("[+] Key verified! Authentication successful.\n");
                return true;
            } else {
                printf("[-] Invalid key or verification failed.\n");
                return false;
            }
        }
        return false;
    }

    return false;
}
