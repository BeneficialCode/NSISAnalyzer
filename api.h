#pragma once

#ifndef NSISCALL
#  define NSISCALL __stdcall
#endif
#if !defined(_WIN32) && !defined(LPTSTR)
#  define LPTSTR TCHAR*
#endif