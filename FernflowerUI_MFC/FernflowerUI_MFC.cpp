
// FernflowerUI_MFC.cpp: 定义应用程序的类行为。
//

#include "stdafx.h"
#include "afxwinappex.h"
#include "afxdialogex.h"
#include "FernflowerUI_MFC.h"
#include "MainFrm.h"
#include "CommonWrapper.h"
#include "ChildFrm.h"
#include "FernflowerUI_MFCDoc.h"
#include "FernflowerUI_MFCView.h"
#include "Md5Checksum.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CFernflowerUIMFCApp

BEGIN_MESSAGE_MAP(CFernflowerUIMFCApp, CWinAppEx)
	ON_COMMAND(ID_APP_ABOUT, &CFernflowerUIMFCApp::OnAppAbout)
	// 基于文件的标准文档命令
	ON_COMMAND(ID_FILE_OPEN, &CFernflowerUIMFCApp::OpenFile)
	ON_COMMAND(ID_EDIT_COPY, &CFernflowerUIMFCApp::OnEditCopy)
	ON_COMMAND(ID_EDIT_SELECT_ALL, &CFernflowerUIMFCApp::OnEditSelectAll)
	// 标准打印设置命令
	ON_COMMAND(ID_FILE_SAVE_AS, &CFernflowerUIMFCApp::OnFileSaveAs)
	ON_COMMAND(ID_FILE_SAVE, &CFernflowerUIMFCApp::OnFileSave)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE, &CFernflowerUIMFCApp::OnUpdateFileSave)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE_AS, &CFernflowerUIMFCApp::OnUpdateFileSaveAs)
	ON_COMMAND(ID_FILE_CLOSE, &CFernflowerUIMFCApp::OnFileClose)
	ON_UPDATE_COMMAND_UI(ID_FILE_CLOSE, &CFernflowerUIMFCApp::OnUpdateFileClose)
	ON_UPDATE_COMMAND_UI(ID_FILE_MRU_FILE1, &CFernflowerUIMFCApp::OnUpdateRecentFileMenu)
END_MESSAGE_MAP()

bool AccessJar(HWND hwnd);
bool DownloadJar();
void AsyncCheckForJava();
void WaitForFinishCheck();

bool ClickedDownloadJDK;

// CFernflowerUIMFCApp 构造

CFernflowerUIMFCApp::CFernflowerUIMFCApp()
{
	m_bHiColorIcons = TRUE;
	RunningDialog = nullptr;
	IsJDKInstalled = false;

	// 支持重新启动管理器
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_ALL_ASPECTS;
#ifdef _MANAGED
	// 如果应用程序是利用公共语言运行时支持(/clr)构建的，则: 
	//     1) 必须有此附加设置，“重新启动管理器”支持才能正常工作。
	//     2) 在您的项目中，您必须按照生成顺序向 System.Windows.Forms 添加引用。
	System::Windows::Forms::Application::SetUnhandledExceptionMode(System::Windows::Forms::UnhandledExceptionMode::ThrowException);
#endif

	// TODO: 将以下应用程序 ID 字符串替换为唯一的 ID 字符串；建议的字符串格式
	//为 CompanyName.ProductName.SubProduct.VersionInformation
	SetAppID(_T("6168218c.FernflowerUI.FernflowerUIMFC.3.4.1.1"));

	// TODO: 在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中
}

// 唯一的 CFernflowerUIMFCApp 对象

CFernflowerUIMFCApp theApp;


// CFernflowerUIMFCApp 初始化

