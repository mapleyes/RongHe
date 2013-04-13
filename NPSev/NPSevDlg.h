// NPSevDlg.h : 头文件
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"


// CNPSevDlg 对话框
class CNPSevDlg : public CDialog
{
// 构造
public:
	CNPSevDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_NPSEV_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持
	

// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

	afx_msg void OnBnClickedButton1();
	afx_msg LRESULT OnDataRec(WPARAM wParam, LPARAM lParam);

public:
	enum { DataSize = 1024};
	TCHAR    m_szData[DataSize];
	HANDLE   m_hReadPipe;
	int  prgOpenDelay;
	HANDLE   m_prgHandle;
	BOOL     isCreadTreaded;
	int		 times[100];

	int  dealIndex;

private:
	CListBox m_lstResult;
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnLbnDblclkList1();
	CListBox m_fileList;
	afx_msg void OnBnClickedButton2();
	CListBox m_TimeInfo;
	CProgressCtrl pProcessbar;
};
