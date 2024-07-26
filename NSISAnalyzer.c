// NSISAnalyzer.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <stdlib.h>
#include <stdio.h>
#include "fileform.h"
#include "lang.h"
#include "NsisIn.h"


int wmain(int argc,const wchar_t* argv[])
{
	if (argc < 2)
	{
		printf("Usage: %ws <file>\n",argv[0]);
		return 1;
	}
	int cl_flags = 0;
	wchar_t* exe_path = (wchar_t*)argv[1];
	const TCHAR* m_Err = _LANG_ERRORWRITINGTEMP;
	m_Err = loadHeaders(cl_flags, exe_path);
	if (m_Err) {
		printf("%ws\n",m_Err);
		return -1;
	}

	header* header = g_header;

	int ne = header->blocks[NB_ENTRIES].num;
	wprintf(L"%d instruction%" NPRIs L"(%d bytes), \n", ne, ne == 1 ? L"" : L"s", ne * sizeof(entry));
	int nlt = header->blocks[NB_LANGTABLES].num;
	wprintf(L"%d language table\n", nlt);

	
	UINT32 dataSize = g_firstheader.length_of_all_following_data - 
		(ThereIsCrc(g_firstheader.flags ) ? 4 : 0);
	wprintf(L"Data size: %d\n", dataSize);

	BOOL isInstaller = (g_firstheader.flags & FH_FLAGS_UNINSTALL) == 0;
	wprintf(L"Installer: %s\n", isInstaller ? L"yes" : L"no");

	struct block_header* strings = &header->blocks[NB_STRINGS];
	struct block_header* langtables = &header->blocks[NB_LANGTABLES];
	struct block_header* sections = &header->blocks[NB_SECTIONS];
	struct block_header* ctlcolors = &header->blocks[NB_CTLCOLORS];
	struct block_header* data = &header->blocks[NB_DATA];

	UINT32 stringPos = strings->offset;
	const UINT32 stringTableSize = langtables->offset - stringPos;
	wprintf(L"String table size: %d\n", stringTableSize);

	wprintf(L"entries num: %d\n", header->blocks[NB_ENTRIES].num);

	


	return 0;
}