BOOL CFernflowerUIMFCApp::InitInstance()
{
	Initing = true;

	//多语言支持
	if (GetSystemDefaultUILanguage() != MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED))
	{
		SetProcessPreferredUILanguages(MUI_LANGUAGE_NAME, L"en-US\0zh-CN\0\0", nullptr);
		SetThreadPreferredUILanguages(MUI_LANGUAGE_NAME, L"en-US\0zh-CN\0\0", nullptr);
		SetThreadUILanguage(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US));
		SetThreadLocale(MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT));
		Language = AppLanguage::English;
	}
	else
	{
		Language = AppLanguage::Chinese;
	}

	//初始化反编译选项数据
	m_DecompOption.InitTooltip();

	// 如果一个运行在 Windows XP 上的应用程序清单指定要
	// 使用 ComCtl32.dll 版本 6 或更高版本来启用可视化方式，
	//则需要 InitCommonControlsEx()。  否则，将无法创建窗口。
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// 将它设置为包括所有要在应用程序中使用的
	// 公共控件类。
	InitCtrls.dwICC = ICC_WIN95_CLASSES | ICC_STANDARD_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinAppEx::InitInstance();

	MtxRmCache = OpenMutex(SYNCHRONIZE, FALSE, L"6168218c.FernflowerUI.RemovingCache");
	if (MtxRmCache)
	{
		CloseHandle(MtxRmCache);
		return FALSE;
	}
	EvtIsToRmCache = OpenEvent(EVENT_MODIFY_STATE | SYNCHRONIZE, TRUE, L"6168218c.FernflowerUI.IsToRmCache");
	if (!EvtIsToRmCache)
	{
		EvtIsToRmCache = CreateEvent(nullptr, TRUE, FALSE, L"6168218c.FernflowerUI.IsToRmCache");
		if (!EvtIsToRmCache)
		{
			return FALSE;
		}
		ResetEvent(EvtIsToRmCache);
	}
	SemInstanceCount = OpenSemaphore(SEMAPHORE_MODIFY_STATE | SYNCHRONIZE, FALSE, L"6168218c.FernflowerUI.InstanceCount");
	if (!SemInstanceCount)
	{
		SemInstanceCount = CreateSemaphore(nullptr, 0, LONG_MAX, L"6168218c.FernflowerUI.InstanceCount");
		if (!SemInstanceCount)
		{
			return FALSE;
		}
	}
	else
	{
		LONG PrevCount;
		ReleaseSemaphore(SemInstanceCount, 1, &PrevCount);
	}
	EnableRecovery = OpenEvent(EVENT_MODIFY_STATE | SYNCHRONIZE, TRUE, L"6168218c.FernflowerUI.EnableRecovery");
	if (!EnableRecovery)
	{
		EnableRecovery = CreateEvent(nullptr, TRUE, FALSE, L"6168218c.FernflowerUI.EnableRecovery");
		if (!EnableRecovery)
		{
			return FALSE;
		}
	}


	// 初始化 OLE 库
	if (!AfxOleInit())
	{
		AfxMessageBox(L"OLE库初始化失败!");
		return FALSE;
	}

	AfxEnableControlContainer();

	EnableTaskbarInteraction(FALSE);
	CommonWrapper::CLaunchDialog * LaunchDlg = new CommonWrapper::CLaunchDialog;
	LaunchDlg->Create(IDD_LAUNCHBAR);
	LaunchDlg->ShowWindow(SW_SHOW);
	CoInitialize(nullptr);

	// 使用 RichEdit 控件需要 AfxInitRichEdit2()
	AfxInitRichEdit2();

	// 标准初始化
	// 如果未使用这些功能并希望减小
	// 最终可执行文件的大小，则应移除下列
	// 不需要的特定初始化例程
	// 更改用于存储设置的注册表项
	// TODO: 应适当修改该字符串，
	// 例如修改为公司或组织名
	SetRegistryKey(_T("6168218c"));
	LoadStdProfileSettings(16);  // 加载标准 INI 文件选项(包括 MRU)


	InitContextMenuManager();

	InitKeyboardManager();

	InitTooltipManager();
	CMFCToolTipInfo ttParams;
	ttParams.m_bVislManagerTheme = TRUE;
	theApp.GetTooltipManager()->SetTooltipParams(AFX_TOOLTIP_TYPE_ALL,
		RUNTIME_CLASS(CMFCToolTipCtrl), &ttParams);

	// 注册应用程序的文档模板。  文档模板
	// 将用作文档、框架窗口和视图之间的连接
	CMultiDocTemplate* pDocTemplate;
	pDocTemplate = new CMultiDocTemplate(IDR_FernflowerUITYPE,
		RUNTIME_CLASS(CFernflowerUIMFCDoc),
		RUNTIME_CLASS(CChildFrame), // 自定义 MDI 子框架
		RUNTIME_CLASS(CFernflowerUIMFCView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);

//	EnableShellOpen();
//	RegisterShellFileTypes();

	HRESULT hr;
	//创建List  
	ICustomDestinationList *pList = NULL;
	hr = CoCreateInstance(CLSID_DestinationList, NULL,
		CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pList));
	if (SUCCEEDED(hr))
	{
		//BeginList  
		UINT uMinSlots;
		IObjectArray *pOARemoved = NULL;
		hr = pList->BeginList(&uMinSlots, IID_PPV_ARGS(&pOARemoved));
		if (SUCCEEDED(hr))
		{
			//ObjectCollection  
			IObjectCollection *pOCTasks = NULL;
			hr = CoCreateInstance(CLSID_EnumerableObjectCollection, NULL,
				CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pOCTasks));
			if (SUCCEEDED(hr))
			{
				IShellLink *pSLAutoRun = NULL;
				hr = CoCreateInstance(CLSID_ShellLink, NULL,
					CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pSLAutoRun));
				if (SUCCEEDED(hr))
				{
					//应用程序路径  
					CString AppLocate;
					AfxGetModuleFileName(AfxGetInstanceHandle(), AppLocate);
					hr = pSLAutoRun->SetPath(AppLocate);
					if (SUCCEEDED(hr))
					{
							//图标  
						CString ShortPath;
						AfxGetModuleShortFileName(AfxGetInstanceHandle(), ShortPath);
						hr = pSLAutoRun->SetIconLocation(ShortPath, 0);
						if (SUCCEEDED(hr))
						{
							//命令行参数  
							hr = pSLAutoRun->SetArguments(_T("OpenFile"));
							if (SUCCEEDED(hr))
							{
								IPropertyStore *pPS = NULL;
								hr = pSLAutoRun->QueryInterface(IID_PPV_ARGS(&pPS));
								if (SUCCEEDED(hr))
								{
									PROPVARIANT pvTitle;
									hr = InitPropVariantFromString(IsInChinese()?L"新反编译任务":L"New Decompile Task", &pvTitle);
									if (SUCCEEDED(hr))
									{
										hr = pPS->SetValue(PKEY_Title, pvTitle);
										if (SUCCEEDED(hr))
										{
											hr = pPS->Commit();
										}
										PropVariantClear(&pvTitle);
									}
									pPS->Release();
								}
								if (SUCCEEDED(hr))
								{
									hr = pOCTasks->AddObject(pSLAutoRun);
								}
							}
						}
					}
					pSLAutoRun->Release();
				}
				if (SUCCEEDED(hr))
				{
					//ObjectArray  
					IObjectArray *pOATasks = NULL;
					hr = pOCTasks->QueryInterface(IID_PPV_ARGS(&pOATasks));
					if (SUCCEEDED(hr))
					{
						hr = pList->AddUserTasks(pOATasks);
						if (SUCCEEDED(hr))
						{
							hr = pList->CommitList();
						}
						pOATasks->Release();
					}
				}
				pOCTasks->Release();
			}
			pOARemoved->Release();
		}
		pList->Release();
	}


	// 创建主 MDI 框架窗口
	m_bDeferShowOnFirstWindowPlacementLoad = TRUE;
	CMainFrame* pMainFrame = new CMainFrame;
	if (!pMainFrame || !pMainFrame->LoadFrame(IDR_MAINFRAME, WS_OVERLAPPEDWINDOW | WS_MAXIMIZE | FWS_ADDTOTITLE))
	{
		delete pMainFrame;
		return FALSE;
	}
	m_pMainWnd = pMainFrame;
	m_pMainWnd->DragAcceptFiles();


	// 分析标准 shell 命令、DDE、打开文件操作的命令行
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	FontFaceName = GetProfileStringW(L"Font", L"FaceName", L"Microsoft YaHei");
	FontSize = (int)GetProfileInt(L"Font", L"FontSize", 12);
	EnableAutoSave = this->GetProfileInt(L"Decomp", L"AutoSave", TRUE);
	IsQuickDecomp = this->GetProfileIntW(L"Decomp", L"IsQuick", FALSE);
	EnableIgnoreCache = this->GetProfileIntW(L"Decomp", L"IgnoreCache", FALSE);
	CStringW Path = this->GetProfileStringW(L"ShellViewPath", L"PathName", L"Not Found");
	UINT Enabled = this->GetProfileIntW(L"ShellViewPath", L"EnableRecovery", 1);
	LPBYTE Buffer;
	UINT size;
	this->GetProfileBinary(L"Decomp", L"ArgumentsData", &Buffer, &size);
	if (size>0)
	{
		CMemFile memFile;
		memFile.Attach(Buffer, size);
		CArchive Ar(&memFile, CArchive::load);
		Ar >> m_DecompOption;
		Ar.Close();
		delete [] memFile.Detach();
		CommonWrapper::GetMainFrame()->m_wndProperties.SyncWithOptionData();
	}
	if (Enabled)
	{
		SetEvent(EnableRecovery);
	}
	else
	{
		ResetEvent(EnableRecovery);
	}
	if (Path != L"Not Found")
	{
		if (WaitForSingleObject(EnableRecovery, 0) != WAIT_TIMEOUT)
		{
			static_cast<CMainFrame*>(m_pMainWnd)->m_wndFileView.m_ShellTreeView.ShowWindow(SW_HIDE);
			static_cast<CMainFrame*>(m_pMainWnd)->m_wndFileView.m_ShellTreeView.SelectPath(Path);
			static_cast<CMainFrame*>(m_pMainWnd)->m_wndFileView.m_ShellTreeView.ShowWindow(SW_SHOW);
		}
		PathToSave = Path;
	}
	LaunchDlg->ShowWindow(SW_HIDE);
	LaunchDlg->DestroyWindow();
	delete LaunchDlg;

	// 调度在命令行中指定的命令。  如果
	// 用 /RegServer、/Register、/Unregserver 或 /Unregister 启动应用程序，则返回 FALSE。
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	if (CommonWrapper::GetMainFrame()->MDIGetActive())
	{
		CommonWrapper::GetMainFrame()->MDIGetActive()->MDIDestroy();
	}

	m_pMainWnd->SetWindowText(L"FernFlowerUI");

	// 主窗口已初始化，因此显示它并对其进行更新
	pMainFrame->m_wndCaptionBar.ShowWindow(SW_HIDE);
	pMainFrame->m_wndFileView.ShowPane(true, false, true);
	pMainFrame->ShowWindow(m_nCmdShow);
	pMainFrame->UpdateWindow();

	AccessJar(AfxGetMainWnd()->m_hWnd);

	AsyncCheckForJava();

	/*if (cmdInfo.m_nShellCommand == CCommandLineInfo::FileOpen)
	{
		DoDecomplie(cmdInfo.m_strFileName);
	}*/

	Initing = false;

	return TRUE;
}

