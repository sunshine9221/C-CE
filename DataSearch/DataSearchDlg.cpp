
// DataSearchDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "DataSearch.h"
#include "DataSearchDlg.h"
#include "afxdialogex.h"
#include "SearchTool.h"
#include <capstone/platform.h>
#include <capstone/capstone.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#pragma comment(lib,"Psapi.lib")
#ifdef WIN64
#pragma comment(lib,"capstone-x64.lib")
#else
#pragma comment(lib,"capstone.lib")
#endif

// CDataSearchDlg 对话框
#define PAGE_WRITE_FLAGS (PAGE_WRITECOPY | PAGE_EXECUTE_READWRITE | PAGE_READWRITE | PAGE_EXECUTE_WRITECOPY)


CDataSearchDlg::CDataSearchDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_DATASEARCH_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_isinject = FALSE;
	m_npid = -1;
	m_total = 0;
	m_hProcess = NULL;
	m_curpos = 0;
}

void CDataSearchDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_PROCESS, m_pidlist);
	DDX_Control(pDX, IDC_LIST_MODULE, m_module);
	DDX_Control(pDX, IDC_EDIT_SEARCH, m_search);
	DDX_Control(pDX, IDC_EDIT_PID, m_pid_search);
	DDX_Control(pDX, IDC_LST_ADDR, m_lstAddr);
	DDX_Control(pDX, IDC_ADD_ADDR, m_addr);
	DDX_Control(pDX, IDC_SEARCH_PROC, m_progress);
	DDX_Control(pDX, IDC_COMBO_TYPE, m_DataType);
	DDX_Control(pDX, IDC_BTN_MEM_SEARCH, m_btn_search);
	DDX_Control(pDX, IDC_SEARCH_TEXT, m_data);
	DDX_Control(pDX, IDC_EDIT_MODIFY, m_value);
	DDX_Control(pDX, IDC_EDIT_MEMADDR, m_address);
	DDX_Control(pDX, IDC_EDIT_MEM, m_memshow);
	DDX_Control(pDX, IDC_EDIT_CODE, m_code);
	DDX_Control(pDX, IDC_CK_ISBIG, m_btnIsBig);
	DDX_Control(pDX, IDC_LIST_FAV, m_favorite);
	DDX_Control(pDX, IDC_DASM, m_dasm);
}

BEGIN_MESSAGE_MAP(CDataSearchDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CDataSearchDlg::OnBnClickedOk)
	ON_BN_CLICKED(ID_BTN_DONE, &CDataSearchDlg::OnBnClickedBtnDone)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_PROCESS, &CDataSearchDlg::OnLvnItemchangedListProcess)
	ON_EN_CHANGE(IDC_EDIT_SEARCH, &CDataSearchDlg::OnEnChangeEditSearch)
	ON_EN_CHANGE(IDC_EDIT_PID, &CDataSearchDlg::OnEnChangeEditPid)
	ON_BN_CLICKED(IDC_BTN_MEM_SEARCH, &CDataSearchDlg::OnBnClickedBtnMemSearch)
	ON_MESSAGE(WM_PROGRESS, &CDataSearchDlg::OnProgress)
	ON_MESSAGE(WM_CODE_PROGRESS, &CDataSearchDlg::OnCodeProgress)
	ON_MESSAGE(WM_SECOND, &CDataSearchDlg::OnSecond)
	ON_BN_CLICKED(IDC_BTN_MEM_MODIFY, &CDataSearchDlg::OnBnClickedBtnMemModify)
	ON_BN_CLICKED(IDC_SECOND_SEARCH, &CDataSearchDlg::OnBnClickedSecondSearch)
	ON_BN_CLICKED(IDC_BTN_MEM_SHOW, &CDataSearchDlg::OnBnClickedBtnMemShow)
	ON_BN_CLICKED(IDC_BTN_ADD_ADDR, &CDataSearchDlg::OnBnClickedBtnAddAddr)
	ON_NOTIFY(NM_THEMECHANGED, IDC_SCROLLBAR1, &CDataSearchDlg::OnNMThemeChangedScrollbar)
	ON_BN_CLICKED(IDC_BTN_DASM, &CDataSearchDlg::OnBnClickedBtnDasm)
	ON_BN_CLICKED(IDC_UPDATE_MEM, &CDataSearchDlg::OnBnClickedUpdateMem)
	ON_BN_CLICKED(IDC_BTN_CLEAR, &CDataSearchDlg::OnBnClickedBtnClear)
	ON_NOTIFY(NM_RCLICK, IDC_LST_ADDR, &CDataSearchDlg::OnNMRClickLstAddr)
	ON_COMMAND(ID_DELSEL, &CDataSearchDlg::OnDelsel)
	ON_COMMAND(ID_LOCKSEL, &CDataSearchDlg::OnLocksel)
	ON_COMMAND(ID_COPYSEL, &CDataSearchDlg::OnCopysel)
	ON_COMMAND(ID_ADD_WATCH, &CDataSearchDlg::OnAddWatch)
	ON_COMMAND(ID_FRUSH, &CDataSearchDlg::OnFrush)
	ON_COMMAND(ID_COPY_ALL, &CDataSearchDlg::OnCopyAll)
	ON_COMMAND(ID_SELECT_ALL, &CDataSearchDlg::OnSelectAll)
	ON_COMMAND(ID_SELECT_REVERSE, &CDataSearchDlg::OnSelectReverse)
	ON_COMMAND(ID_DELETE_ALL, &CDataSearchDlg::OnDeleteAll)
	ON_BN_CLICKED(IDC_BTN_LOCATE, &CDataSearchDlg::OnBnClickedBtnLocate)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_MODULE, &CDataSearchDlg::OnNMRClickListModule)
	ON_COMMAND(ID_COPY_MODULE, &CDataSearchDlg::OnCopyModule)
	ON_COMMAND(ID_FRUSH_MOD, &CDataSearchDlg::OnFrushMod)
	ON_BN_CLICKED(IDC_BTN_TEST, &CDataSearchDlg::OnBnClickedBtnTest)
