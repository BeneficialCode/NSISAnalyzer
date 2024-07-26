#include "NsisIn.h"

/* NSIS can use one name for two CSIDL_*** and CSIDL_COMMON_*** items (CurrentUser / AllUsers)
   Some NSIS shell names are not identical to WIN32 CSIDL_* names.
   NSIS doesn't use some CSIDL_* values. But we add name for all CSIDL_ (marked with '+'). */

static const char* const kShellStrings[] =
{
    "DESKTOP"     // +
  , "INTERNET"    // +
  , "SMPROGRAMS"  // CSIDL_PROGRAMS
  , "CONTROLS"    // +
  , "PRINTERS"    // +
  , "DOCUMENTS"   // CSIDL_PERSONAL
  , "FAVORITES"   // CSIDL_FAVORITES
  , "SMSTARTUP"   // CSIDL_STARTUP
  , "RECENT"      // CSIDL_RECENT
  , "SENDTO"      // CSIDL_SENDTO
  , "BITBUCKET"   // +
  , "STARTMENU"
  , NULL          // CSIDL_MYDOCUMENTS = CSIDL_PERSONAL
  , "MUSIC"       // CSIDL_MYMUSIC
  , "VIDEOS"      // CSIDL_MYVIDEO
  , NULL
  , "DESKTOP"     // CSIDL_DESKTOPDIRECTORY
  , "DRIVES"      // +
  , "NETWORK"     // +
  , "NETHOOD"
  , "FONTS"
  , "TEMPLATES"
  , "STARTMENU"   // CSIDL_COMMON_STARTMENU
  , "SMPROGRAMS"  // CSIDL_COMMON_PROGRAMS
  , "SMSTARTUP"   // CSIDL_COMMON_STARTUP
  , "DESKTOP"     // CSIDL_COMMON_DESKTOPDIRECTORY
  , "APPDATA"     // CSIDL_APPDATA         !!! "QUICKLAUNCH"
  , "PRINTHOOD"
  , "LOCALAPPDATA"
  , "ALTSTARTUP"
  , "ALTSTARTUP"  // CSIDL_COMMON_ALTSTARTUP
  , "FAVORITES"   // CSIDL_COMMON_FAVORITES
  , "INTERNET_CACHE"
  , "COOKIES"
  , "HISTORY"
  , "APPDATA"     // CSIDL_COMMON_APPDATA
  , "WINDIR"
  , "SYSDIR"
  , "PROGRAM_FILES" // +
  , "PICTURES"    // CSIDL_MYPICTURES
  , "PROFILE"
  , "SYSTEMX86" // +
  , "PROGRAM_FILESX86" // +
  , "PROGRAM_FILES_COMMON" // +
  , "PROGRAM_FILES_COMMONX8" // +  CSIDL_PROGRAM_FILES_COMMONX86
  , "TEMPLATES"   // CSIDL_COMMON_TEMPLATES
  , "DOCUMENTS"   // CSIDL_COMMON_DOCUMENTS
  , "ADMINTOOLS"  // CSIDL_COMMON_ADMINTOOLS
  , "ADMINTOOLS"  // CSIDL_ADMINTOOLS
  , "CONNECTIONS" // +
  , NULL
  , NULL
  , NULL
  , "MUSIC"       // CSIDL_COMMON_MUSIC
  , "PICTURES"    // CSIDL_COMMON_PICTURES
  , "VIDEOS"      // CSIDL_COMMON_VIDEO
  , "RESOURCES"
  , "RESOURCES_LOCALIZED"
  , "COMMON_OEM_LINKS" // +
  , "CDBURN_AREA"
  , NULL // unused
  , "COMPUTERSNEARME" // +
};

static const char* const kVarStrings[] =
{
	"CMDLINE"
  , "INSTDIR"
  , "OUTDIR"
  , "EXEDIR"
  , "LANGUAGE"
  , "TEMP"
  , "PLUGINSDIR"
  , "EXEPATH"   // NSIS 2.26+
  , "EXEFILE"   // NSIS 2.26+
  , "HWNDPARENT"
  , "_CLICK"    // is set from page->clicknext
  , "_OUTDIR"   // NSIS 2.04+
};

static const unsigned kNumInternalVars = 20 + Z7_ARRAY_SIZE(kVarStrings);

BOOL ThereIsCrc(int flags) {
	return (flags & FH_FLAGS_FORCE_CRC) != 0 ||
		(flags & FH_FLAGS_NO_CRC) == 0;
}