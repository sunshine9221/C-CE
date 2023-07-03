#include "pch.h"
#include "SearchTool.h"
#ifndef WM_PROGRESS
#define WM_PROGRESS (WM_USER+1)
#endif
#ifndef WM_SECOND
#define WM_SECOND (WM_USER+2)
#endif
#ifndef WM_CODE_PROGRESS
#define WM_CODE_PROGRESS (WM_USER+3)
#endif

#define PAGE_EXCUTE_FLAGS (PAGE_EXECUTE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY)

#ifndef PAGE_WRITE_FLAGS
#define PAGE_WRITE_FLAGS (PAGE_WRITECOPY | PAGE_EXECUTE_READWRITE | PAGE_READWRITE | PAGE_EXECUTE_WRITECOPY)
#endif

SearchTool::SearchToolHelper SearchTool::_;
SearchTool* SearchTool::m_instance = NULL;

SearchTool* SearchTool::Instance()
{
	if (m_instance == NULL) {
		m_instance = new SearchTool();
	}
	return m_instance;
}

void SearchTool::SetHWND(HWND hWnd)
{
	m_hWnd = hWnd;
}

void SearchTool::SetProcess(HANDLE hProcess)
{
	m_hProcess = hProcess;
}

int SearchTool::SearchCode(MODULEENTRY32 entry, std::string Data, int nDataType)
{
	TRACE("module:%s Data:%s type:%d\r\n", entry.szModule, Data.c_str(), nDataType);
	if (m_hProcess == NULL)return 0;
	Result result(Data, nDataType, entry.szModule, entry.szExePath);
	SIZE_T nIndex = 0;
	do {
		MEMORY_BASIC_INFORMATION mbi;
		if (VirtualQueryEx(m_hProcess, entry.modBaseAddr + nIndex,
			&mbi, sizeof(mbi)) == 0)
		{
			break;
		}
		TRACE("addr %08llX size %08X protect %08X\r\n", entry.modBaseAddr + nIndex, mbi.RegionSize, mbi.Protect);
		if (mbi.Protect & PAGE_EXCUTE_FLAGS)
		{//TODO:有时候可能需要搜索代码段的内容，修改逻辑
			switch (nDataType) {
			case 0://int8
				SearchInt8(Data, entry.modBaseAddr + nIndex, mbi.RegionSize, result);
				break;
			case 1://int16
				SearchInt16(Data, entry.modBaseAddr + nIndex, mbi.RegionSize, result);
				break;
			case 2://int32
				SearchInt32(Data, entry.modBaseAddr + nIndex, mbi.RegionSize, result);
				break;
			case 3://int64
				SearchInt64(Data, entry.modBaseAddr + nIndex, mbi.RegionSize, result);
				break;
			case 4://float
				SearchFloat(Data, entry.modBaseAddr + nIndex, mbi.RegionSize, result);
				break;
			case 5://double
				SearchDouble(Data, entry.modBaseAddr + nIndex, mbi.RegionSize, result);
				break;
			case 6://string
				SearchStr(Data, entry.modBaseAddr + nIndex, mbi.RegionSize, result);
				break;
			}
		}
		nIndex += mbi.RegionSize;
	} while (nIndex < entry.modBaseSize);
	::SendMessage(m_hWnd, WM_CODE_PROGRESS, entry.modBaseSize, (LPARAM)&result);
	return 0;
}