END_MESSAGE_MAP()


// CDataSearchDlg 消息处理程序

void CDataSearchDlg::SetClipboardCopy(const std::string& text)
{
	OpenClipboard();
	EmptyClipboard();
	HGLOBAL clip = GlobalAlloc(GMEM_DDESHARE, text.size() + 1);
	if (clip == NULL) {
		TRACE("GlobalAlloc错误：%d\r\n", GetLastError());
		MessageBox("内存不足！");
		return;
	}
	char* pbuf = (char*)GlobalLock(clip);
	if (pbuf == NULL) {
		TRACE("GlobalLock错误：%d\r\n", GetLastError());
		MessageBox("内存无法锁定！");
		return;
	}
	memcpy(pbuf, text.c_str(), text.size());
	GlobalUnlock(clip);
	SetClipboardData(CF_TEXT, clip);
	CloseClipboard();
}

BOOL CDataSearchDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	m_npid = -1;
	DWORD style = m_pidlist.GetExtendedStyle();
	m_pidlist.SetExtendedStyle(style | LVS_EX_FULLROWSELECT);
	m_pidlist.InsertColumn(0, _T("主进程ID"), 0, 80);
	m_pidlist.InsertColumn(1, _T("父进程ID"), 0, 80);
	m_pidlist.InsertColumn(2, _T("进程名称"), 0, 160);
	style = m_module.GetExtendedStyle();
	m_module.SetExtendedStyle(style | LVS_EX_FULLROWSELECT);
	m_module.InsertColumn(0, _T("模块ID"), 0, 50);
	m_module.InsertColumn(1, _T("模块句柄"), 0, 140);
	m_module.InsertColumn(2, _T("模块地址"), 0, 140);
	m_module.InsertColumn(3, _T("模块大小"), 0, 80);
	m_module.InsertColumn(4, _T("模块名称"), 0, 160);
	m_module.InsertColumn(5, _T("模块路径"), 0, 350);
	style = m_lstAddr.GetExtendedStyle();
	m_lstAddr.SetExtendedStyle(style | LVS_EX_CHECKBOXES | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);
	m_favorite.InsertColumn(0, _T("地址"), 0, 140);
	m_favorite.InsertColumn(1, _T("数据"), 0, 80);
	m_favorite.InsertColumn(2, _T("含义"), 0, 80);
	style = m_favorite.GetExtendedStyle();
	m_favorite.SetExtendedStyle(style | LVS_EX_CHECKBOXES | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);
	m_lstAddr.InsertColumn(0, _T("地址"), 0, 140);
	m_lstAddr.InsertColumn(1, _T("首次结果"), 0, 80);
	m_isinject = FALSE;
	m_DataType.SetCurSel(2);
	UpdatePid();
	SearchTool::Instance()->SetHWND(GetSafeHwnd());
	m_pool.Start(4);
	SYSTEM_INFO info;
	GetSystemInfo(&info);
	SearchTool::Instance()->SetSystemInfo(info);
	m_rmenu.LoadMenu(IDR_MENU_ADDR);
	TRACE("value %p func %p\r\n", &m_rmenu, &CDataSearchDlg::OnInitDialog);
	m_mem_font.CreateFont(16,
		0, // nWidth 
		0, // nEscapement 
		0, // nOrientation 
		FW_THIN, // nWeight 
		FALSE, // bItalic 
		FALSE, // bUnderline 
		0, // cStrikeOut 
		ANSI_CHARSET, // nCharSet 
		OUT_DEFAULT_PRECIS, // nOutPrecision 
		CLIP_DEFAULT_PRECIS, // nClipPrecision 
		DEFAULT_QUALITY, // nQuality 
		DEFAULT_PITCH | FF_SWISS, // nPitchAndFamily 
		_T("Courier")
	);
	m_memshow.SetFont(&m_mem_font);
	m_once = false;
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CDataSearchDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CDataSearchDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CDataSearchDlg::OnBnClickedOk()
{
	if (m_isinject == TRUE) {
		MessageBox("已经注入过了！", "停止注入", MB_OK | MB_ICONWARNING);
		return;
	}
	TRACE("进程ID:%d\r\n", m_npid);
	if (m_npid >= 0) {
		HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, m_npid);
		TRACE("进程句柄：%016llX\r\n", hProc);
		if (hProc != NULL && (hProc != INVALID_HANDLE_VALUE)) {
#ifdef WIN32
			CString dll_path = "C:\\Users\\25052\\Desktop\\datasearch\\Debug\\inject.dll";
#else
			CString dll_path = "E:\\edoyun\\public\\DataSearch\\x64\\Debug\\inject.dll";
#endif
			LoadLibraryA((LPCTSTR)dll_path);
			LPVOID lpStr = VirtualAllocEx(hProc, NULL, dll_path.GetLength() + 1, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
			if (lpStr) {
				SIZE_T dwWrite = 0;
				//ReadProcessMemory(hProc,)
				if (WriteProcessMemory(hProc, lpStr, (LPCVOID)(LPCTSTR)dll_path, dll_path.GetLength(), &dwWrite))
				{
					HMODULE hModule = GetModuleHandle("kernel32.dll");
					if (hModule != NULL) {
						LPVOID lpBaseAddress = (LPVOID)GetProcAddress(hModule, "LoadLibraryA");
						if (lpBaseAddress != NULL) {
							DWORD nThreadID = 0;
							HANDLE hThread = CreateRemoteThread(hProc, NULL, 0, (LPTHREAD_START_ROUTINE)lpBaseAddress, lpStr, 0, &nThreadID);
							if (hThread == NULL) {
								TRACE("启动失败：%d\r\n", GetLastError());
							}
							m_isinject = TRUE;
							UpdateModule(m_npid);
						}
						else {
							TRACE("启动失败：%d\r\n", GetLastError());
						}
					}
					else {
						TRACE("启动失败：%d\r\n", GetLastError());
					}
				}
				else {
					TRACE("启动失败：%d\r\n", GetLastError());
				}
			}
			else {
				TRACE("启动失败：%d\r\n", GetLastError());
			}
			CloseHandle(hProc);
		}
		else {
			TRACE("启动失败：%d\r\n", GetLastError());
		}
	}
}


