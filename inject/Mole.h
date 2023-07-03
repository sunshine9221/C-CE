#pragma once
#include <list>
#include <string>
#include <TlHelp32.h>
//#include <Psapi.h>


class CRecord
{
public:
	CRecord() {
		addr = NULL;
		size = 0;
	}
	CRecord(BYTE* paddr, size_t sz) {
		addr = paddr;
		size = sz;
	}
	CRecord(const CRecord& record) {
		addr = record.addr;
		size = record.size;
	}
	~CRecord() {

	}
	CRecord& operator=(const CRecord& record) {
		if (this != &record) {
			addr = record.addr;
			size = record.size;
		}
		return *this;
	}
	bool operator==(WORD data)
	{
		return *(WORD*)addr == data;
	}
	bool operator==(DWORD data)
	{
		return *(DWORD*)addr == data;
	}
	bool operator==(double data)
	{
		return *(double*)addr == data;
	}
	bool isequal(BYTE* data, size_t sz) {
		return memcmp(data, addr, sz) == 0;
	}
public:
	BYTE* addr;
	size_t size;
};

class CMole
{
public:
	static CMole* instance();
	int SearchData(BYTE* data, size_t sz);
protected:
	CMole() {}
	~CMole() {
		lstResults.clear();
	}
	int FirstSearch(BYTE* data, size_t sz);
	int SecondSearch(BYTE* data, size_t sz);
	int Search(const std::string& data, BYTE* mem, size_t size);
public:
	std::list<CRecord> lstResults;
private:
	static CMole* m_instance;
};