void CFernflowerUIMFCApp::DoDecomplie(const CStringW & Source)
{
	if (!AccessJar(AfxGetMainWnd()->m_hWnd))
	{
		return;
	}
	CFile File;
	if (!File.Open(Source, CFile::shareDenyNone | CFile::OpenFlags::modeRead))
	{
		AfxGetMainWnd()->MessageBox(IsInChinese() ? L"所选文件不存在!" : L"The chosen file doesn't exist!", IsInChinese() ? L"错误" : L"Error", MB_ICONERROR);
		return;
	}
	CommonWrapper::GetMainFrame()->m_wndProperties.UpdateOptionData();
	CString DisplayName = File.GetFileName();
	File.Close();
	JarFilePath = Source;
	//I think these code looks like Java,especially in "if"
	if (Source.Right(6).CompareNoCase(L".class") == 0)
	{
		Flag = DecompFlags::DecompClass;
	}
	else if ((Source.Right(4).CompareNoCase(L".jar") == 0) || (Source.Right(4).CompareNoCase(L".zip") == 0))
	{
		Flag = DecompFlags::DecompJar;
	}
	CString Md5 = CMD5Checksum::GetMD5(Source);
	if (Md5ofFile == Md5)
	{
		CommonWrapper::GetMainFrame()->m_wndClassView.ShowPane(true, false, true);
		return;
	}
	Md5ofFile = Md5;
	AddToRecentFileList(Source);
//	SHAddToRecentDocs(SHARD_PATHW, (LPCWSTR)Source);
	static_cast<CMainFrame*>(AfxGetMainWnd())->m_wndOutput.OutputLog((const wchar_t *)(Source), Flag, DisplayName);
}