void CDataSearchDlg::UpdatePid()
{
	m_pidlist.DeleteAllItems();
	PROCESSENTRY32 pe32;
	pe32.dwSize = sizeof(pe32);	//该结构体的大小

	//给系统内的所有进程拍一个快照
	HANDLE hProcess = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcess == INVALID_HANDLE_VALUE)
	{
		TRACE("失败\r\n");
		return;
	}

	//遍历进程快照，轮流显示每个进程的信息
	BOOL bMore = Process32First(hProcess, &pe32);
	int i = 0;
	while (bMore)
	{
		//pe32.szExeFile为多字节
		TRACE("进程名称：%s\n父进程ID:%u 进程ID: %u\n\n", pe32.szExeFile, pe32.th32ParentProcessID, pe32.th32ProcessID);
		char pid[10] = "";
		snprintf(pid, sizeof(pid), "%d", pe32.th32ProcessID);
		m_pidlist.InsertItem(i, pid);
		snprintf(pid, sizeof(pid), "%d", pe32.th32ParentProcessID);
		m_pidlist.SetItemText(i, 1, pid);
		m_pidlist.SetItemText(i, 2, pe32.szExeFile);
		bMore = Process32Next(hProcess, &pe32);
		i++;
	}

	//最后关闭操作完成后的句柄
	CloseHandle(hProcess);

}

void CDataSearchDlg::UpdateModule(DWORD nPID)
{
	m_module.DeleteAllItems();
	MODULEENTRY32 entry;
	entry.dwSize = sizeof(MODULEENTRY32);
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, nPID);
	if (hSnapshot == INVALID_HANDLE_VALUE)
	{
		TRACE("失败！pid:%d %d\r\n", nPID, GetLastError());
		return;
	}
	BOOL bMore = Module32First(hSnapshot, &entry);
	int i = 0;
	m_isinject = FALSE;
	m_total = 0;
	while (bMore) {
		//pe32.szExeFile为多字节
		TRACE("模块名称：%s\n模块路径:%s 模块ID: %u 模块句柄：%08llX\n\n",
			entry.szModule, entry.szExePath, entry.th32ModuleID, entry.hModule);
		char pid[32] = "";
		snprintf(pid, sizeof(pid), "%d", entry.th32ModuleID);
		m_module.InsertItem(i, pid);
		snprintf(pid, sizeof(pid), "%016llX", (UINT64)entry.hModule);//
		m_module.SetItemText(i, 1, pid);
		snprintf(pid, sizeof(pid), "%016llX", (UINT64)entry.modBaseAddr);
		m_module.SetItemText(i, 2, pid);
		snprintf(pid, sizeof(pid), "%08X", (UINT)entry.modBaseSize);
		m_module.SetItemText(i, 3, pid);
		m_module.SetItemText(i, 4, entry.szModule);
		if (CString(entry.szModule) == "") {
			m_isinject = TRUE;
		}
		m_module.SetItemText(i, 5, entry.szExePath);
		m_lstModules.push_back(entry);
		m_total += entry.modBaseSize;
		bMore = Module32Next(hSnapshot, &entry);
		i++;
	}
}


void CDataSearchDlg::OnBnClickedBtnDone()
{
	UpdatePid();
}


void CDataSearchDlg::OnLvnItemchangedListProcess(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
	if (pNMLV && pNMLV->iItem >= 0) {
		CString sPid = m_pidlist.GetItemText(pNMLV->iItem, 0);
		m_npid = (DWORD)atoi((LPCTSTR)sPid);
		UpdateModule(m_npid);
	}
	else {
		m_npid = -1;
	}
}