int SearchTool::Search(MODULEENTRY32 entry, std::string Data, int nDataType)
{
	TRACE("module:%s Data:%s type:%d\r\n", entry.szModule, Data.c_str(), nDataType);
	if (m_hProcess == NULL)return 0;
	Result result(Data, nDataType, entry.szModule, entry.szExePath);
	SIZE_T nIndex = 0;
	do {
		MEMORY_BASIC_INFORMATION mbi;
		if (VirtualQueryEx(m_hProcess, entry.modBaseAddr + nIndex,
			&mbi, sizeof(mbi)) == 0)
		{
			break;
		}
		TRACE("addr %08X size %08X protect %08X\r\n", entry.modBaseAddr + nIndex, mbi.RegionSize, mbi.Protect);
		if (mbi.Protect & (PAGE_WRITE_FLAGS))
		{//TODO:有时候可能需要搜索代码段的内容，修改逻辑
			switch (nDataType) {
			case 0://int8
				SearchInt8(Data, entry.modBaseAddr + nIndex, mbi.RegionSize, result);
				break;
			case 1://int16
				SearchInt16(Data, entry.modBaseAddr + nIndex, mbi.RegionSize, result);
				break;
			case 2://int32
				SearchInt32(Data, entry.modBaseAddr + nIndex, mbi.RegionSize, result);
				break;
			case 3://int64
				SearchInt64(Data, entry.modBaseAddr + nIndex, mbi.RegionSize, result);
				break;
			case 4://float
				SearchFloat(Data, entry.modBaseAddr + nIndex, mbi.RegionSize, result);
				break;
			case 5://double
				SearchDouble(Data, entry.modBaseAddr + nIndex, mbi.RegionSize, result);
				break;
			case 6://string
				SearchStr(Data, entry.modBaseAddr + nIndex, mbi.RegionSize, result);
				break;
			}
		}
		nIndex += mbi.RegionSize;
	} while (nIndex < entry.modBaseSize);
	::SendMessage(m_hWnd, WM_PROGRESS, entry.modBaseSize, (LPARAM)&result);
	return 0;
}

int SearchTool::SearchMem(BYTE* begin, SIZE_T nSize, std::string Data, int nDataType)
{
	TRACE("begin:%08X size:%08X data:%s type:%d\r\n", begin, nSize, Data.c_str(), nDataType);
	if (m_hProcess == NULL)return 0;
	Result result(Data, nDataType, "", "");
	switch (nDataType) {
	case 0://int8
		SearchInt8(Data, begin, nSize, result);
		break;
	case 1://int16
		SearchInt16(Data, begin, nSize, result);
		break;
	case 2://int32
		SearchInt32(Data, begin, nSize, result);
		break;
	case 3://int64
		SearchInt64(Data, begin, nSize, result);
		break;
	case 4://float
		SearchFloat(Data, begin, nSize, result);
		break;
	case 5://double
		SearchDouble(Data, begin, nSize, result);
		break;
	case 6://string
		SearchStr(Data, begin, nSize, result);
		break;
	}
	::SendMessage(m_hWnd, WM_PROGRESS, nSize, (LPARAM)&result);
	return 0;
}

void SearchTool::SetTime(int nTime)
{
	m_nTime = nTime;
}

void SearchTool::SetSystemInfo(const SYSTEM_INFO& info)
{
	memcpy(&m_info, &info, sizeof(info));
}

int SearchTool::ModifyMemory(SIZE_T nAddress, std::string Data, int nDataType)
{
	switch (nDataType)
	{
	case 0:
	{
		BYTE d = 0;
		if (Data.size() == 1) {
			d = (BYTE)Data.at(0);
		}
		else if (Data.substr(0, 2) == "0x") {
			d = (BYTE)strtoul(Data.c_str(), NULL, 16);
		}
		else {
			d = (BYTE)strtoul(Data.c_str(), NULL, 10);
		}
		WriteProcessMemory(m_hProcess, (LPVOID)nAddress, &d, 1, NULL);
	}
	break;
	case 1:
	{
		WORD d = 0;
		if (Data.substr(0, 2) == "0x") {
			d = (WORD)strtoul(Data.c_str(), NULL, 16);
		}
		else {
			d = (WORD)strtoul(Data.c_str(), NULL, 10);
		}
		WriteProcessMemory(m_hProcess, (LPVOID)nAddress, &d, 2, NULL);
	}
	break;
	case 2:
	{
		DWORD d = 0;
		if (Data.substr(0, 2) == "0x") {
			d = (DWORD)strtoul(Data.c_str(), NULL, 16);
		}
		else {
			d = (DWORD)strtoul(Data.c_str(), NULL, 10);
		}
		WriteProcessMemory(m_hProcess, (LPVOID)nAddress, &d, 4, NULL);
	}
	break;
	case 3:
	{
		UINT64 d = 0;
		if (Data.substr(0, 2) == "0x") {
			d = (UINT64)strtoul(Data.c_str(), NULL, 16);
		}
		else {
			d = (UINT64)strtoul(Data.c_str(), NULL, 10);
		}
		WriteProcessMemory(m_hProcess, (LPVOID)nAddress, &d, 8, NULL);
	}
	break;
	case 4:
	{
		float d = strtof(Data.c_str(), NULL);
		WriteProcessMemory(m_hProcess, (LPVOID)nAddress, &d, 4, NULL);
	}
	break;
	case 5:
	{
		double d = strtod(Data.c_str(), NULL);
		WriteProcessMemory(m_hProcess, (LPVOID)nAddress, &d, 8, NULL);
	}
	break;
	case 6:
		WriteProcessMemory(m_hProcess, (LPVOID)nAddress, Data.c_str(), Data.size(), NULL);
		break;
	}
	return 0;
}

