#pragma once
#include "Platform.h"

HANDLE myOpenFile(const TCHAR* fn, DWORD da, DWORD cd);

TCHAR* mystrcpy(TCHAR* out,const TCHAR* in);

// Separates a full path to the directory portion and file name portion
// and returns the pointer to the filename portion.
TCHAR* trimslashtoend(TCHAR* buf);
TCHAR* trimextension(TCHAR* buf);

BOOL myReadFile(HANDLE h, LPVOID buf, DWORD cb);

void mini_memcpy(void* out, const void* in, UINT_PTR cb);

BOOL myWriteFile(HANDLE h, const void* buf, DWORD cb);

int mystrlen(const TCHAR* in);