#pragma once

#include "fileform.h"

typedef struct
{
	int offset;
	int size;
	TCHAR text[NSIS_MAX_STRLEN];
}license_file;

#define CR_LF "\x0D\x0A"

#define RINOZ(x) { int _tt_ = (x); if (_tt_ != 0) return _tt_; }

#define Z7_NSIS_WIN_GENERIC_READ    ((UINT32)1 << 31)
#define Z7_NSIS_WIN_GENERIC_WRITE   ((UInt32)1 << 30)
#define Z7_NSIS_WIN_GENERIC_EXECUTE ((UINT32)1 << 29)
#define Z7_NSIS_WIN_GENERIC_ALL     ((UINT32)1 << 28)

#define Z7_NSIS_WIN_CREATE_NEW        1
#define Z7_NSIS_WIN_CREATE_ALWAYS     2
#define Z7_NSIS_WIN_OPEN_EXISTING     3
#define Z7_NSIS_WIN_OPEN_ALWAYS       4
#define Z7_NSIS_WIN_TRUNCATE_EXISTING 5

// #define kVar_CMDLINE    20
#define kVar_INSTDIR    21
#define kVar_OUTDIR     22
#define kVar_EXEDIR     23
// #define kVar_LANGUAGE   24
#define kVar_TEMP       25
#define kVar_PLUGINSDIR 26
#define kVar_EXEPATH    27  // NSIS 2.26+
// #define kVar_EXEFILE    28  // NSIS 2.26+
#define kVar_HWNDPARENT_225 27

#define kVar_HWNDPARENT     29

#define kVar_Spec_OUTDIR_225  29  // NSIS 2.04 - 2.25
#define kVar_Spec_OUTDIR      31  // NSIS 2.26+

#ifndef Z7_ARRAY_SIZE
#define Z7_ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#endif

#define GET_NUM_INTERNAL_VARS (IsNsis200 ? kNumInternalVars - 3 : IsNsis225 ? kNumInternalVars - 2 : kNumInternalVars)

BOOL ThereIsCrc(int flags);