int SearchTool::ReadMemory(SIZE_T nAddress, std::string& Data, int nDataType)
{
	char buffer[1024] = "";
	BOOL ret = 0;
	switch (nDataType)
	{
	case 0:
	{
		BYTE d = 0;
		ret = ReadProcessMemory(m_hProcess, (LPVOID)nAddress, (LPVOID)&d, 1, NULL);
		snprintf(buffer, sizeof(buffer), "%d", d);
	}
	break;
	case 1:
	{
		WORD d = 0;
		ret = ReadProcessMemory(m_hProcess, (LPVOID)nAddress, (LPVOID)&d, 2, NULL);
		snprintf(buffer, sizeof(buffer), "%d", d);
	}
	break;
	case 2:
	{
		DWORD d = 0;
		ret = ReadProcessMemory(m_hProcess, (LPVOID)nAddress, (LPVOID)&d, 4, NULL);
		snprintf(buffer, sizeof(buffer), "%d", d);
	}
	break;
	case 3:
	{
		UINT64 d = 0;
		ret = ReadProcessMemory(m_hProcess, (LPVOID)nAddress, (LPVOID)&d, 8, NULL);
		snprintf(buffer, sizeof(buffer), "%lld", d);
	}
	break;
	case 4:
	{
		float d = 0.0f;
		ret = ReadProcessMemory(m_hProcess, (LPVOID)nAddress, (LPVOID)&d, 4, NULL);
		snprintf(buffer, sizeof(buffer), "%f", d);
	}
	break;
	case 5:
	{
		double d = 0.0;
		ret = ReadProcessMemory(m_hProcess, (LPVOID)nAddress, (LPVOID)&d, 8, NULL);
		snprintf(buffer, sizeof(buffer), "%f", d);
	}
	break;
	case 6:
		SIZE_T cb = 0;
		ret = ReadProcessMemory(m_hProcess, (LPVOID)nAddress, (LPVOID)&buffer, Data.size(), &cb);
		Data.resize(cb);
		memcpy((void*)Data.c_str(), buffer, cb);
		return 0;
	}
	Data = buffer;
	TRACE("%08llX ret = %d\r\n", nAddress, ret);
	return 0;
}

int SearchTool::DasmCode(SIZE_T nAddress, std::vector<CSInsn>& vInsn)
{
	MEMORY_BASIC_INFORMATION mbi;
	if (VirtualQueryEx(m_hProcess, (LPCVOID)nAddress, &mbi, sizeof(mbi))) {
		std::vector<BYTE> buffer(64);
		SIZE_T sz = 0;
		if (ReadProcessMemory(m_hProcess, (LPCVOID)nAddress, buffer.data(), buffer.size(), &sz) == FALSE) {
			return -1;
		}
		buffer.resize(sz);
		csh handle;
		int ret = (int)cs_open(CS_ARCH_X86, CS_MODE_32, &handle);
		if (ret != CS_ERR_OK) {
			TRACE("cs_open failed:%d\r\n", ret);
			return -2;
		}
		cs_option(handle, CS_OPT_DETAIL, CS_OPT_ON);
		cs_insn* insn;
		uint8_t* code = (uint8_t*)buffer.data();
		size_t count = cs_disasm(handle, code, buffer.size(), nAddress, 0, &insn);
		if (count > 0) {
			vInsn.resize(count);
			for (size_t j = 0; j < count; j++) {
				vInsn[j] = insn[j];
				TRACE("%llX:\t%s\t%s\r\n", insn[j].address, insn[j].mnemonic, insn[j].op_str);
			}
			cs_free(insn, count);
		}
		cs_close(&handle);
	}
	return 0;
}