int CFernflowerUIMFCApp::ExitInstance()
{
	//TODO: 处理可能已添加的附加资源
	::CoUninitialize();

	CStringW Path = PathToSave;
	this->WriteProfileStringW(L"Font", L"FaceName", FontFaceName);
	this->WriteProfileInt(L"Font", L"FontSize", FontSize);
	this->WriteProfileStringW(L"ShellViewPath", L"PathName", Path);
	this->WriteProfileInt(L"ShellViewPath", L"EnableRecovery", WaitForSingleObject(EnableRecovery,0)!=WAIT_TIMEOUT);
	this->WriteProfileInt(L"Decomp", L"IsQuick", IsQuickDecomp);
	this->WriteProfileInt(L"Decomp", L"AutoSave", EnableAutoSave);
	this->WriteProfileInt(L"Decomp", L"IgnoreCache", EnableIgnoreCache);
	CMemFile memFile;
	CArchive Ar(&memFile, CArchive::store);
	Ar << m_DecompOption;
	Ar.Close();
	size_t length = memFile.GetLength();
	BYTE * pBuf = memFile.Detach();
	this->WriteProfileBinary(L"Decomp", L"ArgumentsData", pBuf, length);
	delete [] pBuf;

	bool IsToRmCache = (WaitForSingleObject(EvtIsToRmCache, 0) != WAIT_TIMEOUT);
	bool IsLastOne = (WaitForSingleObject(SemInstanceCount, 0) == WAIT_TIMEOUT);

	if (IsToRmCache&&IsLastOne)
	{
		MtxRmCache = CreateMutex(nullptr, FALSE, L"6168218c.FernflowerUI.RemovingCache");
		if (MtxRmCache==nullptr)
		{
			MessageBox(nullptr, IsInChinese()?L"清除缓存失败":L"Failed to clear the cache!", IsInChinese()?L"错误":L"Error", MB_ICONERROR);
		}
		else
		{
			SHFILEOPSTRUCT RmCache = { 0 };
			char * buf;
			size_t len;
			if (!_dupenv_s(&buf, &len, "USERPROFILE"))
			{
				CStringW CachePath, CacheDir;
				CachePath += buf;
				CachePath += L"\\AppData\\Local\\FernFlowerUI\\Cache\\";
				CacheDir = (CStringW)buf + L"\\AppData\\Local\\FernFlowerUI\\Cache";
				CachePath += L'\0';
				free(buf);
				RmCache.lpszProgressTitle = L"正在删除缓存";
				RmCache.fFlags = FOF_NOCONFIRMATION | FOF_SILENT | FOF_ALLOWUNDO;
				RmCache.wFunc = FO_DELETE;
				RmCache.pFrom = CachePath;
				RmCache.pTo = nullptr;
				DWORD result;
				if (result = SHFileOperation(&RmCache))
				{
					MessageBox(nullptr, IsInChinese()?L"清除缓存失败":L"Failed to clear the cache!", IsInChinese()?L"错误":L"Error", MB_ICONERROR);
				}
				CreateDirectory(CacheDir, nullptr);
			}
			else
			{
				MessageBox(nullptr, IsInChinese() ? L"清除缓存失败" : L"Failed to clear the cache!", IsInChinese() ? L"错误" : L"Error", MB_ICONERROR);
			}
			CloseHandle(MtxRmCache);
		}
	}

	CloseHandle(EnableRecovery);
	CloseHandle(EvtIsToRmCache);
	CloseHandle(SemInstanceCount);

	AfxOleTerm(FALSE);

	return CWinAppEx::ExitInstance();
}

// CFernflowerUIMFCApp 消息处理程序


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

// 用于运行对话框的应用程序命令
void CFernflowerUIMFCApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

void CFernflowerUIMFCApp::OnEditCopy()
{
	// TODO: 在此添加命令处理程序代码
	if (CommonWrapper::GetMainFrame()->MDIGetActive())
	{
		CFernflowerUIMFCView * View = static_cast<CFernflowerUIMFCView*>(static_cast<CMainFrame*>(AfxGetMainWnd())->MDIGetActive()->GetActiveView());
		View->m_wndEdit.Copy();
	}
	else
	{
		m_pMainWnd->MessageBox(L"尚未打开任何Java文件", L"错误", MB_ICONERROR);
	}
}

void CFernflowerUIMFCApp::OnEditSelectAll()
{
	// TODO: 在此添加命令处理程序代码
	CFernflowerUIMFCView * View = static_cast<CFernflowerUIMFCView*>(static_cast<CMainFrame*>(AfxGetMainWnd())->MDIGetActive()->GetActiveView());
	View->m_wndEdit.SetSel(0, -1);
}

void CFernflowerUIMFCApp::OpenFile()
{
	if (!AccessJar(AfxGetMainWnd()->m_hWnd))
	{
		return;
	}
	CFileDialog OpenDlg(true, nullptr, nullptr, OFN_EXPLORER | OFN_FORCESHOWHIDDEN, 
		IsInChinese()?L"Jar文件 (*.jar)|*.jar|Zip文件 (*.zip)|*.zip|Java类 (*.class)|*.class||":
		L"Java Archive File (*.jar)|*.jar|Zip File (*.zip)|*.zip|Java Class (*.class)|*.class||", m_pMainWnd);
	static_cast<CMainFrame*>(AfxGetMainWnd())->m_wndStatusBar.SetPaneText(ID_SEPARATOR,IsInChinese()? L"请选择反编译源文件":L"Please choose the source Jar file");
	if (OpenDlg.DoModal() == IDOK)
	{
		DoDecomplie(OpenDlg.GetPathName());
	}
	else
	{
		return;
	}
}

// CFernflowerUIMFCApp 自定义加载/保存方法

