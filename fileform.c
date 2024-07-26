#include "fileform.h"
#include "util.h"
#include "lang.h"

#include "crc32.h"
#include "zlib/ZLIB.H"

struct block_header g_blocks[BLOCKS_NUM];
header* g_header;
int g_flags;
UINT g_filehdrsize;
int g_is_uninstaller;
firstheader g_firstheader;
int g_has_external_file;
int g_is_stub_installer;
#ifdef NSIS_CONFIG_CRC_SUPPORT
int g_do_crc = 0;
#endif //NSIS_CONFIG_CRC_SUPPORT

HANDLE g_db_hFile = INVALID_HANDLE_VALUE;
#ifdef NSIS_CONFIG_EXTERNAL_FILE_SUPPORT
HANDLE g_dbex_hFile = INVALID_HANDLE_VALUE;
#endif //NSIS_CONFIG_EXTERNAL_FILE_SUPPORT
static z_stream g_inflate_stream;

static MAXSIZETYPE m_length;
static UINT m_pos = 0;

const TCHAR* loadHeaders(int cl_flags,wchar_t* state_exe_path)
{
	UINT32 length_of_all_following_data, left;
	crc32_t crc = 0, crc_header = 0;
	void* data;
	firstheader h = { 0 };
	header* header;

	HANDLE db_hFile;

	g_db_hFile = db_hFile = myOpenFile(state_exe_path, GENERIC_READ, OPEN_EXISTING);
	if (db_hFile == INVALID_HANDLE_VALUE)
	{
		return L"Error launching installer";
	}

	TCHAR state_exe_directory[NSIS_MAX_STRLEN];
	mystrcpy(state_exe_directory, state_exe_path);
	TCHAR state_exe_file[NSIS_MAX_STRLEN];
	mystrcpy(state_exe_file, trimslashtoend(state_exe_directory));

	left = m_length = GetFileSize(db_hFile, NULL);
	while (left > 0)
	{
		static char temp[32768];
		DWORD l = min(left, (g_filehdrsize ? 32768UL : 512UL));
		if (!ReadSelfFile(temp, l))
		{
			return _LANG_INVALIDCRC;
		}

		if (!g_filehdrsize)
		{
			mini_memcpy(&h, temp, 32);
			if (
				(h.flags & (~FH_FLAGS_MASK)) == 0 &&
				h.siginfo == FH_SIG &&
				h.nsinst[2] == FH_INT3 &&
				h.nsinst[1] == FH_INT2 &&
				h.nsinst[0] == FH_INT1
				)
			{
				g_filehdrsize = m_pos;
				cl_flags |= h.flags;

				if (h.length_of_all_following_data > left)
					return _LANG_INVALIDCRC;

				if ((cl_flags & FH_FLAGS_FORCE_CRC) == 0)
				{
					if (cl_flags & FH_FLAGS_NO_CRC)
						break;
				}

				g_do_crc++;
				l = sizeof(firstheader);
				left = l;

				m_pos = m_pos + h.length_of_all_following_data - l - sizeof(crc32_t);

				SetSelfFilePointer(m_pos + l - sizeof(crc32_t));

				if (l > left)
					l = left;

				mini_memcpy(&g_firstheader, &h, sizeof(firstheader));
			}
		}

		if (left < m_length)
			crc = CRC32(crc, (unsigned char*)temp, l);


		m_pos += l;
		left -= l;
	}
	if (h.flags & FH_FLAGS_UNINSTALL)
		g_is_uninstaller++;
	if (h.flags & FH_FLAGS_HAS_EXTERNAL_FILE)
		g_has_external_file++;

	if (h.flags & FH_FLAGS_IS_STUB_INSTALLER)
		g_is_stub_installer++;

	if (g_has_external_file && !g_is_uninstaller)
	{
		TCHAR path_ext[MAX_PATH];

		mystrcpy(path_ext, state_exe_path);
		mystrcpy(path_ext, trimextension(path_ext));
		wsprintf(path_ext, _T("%s.nsisbin"), path_ext);
		g_dbex_hFile = myOpenFile(path_ext, GENERIC_READ, OPEN_EXISTING);
		if (g_dbex_hFile == INVALID_HANDLE_VALUE && !g_is_stub_installer)
			return _LANG_INVALIDCRC;
	}

	if (!g_filehdrsize)
		return _LANG_INVALIDCRC;

	if (g_do_crc)
	{
		crc32_t fcrc = 0;
		SetSelfFilePointer64(m_pos - sizeof(crc32_t));
		if(!ReadSelfFile(&crc_header,sizeof(crc32_t)))
			return _LANG_INVALIDCRC;
		if (!ReadSelfFile((LPVOID)&fcrc, sizeof(crc32_t)))
			return _LANG_INVALIDCRC;
		crc = CRC32(crc, &crc_header, sizeof(crc32_t));
		if(crc!=fcrc)
			return _LANG_INVALIDCRC;
	}

	SetSelfFilePointer(g_filehdrsize + sizeof(firstheader));

	data = (void*)GlobalAlloc(GPTR, h.length_of_header);

	/*HANDLE hOutFile = CreateFile(L"nsis.bin", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	GetCompressedDataFromDataBlock(-1, hOutFile);*/

	if (GetCompressedDataFromExeHeadToMemory(-1, data, h.length_of_header, crc_header) != h.length_of_header)
	{
		return _LANG_INVALIDCRC;
	}

	header = g_header = data;
	g_flags = header->flags;

	// set offsets to real memory offsets rather than installer's header offset
	left = BLOCKS_NUM;
	while (left--)
	{
		header->blocks[left].offset += (UINT_PTR)data;
	}
	
	header->blocks[NB_DATA].offset = (UINT_PTR)((UINT64)SetFilePointer64(db_hFile, 0, FILE_CURRENT));

	mini_memcpy(&g_blocks, &header->blocks, sizeof(g_blocks));

	return 0;
}