void CDataSearchDlg::OnEnChangeEditSearch()
{
	CString text;
	m_search.GetWindowTextA(text);
	TRACE("%s\r\n", (LPCTSTR)text);
	if (text.GetLength() > 0) {
		int count = m_pidlist.GetItemCount();
		for (int i = 0; i < count; i++)
		{
			CString name = m_pidlist.GetItemText(i, 2);
			if (name.Find(text) >= 0) {
				m_pidlist.EnsureVisible(i, FALSE);
				TRACE("item = %d\r\n", i);
				return;
			}
		}
	}
}


void CDataSearchDlg::OnEnChangeEditPid()
{
	CString text;
	m_pid_search.GetWindowTextA(text);
	TRACE("%s\r\n", (LPCTSTR)text);
	if (text.GetLength() > 0) {
		int count = m_pidlist.GetItemCount();
		for (int i = 0; i < count; i++)
		{
			CString name = m_pidlist.GetItemText(i, 0);
			if (name.Find(text) >= 0) {
				m_pidlist.EnsureVisible(i, FALSE);
				TRACE("item = %d\r\n", i);
				return;
			}
		}
	}
}


void CDataSearchDlg::OnBnClickedBtnMemSearch()
{
	CString text;
	m_btn_search.GetWindowText(text);
	if (text == "内存搜索") {
		if (m_npid == -1) {
			MessageBox("请选择进程！", "进程为空", MB_OK | MB_ICONERROR);
			return;
		}
		if (m_lstModules.size() <= 0) {
			MessageBox("无法获取进程信息！", "进程无法访问！", MB_OK | MB_ICONERROR);
			return;
		}
		if (m_DataType.GetCurSel() == -1) {
			MessageBox("请选择数据类型！", "没有类型", MB_OK | MB_ICONERROR);
			return;
		}
		m_data.GetWindowText(text);
		if (text.GetLength() <= 0) {
			MessageBox("请输入搜索数据！", "没有数据", MB_OK | MB_ICONERROR);
			return;
		}
		m_hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, m_npid);
		TRACE("进程句柄：%016llX\r\n", m_hProcess);
		if (m_hProcess == NULL || (m_hProcess == INVALID_HANDLE_VALUE)) {
			MessageBox("打开进程失败，权限不足！", "打开失败", MB_OK | MB_ICONERROR);
			return;
		}
		SearchTool::Instance()->SetProcess(m_hProcess);
		m_progress.SetRange(0, 1000);
		m_progress.SetPos(0);
		m_count = 0;
		m_curpos = 0;
		m_btn_search.SetWindowText("取消搜索");

		//BYTE* pMin = (BYTE*)(0x1000 * (SIZE_T)(SearchTool::Instance()->m_info.lpMinimumApplicationAddress));
		BYTE* pMin = (BYTE*)(SearchTool::Instance()->m_info.lpMinimumApplicationAddress);
		//0x10000
		//BYTE* pMax = (BYTE*)(0x1000 * (SIZE_T)(SearchTool::Instance()->m_info.lpMaximumApplicationAddress));
		BYTE* pMax = (BYTE*)(SearchTool::Instance()->m_info.lpMaximumApplicationAddress);
		//0x7FEFFFFF
		TRACE("min %016llX max %016llX\r\n", pMin, pMax);
		size_t nIndex = 0;
		size_t nSize = pMax - pMin;
		m_total = nSize;
		do {
			MEMORY_BASIC_INFORMATION mbi;
			if (VirtualQueryEx(m_hProcess, (LPCVOID)(pMin + nIndex),
				&mbi, sizeof(mbi)) == 0)
			{
				nIndex += SearchTool::Instance()->m_info.dwPageSize;
				SendMessage(WM_PROGRESS, SearchTool::Instance()->m_info.dwPageSize, NULL);
				continue;
			}
			if (mbi.Protect & PAGE_WRITE_FLAGS) {
				m_pool.AddTask(Task(
					&SearchTool::SearchMem, SearchTool::Instance(),
					(BYTE*)mbi.BaseAddress, mbi.RegionSize,
					std::string((LPCSTR)text), m_DataType.GetCurSel()
				));
			}
			else {
				SendMessage(WM_PROGRESS, mbi.RegionSize, NULL);
			}
			nIndex += mbi.RegionSize;
		} while (nIndex < nSize);
	}
	else {//取消搜索
		m_btn_search.SetWindowText("内存搜索");
		SearchTool::Instance()->SetProcess(NULL);
		CloseHandle(m_hProcess);
		m_hProcess = NULL;

	}
}

