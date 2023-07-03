
// DataSearchDlg.h: 头文件
//

#pragma once
#include <list>
#include <TlHelp32.h>
#include <Psapi.h>
#include <atomic>
#include "ThreadPool.h"
#include "SearchTool.h"
using namespace edoyun;

#define WM_PROGRESS (WM_USER+1)
#define WM_SECOND (WM_USER+2)
#define WM_CODE_PROGRESS (WM_USER+3)

// CDataSearchDlg 对话框
class CDataSearchDlg : public CDialog
{
	// 构造
public:
	CDataSearchDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DATASEARCH_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	void SetClipboardCopy(const std::string& text);
	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	Result m_result;
	Result m_code_result;
	CListCtrl m_pidlist;
	void UpdatePid();
	void UpdateModule(DWORD nPID);
	CListCtrl m_module;
	DWORD m_npid;//当前选中的进程id
	std::atomic<SIZE_T> m_curpos;
	std::atomic<SIZE_T> m_count;
	SIZE_T m_total;
	CEdit m_search;
	BOOL m_isinject;
	CEdit m_pid_search;
	CListCtrl m_lstAddr;
	CEdit m_addr;
	CProgressCtrl m_progress;
	std::list<MODULEENTRY32> m_lstModules;
	CComboBox m_DataType;
	CButton m_btn_search;
	ThreadPool m_pool;
	HANDLE m_hProcess;
	CMenu m_rmenu;
	CEdit m_data;
	CEdit m_value;
	CEdit m_address;
	CEdit m_memshow;
	CEdit m_code;
	CButton m_btnIsBig;
	CListCtrl m_favorite;
	CFont m_mem_font;
	CEdit m_dasm;
	bool m_once;
	SIZE_T m_nSize;
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedBtnDone();
	afx_msg void OnLvnItemchangedListProcess(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEnChangeEditSearch();
	afx_msg void OnEnChangeEditPid();
	afx_msg void OnBnClickedBtnMemSearch();
	afx_msg LRESULT OnProgress(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnCodeProgress(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnSecond(WPARAM wParam, LPARAM lParam);
	afx_msg void OnBnClickedBtnMemModify();
	afx_msg void OnBnClickedSecondSearch();
	afx_msg void OnBnClickedBtnMemShow();
	afx_msg void OnBnClickedBtnAddAddr();
	afx_msg void OnNMThemeChangedScrollbar(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBnClickedBtnDasm();
	afx_msg void OnBnClickedUpdateMem();
	afx_msg void OnBnClickedBtnClear();
	afx_msg void OnNMRClickLstAddr(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDelsel();
	afx_msg void OnLocksel();
	afx_msg void OnCopysel();
	afx_msg void OnAddWatch();
	afx_msg void OnFrush();
	afx_msg void OnCopyAll();
	afx_msg void OnSelectAll();
	afx_msg void OnSelectReverse();
	afx_msg void OnDeleteAll();
	afx_msg void OnBnClickedBtnLocate();
	afx_msg void OnNMRClickListModule(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCopyModule();
	afx_msg void OnFrushMod();
	afx_msg void OnBnClickedBtnTest();
};