void CFernflowerUIMFCApp::PreLoadState()
{
	BOOL bNameValid;
	CString strName;
	bNameValid = strName.LoadString(IDS_EDIT_MENU);
	ASSERT(bNameValid);
	GetContextMenuManager()->AddMenu(strName, IDR_POPUP_EDIT);
	//bNameValid = strName.LoadString(IDS_EXPLORER);
	//ASSERT(bNameValid);
	//GetContextMenuManager()->AddMenu(strName, IDR_POPUP_EXPLORER);
}

void CFernflowerUIMFCApp::LoadCustomState()
{
}

void CFernflowerUIMFCApp::SaveCustomState()
{
}

// CFernflowerUIMFCApp 消息处理程序


DWORD ProcessID;
DWORD dwExitCode;
static bool FinishedCheck;
bool AccessJar(HWND hwnd)
{
/*	if (!theApp.IsJDKInstalled)
	{
		STARTUPINFO StartInfo = { 0 };
		PROCESS_INFORMATION pi;
		StartInfo.cb = sizeof StartInfo;
		StartInfo.dwFlags = STARTF_USESHOWWINDOW;
		StartInfo.wShowWindow = SW_HIDE;
		bool bSucceed = CreateProcess(L"C:\\Windows\\System32\\cmd.exe", L"/c java -version", nullptr, nullptr,
			TRUE, CREATE_NO_WINDOW | HIGH_PRIORITY_CLASS, nullptr, nullptr, &StartInfo, &pi);
		if (!bSucceed)
		{
			AfxGetMainWnd()->MessageBox(IsInChinese() ? L"检测Java虚拟机失败!" :
				L"Failed to detect the existance of JDK/JRE!", IsInChinese() ? L"错误" : L"Error", MB_ICONERROR);
			return false;
		}
		ProcessID = pi.dwProcessId;
		CommonWrapper::CWaitDlg Wait(CommonWrapper::GetMainFrame(), []()->bool {
			HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, ProcessID);
			GetExitCodeProcess(hProcess, &dwExitCode);
			CloseHandle(hProcess);
			return dwExitCode != STILL_ACTIVE;
		}, 5, IsInChinese() ? L"正在检测Java虚拟机" : L"Detecting the existance of JDK/JRE");
		Wait.DoModal();
		if (dwExitCode==1)
		{
			if (MessageBoxA(hwnd, IsInChinese()?"发现未安装Java虚拟机或未正确设置Java,本程序需要Java虚拟机才能正常使用,是否打开下载链接?":
				"It seems that you haven't installed JDK/JRE yet,or you didn't set up JDK/JRE properly.The application requires JDK/JRE to run,would you like to visit the website of Oracle?", 
				IsInChinese()?"警告":"Warning", MB_ICONWARNING | MB_OKCANCEL) == IDOK)
			{
				ShellExecuteA(NULL, "open", "http://www.oracle.com/technetwork/java/javase/overview/index.html", NULL, NULL, SW_SHOW);
			}
			return false;
		}
		else
		{
			theApp.IsJDKInstalled = true;
		}
	}*/
	if (!theApp.Initing)
	{
		WaitForFinishCheck();
	}
	if (ClickedDownloadJDK)
	{
		STARTUPINFO StartInfo = { 0 };
		PROCESS_INFORMATION pi;
		StartInfo.cb = sizeof StartInfo;
		StartInfo.dwFlags = STARTF_USESHOWWINDOW;
		StartInfo.wShowWindow = SW_HIDE;
		bool bSucceed = CreateProcess(L"C:\\Windows\\System32\\cmd.exe", L"/c java -version", nullptr, nullptr,
			TRUE, CREATE_NO_WINDOW | HIGH_PRIORITY_CLASS, nullptr, nullptr, &StartInfo, &pi);
		if (!bSucceed)
		{
			AfxGetMainWnd()->MessageBox(IsInChinese() ? L"检测Java虚拟机失败!" :
				L"Failed to detect the existance of JDK/JRE!", IsInChinese() ? L"错误" : L"Error", MB_ICONERROR);
			return false;
		}
		ProcessID = pi.dwProcessId;
		CommonWrapper::CWaitDlg Wait(CommonWrapper::GetMainFrame(), []()->bool {
			HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, ProcessID);
			GetExitCodeProcess(hProcess, &dwExitCode);
			CloseHandle(hProcess);
			return dwExitCode != STILL_ACTIVE;
		}, 5, IsInChinese() ? L"正在检测Java虚拟机" : L"Detecting the existance of JDK/JRE");
		Wait.DoModal();
		if (dwExitCode == 1)
		{
			if (MessageBoxA(hwnd, IsInChinese() ? "发现未安装Java虚拟机或未正确设置Java,本程序需要Java虚拟机才能正常使用,是否打开下载链接?" :
				"It seems that you haven't installed JDK/JRE yet,or you didn't set up JDK/JRE properly.The application requires JDK/JRE to run,would you like to visit the website of Oracle?",
				IsInChinese() ? "警告" : "Warning", MB_ICONWARNING | MB_OKCANCEL) == IDOK)
			{
				ShellExecuteA(NULL, "open", "http://www.oracle.com/technetwork/java/javase/overview/index.html", NULL, NULL, SW_SHOW);
			}
			return false;
		}
		else
		{
			theApp.IsJDKInstalled = true;
		}
	}
	if (!theApp.IsJDKInstalled&&!theApp.Initing)
	{
		if (MessageBoxA(hwnd, IsInChinese() ? "发现未安装Java虚拟机或未正确设置Java,本程序需要Java虚拟机才能正常使用,是否打开下载链接?" :
			"It seems that you haven't installed JDK/JRE yet,or you didn't set up JDK/JRE properly.The application requires JDK/JRE to run,would you like to visit the website of Oracle?",
			IsInChinese() ? "警告" : "Warning", MB_ICONWARNING | MB_OKCANCEL) == IDOK)
		{
			ShellExecuteA(NULL, "open", "http://www.oracle.com/technetwork/java/javase/overview/index.html", NULL, NULL, SW_SHOW);
			ClickedDownloadJDK = true;
		}
		else
		{
			ClickedDownloadJDK = false;
		}
		return false;
	}
	char * userprofile;
	size_t len;
	if (_dupenv_s(&userprofile, &len, "USERPROFILE"))
	{
		return false;
	}
	std::string saccess = userprofile;
	free(userprofile);
	saccess += "\\AppData\\Local\\FernFlowerUI";
	if (_access(saccess.c_str(), 0) == -1)
	{
		_mkdir(saccess.c_str());
		if (MessageBoxA(hwnd, IsInChinese()?"检测到是第一次使用，是否要下载fernflower.jar?":
			"It seems that you're using FernflowerUI the first time, would you like to download the dependencies?", "FernflowerUI", MB_OKCANCEL | MB_ICONQUESTION) == IDOK)
			return DownloadJar();
		else
		{
			return false;
		}
	}
	else if (_access((saccess + "\\fernflower.jar").c_str(), 0) == -1)
	{
		if (MessageBoxA(hwnd, IsInChinese() ? "找不到%USERPROFILE%\\AppData\\Local\\FernFlowerUI\\fernflower.jar,是否要下载?" :
			"Couldn't access %USERPROFILE%\\AppData\\Local\\FernFlowerUI\\fernflower.jar,would you like to download?", "FernflowerUI", MB_OKCANCEL | MB_ICONQUESTION) == IDOK)
			return DownloadJar();
		else
		{
			return false;
		}
	}
	else
	{
		CStringW strMD5 = CMD5Checksum::GetMD5(CString((saccess + "\\fernflower.jar").c_str()));
		if (strMD5!="77FF107323E36E6D213CE6D8B4E2D32A")
		{
			if (MessageBoxA(hwnd, IsInChinese()?"找不到%USERPROFILE%\\AppData\\Local\\FernFlowerUI\\fernflower.jar,是否要下载?":
				"Couldn't access %USERPROFILE%\\AppData\\Local\\FernFlowerUI\\fernflower.jar,would you like to download?", "FernflowerUI", MB_OKCANCEL | MB_ICONQUESTION) == IDOK)
				return DownloadJar();
			else
			{
				return false;
			}
		}
	}
	return true;
}