int SearchTool::SecondSearch(const Result& result, const CString& text, int nDataType)
{
	std::list<SIZE_T> lstAddr;
	std::string data;
	for (SIZE_T addr : result.lstAddress)
	{
		SearchTool::Instance()->ReadMemory(addr, data, nDataType);
		if (data == (LPCSTR)text) {
			lstAddr.push_back(addr);
		}
	}
	::SendMessage(m_hWnd, WM_SECOND, (WPARAM)&lstAddr, 0);
	return 0;
}

void SearchTool::Release()
{
	if (m_instance) {
		delete m_instance;
		m_instance = NULL;
	}
}

SearchTool::SearchTool()
{
	memset(&m_info, 0, sizeof(m_info));
	m_hWnd = NULL;
	m_hProcess = NULL;
	m_nTime = 0;
	m_isbig = false;
}

SearchTool::~SearchTool()
{
}

int SearchTool::SearchInt8(const std::string& Data, BYTE* addr, SIZE_T size, Result& result)
{
	BYTE d = 0;

	if (Data.size() == 1) {
		d = (BYTE)Data.at(0);
	}
	else if (Data.substr(0, 2) == "0x") {
		d = (BYTE)strtoul(2 + (LPCSTR)Data.c_str(), NULL, 16);
	}
	else {
		d = (BYTE)atoi((LPCSTR)Data.c_str());
	}
	BYTE* buffer = new BYTE[size];
	if (buffer == NULL)return -1;
	SIZE_T rb = 0;
	if (ReadProcessMemory(m_hProcess, addr, buffer, size, &rb)) {
		for (DWORD i = 0; i < rb; i++)
		{
			if (*(BYTE*)(buffer + i) == d) {
				result += (SIZE_T)(addr + i);
			}
		}
	}
	delete[]buffer;
	return 0;
}

int SearchTool::SearchInt16(const std::string& Data, BYTE* addr, SIZE_T size, Result& result)
{
	WORD d = 0;
	if (Data.substr(0, 2) == "0x") {
		d = (WORD)strtoul(2 + (LPCSTR)Data.c_str(), NULL, 16);
	}
	else {
		d = (WORD)atoi((LPCSTR)Data.c_str());
	}
	if (m_isbig) {
		d = htons(d);
	}
	BYTE* buffer = new BYTE[size];
	if (buffer == NULL)return -1;
	SIZE_T rb = 0;
	if (ReadProcessMemory(m_hProcess, addr, buffer, size, &rb)) {
		for (DWORD i = 0; i < rb - 1; i += 2)
		{
			if (*(WORD*)(buffer + i) == d) {
				result += (SIZE_T)(addr + i);
			}
		}
	}
	delete[]buffer;
	return 0;
}

int SearchTool::SearchInt32(const std::string& Data, BYTE* addr, SIZE_T size, Result& result)
{
	DWORD d = 0;
	if (Data.substr(0, 2) == "0x") {
		d = (DWORD)strtoul(2 + (LPCSTR)Data.c_str(), NULL, 16);
		TRACE("data = 0x%08X addr %08X size %08X\r\n", d, addr, size);
	}
	else {
		d = (DWORD)atoi((LPCSTR)Data.c_str());
		TRACE("data = %d addr %08X size %08X\r\n", d, addr, size);
	}
	if (m_isbig) {
		d = htonl(d);
	}
	BYTE* buffer = new BYTE[size];
	if (buffer == NULL)return -1;
	SIZE_T rb = 0;
	if (ReadProcessMemory(m_hProcess, addr, buffer, size, &rb)) {
		for (DWORD i = 0; i < rb - 3; i += 2)
		{
			if (*(DWORD*)(buffer + i) == d) {
				result += (SIZE_T)(addr + i);
				//TRACE("found %08llX\r\n", (SIZE_T)(addr + i));
			}
		}
	}
	else {
		TRACE("ReadMemory Failed:%08X\r\n", addr);
	}
	delete[]buffer;
	return 0;
}

