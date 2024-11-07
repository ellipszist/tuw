#include "exec.h"
#include <vector>
#include "subprocess.h"
#include "string_utils.h"
#ifdef _WIN32
#include "windows.h"
#endif

enum READ_IO_TYPE : int {
    READ_STDOUT = 0,
    READ_STDERR,
};

unsigned ReadIO(subprocess_s &process,
                int read_io_type,
                char *buf, const unsigned buf_size,
                tuwString& str, const size_t str_size) {
    unsigned read_size = 0;
    if (read_io_type == READ_STDOUT) {
        read_size = subprocess_read_stdout(&process, buf, buf_size);
    } else {
        read_size = subprocess_read_stderr(&process, buf, buf_size);
    }

    buf[read_size] = 0;
    str += buf;
    if (str.length() > str_size * 2) {
        str = str.substr(str.length() - str_size, str_size);
    }

    return read_size;
}

void DestroyProcess(subprocess_s &process, int *return_code, tuwString &err_msg) {
    if (subprocess_join(&process, return_code) || subprocess_destroy(&process)) {
        *return_code = -1;
        err_msg = "Failed to manage subprocess.\n";
    }
}

void RedirectOutput(FILE* out, const char* buf,
                    unsigned read_size, bool use_utf8_on_windows) {
    if (read_size) {
#ifdef _WIN32
        if (use_utf8_on_windows) {
            tuwWstring wout = UTF8toUTF16(buf);
            fprintf(out, "%ls", wout.c_str());
        } else {
            fprintf(out, "%s", buf);
        }
#else
        FprintFmt(out, "%s", buf);
#endif
    }
}

inline tuwString TruncateStr(const tuwString& str, size_t size) {
    if (str.size() > size)
        return "..." + str.substr(str.size() - size, size);
    return str;
}

ExecuteResult Execute(const tuwString& cmd, bool use_utf8_on_windows) {
#ifdef _WIN32
    tuwWstring wcmd = UTF8toUTF16(cmd.c_str());

    if (GetStringError() != STR_OK) {
        // Reject the command as it might have unexpected value.
        return { -1,
                 "Fatal error has occored while editing strings.\n",
                 "" };
    }

    int argc;
    wchar_t** parsed = CommandLineToArgvW(wcmd.c_str(), &argc);
    wchar_t** argv = new wchar_t*[argc + 3];
    wchar_t a[] = L"cmd.exe";
    wchar_t b[] = L"/c";
    argv[0] = &a[0];
    argv[1] = &b[0];
    for (int i = 0; i < argc; i++) {
        argv[i + 2] = parsed[i];
    }
    argv[argc + 2] = 0;
#else
    const char* argv[] = {"/bin/sh", "-c", cmd.c_str(), NULL};
#endif

    struct subprocess_s process;
    int options = subprocess_option_inherit_environment
                  | subprocess_option_search_user_path
                  | subprocess_option_enable_async;
    int result = subprocess_create(&argv[0], options, &process);

#ifdef _WIN32
    LocalFree((LPWSTR)parsed);
    delete[] argv;
#endif

    if (0 != result)
        return { -1, "Failed to create a subprocess.\n", ""};

    const unsigned BUF_SIZE = 1024;
    const size_t LAST_LINE_MAX_LEN = BUF_SIZE;
    const size_t ERR_MSG_MAX_LEN = BUF_SIZE * 2;
    char out_buf[BUF_SIZE + 1];
    char err_buf[BUF_SIZE + 1];
    tuwString last_line;
    tuwString err_msg;
    unsigned out_read_size = 0;
    unsigned err_read_size = 0;

    do {
        out_read_size = ReadIO(process, READ_STDOUT,
                               out_buf, BUF_SIZE, last_line, LAST_LINE_MAX_LEN);
        err_read_size = ReadIO(process, READ_STDERR,
                               err_buf, BUF_SIZE, err_msg, ERR_MSG_MAX_LEN);
        RedirectOutput(stdout, out_buf, out_read_size, use_utf8_on_windows);
        RedirectOutput(stderr, err_buf, err_read_size, use_utf8_on_windows);
    } while (subprocess_alive(&process) || out_read_size || err_read_size);

    // Sometimes stdout and stderr still have unread characters
    out_read_size = ReadIO(process, READ_STDOUT, out_buf, BUF_SIZE, last_line, LAST_LINE_MAX_LEN);
    err_read_size = ReadIO(process, READ_STDERR, err_buf, BUF_SIZE, err_msg, ERR_MSG_MAX_LEN);
    RedirectOutput(stdout, out_buf, out_read_size, use_utf8_on_windows);
    RedirectOutput(stderr, err_buf, err_read_size, use_utf8_on_windows);

    int return_code;
    DestroyProcess(process, &return_code, err_msg);

    last_line = GetLastLine(last_line);
    last_line = TruncateStr(last_line, LAST_LINE_MAX_LEN);
    err_msg = TruncateStr(err_msg, ERR_MSG_MAX_LEN);

    return { return_code, err_msg, last_line };
}

ExecuteResult LaunchDefaultApp(const tuwString& url) {
#ifdef _WIN32
    tuwWstring utf16_url = UTF8toUTF16(url.c_str());
    const wchar_t* argv[] = {L"cmd.exe", L"/c", L"start", utf16_url.c_str(), NULL};
#elif defined(__TUW_UNIX__) && !defined(__HAIKU__)
    // Linux and BSD
    const char* argv[] = {"xdg-open", url.c_str(), NULL};
#else
    // macOS and Haiku OS
    const char* argv[] = {"open", url.c_str(), NULL};
#endif
    struct subprocess_s process;
    int options = subprocess_option_inherit_environment
                  | subprocess_option_search_user_path;
    int result = subprocess_create(argv, options, &process);
    if (0 != result)
        return { -1, "Failed to create a subprocess.\n", ""};

    int return_code;
    tuwString err_msg;
    DestroyProcess(process, &return_code, err_msg);

    return { return_code, err_msg, "" };
}
