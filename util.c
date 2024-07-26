#include "util.h"

HANDLE myOpenFile(const TCHAR* fn, DWORD da, DWORD cd) {
    int attr = GetFileAttributes(fn);
    return CreateFile(
        fn,
        da,
        FILE_SHARE_READ,
        NULL,
        cd,
        attr == INVALID_FILE_ATTRIBUTES ? 0 : attr,
        NULL
    );
}

TCHAR* mystrcpy(TCHAR* out, const TCHAR* in) {
    return lstrcpyn(out, in, NSIS_MAX_STRLEN);
}

TCHAR* trimslashtoend(TCHAR* buf)
{
    TCHAR* p = buf + mystrlen(buf);
    do
    {
        if (*p == _T('\\'))
            break;
        p = CharPrev(buf, p);
    } while (p > buf);

    *p = 0;

    return p + 1;
}

BOOL myReadFile(HANDLE h, LPVOID buf, DWORD cb)
{
    DWORD cbio;
    return ReadFile(h, buf, cb, &cbio, NULL) && cb == cbio;
}

// mini_memcpy takes the number of bytes to copy.
void mini_memcpy(void* out, const void* in, UINT_PTR cb)
{
    char* dst = (char*)out, * src = (char*)in;
    while (cb-- > 0) *dst++ = *src++;
}

BOOL myWriteFile(HANDLE h, const void* buf, DWORD cb)
{
    DWORD cbio;
    return WriteFile(h, buf, cb, &cbio, NULL) && cb == cbio;
}

int mystrlen(const TCHAR* in)
{
    return lstrlen(in);
}

// Removes the extension off a filename.
TCHAR* trimextension(TCHAR* buf)
{
    TCHAR* p = buf + mystrlen(buf);
    do
    {
        if (*p == _T('.'))
            break;
        if (*p == _T('\\') || *p == _T('/'))
        {
            p = buf + mystrlen(buf); // security measure, stays in the current directory
            break;
        }
        p = CharPrev(buf, p);
    } while (p > buf);

    *p = 0;

    return buf;
}