int SearchTool::SearchInt64(const std::string& Data, BYTE* addr, SIZE_T size, Result& result)
{
	UINT64 d = 0;
	if (Data.substr(0, 2) == "0x") {
		d = (UINT64)strtoull(2 + Data.c_str(), NULL, 16);
	}
	else {
		d = (UINT64)strtoull(Data.c_str(), NULL, 10);
	}
	if (m_isbig) {
		d = htonl(d >> 32) | (UINT64(htonl(d & 0xFFFFFFFF)) << 32);
	}
	BYTE* buffer = new BYTE[size];
	if (buffer == NULL)return -1;
	SIZE_T rb = 0;
	if (ReadProcessMemory(m_hProcess, addr, buffer, size, &rb)) {
		for (DWORD i = 0; i < rb - 7; i += 4)
		{
			if (*(UINT64*)(buffer + i) == d) {
				result += (SIZE_T)(addr + i);
			}
		}
	}
	delete[]buffer;
	return 0;
}

int SearchTool::SearchFloat(const std::string& Data, BYTE* addr, SIZE_T size, Result& result)
{
	float f = strtof(Data.c_str(), NULL);
	DWORD d = *(DWORD*)&f;
	if (m_isbig) {
		d = htonl(d);
	}
	BYTE* buffer = new BYTE[size];
	if (buffer == NULL)return -1;
	SIZE_T rb = 0;
	if (ReadProcessMemory(m_hProcess, addr, buffer, size, &rb)) {
		for (DWORD i = 0; i < rb - 3; i += 2)
		{
			if (*(DWORD*)(buffer + i) == d) {
				result += (SIZE_T)(addr + i);
			}
		}
	}
	delete[]buffer;
	return 0;
}

int SearchTool::SearchDouble(const std::string& Data, BYTE* addr, SIZE_T size, Result& result)
{
	double D = strtod(Data.c_str(), NULL);
	UINT64 d = *(UINT64*)&D;
	if (m_isbig) {
		d = htonl(d >> 32) | (UINT64(htonl(d & 0xFFFFFFFF)) << 32);
	}
	BYTE* buffer = new BYTE[size];
	if (buffer == NULL)return -1;
	SIZE_T rb = 0;
	if (ReadProcessMemory(m_hProcess, addr, buffer, size, &rb)) {
		for (DWORD i = 0; i < rb - 7; i += 4)
		{
			if (*(UINT64*)(buffer + i) == d) {
				result += (SIZE_T)(addr + i);
			}
		}
	}
	delete[]buffer;
	return 0;
}

int SearchTool::SearchStr(const std::string& Data, BYTE* addr, SIZE_T size, Result& result)
{
	BYTE* d = (BYTE*)Data.c_str();
	size_t len = Data.size();

	BYTE* buffer = new BYTE[size];
	if (buffer == NULL)return -1;
	SIZE_T rb = 0;
	if (ReadProcessMemory(m_hProcess, addr, buffer, size, &rb)) {
		for (DWORD i = 0; i <= rb - len; i++)
		{//TODO:字符串搜索 还可以优化 kmp <== sunday
			if (memcmp((buffer + i), d, len) == 0) {
				result += (SIZE_T)(addr + i);
				TRACE("找到了：%016llX\r\n", (SIZE_T)(addr + i));
			}
		}
	}
	delete[]buffer;
	return 0;
}

CSInsn::CSInsn()
{
	memset(this, 0, sizeof(CSInsn));
}

CSInsn::~CSInsn()
{
	if (detail) {
		delete detail;
		detail = NULL;
	}
}

CSInsn::CSInsn(const CSInsn& insn)
{
	memcpy(this, &insn, sizeof(CSInsn));
	if (detail) {
		detail = new cs_detail;
		memcpy(detail, insn.detail, sizeof(cs_detail));
	}
}

CSInsn::CSInsn(const cs_insn& insn)
{
	memcpy(this, &insn, sizeof(cs_insn));
	if (detail) {
		detail = new cs_detail;
		memcpy(detail, insn.detail, sizeof(cs_detail));
	}
}

CSInsn& CSInsn::operator=(const cs_insn& insn)
{
	memcpy(this, &insn, sizeof(cs_insn));
	if (detail) {
		detail = new cs_detail;
		memcpy(detail, insn.detail, sizeof(cs_detail));
	}
	return *this;
}

CSInsn& CSInsn::operator=(const CSInsn& insn)
{
	if (this != &insn) {
		memcpy(this, &insn, sizeof(cs_insn));
		if (detail) {
			detail = new cs_detail;
			memcpy(detail, insn.detail, sizeof(cs_detail));
		}
	}
	return *this;
}