const int MAXBLOCKSIZE = 1024;
char Buffer[MAXBLOCKSIZE];
unsigned long Number = 1;
int Byte;
//FILE* stream;
HINTERNET hSession, handle;
bool Finished;

bool DownloadJar()
{
	std::ofstream fout;
	std::ios_base::sync_with_stdio(false);
	hSession = InternetOpenA("FernFlowerUI3.4", INTERNET_OPEN_TYPE_PRECONFIG, nullptr, nullptr, 0);
	try
	{
		if (hSession)
		{
			handle = InternetOpenUrlA(hSession, "https://raw.githubusercontent.com/6168218c/Fernflower/master/fernflower.jar",
				nullptr, 0, INTERNET_FLAG_DONT_CACHE, 0);
			if (handle)
			{
				char * filestr;
				size_t length;
				errno_t err = _dupenv_s(&filestr, &length, "USERPROFILE");
				if (err)
				{
					MessageBoxA(nullptr, IsInChinese()?"无法打开%USERPROFILE%":"Failed to access %USERPROFILE%", IsInChinese()?"错误":"Error", MB_ICONERROR);
					return false;
				}
				std::string filename = filestr;
				free(filestr);
				filename += "\\AppData\\Local\\FernFlowerUI\\fernflower.jar";
				fout.open(filename.c_str(), std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
				if (fout.is_open())
				{
					/*CWaitDlg wait(AfxGetMainWnd(), []() ->bool
					{
					InternetReadFile(handle, Buffer, MAXBLOCKSIZE - 1, &Number);
					fwrite(Buffer, sizeof(char), Number, stream);
					bool Downloaded = !(Number > 0);
					return Downloaded;
					},);
					wait.DoModal();
					*/
					Byte = 0;
					CommonWrapper::CProgressBar Progress(AfxGetMainWnd(), []()->int {return Finished ? -1 : Byte; }, 5,
						IsInChinese()? L"正在下载fernflower.jar":L"Downloading fernflower.jar from github.com", 1, 243);
					std::future<void> Download = std::async(std::launch::async, [&]() {
						while (Number > 0)
						{
							InternetReadFile(handle, Buffer, MAXBLOCKSIZE - 1, &Number);
							//fwrite(Buffer, sizeof(char), Number, stream);
							fout.write(Buffer, Number);
							Byte++;
						}
						Finished = true;
					});
					Progress.DoModal();
					//fclose(stream);
					fout.close();
				}
				InternetCloseHandle(handle);
				handle = nullptr;
			}
			else
			{
				InternetCloseHandle(hSession);
				hSession = nullptr;
				throw connection_error();
			}
			InternetCloseHandle(hSession);
			hSession = nullptr;
		}
		else
		{
			throw connection_error();
		}
	}
	catch (const std::exception& ex)
	{
		MessageBox(nullptr, IsInChinese()?CStringW(L"已引发异常:") + CStringW(ex.what()) + L"\n下载失败,尝试手动下载,链接:https://raw.githubusercontent.com/6168218c/Fernflower---forge/master/fernflower.jar":
			CStringW(L"Exceptions found:")+CStringW(ex.what())+CStringW(L"\nDownload failed,Try to download it manually.Link:https://raw.githubusercontent.com/6168218c/Fernflower---forge/master/fernflower.jar"),
			IsInChinese()?L"下载失败":L"Download failed", MB_ICONERROR);
		ShellExecuteA(NULL, "open", "https://raw.githubusercontent.com/6168218c/Fernflower/master/fernflower.jar", NULL, NULL, SW_SHOW);
		std::ios_base::sync_with_stdio(true);
		return false;
	}
	std::ios_base::sync_with_stdio(true);
	return true;
}

void AsyncCheckForJava()
{
	FinishedCheck = false;
	STARTUPINFO StartInfo = { 0 };
	PROCESS_INFORMATION pi;
	StartInfo.cb = sizeof StartInfo;
	StartInfo.dwFlags = STARTF_USESHOWWINDOW;
	StartInfo.wShowWindow = SW_HIDE;
	bool bSucceed = CreateProcess(L"C:\\Windows\\System32\\cmd.exe", L"/c java -version", nullptr, nullptr,
		TRUE, CREATE_NO_WINDOW | HIGH_PRIORITY_CLASS, nullptr, nullptr, &StartInfo, &pi);
	ProcessID = pi.dwProcessId;
	if (!bSucceed)
	{
		AfxGetMainWnd()->MessageBox(IsInChinese() ? L"检测Java虚拟机失败!" :
			L"Failed to detect the existance of JDK/JRE!", IsInChinese() ? L"错误" : L"Error", MB_ICONERROR);
	}
	std::thread Checker([]() {
		dwExitCode = STILL_ACTIVE;
		HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, ProcessID);
		GetExitCodeProcess(hProcess, &dwExitCode);
		while (dwExitCode == STILL_ACTIVE)
		{
			Sleep(5);
			GetExitCodeProcess(hProcess, &dwExitCode);
		}
		CloseHandle(hProcess);
		if (dwExitCode==1)
		{
			theApp.IsJDKInstalled = false;
			if (MessageBoxA(CommonWrapper::GetMainFrame()->GetSafeHwnd(), IsInChinese() ? "发现未安装Java虚拟机或未正确设置Java,本程序需要Java虚拟机才能正常使用,是否打开下载链接?" :
				"It seems that you haven't installed JDK/JRE yet,or you didn't set up JDK/JRE properly.The application requires JDK/JRE to run,would you like to visit the website of Oracle?",
				IsInChinese() ? "警告" : "Warning", MB_ICONWARNING | MB_OKCANCEL) == IDOK)
			{
				ShellExecuteA(NULL, "open", "http://www.oracle.com/technetwork/java/javase/overview/index.html", NULL, NULL, SW_SHOW);
				ClickedDownloadJDK = true;
			}
			else
			{
				ClickedDownloadJDK = false;
			}
		}
		else
		{
			theApp.IsJDKInstalled = true;
		}
		FinishedCheck = true;
	});
	Checker.detach();
}