LRESULT CDataSearchDlg::OnProgress(WPARAM wParam, LPARAM lParam)
{
	m_count++;
	m_curpos += wParam;
	Result* pResult = (Result*)lParam;
	m_progress.SetPos(int(m_curpos * 1000LL / m_total));
	if (pResult) {
		m_result += *pResult;
		if (m_lstAddr.GetItemCount() < RESULT_SIZE) {
			char buffer[32] = "";
			for (auto addr : (*pResult).lstAddress) {
#ifdef WIN64
				snprintf(buffer, sizeof(buffer), "%016llX", addr);
#else
				snprintf(buffer, sizeof(buffer), "%08X", addr);
#endif
				m_lstAddr.InsertItem(0, buffer);
				m_lstAddr.SetItemText(0, 1, (*pResult).strData.c_str());
				if (m_lstAddr.GetItemCount() >= RESULT_SIZE)break;
			}
		}
	}
	TRACE("cur %016llX nSize %016llX\r\n", m_curpos.load(), m_total);
	if (m_curpos >= m_total && (m_hProcess != NULL)) {
		if (m_once) {
			if (m_result.Size() == 1) {
				SIZE_T nAddr = m_result.lstAddress.front();
				SearchTool::Instance()->ModifyMemory(nAddr - 0x8C, std::string("99999"), 2);
				SearchTool::Instance()->ModifyMemory(nAddr - 0x9C, std::string("99999"), 2);
				SearchTool::Instance()->ModifyMemory(nAddr - 0x2D4071D0, std::string("99999"), 2);
			}
		}
		m_btn_search.SetWindowText("内存搜索");
		SearchTool::Instance()->SetProcess(NULL);
		CloseHandle(m_hProcess);
		m_hProcess = NULL;
		TRACE("搜索完成！ %lld\r\n", m_result.Size());
		MessageBox("搜索完成！");

		return 0;
	}
	return 0;
}
/*0x8C
 0x9C
 0x2D4071D0*/

LRESULT CDataSearchDlg::OnCodeProgress(WPARAM wParam, LPARAM lParam)
{
	m_count++;
	m_curpos += wParam;
	Result* pResult = (Result*)lParam;
	m_progress.SetPos(int(m_curpos * 1000LL / m_total));
	if (pResult) {
		m_code_result += *pResult;
		{
			char buffer[32] = "";
			for (auto addr : (*pResult).lstAddress) {
#ifdef WIN64
				snprintf(buffer, sizeof(buffer), "%08llX", addr);
#else
				snprintf(buffer, sizeof(buffer), "%08X", addr);
#endif
				TRACE("Addr: %016llX\r\n", addr);
			}
		}
	}
	if (m_curpos >= m_total && (m_hProcess != NULL)) {
		m_btn_search.SetWindowText("内存搜索");
		SearchTool::Instance()->SetProcess(NULL);
		CloseHandle(m_hProcess);
		m_hProcess = NULL;
		TRACE("搜索完成！ %lld\r\n", m_code_result.Size());
		MessageBox("搜索完成！");
		return 0;
	}
	return 0;
}

LRESULT CDataSearchDlg::OnSecond(WPARAM wParam, LPARAM lParam)
{
	m_lstAddr.DeleteAllItems();
	std::list<SIZE_T>* pResult = (std::list<SIZE_T>*)wParam;
	if (pResult) {
		CString data;
		m_data.GetWindowText(data);
		std::list<SIZE_T>& lstAddr = *pResult;
		int i = 0;
		TRACE("result %lld\r\n", lstAddr.size());
		for (SIZE_T addr : lstAddr) {
			if (i++ >= RESULT_SIZE)break;
			int index = m_lstAddr.GetItemCount();
			char buffer[32];
			snprintf(buffer, sizeof(buffer), "%016llX", addr);
			m_lstAddr.InsertItem(index, buffer);
			m_lstAddr.SetItemText(index, 1, data);
		}
		m_result.Clear();
		m_result.lstAddress = lstAddr;
	}
	m_data.SetReadOnly(false);
	SearchTool::Instance()->SetProcess(NULL);
	CloseHandle(m_hProcess);
	MessageBox("搜索完毕！");
	return 0;
}


void CDataSearchDlg::OnBnClickedBtnMemModify()
{
	if (m_npid == -1) {
		MessageBox("请选择进程！", "进程为空", MB_OK | MB_ICONERROR);
		return;
	}
	CString text;
	m_value.GetWindowText(text);
	if (text.GetLength() <= 0) {
		MessageBox("请输入修改数据！", "没有数据", MB_OK | MB_ICONERROR);
		return;
	}
	m_hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, m_npid);
	SearchTool::Instance()->SetProcess(m_hProcess);
	for (int i = 0; i < m_lstAddr.GetItemCount(); i++)
	{
		if (m_lstAddr.GetCheck(i)) {
			CString address = m_lstAddr.GetItemText(i, 0);
			DWORD addr = strtoul((LPCSTR)address, NULL, 16);
			SearchTool::Instance()->ModifyMemory(addr, (LPCSTR)text, m_DataType.GetCurSel());
		}
	}
	CloseHandle(m_hProcess);
	TRACE("修改完成！\r\n");
}


void CDataSearchDlg::OnBnClickedSecondSearch()
{
	if (m_npid == -1) {
		MessageBox("请选择进程！", "进程为空", MB_OK | MB_ICONERROR);
		return;
	}
	CString text;
	m_data.GetWindowText(text);
	m_data.SetReadOnly();
	if (text.GetLength() <= 0) {
		MessageBox("请输入搜索数据！", "没有数据", MB_OK | MB_ICONERROR);
		return;
	}
	m_hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, m_npid);
	SearchTool::Instance()->SetProcess(m_hProcess);
	if (m_result.Size() >= RESULT_SIZE) {
		m_pool.AddTask(Task(
			&SearchTool::SecondSearch, SearchTool::Instance(),
			m_result, text, m_DataType.GetCurSel()
		));
	}
	else {
		int i = 0;
		for (SIZE_T addr : m_result.lstAddress)
		{
			std::string data;
			SearchTool::Instance()->ReadMemory(addr, data, m_DataType.GetCurSel());
			if (data == (LPCSTR)text) {
				m_lstAddr.SetItemText(i++, 1, data.c_str());
			}
			else {
				m_lstAddr.DeleteItem(i);
			}
		}
		m_data.SetReadOnly(false);
		SearchTool::Instance()->SetProcess(NULL);
		CloseHandle(m_hProcess);
	}
}


