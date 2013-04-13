// NPSevDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "NPSev.h"
#include "NPSevDlg.h"


#include <Psapi.h>
#pragma comment (lib,"Psapi.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#define  MSG_DATAREC  WM_USER+1

#include <windows.h>
#include <iostream>
using namespace std;

CString getAppPath()
{
	char nbuf[255] = {0};
	GetCurrentDirectory( 255, nbuf );
	return (CString)nbuf;
}

CString getFileName( CString fileName, CString folder )
{
	TCHAR path[MAX_PATH];
	::GetModuleFileName(NULL,path,MAX_PATH);
	CString p(path);
	CString filePath;
	int nPos = p.ReverseFind('\\');
	//ASSERT(-1!=nPos);
	filePath = p.Left(nPos+1);

	CString fullFileName = filePath + "\\" + folder + "\\" + fileName;
	if (!PathFileExists(fullFileName))
	{
		fullFileName = fileName;
	}

	return fullFileName;
}

CString    ReturnPath()
{   
	CString    sPath;   
	GetModuleFileName(NULL,sPath.GetBufferSetLength(MAX_PATH+1),MAX_PATH);   
	sPath.ReleaseBuffer    ();   
	int    nPos;   
	nPos=sPath.ReverseFind('\\');   
	sPath=sPath.Left(nPos);   
	return    sPath;   
}

CString    ReturnExtName( CString pathName )
{   
	int    nPos;   
	nPos=pathName.ReverseFind('\\');   
	pathName=pathName.Right(pathName.GetLength() - nPos - 1);   
	return    pathName;   
}

BOOL IsRoot(LPCTSTR lpszPath) 
{ 
	TCHAR szRoot[4]; 
	wsprintf(szRoot,"%c:\\",lpszPath[0]); 
	return (lstrcmp(szRoot,lpszPath)==0); 
}
void FindInAll(CString lpszPath, CListBox *m_lstResult) 
{
	m_lstResult->ResetContent();
	TCHAR szFind[MAX_PATH]; 
	lstrcpy(szFind,lpszPath); //windows API ��lstrcpy������strcpy
	if(!IsRoot(szFind)) 
		lstrcat(szFind,"\\"); 
	lstrcat(szFind,"*.*"); //�������ļ� 
	WIN32_FIND_DATA wfd; 
	HANDLE hFind=FindFirstFile(szFind,& wfd); 
	if(hFind==INVALID_HANDLE_VALUE) // ���û���ҵ������ʧ�� 
		return; 
	do
	{ 
		if(wfd.cFileName[0]=='.')
			continue; //����������Ŀ¼
		if(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			TCHAR szFile[MAX_PATH];
			if(IsRoot(lpszPath))
				wsprintf(szFile,"%s%s",lpszPath,wfd.cFileName);
			else
				wsprintf(szFile,"%s\\%s",lpszPath,wfd.cFileName);
			FindInAll(szFile, m_lstResult); //����ҵ�����Ŀ¼��������Ŀ¼���еݹ�
		}
		else
		{
			TCHAR szFile[MAX_PATH];
			if(IsRoot(lpszPath))
				wsprintf(szFile,"%s%s",lpszPath,wfd.cFileName);
			else
				wsprintf(szFile,"%s\\%s",lpszPath,wfd.cFileName);
			OutputDebugString(szFile);
			OutputDebugString("\n");

			if ( ((CString)szFile).Find( "time.dat" )  == -1 && 
				 ((CString)szFile).Find( "black.mpg" ) == -1 && 
				 ((CString)szFile).Find( "green.mpg" )   == -1 && 
				 ((CString)szFile).Find( "red.mpg" )   == -1 && 
				 ((CString)szFile).Find( "blue.mpg" )  == -1 
			)
				 m_lstResult->AddString(szFile);
		}
	}
	while(FindNextFile(hFind,&wfd));
	FindClose(hFind); //�رղ��Ҿ��
}


