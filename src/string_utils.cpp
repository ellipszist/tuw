#include "string_utils.h"

#ifdef _WIN32
#include "windows/uipriv_windows.hpp"
#else
#include <cstring>
#endif

static const uint32_t FNV_OFFSET_BASIS_32 = 2166136261U;
static const uint32_t FNV_PRIME_32 = 16777619U;

uint32_t Fnv1Hash32(const std::string& str) {
    uint32_t hash = FNV_OFFSET_BASIS_32;
    for (const char& c : str) hash = (FNV_PRIME_32 * hash) ^ c;
    return hash;
}

#ifdef _WIN32
std::string UTF16toUTF8(const wchar_t* str) {
    char* uchar = toUTF8(str);
    std::string ustr = uchar;
    uiprivFree(uchar);
    return ustr;
}

std::wstring UTF8toUTF16(const char* str) {
    wchar_t* widechar = toUTF16(str);
    std::wstring wstr = widechar;
    uiprivFree(widechar);
    return wstr;
}

void PrintFmt(const char* fmt, ...) {
    va_list va;
    va_start(va, fmt);

    va_list va2;
    va_copy(va2, va);
    size_t n = _vscprintf(fmt, va2);
    va_end(va2);
    n++;

    char* buf = reinterpret_cast<char *>(uiprivAlloc(n * sizeof (char), "char[]"));
    vsprintf_s(buf, n, fmt, va);
    va_end(va);

    WCHAR* wfmt = toUTF16(buf);
    wprintf(L"%ls", wfmt);

    uiprivFree(buf);
    uiprivFree(wfmt);
}

// Enable ANSI escape sequences on the console window.
void EnableCSI() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE)
        return;

    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode))
        return;

    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
}

#elif defined(__TUW_UNIX__)
#include <stdarg.h>
#include "ui.h"

class Logger {
 private:
    uiMultilineEntry* m_log_entry;
    std::string m_log_buffer;

 public:
    Logger() : m_log_entry(NULL), m_log_buffer("") {}
    ~Logger() {}

    void SetLogEntry(void* log_entry) {
        m_log_entry = static_cast<uiMultilineEntry*>(log_entry);
        if (m_log_buffer != "") {
            Log(m_log_buffer.c_str());
            m_log_buffer = "";
        }
    }

    void Log(const char* str) {
        if (m_log_entry == NULL) {
            m_log_buffer += str;
        } else {
            uiMultilineEntryAppend(m_log_entry, str);
            uiUnixMuntilineEntryScrollToEnd(m_log_entry);
        }
    }
};

Logger g_logger = Logger();

void SetLogEntry(void* log_entry) {
    g_logger.SetLogEntry(log_entry);
}

void PrintFmt(const char* fmt, ...) {
    va_list va;
    va_start(va, fmt);
    va_list va2;
    va_copy(va2, va);
    size_t size = vsnprintf(NULL, 0, fmt, va2);
    va_end(va2);
    char* buf = new char[size + 1];
    buf[size] = 0;
    vsnprintf(buf, size + 1, fmt, va);
    g_logger.Log(buf);
    printf("%s", buf);
    delete[] buf;
    va_end(va);
}
#endif