void CDataSearchDlg::OnBnClickedBtnMemShow()
{
	CString text;
	m_address.GetWindowText(text);
	size_t nAddress = strtoull((LPCSTR)text, NULL, 16);
	m_hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, m_npid);
	BYTE buffer[32 * 16];
	SIZE_T cb = 0;
	MEMORY_BASIC_INFORMATION bmi;
	VirtualQueryEx(m_hProcess, (LPCVOID)nAddress, &bmi, sizeof(bmi));
	TRACE("protect %08X size %llX base %016llX\r\n", bmi.Protect, bmi.RegionSize, bmi.BaseAddress);
	if (ReadProcessMemory(m_hProcess, (LPCVOID)(nAddress), buffer, sizeof(buffer), &cb) == FALSE)
	{
		TRACE("ReadProcessMemory 失败%d\r\n", GetLastError());
		MessageBox("读取失败！");
		return;
	}

	text.Empty();
	char line[32] = "";
	for (DWORD i = 0; i < cb; i += 16) {
#ifdef WIN64
		snprintf(line, sizeof(line), "%016llX: ", nAddress + i);
#else
		snprintf(line, sizeof(line), "%08X: ", nAddress + i);
#endif
		text += line;
		for (DWORD j = i; (j < i + 16) && (j < cb); j++)
		{
			snprintf(line, sizeof(line), "%02X ", 0xFF & buffer[j]);
			text += line;
		}
		text += "; ";
		for (DWORD j = i; (j < i + 16) && (j < cb); j++)
		{
			if ((buffer[j] < 32) || (buffer[j] > 0x7F)) {
				text += ".";
			}
			else {
				text += (char)buffer[j];
			}
		}
		text += "\r\n";
	}
	m_memshow.SetWindowText(text);
	CloseHandle(m_hProcess);
}


void CDataSearchDlg::OnBnClickedBtnAddAddr()
{
	CString text;
	m_addr.GetWindowText(text);
	m_lstAddr.InsertItem(m_lstAddr.GetItemCount(), text);
}


void CDataSearchDlg::OnNMThemeChangedScrollbar(NMHDR* pNMHDR, LRESULT* pResult)
{
	*pResult = 0;

}

#define X86_CODE32 "\x8d\x4c\x32\x08\x01\xd8\x81\xc6\x34\x12\x00\x00\x05\x23\x01\x00\x00\x36\x8b\x84\x91\x23\x01\x00\x00\x41\x8d\x84\x39\x89\x67\x00\x00\x8d\x87\x89\x67\x00\x00\xb4\xc6\xe9\xea\xbe\xad\xde\xff\xa0\x23\x01\x00\x00\xe8\xdf\xbe\xad\xde\x74\xff"

void CDataSearchDlg::OnBnClickedBtnDasm()
{
	m_hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, m_npid);
	SearchTool::Instance()->SetProcess(m_hProcess);
	CString text;
	m_code.GetWindowText(text);
	if (text.GetLength() == 0) {
		MessageBox("请输入代码地址！");
		return;
	}
	if (m_npid == -1) {
		MessageBox("请选择进程！", "进程为空", MB_OK | MB_ICONERROR);
		return;
	}
	std::vector<CSInsn> vInsn;
	SearchTool::Instance()->DasmCode(strtoull((LPCTSTR)text, NULL, 16), vInsn);
	SearchTool::Instance()->SetProcess(NULL);
	CloseHandle(m_hProcess);
	text.Empty();
	char line[1024] = "";
	for (auto insn : vInsn) {
		snprintf(line, sizeof(line), "%llX:\t%s\t%s\r\n", insn.address, insn.mnemonic, insn.op_str);
		text += line;
		memset(line, 0, sizeof(line));
	}
	m_dasm.SetWindowText(text);
}


void CDataSearchDlg::OnBnClickedUpdateMem()
{
	if (m_npid == -1) {
		MessageBox("请选择进程！", "进程为空", MB_OK | MB_ICONERROR);
		return;
	}
	CString text;
	m_data.GetWindowText(text);
	if (text.GetLength() <= 0) {
		MessageBox("请输入搜索数据！", "没有数据", MB_OK | MB_ICONERROR);
		return;
	}
	m_hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, m_npid);
	SearchTool::Instance()->SetProcess(m_hProcess);
	for (int i = 0; i < m_lstAddr.GetItemCount(); i++)
	{
		CString address = m_lstAddr.GetItemText(i, 0);
		DWORD addr = strtoul((LPCSTR)address, NULL, 16);
		std::string data;
		data.resize(text.GetLength());
		SearchTool::Instance()->ReadMemory(addr, data, m_DataType.GetCurSel());
		TRACE("data=%s\r\n", data.c_str());
		m_lstAddr.SetItemText(i, 1, data.c_str());
	}
	SearchTool::Instance()->SetProcess(NULL);
	CloseHandle(m_hProcess);
}


