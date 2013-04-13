// NPSevDlg.cpp : 实现文件
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
	lstrcpy(szFind,lpszPath); //windows API 用lstrcpy，不是strcpy
	if(!IsRoot(szFind)) 
		lstrcat(szFind,"\\"); 
	lstrcat(szFind,"*.*"); //找所有文件 
	WIN32_FIND_DATA wfd; 
	HANDLE hFind=FindFirstFile(szFind,& wfd); 
	if(hFind==INVALID_HANDLE_VALUE) // 如果没有找到或查找失败 
		return; 
	do
	{ 
		if(wfd.cFileName[0]=='.')
			continue; //过滤这两个目录
		if(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			TCHAR szFile[MAX_PATH];
			if(IsRoot(lpszPath))
				wsprintf(szFile,"%s%s",lpszPath,wfd.cFileName);
			else
				wsprintf(szFile,"%s\\%s",lpszPath,wfd.cFileName);
			FindInAll(szFile, m_lstResult); //如果找到的是目录，则进入此目录进行递归
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
	FindClose(hFind); //关闭查找句柄
}


// FindProcess
// 这个函数唯一的参数是你指定的进程名，如:你的目标进程
// 是 "Notepad.exe",返回值是该进程的ID，失败返回0
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
// 此函数中用上面的 FindProcess 函数获得你的目标进程的ID
// 用WIN API OpenPorcess 获得此进程的句柄，再以TerminateProcess
// 强制结束这个进程
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
// 在 Windows NT/2000/XP 中可能因权限不够导致以上函数失败
// 如以　System 权限运行的系统进程，服务进程
// 用本函数取得　debug 权限即可,Winlogon.exe 都可以终止哦 :)
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


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
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


// CNPSevDlg 对话框




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


// CNPSevDlg 消息处理程序

BOOL CNPSevDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
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

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	dealIndex = 0;
	// TODO: 在此添加额外的初始化代码
	FindInAll( getAppPath() + _T("\\Movie"), &m_fileList );
	isCreadTreaded = false;
	prgOpenDelay = 0;
	SetTimer( 1, 1000, NULL );
	pProcessbar.SetRange(0,m_fileList.GetCount());
	pProcessbar.SetPos(0);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
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

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CNPSevDlg::OnPaint()
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
HCURSOR CNPSevDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

//显示 子进程重定向的输出
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

	//读管道线程
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

//启用ping命令行
void CNPSevDlg::OnBnClickedButton1()
{
	if ( m_fileList.GetCurSel() == -1 ) return;
	m_lstResult.ResetContent();

	HANDLE hRead, hWrite;
	
	//创建匿名管道	
	{
		SECURITY_ATTRIBUTES sa;
		ZeroMemory(&sa,sizeof(sa));
		sa.nLength = sizeof(SECURITY_ATTRIBUTES);
		sa.lpSecurityDescriptor = NULL;  //默认的安全描述符
		sa.bInheritHandle = TRUE;        //这个必须要设定TRUE,参考资料：《windows核心编程》第三章

		if( !CreatePipe(&hRead, &hWrite, &sa, 0) )
		{
			//MessageBox(" CreatePipe return FALSE.");
			m_lstResult.AddString( "CreatePipe return FALSE." );
			return;
		}

		m_hReadPipe = hRead; 
	}

	//创建子进程
	{
		//设置启动参数		
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
	// TODO: 在此添加消息处理程序代码和/或调用默认值
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
					/*< 以下是Part 2: 写入文件>*/ 
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
	// TODO: 在此添加控件通知处理程序代码
	CString TMP;
	m_lstResult.GetText( m_lstResult.GetCurSel(), TMP );
	SetDlgItemText( IDC_EDIT1, TMP );
}


void CNPSevDlg::OnBnClickedButton2()
{
	// TODO: 在此添加控件通知处理程序代码

}