BOOL ReadSelfFile(LPVOID lpBuffer, DWORD nNumberOfBytesToRead) {
	return myReadFile(g_db_hFile, lpBuffer, nNumberOfBytesToRead);
}

DWORD SetSelfFilePointer(LONG lDistanceToMove) {
	LARGE_INTEGER li;
	li.QuadPart = 0;
	return SetFilePointer(g_db_hFile, lDistanceToMove, &li.HighPart, FILE_BEGIN);
}

INT64 SetFilePointer64(HANDLE handle, INT64 lDistanceToMove, DWORD flags) {
	LARGE_INTEGER liDistanceToMove;
	liDistanceToMove.QuadPart = lDistanceToMove;
	liDistanceToMove.LowPart = SetFilePointer(handle, liDistanceToMove.LowPart, &liDistanceToMove.HighPart, flags);
	if (liDistanceToMove.LowPart == INVALID_SET_FILE_POINTER && GetLastError() != ERROR_SUCCESS)
	{
		liDistanceToMove.QuadPart = -1;
	}
	return liDistanceToMove.QuadPart;
}

#define IBUFSIZE 16384
#define OBUFSIZE 32768

// returns -3 if compression error/eof/etc
FIRST_INT_TYPE _dodecomp(int exehead_only, INT64 offset, HANDLE hFileOut, unsigned char* outbuf, int outbuflen, crc32_t ex_crc)
{
	static char inbuffer[IBUFSIZE + OBUFSIZE];
	char* outbuffer;
	int outbuffer_len = outbuf ? outbuflen : OBUFSIZE;
	FIRST_INT_TYPE retval = 0;
	FIRST_INT_TYPE input_len, input_len_orig;

	crc32_t crc = 0;

	outbuffer = outbuf ? (char*)outbuf : (inbuffer + IBUFSIZE);

	mySetFilePointer(offset, exehead_only);
	if (!myReadFileData((LPVOID)&input_len_orig, FIRST_INT_SIZEOF, exehead_only)) return -3;
	if (g_do_crc)
		crc = CRC32(0, &input_len_orig, FIRST_INT_SIZEOF);

	input_len = input_len_orig;


	if (input_len & FIRST_INT_FLAG) // compressed
	{
		TCHAR progress[64];
		int input_len_total;
		ULONGLONG ltc = GetTickCount64(), tc;

		inflateReset(&g_inflate_stream);
		input_len_total = input_len &= FIRST_INT_MASK; // take off top bit.

		while (input_len > 0)
		{
			int l = min(input_len, (FIRST_INT_TYPE)IBUFSIZE);
			int err;

			if (!myReadFileData((LPVOID)inbuffer, l, exehead_only)) 
				return -3;
			if (g_do_crc) 
				crc = CRC32(crc, (unsigned char*)inbuffer, l);
			g_inflate_stream.next_in = (unsigned char*)inbuffer;
			g_inflate_stream.avail_in = l;
			input_len -= l;

			for (;;)
			{
				int u;

				g_inflate_stream.next_out = (unsigned char*)outbuffer;
				g_inflate_stream.avail_out = (unsigned int)outbuffer_len;

				err = inflate(&g_inflate_stream);

				if (err < 0) 
					return -4;

				u = BUGBUG64TRUNCATE(int, (size_t)((char*)g_inflate_stream.next_out - outbuffer));

				tc = GetTickCount64();
				// if there's no output, more input is needed
				if (!u)
					break;

				if (!outbuf)
				{
					if (!myWriteFile(hFileOut, outbuffer, u)) 
						return -2;
					retval += u;
				}
				else
				{
					retval += u;
					outbuffer_len -= u;
					outbuffer = (char*)g_inflate_stream.next_out;
				}
				if (err == Z_STREAM_END)
				{
					if (g_do_crc)
					{
						if (crc != ex_crc) 
							return -3;
					}
					return retval;
				}
			}
		}
	}
	else
	{
		if (!outbuf)
		{
			while (input_len > 0)
			{
				DWORD l = min(input_len, (FIRST_INT_TYPE)outbuffer_len);
				if (!myReadFileData((LPVOID)inbuffer, l, exehead_only)) return -3;
#ifdef NSIS_CONFIG_CRC_SUPPORT
				if (g_do_crc) crc = CRC32(crc, (unsigned char*)inbuffer, l);
#endif
				if (!myWriteFile(hFileOut, inbuffer, l)) return -2;
				retval += l;
				input_len -= l;
			}
		}
		else
		{
			int l = min(input_len, outbuflen);
			if (!myReadFileData((LPVOID)outbuf, l, exehead_only)) return -3;
			if (g_do_crc) crc = CRC32(crc, outbuf, l);
			retval = l;
		}
	}
	if (g_do_crc)
	{
		crc = CRC32(crc, (unsigned char*)&input_len_orig, FIRST_INT_SIZEOF);
		if (crc != ex_crc) return -3;
	}
	return retval;
}

