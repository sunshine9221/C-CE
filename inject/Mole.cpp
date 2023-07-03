#include "pch.h"
#include "Mole.h"


using namespace std;

CMole* CMole::m_instance = NULL;

CMole* CMole::instance()
{
	if (m_instance == NULL) {
		m_instance = new CMole();
	}
	return m_instance;
}

int CMole::SearchData(BYTE* data, size_t sz)
{
	if (lstResults.size()) {
		return SecondSearch(data, sz);
	}
	else {
		return FirstSearch(data, sz);
	}
}

int CMole::FirstSearch(BYTE* data, size_t sz)
{
	string obj;
	obj.resize(sz);
	memcpy((char*)obj.c_str(), data, sz);
	list<MODULEENTRY32> modules;
	MODULEENTRY32 entry;
	entry.dwSize = sizeof(MODULEENTRY32);
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, GetCurrentProcessId());
	if (hSnapshot == INVALID_HANDLE_VALUE)
	{
		//TRACE("失败！pid:%d\r\n", nPID);
		return 0;
	}
	BOOL bMore = Module32First(hSnapshot, &entry);
	int i = 0;
	MEMORY_BASIC_INFORMATION meminfo;
	while (bMore) {
		//pe32.szExeFile为多字节
		//TRACE("模块名称：%s\n模块路径:%s 模块ID: %u 模块句柄：%08X\n\n",
		//	entry.szModule, entry.szExePath, entry.th32ModuleID, entry.hModule);
		modules.push_back(entry);
		bMore = Module32Next(hSnapshot, &entry);
		VirtualQueryEx(GetCurrentProcess(), entry.modBaseAddr, &meminfo, sizeof(meminfo));
		if (meminfo.Protect & (PAGE_READWRITE | PAGE_WRITECOPY | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY))
		{//只需要搜索可以写入的
			Search(obj, entry.modBaseAddr, entry.modBaseSize);
		}
	}
	return 1;
}

int CMole::SecondSearch(BYTE* data, size_t sz)
{
	std::list<CRecord> result;
	if (sz == 2) {
		WORD d = *(WORD*)data;
		std::list<CRecord>::iterator it = lstResults.begin();
		for (; it != lstResults.end(); it++) {
			if (*it == d) {
				result.push_back(*it);
			}
		}
	}
	else if (sz == 4) {
		DWORD d = *(DWORD*)data;
		std::list<CRecord>::iterator it = lstResults.begin();
		for (; it != lstResults.end(); it++) {
			if (*it == d) {
				result.push_back(*it);
			}
		}
	}
	else if (sz == 8) {
		double d = *(double*)data;
		std::list<CRecord>::iterator it = lstResults.begin();
		for (; it != lstResults.end(); it++) {
			if (*it == d) {
				result.push_back(*it);
			}
		}
	}
	else {
		std::list<CRecord>::iterator it = lstResults.begin();
		for (; it != lstResults.end(); it++) {
			if (it->isequal(data, sz)) {
				result.push_back(*it);
			}
		}
	}
	lstResults.clear();
	lstResults = result;
	return 1;
}

int CMole::Search(const std::string& data, BYTE* mem, size_t size)
{
	size_t sz = data.size();
	if (sz == 2) {
		WORD d = *(WORD*)data.c_str();
		WORD* pData = (WORD*)mem;
		for (size_t i = 0; i < size / 2; i++) {
			if (pData[i] == d) {
				lstResults.push_back(CRecord(mem + i * 2, 2));
			}
		}
	}
	else if (sz == 4) {
		DWORD d = *(DWORD*)data.c_str();
		DWORD* pData = (DWORD*)mem;
		for (size_t i = 0; i < size / 4; i++) {
			if (pData[i] == d) {
				lstResults.push_back(CRecord(mem + i * 4, 4));
			}
		}
	}
	else if (sz == 8) {
		double d = *(double*)data.c_str();
		double* pData = (double*)mem;
		for (size_t i = 0; i < size / 8; i++) {
			if (pData[i] == d) {
				lstResults.push_back(CRecord(mem + i * 8, 8));
			}
		}
	}
	else {
		for (size_t i = 0; i < size; i++)
		{
			if (memcmp(data.c_str(), mem + i, sz) == 0)
			{
				lstResults.push_back(CRecord(mem + i, sz));
			}
		}
	}
	return 1;
}
