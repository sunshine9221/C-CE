// CPictureBoxDlg.cpp: 实现文件
//

#include "pch.h"
#include "DataSearch.h"
#include "CPictureBoxDlg.h"
#include "afxdialogex.h"


// CPictureBoxDlg 对话框

IMPLEMENT_DYNAMIC(CPictureBoxDlg, CDialog)

CPictureBoxDlg::CPictureBoxDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_DIALOG_SUB, pParent)
{

}

CPictureBoxDlg::~CPictureBoxDlg()
{
}

void CPictureBoxDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_PIC, m_picture);
}


BEGIN_MESSAGE_MAP(CPictureBoxDlg, CDialog)
	ON_NOTIFY(NM_THEMECHANGED, IDC_SCROLLBAR1, &CPictureBoxDlg::OnNMThemeChangedScrollbar)
END_MESSAGE_MAP()


// CPictureBoxDlg 消息处理程序


void CPictureBoxDlg::OnNMThemeChangedScrollbar(NMHDR* pNMHDR, LRESULT* pResult)
{
	*pResult = 0;
	//拿到滚动信息
	m_picture.MoveWindow(NULL);//todo:移动窗口
}