void CDataSearchDlg::OnBnClickedBtnClear()
{
	m_lstAddr.DeleteAllItems();
	m_result.Clear();
}


void CDataSearchDlg::OnNMRClickLstAddr(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	CPoint pt;
	GetCursorPos(&pt);
	CMenu* sub = m_rmenu.GetSubMenu(0);
	sub->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, this);
	*pResult = 0;

}

void CDataSearchDlg::OnDelsel()
{//删除选中
	int count = m_lstAddr.GetItemCount();
	for (int i = count - 1; i >= 0; i--)
	{
		if (m_lstAddr.GetCheck(i)) {
			m_lstAddr.DeleteItem(i);
		}
	}
}


void CDataSearchDlg::OnLocksel()
{//锁定选中
	// TODO: 在此添加命令处理程序代码
}


void CDataSearchDlg::OnCopysel()
{//复制选中的地址值
	int count = m_lstAddr.GetItemCount();
	if (count == 0) {
		MessageBox("没有搜索结果，无数据！");
		return;
	}

	std::string text;
	for (int i = 0; i < count; i++)
	{
		if (m_lstAddr.GetCheck(i)) {
			CString addr = m_lstAddr.GetItemText(i, 0);
			CString value = m_lstAddr.GetItemText(i, 1);
			char buffer[64] = "";
			snprintf(buffer, sizeof(buffer), "%s:%s\r\n", (LPCTSTR)addr, (LPCTSTR)value);
			text += buffer;
		}
	}
	if (text.size() == 0) {
		MessageBox("没有勾选数据！");
		return;
	}
	TRACE("%s\r\n", text.c_str());
	SetClipboardCopy(text);
}


void CDataSearchDlg::OnAddWatch()
{//关注选中地址
	int count = m_lstAddr.GetItemCount();
	for (int i = 0; i < count; i++)
	{
		if (m_lstAddr.GetCheck(i)) {
			m_favorite.InsertColumn(m_favorite.GetItemCount(), "");
		}
	}
}


void CDataSearchDlg::OnFrush()
{//刷新数据
	OnBnClickedUpdateMem();
}


void CDataSearchDlg::OnCopyAll()
{//复制所有地址
	int count = m_lstAddr.GetItemCount();
	if (count >= RESULT_SIZE) {
		MessageBox("当前结果太多，无法复制！");
		return;
	}
	if (count == 0) {
		MessageBox("没有搜索结果，无数据！");
		return;
	}
	std::string text;
	for (int i = 0; i < count; i++)
	{
		CString addr = m_lstAddr.GetItemText(i, 0);
		CString value = m_lstAddr.GetItemText(i, 1);
		char buffer[64] = "";
		snprintf(buffer, sizeof(buffer), "%s:%s\r\n", (LPCTSTR)addr, (LPCTSTR)value);
		text += buffer;
	}
	TRACE("%s\r\n", text.c_str());
	EmptyClipboard();
	HGLOBAL clip = GlobalAlloc(GMEM_DDESHARE, text.size() + 1);
	if (clip == NULL) {
		TRACE("GlobalAlloc错误：%d\r\n", GetLastError());
		MessageBox("内存不足！");
		return;
	}
	char* pbuf = (char*)GlobalLock(clip);
	if (pbuf == NULL) {
		TRACE("GlobalLock错误：%d\r\n", GetLastError());
		MessageBox("内存无法锁定！");
		return;
	}
	memcpy(pbuf, text.c_str(), text.size());
	GlobalUnlock(clip);
	SetClipboardData(CF_TEXT, clip);
	CloseClipboard();
}


void CDataSearchDlg::OnSelectAll()
{//全选
	int count = m_lstAddr.GetItemCount();
	for (int i = 0; i < count; i++)
	{
		m_lstAddr.SetCheck(i, TRUE);
	}
}


void CDataSearchDlg::OnSelectReverse()
{//反选
	int count = m_lstAddr.GetItemCount();
	for (int i = 0; i < count; i++)
	{
		if (m_lstAddr.GetCheck(i)) {
			m_lstAddr.SetCheck(i, FALSE);
		}
		else {
			m_lstAddr.SetCheck(i, TRUE);
		}
	}
}


void CDataSearchDlg::OnDeleteAll()
{//清除地址列表
	m_result.Clear();
	m_lstAddr.DeleteAllItems();
}


