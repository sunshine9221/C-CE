#pragma once
#include <Windows.h>
#include <stdio.h>
#include <string>
#include <vector>

using PEString = std::string;
using PESectionHeaders = std::vector<IMAGE_SECTION_HEADER>;
using PEImageDataDirectory = std::vector<IMAGE_DATA_DIRECTORY>;
using PEExportDirectory = std::vector<IMAGE_EXPORT_DIRECTORY>;
using PEImportDescriptor = std::vector<IMAGE_IMPORT_DESCRIPTOR>;
using PEResourceDirectory = std::vector<IMAGE_RESOURCE_DIRECTORY>;
using ResourceDirectoryEntry = IMAGE_RESOURCE_DIRECTORY_ENTRY;
using PESection = std::vector<BYTE>;
using PEData = std::vector<BYTE>;
using PESectionData = std::vector<PEData>;

class PortableExecutableFile
{
public:
	PortableExecutableFile();
	~PortableExecutableFile();
	int Load(const PEString& sPath);
	int LoadFromMem(UINT nPid, UINT64 nAddress, size_t nSize);
private:
	IMAGE_DOS_HEADER m_ImageDosHeader;
	PEData m_ImageDosStub;
	IMAGE_NT_HEADERS m_ImageNtHeaders;
	PESectionHeaders m_SectionHeaders;
	PESectionData m_SectionDatas;
	PEData m_Stub;//其他残留数据
	PEString m_path;
	FILE* m_pFile;
	UINT m_nPid;
	UINT64 m_nAddress;
	size_t m_nSize;
};

