#include "PortableExecutableFile.h"

PortableExecutableFile::PortableExecutableFile()
{
}

PortableExecutableFile::~PortableExecutableFile()
{
}

int PortableExecutableFile::Load(const PEString& sPath)
{
	errno_t ret = fopen_s(&m_pFile, sPath.c_str(), "rb");
	if (ret != 0 && (m_pFile == NULL))
		return -1;
	fread(&m_ImageDosHeader, 1, sizeof(m_ImageDosHeader), m_pFile);
	m_ImageDosStub.resize(m_ImageDosHeader.e_lfanew - sizeof(m_ImageDosHeader));
	fread(m_ImageDosStub.data(), 1, m_ImageDosStub.size(), m_pFile);
	fread(&m_ImageNtHeaders, 1, sizeof(m_ImageNtHeaders), m_pFile);
	m_SectionHeaders.resize(m_ImageNtHeaders.FileHeader.NumberOfSections);
	IMAGE_SECTION_HEADER* pSectionHeader = IMAGE_FIRST_SECTION(&m_ImageNtHeaders);
	memcpy(m_SectionHeaders.data(), pSectionHeader, sizeof(IMAGE_SECTION_HEADER) * m_ImageNtHeaders.FileHeader.NumberOfSections);
	m_SectionDatas.resize(15);
	for (int i = 0; i < 15; i++)
	{
		if (m_ImageNtHeaders.OptionalHeader.DataDirectory[i].Size > 0) {
			m_SectionDatas[i].resize(m_ImageNtHeaders.OptionalHeader.DataDirectory[i].Size);
		}
	}

	fclose(m_pFile);
	return 0;
}

int PortableExecutableFile::LoadFromMem(UINT nPid, UINT64 nAddress, size_t nSize)
{
	return 0;
}