// FindProcess
// �������Ψһ�Ĳ�������ָ���Ľ���������:���Ŀ�����
// �� "Notepad.exe",����ֵ�Ǹý��̵�ID��ʧ�ܷ���0
//
DWORD FindProcess(char *strProcessName)
{
	DWORD aProcesses[1024], cbNeeded, cbMNeeded;
	HMODULE hMods[1024];
	HANDLE hProcess;
	char szProcessName[MAX_PATH];
	if ( !EnumProcesses( aProcesses, sizeof(aProcesses), &cbNeeded ) )  return 0;
	for(int i=0; i< (int) (cbNeeded / sizeof(DWORD)); i++)
	{
		//_tprintf(_T("%d\t"), aProcesses[i]);
		hProcess = OpenProcess(  PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, aProcesses[i]);
		EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbMNeeded);
		GetModuleFileNameEx( hProcess, hMods[0], szProcessName,sizeof(szProcessName));

		if(strstr(szProcessName, strProcessName))
		{
			//_tprintf(_T("%s;"), szProcessName);
			return(aProcesses[i]);
		}
		//_tprintf(_T("\n"));
	}

	return 0;
}
//
// Function: ErrorForce
// �˺������������ FindProcess ����������Ŀ����̵�ID
// ��WIN API OpenPorcess ��ô˽��̵ľ��������TerminateProcess
// ǿ�ƽ����������
//
VOID KillProcess()
{
	// When the all operation fail this function terminate the "winlogon" Process for force exit the system.
	    HANDLE hYourTargetProcess = OpenProcess(PROCESS_TERMINATE|PROCESS_QUERY_INFORMATION |   // Required by Alpha
		PROCESS_CREATE_THREAD     |   // For CreateRemoteThread
		PROCESS_VM_OPERATION      |   // For VirtualAllocEx/VirtualFreeEx
		PROCESS_VM_WRITE,             // For WriteProcessMemory
		FALSE, FindProcess("ffplay.exe"));
	if(hYourTargetProcess == NULL)
	{
		return;
	}
	TerminateProcess(hYourTargetProcess, 0);
	return;
}
//
// GetDebugPriv
// �� Windows NT/2000/XP �п�����Ȩ�޲����������Ϻ���ʧ��
// ���ԡ�System Ȩ�����е�ϵͳ���̣��������
// �ñ�����ȡ�á�debug Ȩ�޼���,Winlogon.exe ��������ֹŶ :)
//
BOOL GetDebugPriv()
{
	HANDLE hToken;
	LUID sedebugnamue;
	TOKEN_PRIVILEGES tkp;
	if ( ! OpenProcessToken( GetCurrentProcess(),
		TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken ) )
	{
		return FALSE;
	}

	if ( ! LookupPrivilegeValue( NULL, SE_DEBUG_NAME, &sedebugnamue ) )
	{
		CloseHandle( hToken );
		return FALSE;
	}
	tkp.PrivilegeCount = 1;
	tkp.Privileges[0].Luid = sedebugnamue;
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	if (!AdjustTokenPrivileges( hToken, FALSE, &tkp, sizeof tkp, NULL, NULL ) )
	{
		CloseHandle( hToken );
		return FALSE;
	}
	return TRUE;
}


// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CNPSevDlg �Ի���




CNPSevDlg::CNPSevDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CNPSevDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CNPSevDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_lstResult);
	DDX_Control(pDX, IDC_LIST2, m_fileList);
	DDX_Control(pDX, IDC_LIST3, m_TimeInfo);
	DDX_Control(pDX, IDC_PROGRESS1, pProcessbar);
}

BEGIN_MESSAGE_MAP(CNPSevDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_MESSAGE(MSG_DATAREC,CNPSevDlg::OnDataRec)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON1, &CNPSevDlg::OnBnClickedButton1)
	ON_WM_TIMER()
	ON_LBN_DBLCLK(IDC_LIST1, &CNPSevDlg::OnLbnDblclkList1)
	ON_BN_CLICKED(IDC_BUTTON2, &CNPSevDlg::OnBnClickedButton2)
END_MESSAGE_MAP()


// CNPSevDlg ��Ϣ�������

BOOL CNPSevDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	dealIndex = 0;
	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	FindInAll( getAppPath() + _T("\\Movie"), &m_fileList );
	isCreadTreaded = false;
	prgOpenDelay = 0;
	SetTimer( 1, 1000, NULL );
	pProcessbar.SetRange(0,m_fileList.GetCount());
	pProcessbar.SetPos(0);

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CNPSevDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CNPSevDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CNPSevDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