void WaitForFinishCheck()
{
	CommonWrapper::CWaitDlg Wait(CommonWrapper::GetMainFrame(),
		[]() {return FinishedCheck; }, 5, IsInChinese() ? L"正在检测Java虚拟机" : L"Detecting the existance of JDK/JRE");
	Wait.DoModal();
}


void CFernflowerUIMFCApp::OnFileSaveAs()
{
	// TODO: 在此添加命令处理程序代码
	if (CommonWrapper::GetMainFrame()->MDIGetActive())
	{
		CFileDialog FileDialog(false,nullptr,static_cast<CChildFrame*>(CommonWrapper::GetMainFrame()->MDIGetActive())->m_strTitle,
			OFN_EXPLORER|OFN_FORCESHOWHIDDEN,IsInChinese()? L"Java源文件 (*.java)|*.java|所有文件 (*.*)|*.*||":L"Java Source File (*.java)|*.java|All Files (*.*)|*.*||",m_pMainWnd);
		if (FileDialog.DoModal() == IDOK)
		{
			CString Content;
			static_cast<CFernflowerUIMFCView*>(CommonWrapper::GetMainFrame()->MDIGetActive()->GetActiveView())->m_wndEdit.GetWindowTextW(Content);
			//Content.Replace(L"      ", L"   ");
			CStdioFile File;
			File.Open(FileDialog.GetPathName(), CFile::modeCreate | CFile::modeWrite | CFile::OpenFlags::shareDenyWrite);
			File.WriteString(Content);
			File.Close();
		}
	}
	else
	{
		m_pMainWnd->MessageBox(L"尚未打开任何Java文件", L"错误", MB_ICONERROR);
	}
}


CDocument* CFernflowerUIMFCApp::OpenDocumentFile(LPCTSTR lpszFileName)
{
	// TODO: 在此添加专用代码和/或调用基类
	// 主窗口已初始化，因此显示它并对其进行更新
	if (theApp.Initing)
	{
		m_pMainWnd->SetWindowText(L"FernFlowerUI");
		CommonWrapper::GetMainFrame()->m_wndCaptionBar.ShowWindow(SW_HIDE);
		CommonWrapper::GetMainFrame()->m_wndFileView.ShowPane(true, false, true);
		CommonWrapper::GetMainFrame()->RecalcLayout(FALSE);
		CommonWrapper::GetMainFrame()->ShowWindow(m_nCmdShow);
		CommonWrapper::GetMainFrame()->UpdateWindow();
	}
	if (CString(lpszFileName) == L"OpenFile")
	{
		OpenFile();
		return (CDocument*)TRUE;
	}

	DoDecomplie(lpszFileName);
	// 只要不为NULL都可以,仅是为了防止MRU中此项被移除
	return (CDocument*)TRUE;
}