void CDataSearchDlg::OnBnClickedBtnLocate()
{
	if (m_npid == -1) {
		MessageBox("请选择进程！", "进程为空", MB_OK | MB_ICONERROR);
		return;
	}
	if (m_lstModules.size() <= 0) {
		MessageBox("无法获取进程信息！", "进程无法访问！", MB_OK | MB_ICONERROR);
		return;
	}
	if (m_DataType.GetCurSel() == -1) {
		MessageBox("请选择数据类型！", "没有类型", MB_OK | MB_ICONERROR);
		return;
	}
	m_hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, m_npid);
	TRACE("进程句柄：%016llX\r\n", m_hProcess);
	if (m_hProcess == NULL || (m_hProcess == INVALID_HANDLE_VALUE)) {
		MessageBox("打开进程失败，权限不足！", "打开失败", MB_OK | MB_ICONERROR);
		return;
	}
	SearchTool::Instance()->SetProcess(m_hProcess);
	m_progress.SetRange(0, 1000);
	m_progress.SetPos(0);
	m_count = 0;
	m_curpos = 0;
	m_code_result.Clear();
	std::string text("\x7F\x45\x4c\x46\x01\x01\x01\x00");
	/*for (auto& entry : m_lstModules) {
		TRACE("module:%s %016llX %016llX\r\n",
			entry.szModule, entry.modBaseAddr, entry.modBaseSize);
		m_pool.AddTask(Task(&SearchTool::Search,
			SearchTool::Instance(), entry,
			text, 6));
	}*/
	BYTE* pMin = (BYTE*)(SearchTool::Instance()->m_info.lpMinimumApplicationAddress);
	//0x10000
	BYTE* pMax = (BYTE*)(SearchTool::Instance()->m_info.lpMaximumApplicationAddress);
	//0x7FEFFFFF
	TRACE("min %016llX max %016llX\r\n", pMin, pMax);
	SIZE_T nIndex = 0;
	SIZE_T nSize = pMax - pMin;
	m_total = nSize;
	do {
		MEMORY_BASIC_INFORMATION mbi;
		if (VirtualQueryEx(m_hProcess, (LPCVOID)(pMin + nIndex),
			&mbi, sizeof(mbi)) == 0)
		{
			nIndex += SearchTool::Instance()->m_info.dwPageSize;
			SendMessage(WM_PROGRESS, SearchTool::Instance()->m_info.dwPageSize, NULL);
			continue;
		}
		if (mbi.Protect & PAGE_WRITE_FLAGS) {
			m_pool.AddTask(Task(
				&SearchTool::SearchMem, SearchTool::Instance(),
				(BYTE*)mbi.BaseAddress, mbi.RegionSize, text, 6));
		}
		else {
			SendMessage(WM_PROGRESS, mbi.RegionSize, NULL);
		}
		nIndex += mbi.RegionSize;
	} while (nIndex < nSize);
}


void CDataSearchDlg::OnNMRClickListModule(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	CPoint pt;
	GetCursorPos(&pt);
	CMenu* sub = m_rmenu.GetSubMenu(1);
	sub->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, this);
	*pResult = 0;
}


void CDataSearchDlg::OnCopyModule()
{
	std::string text;
	std::vector<char> line;
	line.resize(4096);
	for (auto entry : m_lstModules)
	{
		TRACE("模块名称：%s\n模块路径:%s 模块ID: %u 模块句柄：%08llX\n\n",
			entry.szModule, entry.szExePath, entry.th32ModuleID, entry.hModule);
		snprintf(line.data(), line.size(),
			"模块ID: %u 模块句柄：%016llX 模块地址：%016llX 模块大小：%08X 模块名称：%s 模块地址：%s\r\n",
			entry.th32ModuleID, (SIZE_T)entry.hModule, (SIZE_T)entry.modBaseAddr, entry.modBaseSize, entry.szModule, entry.szExePath);
		text += line.data();
	}
	SetClipboardCopy(text);
}


void CDataSearchDlg::OnFrushMod()
{
	if (m_npid != -1) {
		UpdateModule(m_npid);
	}
}


void CDataSearchDlg::OnBnClickedBtnTest()
{
	BYTE data[] = {
		0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,
		0x00,0xEE,0xA7,0x9A,
		0x00,0x00,0x00,0x00
	};
	m_once = true;
	std::string sData;
	sData.resize(sizeof(data));
	memcpy((void*)sData.c_str(), data, sizeof(data));
	m_hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, m_npid);
	SearchTool::Instance()->SetProcess(m_hProcess);
	m_progress.SetRange(0, 1000);
	m_progress.SetPos(0);
	m_count = 0;
	m_curpos = 0;
	m_code_result.Clear();
	BYTE* pMin = (BYTE*)0x20000000; //(BYTE*)(SearchTool::Instance()->m_info.lpMinimumApplicationAddress);
	//0x10000
	BYTE* pMax = (BYTE*)0x55000000;//(SearchTool::Instance()->m_info.lpMaximumApplicationAddress);
	//0x7FEFFFFF
	TRACE("min %016llX max %016llX\r\n", pMin, pMax);
	size_t nIndex = 0;
	size_t nSize = pMax - pMin;
	m_total = nSize;
	do {
		MEMORY_BASIC_INFORMATION mbi;
		if (VirtualQueryEx(m_hProcess, (LPCVOID)(pMin + nIndex),
			&mbi, sizeof(mbi)) == 0)
		{
			nIndex += SearchTool::Instance()->m_info.dwPageSize;
			SendMessage(WM_PROGRESS, SearchTool::Instance()->m_info.dwPageSize, NULL);
			continue;
		}
		TRACE("%016llX %08X Protect %08X\r\n", (BYTE*)mbi.BaseAddress, mbi.RegionSize, mbi.Protect);
		if (mbi.Protect & PAGE_WRITE_FLAGS) {

			m_pool.AddTask(Task(
				&SearchTool::SearchMem, SearchTool::Instance(),
				(BYTE*)mbi.BaseAddress, mbi.RegionSize, sData, 6));

		}
		else {
			SendMessage(WM_PROGRESS, mbi.RegionSize, NULL);
		}
		nIndex += mbi.RegionSize;
	} while (nIndex < nSize);
}