//��ʾ �ӽ����ض�������
LRESULT CNPSevDlg::OnDataRec(WPARAM wParam, LPARAM lParam)
{
	CString tmp = m_szData;

	m_lstResult.AddString(m_szData);

	OutputDebugString( tmp + "\n" );

	pProcessbar.SetPos(dealIndex);

	BOOL isFound = false;
	for ( int i=0;i<m_lstResult.GetCount(); i++ )
	{
		CString TMP = "";
		m_lstResult.GetText( i, TMP );
		int s = TMP.Find( "Duration: " );
		CString cutStart = TMP.Right( TMP.GetLength() - s );
		int e = s + cutStart.Find( "," ) ;

		if ( s > 0 )
		{
			//SetDlgItemText( IDC_EDITTMP.Mid( s+10, e-s-10 )1,  );
			CString timeStr = TMP.Mid( s+10, e-s-10 );

			CString fileName;
			m_fileList.GetText( m_fileList.GetCurSel(), fileName );
			fileName = ReturnExtName( fileName );

			int h = StrToInt( timeStr.Mid( 0, 2 ) );
			int m = StrToInt( timeStr.Mid( 3, 2 ) );
			int s = StrToInt( timeStr.Mid( 6, 2 ) );
			int t = StrToInt( timeStr.Mid( 9, 2 ) );

			times[dealIndex-1] = (h*3600 + m*60 + s)*1000 + t * 10;

			char buf[255]={0};
			sprintf( buf, "%d", times[dealIndex-1] );
			//m_TimeInfo.AddString( buf );
			m_TimeInfo.AddString( fileName + " --- " + timeStr + "  " + (CString)buf );

			//OutputDebugString( (CString)(fileName + " --- " + timeStr + "  " + (CString)buf) + "\n" );

			isFound = true;
			prgOpenDelay = 0;
		}
	}

	return 0;
}


namespace {

	//���ܵ��߳�
	UINT ReadPipeProc( LPVOID pParam )
	{
		CNPSevDlg * pAttachWnd = static_cast<CNPSevDlg *>(pParam);
		HANDLE hRead = pAttachWnd->m_hReadPipe;
		HWND   hWnd  = pAttachWnd->GetSafeHwnd();
		
		DWORD bytesRead;
		while( 1 )
		{
			int len = sizeof(pAttachWnd->m_szData);
			ZeroMemory(&pAttachWnd->m_szData,len);

			if( !ReadFile(hRead, pAttachWnd->m_szData, len-1, &bytesRead, NULL) )
				break;

			SendMessage(hWnd,MSG_DATAREC,0,0);

			//OutputDebugString( "End Tread ... \n" );
		}

		//OutputDebugString( "End Tread ... \n" );

		return 0;
	}

}

//����ping������
void CNPSevDlg::OnBnClickedButton1()
{
	if ( m_fileList.GetCurSel() == -1 ) return;
	m_lstResult.ResetContent();

	HANDLE hRead, hWrite;
	
	//���������ܵ�	
	{
		SECURITY_ATTRIBUTES sa;
		ZeroMemory(&sa,sizeof(sa));
		sa.nLength = sizeof(SECURITY_ATTRIBUTES);
		sa.lpSecurityDescriptor = NULL;  //Ĭ�ϵİ�ȫ������
		sa.bInheritHandle = TRUE;        //�������Ҫ�趨TRUE,�ο����ϣ���windows���ı�̡�������

		if( !CreatePipe(&hRead, &hWrite, &sa, 0) )
		{
			//MessageBox(" CreatePipe return FALSE.");
			m_lstResult.AddString( "CreatePipe return FALSE." );
			return;
		}

		m_hReadPipe = hRead; 
	}

	//�����ӽ���
	{
		//������������		
		STARTUPINFO si;
		PROCESS_INFORMATION pi;
		si.cb = sizeof(STARTUPINFO);
		GetStartupInfo(&si);
		si.hStdError = hWrite;
		si.hStdOutput = hWrite;
		si.wShowWindow = SW_HIDE;
		si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;

		//if ( !isCreadTreaded )
		{
			AfxBeginThread(ReadPipeProc,this,NULL);
			isCreadTreaded = true;
		}

		CString fileName;
		m_fileList.GetText( m_fileList.GetCurSel(), fileName );

		fileName = "ffplay.exe -nodisp -stats -vn -an  " + fileName;

		if(!CreateProcess(NULL, fileName.GetBuffer(),NULL,NULL,TRUE,NULL,NULL,NULL,&si, &pi))
		{
			//MessageBox("CreateProcess failed!");
			m_lstResult.AddString( "CreateProcess failed!" );
			return;
		}
		m_prgHandle = pi.hProcess;
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);

		//SetTimer( 1, 1000, NULL );
		prgOpenDelay = 5;
	}
}


void CNPSevDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	switch ( nIDEvent )
	{
	case 1:
		{
			if ( prgOpenDelay > 0 ) prgOpenDelay--;

			if ( prgOpenDelay == 0 )
			{
				if ( dealIndex < m_fileList.GetCount() )
				{
					m_fileList.SetCurSel( dealIndex );
					dealIndex++;
					OnBnClickedButton1();
				}
				else
				{
					KillTimer(1);

					CString fname = getFileName( "time.dat", "Movie" );
					CFile createBinFile( fname, CFile::modeCreate | CFile::typeBinary ); 
					createBinFile.Close();
					/*< ������Part 2: д���ļ�>*/ 
					CFile writeBinFile( fname, CFile::modeReadWrite | CFile::typeBinary ); 
					int Count = m_fileList.GetCount();
					writeBinFile.Write( &Count, sizeof(int)); 

					for ( int i=0;i<Count;i++ )
					{
						CString fileName;
						m_fileList.GetText( i, fileName );
						fileName = ReturnExtName( fileName );

						char buf[100];
						memset( buf, 0, 100 );
						memcpy( buf, fileName.GetBuffer(), fileName.GetLength() );

						writeBinFile.Write( buf,      100);
						writeBinFile.Write( &times[i], sizeof(int)); 
					}

					writeBinFile.Close();
					ShellExecute( GetSafeHwnd(), "open", getAppPath() + "\\Player.exe", "", getAppPath(), SW_SHOW );
					exit(0);
				}
			}

			if ( prgOpenDelay == 11114 )
			{
				//KillTimer(1);
				BOOL isFound = false;
				for ( int i=0;i<m_lstResult.GetCount(); i++ )
				{
					CString TMP = "";
					m_lstResult.GetText( i, TMP );
					int s = TMP.Find( "Duration: " );
					int e = TMP.Find( ", start:" ) ;
					if ( s > 0 )
					{
						//SetDlgItemText( IDC_EDITTMP.Mid( s+10, e-s-10 )1,  );
						CString timeStr = TMP.Mid( s+10, e-s-10 );
						
						CString fileName;
						m_fileList.GetText( m_fileList.GetCurSel(), fileName );
						fileName = ReturnExtName( fileName );

						int h = StrToInt( timeStr.Mid( 0, 2 ) );
						int m = StrToInt( timeStr.Mid( 3, 2 ) );
						int s = StrToInt( timeStr.Mid( 6, 2 ) );
						int t = StrToInt( timeStr.Mid( 9, 2 ) );

						times[dealIndex-1] = (h*3600 + m*60 + s)*1000 + t * 10;

						char buf[255]={0};
						sprintf( buf, "%d", times[dealIndex-1] );
						//m_TimeInfo.AddString( buf );
						m_TimeInfo.AddString( fileName + " --- " + timeStr + "  " + (CString)buf );
						isFound = true;
					}
				}
				if ( !isFound ) m_TimeInfo.AddString( "NOT FOUND" ); //SetDlgItemText( IDC_EDIT1, "NOT FOUND" );
				prgOpenDelay = 0;
			}
			
		} break;
	}

	CDialog::OnTimer(nIDEvent);
}


void CNPSevDlg::OnLbnDblclkList1()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CString TMP;
	m_lstResult.GetText( m_lstResult.GetCurSel(), TMP );
	SetDlgItemText( IDC_EDIT1, TMP );
}


void CNPSevDlg::OnBnClickedButton2()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������

}
