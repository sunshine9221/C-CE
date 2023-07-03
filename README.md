# C-CE
基于c/c++实现的ce工具


static SearchTool* Instance();//单例
void SetHWND(HWND hWnd);//设置句柄
void SetProcess(HANDLE hProcess);//设置线程
int SearchCode(MODULEENTRY32 entry, std::string Data, int nDataType);//搜索代码
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


![结构](https://github.com/sunshine9221/C-CE/assets/77560185/f103aeb1-75bc-4e64-89f9-c694be5ce40f)
