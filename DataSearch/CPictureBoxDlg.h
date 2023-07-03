#pragma once


// CPictureBoxDlg 对话框

class CPictureBoxDlg : public CDialog
{
	DECLARE_DYNAMIC(CPictureBoxDlg)

public:
	CPictureBoxDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CPictureBoxDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_SUB };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnNMThemeChangedScrollbar(NMHDR* pNMHDR, LRESULT* pResult);
	CStatic m_picture;
};
