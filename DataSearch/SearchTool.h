#pragma once
#include <Windows.h>
#include <TlHelp32.h>
#include <list>
#include <vector>
#include <string>
#include <capstone/capstone.h>
class CSInsn :public cs_insn
{
public:
	CSInsn();
	~CSInsn();
	CSInsn(const CSInsn& insn);
	CSInsn(const cs_insn& insn);
	CSInsn& operator=(const cs_insn& insn);
	CSInsn& operator=(const CSInsn& insn);
};
class Result
{
public:
	Result() {
		nType = -1;
		nTime = 0;
	}
	Result(const std::string& data, int nDataType, const std::string& mod, const std::string& path) {
		strData = data;
		nType = nDataType;
		strModule = mod;
		strPath = path;
	}
	Result(const Result& result) {
		lstAddress = result.lstAddress;
		strData = result.strData;
		nType = result.nType;
		strModule = result.strModule;
		strPath = result.strPath;
	}
	~Result() {}
	Result& operator=(const Result& result) {
		if (this != &result) {
			lstAddress = result.lstAddress;
			strData = result.strData;
			nType = result.nType;
			strModule = result.strModule;
			strPath = result.strPath;
		}
		return *this;
	}
	Result& operator+=(SIZE_T addr) {
		lstAddress.push_back(addr);
		return*this;
	}
	Result& operator+=(const Result& result) {
		lstAddress.insert(lstAddress.end(), result.lstAddress.begin(), result.lstAddress.end());
		return*this;
	}
	void Clear() {
		lstAddress.clear();
	}
	size_t Size() { return lstAddress.size(); }
public:
	std::list<SIZE_T> lstAddress;
	std::string strData;
	int nType;
	std::string strModule;
	std::string strPath;
	int nTime;//第几次，从1开始
};

class SearchTool
{
public:
	static SearchTool* Instance();
	void SetHWND(HWND hWnd);
	void SetProcess(HANDLE hProcess);
	int SearchCode(MODULEENTRY32 entry, std::string Data, int nDataType);
	int Search(MODULEENTRY32 entry, std::string Data, int nDataType);
	int SearchMem(BYTE* begin, SIZE_T nSize, std::string Data, int nDataType);
	void SetTime(int nTime);//设置次数
	void SetSystemInfo(const SYSTEM_INFO& info);
	int ModifyMemory(SIZE_T nAddress, std::string Data, int nDataType);
	int ReadMemory(SIZE_T nAddress, std::string& Data, int nDataType);
	int DasmCode(SIZE_T nAddress, std::vector<CSInsn>& lstInsn);
	void SetEndian(bool isBigEndian = false) {
		m_isbig = isBigEndian;
	}
	int SecondSearch(const Result& result, const CString& text, int nDataType);
protected:
	static void Release();
	SearchTool();
	~SearchTool();
	int SearchInt8(const std::string& Data, BYTE* addr, SIZE_T size, Result& result);
	int SearchInt16(const std::string& Data, BYTE* addr, SIZE_T size, Result& result);
	int SearchInt32(const std::string& Data, BYTE* addr, SIZE_T size, Result& result);
	int SearchInt64(const std::string& Data, BYTE* addr, SIZE_T size, Result& result);
	int SearchFloat(const std::string& Data, BYTE* addr, SIZE_T size, Result& result);
	int SearchDouble(const std::string& Data, BYTE* addr, SIZE_T size, Result& result);
	int SearchStr(const std::string& Data, BYTE* addr, SIZE_T size, Result& result);
public:
	SYSTEM_INFO m_info;
private:
	bool m_isbig;//true 大顶端 false 小顶端
	int m_nTime;
	HWND m_hWnd;
	HANDLE m_hProcess;
	static SearchTool* m_instance;
	static class SearchToolHelper {
	public:
		SearchToolHelper() {
			SearchTool::Instance();
		}
		~SearchToolHelper() {
			SearchTool::Release();
		}
	}_;
};

#define RESULT_SIZE 200