void mySetFilePointer(INT64 offset, int exehead_only) {
	if (offset < 0) return;
#ifdef NSIS_CONFIG_EXTERNAL_FILE_SUPPORT
	if (g_has_external_file && !exehead_only)
	{
		SetExternalFilePointer(offset);
	}
	else
#endif //NSIS_CONFIG_EXTERNAL_FILE_SUPPORT
	{
		offset += g_blocks[NB_DATA].offset;
		SetSelfFilePointer64(offset);
	}
}

INT64 SetSelfFilePointer64(INT64 lDistanceToMove)
{
	return SetFilePointer64(g_db_hFile, lDistanceToMove, FILE_BEGIN);
}


BOOL ReadExternalFile(LPVOID lpBuffer, DWORD nNumberOfBytesToRead)
{
	return myReadFile(g_dbex_hFile, lpBuffer, nNumberOfBytesToRead);
}

INT64 SetExternalFilePointer(INT64 lDistanceToMove)
{
	return SetFilePointer64(g_dbex_hFile, lDistanceToMove, FILE_BEGIN);
}

int myReadFileData(LPVOID buffer, const int len, const int exehead_only) {
#ifdef NSIS_CONFIG_EXTERNAL_FILE_SUPPORT
	if (g_has_external_file && !exehead_only)
	{
		if (!ReadExternalFile(buffer, len)) return 0;
	}
	else
#endif //NSIS_CONFIG_EXTERNAL_FILE_SUPPORT
	{
		if (!ReadSelfFile(buffer, len)) return 0;
	}
	return 1;
}