void CFernflowerUIMFCApp::OnFileSave()
{
	// TODO: 在此添加命令处理程序代码
	if (CommonWrapper::GetMainFrame()->IsShowingWindowManager)
	{
		OnFileSaveAs();
	}
	if (Flag==CFernflowerUIMFCApp::DecompFlags::DecompClass)
	{
		CStringW FileName;
		char * UserProFile;
		size_t size;
		if (_dupenv_s(&UserProFile, &size, "USERPROFILE"))
		{
			AfxGetMainWnd()->MessageBox(IsInChinese() ? _T("搜索%USERPROFILE%失败!") : _T("Failed to access %USERPROFILE%"), IsInChinese() ? _T("错误") : _T("Error"), MB_ICONERROR);
			return;
		}
		CStringW JarPath;
		JarPath = UserProFile;
		JarPath += L"\\AppData\\Local\\FernFlowerUI\\Cache\\";
		JarPath += theApp.Md5ofFile;
		JarPath += L"\\*.*";
		free(UserProFile);
		CFileFind FindFile;
		bool b = FindFile.FindFile(JarPath);
		if (b)
		{
			b = FindFile.FindNextFile();
			if (b)
			{
				FileName = FindFile.GetFilePath();
			}
		}
		if (!b)
		{
			CommonWrapper::GetMainFrame()->MessageBox(IsInChinese() ? L"找不到Java文件!" : L"Could not find the Java Source File!", IsInChinese() ? L"保存失败" : L"Failed to Save", MB_ICONERROR);
			return;
		}
		CFileDialog FileDialog(false, nullptr, static_cast<CChildFrame*>(CommonWrapper::GetMainFrame()->MDIGetActive())->m_strTitle,
			OFN_EXPLORER | OFN_FORCESHOWHIDDEN, IsInChinese() ? L"Java源文件 (*.java)|*.java|所有文件 (*.*)|*.*||" : L"Java Source File (*.java)|*.java|All Files (*.*)|*.*||", m_pMainWnd);
		if (FileDialog.DoModal() == IDOK)
		{
			CopyFile(FileName, FileDialog.GetPathName(), FALSE);
		}
		return;
	}
	char * UserProFile;
	size_t size;
	if (_dupenv_s(&UserProFile, &size, "USERPROFILE"))
	{
		AfxGetMainWnd()->MessageBox(IsInChinese() ? _T("搜索%USERPROFILE%失败!") : _T("Failed to access %USERPROFILE%"), IsInChinese() ? _T("错误") : _T("Error"), MB_ICONERROR);
		return;
	}
	CStringW JarPath;
	JarPath = UserProFile;
	JarPath += L"\\AppData\\Local\\FernFlowerUI\\Cache\\";
	JarPath += theApp.Md5ofFile;
	JarPath += L"\\JarCache.jar";
	free(UserProFile);
	LPWSTR lpStr = new wchar_t[MAX_PATH + 1];
	GetFileTitle(JarFilePath, lpStr, MAX_PATH);
	CFileDialog FileDialog(false, nullptr, lpStr,
		OFN_EXPLORER | OFN_FORCESHOWHIDDEN, IsInChinese() ? L"Jar文件 (*.jar)|*.jar|所有文件 (*.*)|*.*||" : L"Java Archive File (*.jar)|*.jar|All Files (*.*)|*.*||", m_pMainWnd);
	if (FileDialog.DoModal() == IDOK)
	{
		if (!CopyFile(JarPath, FileDialog.GetPathName(), FALSE))
		{
			AfxGetMainWnd()->MessageBox(IsInChinese() ? L"保存失败!" : L"Failed to save the jar file!", IsInChinese() ? _T("错误") : _T("Error"), MB_ICONERROR);
		}
	}
	delete lpStr;
}


void CFernflowerUIMFCApp::OnUpdateFileSave(CCmdUI *pCmdUI)
{
	// TODO: 在此添加命令更新用户界面处理程序代码
	if (theApp.Md5ofFile == L"")
	{
		pCmdUI->Enable(false);
	}
	else
	{
		pCmdUI->Enable();
	}
}


void CFernflowerUIMFCApp::OnUpdateFileSaveAs(CCmdUI *pCmdUI)
{
	// TODO: 在此添加命令更新用户界面处理程序代码
	if (theApp.Md5ofFile == L"")
	{
		pCmdUI->Enable(false);
	}
	else
	{
		pCmdUI->Enable();
	}
}


void CFernflowerUIMFCApp::OnFileClose()
{
	// TODO: 在此添加命令处理程序代码
	CommonWrapper::GetMainFrame()->m_wndOutput.ClearAllInfo();
	CommonWrapper::GetMainFrame()->SetWindowText(L"FernFlowerUI");
}


void CFernflowerUIMFCApp::OnUpdateFileClose(CCmdUI *pCmdUI)
{
	// TODO: 在此添加命令更新用户界面处理程序代码
	if (theApp.Md5ofFile == L"")
	{
		pCmdUI->Enable(false);
	}
	else
	{
		pCmdUI->Enable();
	}
}


void CFernflowerUIMFCApp::OnUpdateRecentFileMenu(CCmdUI *pCmdUI)
{
	// TODO: 在此添加命令更新用户界面处理程序代码
	ASSERT_VALID(this);
	ENSURE_ARG(pCmdUI != NULL);
	if (m_pRecentFileList == NULL) // no MRU files
		pCmdUI->Enable(FALSE);
	else
	{
		CCmdUI CmdUI = *pCmdUI;
		CMenu * pMenu = CmdUI.m_pMenu;
		if (pMenu)
		{
			CMenu * pSubMenu = pMenu->GetSubMenu(5);
			if (pSubMenu)
			{
				CmdUI.m_pMenu = pSubMenu;
				CmdUI.m_pParentMenu = pMenu;
				CmdUI.m_nIndex = 0;
				m_pRecentFileList->UpdateMenu(&CmdUI);
			}
		}
	}
}
