// NPSevDlg.h : ͷ�ļ�
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"


// CNPSevDlg �Ի���
class CNPSevDlg : public CDialog
{
// ����
public:
	CNPSevDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_NPSEV_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��
	

// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
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
