
#include "stdafx.h"

#include "OutputWnd.h"
#include "Resource.h"
#include "MainFrm.h"
#include "CommonWrapper.h"
#include "Md5Checksum.h"
#include "unzip.h"
#include "zip.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COutputBar

std::mutex mut;
std::condition_variable MainInited;
std::condition_variable WorkEnded;
std::atomic_bool FinishReading, FinishUnzipping, bAbortDecomplie;
static std::atomic_bool Finished;
DWORD ExitCode;
CStatic * pStaticText;
static HANDLE MtxDecomp;

CProgressDlg::CProgressDlg(CWnd * MainWnd) : CDialogEx(IDD_DECOMPLIEBAR,MainWnd),DoProcessInit(true),isExited(STILL_ACTIVE),NewFileSize(0),DecomplieProcess(INVALID_HANDLE_VALUE)
{
}

LRESULT CProgressDlg::OnInitDialog(WPARAM wParam,LPARAM lParam)
{
	LRESULT bRet = HandleInitDialog(wParam, lParam);
	this->SetTimer(ID_TIMER_DOPROGRESS, 40, nullptr);
	m_wndProgressBar = static_cast<CProgressCtrl*>(GetDlgItem(IDC_PROGRESS1));
	m_wndProgressBar->SetRange32(0, INT_MAX / 2);
	m_wndProgressBar->GetRange(iLow, iHigh);
	m_wndProgressBar->SetPos(iLow);
	m_wndProgressBar->SetStep(1);
	Pre = 0;
	m_pStaticText = static_cast<CStatic*>(GetDlgItem(ID_MESSAGE_NOTICE));
	pStaticText = m_pStaticText;
	m_pStaticText->SetWindowTextW(L"正在反编译,请耐心等待...");
	if (m_Font.CreatePointFont(120, L"Microsoft YaHei"))
	{
		static_cast<CStatic*>(GetDlgItem(IDC_GROUPTITLE))->SetFont(&m_Font);
	}
	//unused
	//static_cast<CMainFrame*>(AfxGetMainWnd())->m_wndStatusBar.EnablePaneProgressBar(ID_SEPARATOR, INT_MAX / 2);
	return bRet;
}

void CProgressDlg::OnShowWindow(BOOL bShow,UINT nStatus)
{
//	this->ModifyStyle(WS_POPUP, WS_CHILD);
//	this->SetParent(AfxGetMainWnd());
	theApp.RunningDialog = this;
	this->CDialogEx::OnShowWindow(bShow, nStatus);
//	AfxGetMainWnd()->ModifyStyleEx(true, WS_EX_TOOLWINDOW);
//	PostMessage(WM_COMMAND, ID_DOPROGRESS);
}

void CProgressDlg::DoProgress()
{
	if (DoProcessInit)
	{
		if (!static_cast<CMainFrame*>(AfxGetMainWnd())->GetOutput()->CanUseProgress)
		{
			m_wndProgressBar->ModifyStyle(false, PBS_MARQUEE);
		}
		DecomplieProcess = OpenProcess(PROCESS_QUERY_INFORMATION, false, static_cast<CMainFrame*>(AfxGetMainWnd())->GetOutput()->pi.dwProcessId);
		if (DecomplieProcess == INVALID_HANDLE_VALUE)
		{
			MessageBox(IsInChinese()?L"出现未知错误,无法显示进度":L"An unexpected error occoured.\nUnable to display the decompiling progress!", IsInChinese()?L"错误":L"Error", MB_ICONERROR);
			static_cast<CMainFrame*>(AfxGetMainWnd())->GetOutput()->CanUseProgress = false;
		}
//		AfxGetMainWnd()->EnableWindow(true);
		mut.unlock();
		DoProcessInit = false;
	}
	if (isExited == STILL_ACTIVE)
	{
		if(DecomplieProcess!=INVALID_HANDLE_VALUE)
			GetExitCodeProcess(DecomplieProcess, &isExited);
		if (static_cast<CMainFrame*>(AfxGetMainWnd())->GetOutput()->CanUseProgress)
		{
			NewFileSize = GetFileSize(static_cast<CMainFrame*>(AfxGetMainWnd())->GetOutput()->hfile_new, nullptr);
			int Pos = int(double(NewFileSize) / double(static_cast<CMainFrame*>(AfxGetMainWnd())->GetOutput()->ChosenFileSize)*(iHigh - iLow)) + iLow;
			if (Pos!=Pre)
			{
				if (Pos == (Pre + 1))
					m_wndProgressBar->StepIt();
				else
					m_wndProgressBar->SetPos(Pos);
				//unused
				//static_cast<CMainFrame*>(AfxGetMainWnd())->m_wndStatusBar.SetPaneProgress(ID_SEPARATOR, Pos);
				Pre = m_wndProgressBar->GetPos();
				static_cast<CMainFrame*>(AfxGetMainWnd())->m_pTaskBar->SetProgressValue(AfxGetMainWnd()->GetSafeHwnd(), Pos, INT_MAX / 2);
			}
		}
		else
		{
			static_cast<CMainFrame*>(AfxGetMainWnd())->m_pTaskBar->SetProgressState(AfxGetMainWnd()->GetSafeHwnd(), TBPF_INDETERMINATE);
		}
		return;
	}
	CloseHandle(DecomplieProcess);
	KillTimer(ID_TIMER_DOPROGRESS);
	if (isExited != 0)
	{
		std::ostringstream sout;
		const char * tmp = IsInChinese() ? "Java虚拟机异常退出，退出码为" : "JVM exited abnormally,the exit code is:";
		sout << tmp << isExited;
		bAbortDecomplie = true;
		MessageBoxA(AfxGetMainWnd()->m_hWnd, sout.str().c_str(), IsInChinese()?"错误":"Error", MB_ICONERROR);
		static_cast<CMainFrame*>(AfxGetMainWnd())->m_pTaskBar->SetProgressState(AfxGetMainWnd()->GetSafeHwnd(), TBPF_ERROR);
		ExitCode = isExited;
	}
	static_cast<CMainFrame*>(AfxGetMainWnd())->m_pTaskBar->SetProgressState(AfxGetMainWnd()->GetSafeHwnd(), TBPF_NORMAL);
	static_cast<CMainFrame*>(AfxGetMainWnd())->m_pTaskBar->SetProgressState(AfxGetMainWnd()->GetSafeHwnd(), TBPF_INDETERMINATE);
	//unused
	//static_cast<CMainFrame*>(AfxGetMainWnd())->m_wndStatusBar.EnablePaneProgressBar(ID_SEPARATOR, -1);
	pStaticText = nullptr;
	theApp.RunningDialog = nullptr;
	EndDialog(true);
//	AfxGetMainWnd()->ModifyStyleEx(WS_EX_TOOLWINDOW, true);
}

void CProgressDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

void CProgressDlg::OnTimer(UINT_PTR nIDEvent)
{
	switch (nIDEvent)
	{
	case ID_TIMER_DOPROGRESS:
		SendMessage(WM_COMMAND, ID_DOPROGRESS);
		default:
			CDialogEx::OnTimer(nIDEvent);
	}
}

BEGIN_MESSAGE_MAP(CProgressDlg, CDialogEx)
	ON_MESSAGE(WM_INITDIALOG,&CProgressDlg::OnInitDialog)
	ON_COMMAND(ID_DOPROGRESS,&CProgressDlg::DoProgress)
	ON_WM_SHOWWINDOW()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDCANCEL, &CProgressDlg::OnBnClickedCancel)
END_MESSAGE_MAP()

COutputWnd::COutputWnd()
{
}

COutputWnd::~COutputWnd()
{
}

BEGIN_MESSAGE_MAP(COutputWnd, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
END_MESSAGE_MAP()

int COutputWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rectDummy;
	rectDummy.SetRectEmpty();

	// 创建选项卡窗口: 
	if (!m_wndTabs.Create(CMFCTabCtrl::STYLE_FLAT, rectDummy, this, 1))
	{
		TRACE0("未能创建输出选项卡窗口\n");
		return -1;      // 未能创建
	}

	// 创建输出窗格: 
	const DWORD dwStyle = LBS_NOINTEGRALHEIGHT | WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL;

	if (!m_wndOutputBuild.Create(dwStyle, rectDummy, &m_wndTabs, 2))
	{
		TRACE0("未能创建输出窗口\n");
		return -1;      // 未能创建
	}

	UpdateFonts();

	CString strTabName;
	BOOL bNameValid;

	// 将列表窗口附加到选项卡: 
	bNameValid = strTabName.LoadString(IDS_BUILD_TAB);
	ASSERT(bNameValid);
	m_wndTabs.AddTab(&m_wndOutputBuild, strTabName, (UINT)0);

	// 使用一些虚拟文本填写输出选项卡(无需复杂数据)
	FillBuildWindow();

	return 0;
}

void COutputWnd::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);

	// 选项卡控件应覆盖整个工作区: 
	m_wndTabs.SetWindowPos(NULL, -1, -1, cx, cy, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
}

void COutputWnd::AdjustHorzScroll(CListBox& wndListBox)
{
	CClientDC dc(this);
	CFont* pOldFont = dc.SelectObject(&afxGlobalData.fontRegular);

	int cxExtentMax = 0;

	for (int i = 0; i < wndListBox.GetCount(); i++)
	{
		CString strItem;
		wndListBox.GetText(i, strItem);

		cxExtentMax = max(cxExtentMax, (int)dc.GetTextExtent(strItem).cx);
	}

	wndListBox.SetHorizontalExtent(cxExtentMax);
	dc.SelectObject(pOldFont);
}

void COutputWnd::FillBuildWindow()
{
	m_wndOutputBuild.AddString(IsInChinese() ? _T("反编译输出正显示在此处。") : _T("The decompling output is shown here"));
	m_wndOutputBuild.AddString(IsInChinese() ? _T("输出正显示在列表视图的行中") : _T("The output is shown in the lines of the List View"));
	m_wndOutputBuild.AddString(IsInChinese() ? _T("但您可以根据需要更改其显示方式...") : _T("But you can change the way it displays according to need"));
}

int COutputWnd::OutputLog(const std::wstring & Log, CFernflowerUIMFCApp::DecompFlags Flag, const CStringW & DisplayName)
{
	std::ifstream fin;
	bool IsDecomplieDirExisted = false;
	bAbortDecomplie = false;
	AfxGetMainWnd()->SetWindowTextW(L"FernFlowerUI");
	CStringW FernflowerPath = L"fernflower" + theApp.Md5ofFile + L".jar";
	CStringA FernflowerName;
	FernflowerName = FernflowerPath;
	static_cast<CMainFrame*>(AfxGetMainWnd())->m_wndStatusBar.SetPaneText(ID_SEPARATOR, IsInChinese() ? L"正在启动反编译进程" : L"Launching the decompile process");
	wchar_t * buf;
	size_t size;
	if (_wdupenv_s(&buf, &size, L"USERPROFILE"))
	{
		AfxGetMainWnd()->MessageBox(IsInChinese() ? _T("搜索%USERPROFILE%失败!") : _T("Failed to access %USERPROFILE%"), IsInChinese() ? _T("错误") : _T("Error"), MB_ICONERROR);
		return false;
	}
	else
	{
		CStringW Directory = buf;
		CStringW OrigJar = buf;
		OrigJar += L"\\AppData\\Local\\FernFlowerUI\\fernflower.jar";
		Directory += L"\\AppData\\Local\\FernFlowerUI\\Cache";
		CreateDirectory(Directory, nullptr);
		Directory += L"\\";
		Directory += theApp.Md5ofFile;
		FernflowerPath = Directory + L"\\" + FernflowerPath;
		while (!theApp.EnableIgnoreCache)
		{
			if (_waccess(Directory, 0) != -1)
			{
				CString OptionPath = Directory + L"\\OptionData.bin";
				CFile File;
				bool bDifferent = true;
				if (File.Open(OptionPath, CFile::modeRead | CFile::shareDenyWrite))
				{
					CFile Tmp;
					Tmp.Open(Directory + L"\\Temp.bin", CFile::modeWrite | CFile::modeCreate | CFile::shareDenyWrite);
					CArchive ar(&Tmp, CArchive::store);
					ar << theApp.m_DecompOption;
					ar.Close();
					File.Close();
					Tmp.Close();
					if (CMD5Checksum::GetMD5(Directory + L"\\Temp.bin") == CMD5Checksum::GetMD5(OptionPath))
					{
						bDifferent = false;
					}
					CFile::Remove(Directory + L"\\Temp.bin");
				}
				if (MtxDecomp = OpenMutex(MUTEX_ALL_ACCESS | SYNCHRONIZE, FALSE, CString(L"6168218c.FernflowerUI.Decomp") + theApp.Md5ofFile))
				{
					DWORD dwResult = WAIT_TIMEOUT;
					DWORD dwErrcode = ERROR_SUCCESS;
					auto scan = [&]()->bool {
					BOOL bRet= (dwResult = WaitForSingleObject(MtxDecomp, 0)) != WAIT_TIMEOUT;
					if (bRet)
					{
						dwErrcode = GetLastError();
					}
					return bRet; };
					CommonWrapper::CWaitDlg Wait(AfxGetMainWnd(), scan, 5, IsInChinese() ? L"正在完成操作..." : L"Finishing...");
					Wait.DoModal();
					if (dwResult == WAIT_OBJECT_0)
					{
						ReleaseMutex(MtxDecomp);
						CloseHandle(MtxDecomp);
						File.Open(OptionPath, CFile::modeRead | CFile::shareDenyWrite);
						CArchive ar(&File, CArchive::load);
						ar >> theApp.m_DecompOption;
						CommonWrapper::GetMainFrame()->m_wndProperties.SyncWithOptionData();
						CommonWrapper::GetMainFrame()->m_wndCaptionBar.SetText(IsInChinese() ? L"由于已有另一实例成功反编译,反编译参数自动与该实例同步" :
							L"Another instance has already decompiled successfully,so the decompile arguments have been synchronized with the instance automatically.", 
							CMFCCaptionBar::ALIGN_LEFT);
						CommonWrapper::GetMainFrame()->m_wndCaptionBar.SetImageToolTip(IsInChinese() ? L"提示" : L"Tips",
							IsInChinese() ? L"由于已有另一实例成功反编译,反编译参数自动与该实例同步" :
							L"Another instance has already decompiled successfully,so the decompile arguments have been synchronized with the instance automatically.");
						CommonWrapper::GetMainFrame()->m_wndCaptionBar.ShowWindow(SW_SHOW);
						CommonWrapper::GetMainFrame()->RecalcLayout(FALSE);
					}
					else if (dwResult == WAIT_ABANDONED)
					{
						HWND hWndDecomplie = ::FindWindow(L"ConsoleWindowClass", CString(L"6168218c.FernflowerUI.Decomplie.") + theApp.Md5ofFile);
						::SendMessage(hWndDecomplie, WM_CLOSE, 0, 0);
						ReleaseMutex(MtxDecomp);
						CloseHandle(MtxDecomp);
						break;
					}
					else if (dwResult == WAIT_FAILED)
					{
						LPTSTR buf;
						FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, nullptr, GetLastError(), GetThreadUILanguage(), (LPTSTR)&buf, 0, nullptr);
						CString strError = buf;
						LocalFree(buf);
						CommonWrapper::GetMainFrame()->MessageBox(IsInChinese() ? (L"灾难性错误:\n" + strError + L"反编译已取消") : (L"Fatal:\n" + strError + L"\nThe decompilation is cancelled."), IsInChinese() ? L"错误" : L"Error", MB_ICONERROR);
						CloseHandle(MtxDecomp);
						ClearAllInfo();
						return false;
					}
				}
				else
				{
					if (_waccess(FernflowerPath, 0) != -1)
					{
						HWND hWndDecomplie = ::FindWindow(L"ConsoleWindowClass", CString(L"6168218c.FernflowerUI.Decomplie.") + theApp.Md5ofFile);
						::SendMessage(hWndDecomplie, WM_CLOSE, 0, 0);
						DeleteFile(FernflowerPath);
						break;
					}
					else if (bDifferent)
					{
						if (CommonWrapper::GetMainFrame()->MessageBox(
							IsInChinese() ? L"发现之前反编译数据的反编译参数与当前反编译参数不同,是否重新反编译?" : L"It seems that the decompile arguments of the former decompilation data are different from the current arguments,would you like to decompile again?",
							IsInChinese() ? L"反编译参数不同" : L"Decompile arguments are different", MB_OKCANCEL | MB_ICONQUESTION) == IDOK)
						{
							break;
						}
					}
				}
				CStringW ZipPlace;
				ZipPlace = buf;
				ZipPlace += L"\\AppData\\Local\\FernFlowerUI\\Cache\\";
				ZipPlace += theApp.Md5ofFile;
				ZipPlace += L"\\JarCache";
				if (!RemoveDirectory(ZipPlace) && !RemoveDirectory(Directory))
				{
					static_cast<CMainFrame*>(AfxGetMainWnd())->m_pTaskBar->SetProgressState(AfxGetMainWnd()->GetSafeHwnd(), TBPF_INDETERMINATE);
					FinishReading = Finished = false;
					if (Flag == CFernflowerUIMFCApp::DecompFlags::DecompJar)
					{
						szOutput = Log;
						szFile = Log;
					}
					else
					{
						CStringW ZipPath = buf;
						szOutput = buf;
						ZipPath += L"\\AppData\\Local\\FernFlowerUI\\Cache\\";
						ZipPath += theApp.Md5ofFile;
						ZipPath += L"\\MergedClass.jar";
						HZIP hz = CreateZip(ZipPath, nullptr);
						ZipAdd(hz, DisplayName, Log.c_str());
						CloseZip(hz);
						szOutput = ZipPath;
						szFile = ZipPath;
					}
					for (size_t i = szOutput.length(); szOutput[i] != '\\'; i--)
					{
						szOutput[i] = L'\0';
					}
					for (int i = m_wndOutputBuild.GetCount(); i >= 0; i--)
					{
						m_wndOutputBuild.DeleteString(i);
					}
					for (auto it = CommonWrapper::GetMainFrame()->m_MDIChildWndMap.begin(); it != CommonWrapper::GetMainFrame()->m_MDIChildWndMap.end(); it = CommonWrapper::GetMainFrame()->m_MDIChildWndMap.begin())
					{
						it->second->DestroyWindow();
					}
					if (CommonWrapper::GetMainFrame()->m_wndClassView.IsMDITabbed())
					{
						CommonWrapper::GetMainFrame()->m_wndClassView.ShowPane(false, false, false);
					}
					CStringW ss = buf;
					free(buf);
					ss += L"\\AppData\\Local\\FernFlowerUI\\Cache\\";
					ss += theApp.Md5ofFile;
					ss += L"\\DecomplieLog.txt";
					std::future<bool>scan = std::async(std::launch::async, &ScanLog, ss, AfxGetMainWnd(),false);
					auto scanforclasses = [&]() {FinishReading = false; CommonWrapper::GetMainFrame()->m_wndClassView.ShowClassView(ZipPlace); FinishReading = true; };
					std::thread ScanClasses(scanforclasses);
					ScanClasses.detach();
					CommonWrapper::CWaitDlg Wait(AfxGetMainWnd(), []() {return Finished && FinishReading; }, 10,
						IsInChinese() ? L"正在分析反编译输出,即将显示类视图..." : L"Searching the output,we will display the Class View soon.");
					mut.lock();
					Wait.DoModal();
					mut.unlock();
					CStringW Path = theApp.PathToSave;
					AfxGetApp()->WriteProfileStringW(L"ShellViewPath", L"PathName", Path);
					CommonWrapper::GetMainFrame()->m_wndClassView.AdjustLayout();
					CommonWrapper::GetMainFrame()->m_wndClassView.ShowPane(true, false, true);
					static_cast<CMainFrame*>(AfxGetMainWnd())->m_pTaskBar->SetProgressState(AfxGetMainWnd()->GetSafeHwnd(), TBPF_NOPROGRESS);
					if (theApp.EnableMDIReload)
					{
						if (_tdupenv_s(&buf, &size, _T("USERPROFILE")))
						{
							AfxGetMainWnd()->MessageBox(IsInChinese() ? _T("搜索%USERPROFILE%失败!") : _T("Failed to access %USERPROFILE%"), IsInChinese() ? _T("错误") : _T("Error"), MB_ICONERROR);
							ClearAllInfo();
							return false;
						}
						CStringW StatePath = buf;
						free(buf);
						StatePath += L"\\AppData\\Local\\FernFlowerUI\\Cache\\";
						StatePath += theApp.Md5ofFile;
						StatePath += L"\\MDIState.bin";
						CommonWrapper::GetMainFrame()->LoadMDIState(StatePath);
					}
					return true;
				}
				else
				{
					break;
				}
			}
			else
			{
				break;
			}
		}
		CreateDirectory(Directory, nullptr);
		free(buf);
		if (!CopyFile(OrigJar,FernflowerPath,FALSE))
		{
			AfxGetMainWnd()->MessageBox(IsInChinese()?_T("反编译进程启动失败!"):_T("Failed to launch the decompiling process"),IsInChinese()? _T("错误"):_T("Error"), MB_ICONERROR);
			ClearAllInfo();
			return false;
		}
	}
	for (int i = m_wndOutputBuild.GetCount(); i >= 0; i--)
	{
		m_wndOutputBuild.DeleteString(i);
	}
	for (auto it = CommonWrapper::GetMainFrame()->m_MDIChildWndMap.begin(); it != CommonWrapper::GetMainFrame()->m_MDIChildWndMap.end(); it = CommonWrapper::GetMainFrame()->m_MDIChildWndMap.begin())
	{
		it->second->DestroyWindow();
	}
	if (CommonWrapper::GetMainFrame()->m_wndClassView.IsMDITabbed())
	{
		CommonWrapper::GetMainFrame()->m_wndClassView.ShowPane(false, false, false);
	}
	if (Flag == CFernflowerUIMFCApp::DecompFlags::DecompJar)
	{
		szOutput = Log;
		szFile = Log;
	}
	else
	{
		TCHAR * buf;
		size_t size;
		if (_tdupenv_s(&buf,&size,_T("USERPROFILE")))
		{
			AfxGetMainWnd()->MessageBox(IsInChinese() ? _T("搜索%USERPROFILE%失败!") : _T("Failed to access %USERPROFILE%"), IsInChinese() ? _T("错误") : _T("Error"), MB_ICONERROR);
			ClearAllInfo();
			return false;
		}
		CStringW ZipPath = buf;
		szOutput = buf;
		free(buf);
		ZipPath += L"\\AppData\\Local\\FernFlowerUI\\Cache\\";
		ZipPath += theApp.Md5ofFile;
		ZipPath += L"\\MergedClass.jar";
		HZIP hz = CreateZip(ZipPath, nullptr);
		ZipAdd(hz, DisplayName, Log.c_str());
		CloseZip(hz);
		szOutput = ZipPath;
		szFile = ZipPath;
	}
	VERIFY(MtxDecomp = CreateMutex(nullptr, TRUE, CString(L"6168218c.FernflowerUI.Decomp") + theApp.Md5ofFile));
	for (size_t i = szOutput.length(); szOutput[i] != '\\'; i--)
	{
		//szOutput[i] = L'\0';
		if (szOutput[i - 1] == L'\\')
		{
			szOutput.erase(i);
		}
	}
	command = L"/c java -jar ";
	command += FernflowerPath;
	command += L"  ";
	command += theApp.m_DecompOption.GetOptionString();
//	command += L"-ind=\"      \"";
	CStringW ss;
	ss += szOutput.c_str();
	ss += L"decompile";
	int iTextLen = WideCharToMultiByte(CP_ACP,
		0,
		ss,
		-1,
		NULL,
		0,
		NULL,
		NULL);
	char * tmp = new char[iTextLen + 1];
	memset(tmp, 0, sizeof(char)*(iTextLen + 1));
	WideCharToMultiByte(CP_ACP, 0, ss, -1, tmp, iTextLen, nullptr, nullptr);
	if (_access(tmp, 0) == ENOENT)
	{
		IsDecomplieDirExisted = false;
	}
	else
	{
		IsDecomplieDirExisted = true;
	}
	_mkdir(tmp);
	delete[] tmp;
	if (_wdupenv_s(&buf, &size, L"USERPROFILE"))
	{
		AfxGetMainWnd()->MessageBox(IsInChinese() ? _T("搜索%USERPROFILE%失败!") : _T("Failed to access %USERPROFILE%"), IsInChinese() ? _T("错误") : _T("Error"), MB_ICONERROR);
		ClearAllInfo();
		CloseHandle(MtxDecomp);
		return false;
	}
	ss = buf;
	free(buf);
	ss += L"\\AppData\\Local\\FernFlowerUI\\Cache\\";
	ss += theApp.Md5ofFile;
	ss += L"\\DecomplieLog.txt";
	command += L"  ";
	command += szFile.c_str();
	command += L"  ";
	command += szOutput.c_str();
	command += L"decompile\\";
	command += L"  >";
	command += ss;
	if (_waccess(ss,0)!=-1)
	{
		if (!DeleteFile(ss))
		{
			AfxGetMainWnd()->MessageBox(IsInChinese()?L"删除旧输出失败,输出信息可能异常":
				L"Failed to delete the old log,the decompiling output may be wrong", L"警告", MB_ICONWARNING);
		}
	}
	HANDLE hOutput = CreateFileW(ss, GENERIC_READ, FILE_SHARE_WRITE | FILE_SHARE_READ, nullptr, CREATE_ALWAYS, FILE_FLAG_SEQUENTIAL_SCAN, nullptr);
	CloseHandle(hOutput);
	CStringW newfile;
	{
		CanUseProgress = false;
		//SHFILEINFOW FileInfo = { 0 };
		//SHGetFileInfoW(static_cast<CMainFrame*>(AfxGetMainWnd())->GetOutput()->szFile.c_str(),
		//	0, &FileInfo, sizeof(FileInfo), SHGFI_DISPLAYNAME);
		CFile NewJar;
		NewJar.Open(szFile.c_str(), CFile::modeNoTruncate | CFile::modeRead | CFile::shareDenyWrite | CFile::modeCreate);
		newfile = static_cast<CMainFrame*>(AfxGetMainWnd())->GetOutput()->szOutput.c_str();
		newfile += L"decompile\\";
		//newfile += FileInfo.szDisplayName;
		newfile += NewJar.GetFileName();
		NewJar.Close();
		HANDLE hfile_old = CreateFileW(static_cast<CMainFrame*>(AfxGetMainWnd())->GetOutput()->szFile.c_str(),
			GENERIC_READ, FILE_SHARE_WRITE | FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, nullptr);
		if (hfile_old == INVALID_HANDLE_VALUE)
		{
			MessageBox(IsInChinese() ? L"打开源文件失败,无法显示进度" : L"Failed to access the source file.\nUnable to display the decompiling progress", IsInChinese() ? L"错误" : L"Error", MB_ICONERROR);
			CanUseProgress = false;
		}
		else
		{
			CanUseProgress = true;
			ChosenFileSize = GetFileSize(hfile_old, nullptr);
		}
		CloseHandle(hfile_old);
		hfile_new = CreateFileW(newfile, GENERIC_READ, FILE_SHARE_WRITE | FILE_SHARE_READ, nullptr, CREATE_ALWAYS, FILE_FLAG_SEQUENTIAL_SCAN, nullptr);
		if (hfile_new == INVALID_HANDLE_VALUE)
		{
			MessageBox(IsInChinese() ? L"打开输出文件失败,无法显示进度" : L"Failed to access the output file.\nUnable to display the decompiling progress", IsInChinese() ? L"错误" : L"Error", MB_ICONERROR);
			CanUseProgress = false;
		}
		STARTUPINFOW startinfo = { 0 };
		startinfo.cb = sizeof(STARTUPINFOW);
		startinfo.dwFlags = STARTF_USESHOWWINDOW;
		CStringW ConsoleWindowTitle = CString(L"6168218c.FernflowerUI.Decomplie.") + theApp.Md5ofFile;
		startinfo.lpTitle = ConsoleWindowTitle.GetBuffer();
		startinfo.wShowWindow = SW_HIDE;
		const wchar_t * com = static_cast<CMainFrame*>(AfxGetMainWnd())->GetOutput()->command;
		bool b = CreateProcessW(L"C:\\Windows\\System32\\cmd.exe", const_cast<wchar_t*>(com), nullptr, nullptr, false,
			CREATE_DEFAULT_ERROR_MODE | CREATE_NEW_PROCESS_GROUP | CREATE_NEW_CONSOLE|IDLE_PRIORITY_CLASS,
			nullptr, nullptr, &startinfo, &pi);
		char * buf;
		size_t len;
		if (_dupenv_s(&buf,&len,"USERPROFILE"))
		{
			AfxGetMainWnd()->MessageBox(IsInChinese() ? _T("搜索%USERPROFILE%失败!") : _T("Failed to access %USERPROFILE%"), IsInChinese() ? _T("错误") : _T("Error"), MB_ICONERROR);
			ClearAllInfo();
			CloseHandle(MtxDecomp);
			return false;
		}
		while (!theApp.IsQuickDecomp)
		{
			if (ChosenFileSize<=1000000)
			{
				break;
			}
			static_cast<CMainFrame*>(AfxGetMainWnd())->m_wndStatusBar.SetPaneText(ID_SEPARATOR,IsInChinese()? L"正在设置进程相关性" : L"Setting the process affinity mask");
			CStringW str;
			str += "/c jps >";
			str += buf;
			str += "\\AppData\\Local\\FernFlowerUI\\Cache\\";
			str += theApp.Md5ofFile;
			str += L"\\ProcessIDCache.txt";
			STARTUPINFOW start = { 0 };
			start.cb = sizeof(STARTUPINFOW);
			start.dwFlags = STARTF_USESHOWWINDOW;
			start.wShowWindow = SW_HIDE;
			PROCESS_INFORMATION jps;
			bool bn = CreateProcessW(L"C:\\Windows\\System32\\cmd.exe", str.GetBuffer(), nullptr, nullptr, false,
				CREATE_DEFAULT_ERROR_MODE | CREATE_NEW_PROCESS_GROUP | CREATE_NEW_CONSOLE | HIGH_PRIORITY_CLASS,
				nullptr, nullptr, &start, &jps);
			if (!bn)
			{
				AfxGetMainWnd()->MessageBox(IsInChinese() ? L"设置进程相关性失败,CPU占用可能会过高!" : L"Failed to set the progress affinity mask,The CPU may be too high!", IsInChinese() ? L"警告" : L"Warning", MB_ICONWARNING);
				break;
			}
			Finished = false;
			CommonWrapper::CWaitDlg WaitForJPS(AfxGetMainWnd(), []() {return (bool)Finished; }, 10,IsInChinese()? L"正在设置进程相关性...":L"Setting the process affinity mask...");
			std::thread JPSProcess = std::thread([](const PROCESS_INFORMATION & pi)
			{
				DWORD exit = STILL_ACTIVE;
				while (exit == STILL_ACTIVE)
				{
					GetExitCodeProcess(pi.hProcess, &exit);
					Sleep(50);
				}
				Finished = true;
			},jps );
			JPSProcess.detach();
			WaitForJPS.DoModal();
			Finished = false;
			std::string ProcessFile = buf;
			ProcessFile += "\\AppData\\Local\\FernFlowerUI\\Cache\\";
			CStringA Str;
			Str = theApp.Md5ofFile;
			ProcessFile += Str;
			ProcessFile += "\\ProcessIDCache.txt";
			fin.open(ProcessFile);
			DWORD JavaProcessID;
			try
			{
				std::string tmp;
				std::istringstream sin;
				while ((std::getline(fin, tmp)) && (tmp.find(FernflowerName) == std::string::npos))
				{
					continue;
				}
				sin.str(tmp);
				sin >> JavaProcessID;
			}
			catch (const std::exception & ex)
			{
				MessageBoxA(AfxGetMainWnd()->m_hWnd, ex.what(), IsInChinese()?"发生异常":"An exception occoured.", MB_ICONERROR);
			}
			HANDLE JavaProcess = OpenProcess(PROCESS_ALL_ACCESS, false, JavaProcessID);
			if (JavaProcess == INVALID_HANDLE_VALUE || !SetProcessAffinityMask(JavaProcess, 0x1))
			{
				AfxGetMainWnd()->MessageBox(IsInChinese() ? L"设置进程相关性失败,CPU占用可能会过高!" : L"Failed to set the progress affinity mask,The CPU may be too high!", IsInChinese() ? L"警告" : L"Warning", MB_ICONWARNING);
			}
			CloseHandle(JavaProcess);
			CloseHandle(jps.hProcess);
			CloseHandle(jps.hThread);
			break;
		}
		free(buf);
		fin.close();
		if (!b)
		{
			AfxGetMainWnd()->MessageBox(IsInChinese() ? L"创建线程失败!" : L"Failed to create the worker thread!", IsInChinese() ? L"错误" : L"Error", MB_ICONERROR);
			CloseHandle(hfile_new);
			ClearAllInfo();
			CloseHandle(MtxDecomp);
			return false;
		}
		else
		{
			AfxGetMainWnd()->SetWindowText(IsInChinese()?L"FernFlowerUI - 正在反编译，请耐心等待":L"FernFlowerUI - Decompiling...Please wait");
			CProgressDlg ProgressDlg(AfxGetMainWnd());
			static_cast<CMainFrame*>(AfxGetMainWnd())->m_wndStatusBar.SetPaneText(ID_SEPARATOR,IsInChinese()? L"正在反编译":L"Decompling");
			mut.lock();
			std::future<bool>scan = std::async(std::launch::async,&ScanLog,ss ,AfxGetMainWnd(),true);
			ProgressDlg.DoModal();
			CloseHandle(hfile_new);
			mut.lock();
			if (ExitCode == 0)
			{
				FinishReading = false;
				std::thread UnZiper(UnZipJar, AfxGetMainWnd(), newfile);
				UnZiper.detach();
			}
			else
			{
				ClearAllInfo();
				FinishReading = true;
			}
			ExitCode = 0;
			CommonWrapper::CWaitDlg WaitDlg(AfxGetMainWnd(), []() {return Finished && FinishReading; }, 10, 
				IsInChinese() ? L"正在分析反编译输出,即将显示类视图..." : L"Searching the output,we will display the Class View soon.");
			WaitDlg.DoModal();
			if (!scan.get())
			{
				if (AfxGetMainWnd()->MessageBox(IsInChinese() ? L"打开日志失败,是否使用电脑默认文本编辑器打开?" : L"Failed to read the decompile log.Would you like to open it through your default text editor?", IsInChinese() ? L"错误" : L"Error", MB_ICONERROR) == IDOK)
				{
					ShellExecute(AfxGetMainWnd()->m_hWnd, L"open", ss, nullptr, nullptr, SW_SHOW);
				}
			}
			mut.unlock();
		}
		AfxGetMainWnd()->SetWindowText(L"FernFlowerUI");
		//HANDLE hLogtxt = CreateFile(ss, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, nullptr);
	}
	Finished = false;
	FinishReading = false;
	static_cast<CMainFrame*>(AfxGetMainWnd())->m_pTaskBar->SetProgressState(AfxGetMainWnd()->GetSafeHwnd(), TBPF_NOPROGRESS);
	if (theApp.EnableAutoSave && !bAbortDecomplie)
	{
		if (Flag == CFernflowerUIMFCApp::DecompFlags::DecompClass)
		{
			szOutput = Log;
			for (size_t i = szOutput.length(); szOutput[i] != '\\'; i--)
			{
				szOutput[i] = L'\0';
			}
			CStringW Display = DisplayName;
			Display.Delete(DisplayName.GetLength() - 6, 6);
			Display += L".java";
			CStringW Output = szOutput.c_str();
			Output += L"decompile\\";
			Output += Display;
			TCHAR * buf;
			size_t size;
			if (_tdupenv_s(&buf, &size, _T("USERPROFILE")))
			{
				AfxGetMainWnd()->MessageBox(IsInChinese() ? _T("搜索%USERPROFILE%失败!") : _T("Failed to access %USERPROFILE%"), IsInChinese() ? _T("错误") : _T("Error"), MB_ICONERROR);
				ClearAllInfo();
				CloseHandle(MtxDecomp);
				return false;
			}
			CStringW ClassPath = buf;
			free(buf);
			ClassPath += L"\\AppData\\Local\\FernFlowerUI\\Cache\\";
			ClassPath += theApp.Md5ofFile;
			ClassPath += L"\\JarCache";
			ClassPath += Display;
			CopyFile(ClassPath, Output, FALSE);
		}
		else if (theApp.bMultiSelection)
		{
			HZIP hz = OpenZip(newfile, nullptr);
			ZIPENTRY ze;
			GetZipItem(hz, -1, &ze);
			int numitems = ze.index;
			CString Base = szOutput.c_str();
			SetUnzipBaseDir(hz, Base+L"decompile");
			for (int zi = 0; zi < numitems; zi++)
			{
				GetZipItem(hz, zi, &ze);
				DWORD ret = UnzipItem(hz, zi, ze.name);
			}
			CloseZip(hz);
			DeleteFile(newfile);
		}
	}
	else
	{
		if (Flag == CFernflowerUIMFCApp::DecompFlags::DecompJar)
		{
			DeleteFile(newfile);
			CStringW Output = szOutput.c_str();
			Output += L"decompile";
			if (!IsDecomplieDirExisted)
			{
				RemoveDirectory(Output);
			}
		}
	}
	CStringW Path = theApp.PathToSave;
	AfxGetApp()->WriteProfileStringW(L"ShellViewPath", L"PathName", Path);
	DeleteFile(FernflowerPath);
	if (!bAbortDecomplie)
	{
		if (_tdupenv_s(&buf, &size, _T("USERPROFILE")))
		{
			AfxGetMainWnd()->MessageBox(IsInChinese() ? _T("搜索%USERPROFILE%失败!") : _T("Failed to access %USERPROFILE%"), IsInChinese() ? _T("错误") : _T("Error"), MB_ICONERROR);
			ClearAllInfo();
			CloseHandle(MtxDecomp);
			return false;
		}
		CStringW OptionPath = buf;
		free(buf);
		OptionPath += L"\\AppData\\Local\\FernFlowerUI\\Cache\\";
		OptionPath += theApp.Md5ofFile;
		OptionPath += L"\\OptionData.bin";
		CFile File;
		if (File.Open(OptionPath, CFile::modeWrite | CFile::modeCreate | CFile::shareDenyWrite))
		{
			CArchive ar(&File, CArchive::store);
			ar << theApp.m_DecompOption;
			ar.Close();
			File.Close();
		}
		if (theApp.EnableMDIReload)
		{
			if (_tdupenv_s(&buf, &size, _T("USERPROFILE")))
			{
				AfxGetMainWnd()->MessageBox(IsInChinese() ? _T("搜索%USERPROFILE%失败!") : _T("Failed to access %USERPROFILE%"), IsInChinese() ? _T("错误") : _T("Error"), MB_ICONERROR);
				ClearAllInfo();
				CloseHandle(MtxDecomp);
				return false;
			}
			CStringW StatePath = buf;
			free(buf);
			StatePath += L"\\AppData\\Local\\FernFlowerUI\\Cache\\";
			StatePath += theApp.Md5ofFile;
			StatePath += L"\\MDIState.bin";
			CommonWrapper::GetMainFrame()->LoadMDIState(StatePath);
		}
		CommonWrapper::GetMainFrame()->m_wndClassView.AdjustLayout();
		CommonWrapper::GetMainFrame()->m_wndClassView.ShowPane(true, false, true);
		ReleaseMutex(MtxDecomp);
	}
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	CloseHandle(MtxDecomp);
	if (bAbortDecomplie)
	{
		return -1;
	}
	else
	{
		return true;
	}
}

void COutputWnd::ClearAllInfo()
{
	size_t size;
	TCHAR* buf;
	if (theApp.EnableMDIReload && (theApp.Md5ofFile != L""))
	{
		if (_tdupenv_s(&buf, &size, _T("USERPROFILE")))
		{
			AfxGetMainWnd()->MessageBox(IsInChinese() ? _T("搜索%USERPROFILE%失败!") : _T("Failed to access %USERPROFILE%"), IsInChinese() ? _T("错误") : _T("Error"), MB_ICONERROR);
			return;
		}
		CStringW StatePath = buf;
		free(buf);
		StatePath += L"\\AppData\\Local\\FernFlowerUI\\Cache\\";
		StatePath += theApp.Md5ofFile;
		StatePath += L"\\MDIState.bin";
		CommonWrapper::GetMainFrame()->SaveMDIState(StatePath);
	}
	theApp.Md5ofFile = L"";
	for (int i = m_wndOutputBuild.GetCount(); i >= 0; i--)
	{
		m_wndOutputBuild.DeleteString(i);
	}
	for (auto it = CommonWrapper::GetMainFrame()->m_MDIChildWndMap.begin(); it != CommonWrapper::GetMainFrame()->m_MDIChildWndMap.end(); it = CommonWrapper::GetMainFrame()->m_MDIChildWndMap.begin())
	{
		it->second->DestroyWindow();
	}
	CommonWrapper::GetMainFrame()->m_wndClassView.ClearAll();
	if (CommonWrapper::GetMainFrame()->m_wndFileView.IsMDITabbed())
		CommonWrapper::GetMainFrame()->m_wndFileView.ShowPane(FALSE, FALSE, FALSE);
	CommonWrapper::GetMainFrame()->m_wndFileView.ShowPane(TRUE, FALSE, TRUE);
	CommonWrapper::GetMainFrame()->OnOpenstartPage();
}

bool ScanLog(const CStringW& filename, CWnd* MainWnd, bool NeedInit)
{
	Finished = false;
	if (NeedInit)
	{
		mut.lock();
		mut.unlock();
	}

	std::string contact;
	COutputWnd & out = *(dynamic_cast<CMainFrame*>(MainWnd)->GetOutput());
	CStringW addstr;
	std::ifstream fin;
	fin.open(filename, std::ios_base::in);
	if (!fin)
	{
		out.FillBuildWindow();
		Finished = true;
		return false;
	}
	else
	{
		try
		{
			while (mut.try_lock()&&(!bAbortDecomplie))
			{
				mut.unlock();
				fin.clear();
				if (std::getline(fin, contact))
				{
					addstr = contact.c_str();
					out.m_wndOutputBuild.AddString(addstr);
					if (AtlIsValidAddress(pStaticText, sizeof CStatic))
					{
						HWND hText = pStaticText->GetSafeHwnd();
						int iIndex = addstr.Find(L"ing", 0);
						if (IsWindow(hText) && (iIndex != -1))
						{
							if (IsInChinese())
							{
								addstr.Replace(L"Processing", L"正在反编译");
								addstr.Replace(L"Decompiling", L"正在反编译");
								addstr.Replace(L"INFO", L"信息");
								addstr.Replace(L"TRACE", L"跟踪信息");
								addstr.Replace(L"method", L"成员函数");
								addstr.Replace(L"class", L"类");
							}
							SetWindowText(hText, addstr);
						}
					}
				}
				Sleep(2);
			}
			//This block will delete the old cache that is useless
			if (bAbortDecomplie)
			{
				fin.close();
				mut.unlock();
				while (!DeleteFile(filename))
				{
					continue;
				}
				char * UserProFile;
				size_t size;
				if (_dupenv_s(&UserProFile, &size, "USERPROFILE"))
				{
					AfxGetMainWnd()->MessageBox(IsInChinese() ? _T("搜索%USERPROFILE%失败!") : _T("Failed to access %USERPROFILE%"), IsInChinese() ? _T("错误") : _T("Error"), MB_ICONERROR);
					return false;
				}
				CStringW CachePath;
				CachePath = UserProFile;
				CachePath += L"\\AppData\\Local\\FernFlowerUI\\Cache\\";
				CachePath += theApp.Md5ofFile;
				SHFILEOPSTRUCT CacheToRemove = { 0 };
				CacheToRemove.hwnd = CommonWrapper::GetMainFrame()->GetSafeHwnd();
				CacheToRemove.wFunc = FO_DELETE;
				CacheToRemove.lpszProgressTitle = IsInChinese() ? L"正在删除文件" : L"Deleting the out-dated cache";
				CStringW Tmp = CachePath + L"\\";
				Tmp += L'\0';
				CacheToRemove.pFrom = Tmp;
				CacheToRemove.pTo = nullptr;
				CacheToRemove.fFlags = FOF_NOCONFIRMATION | FOF_SILENT;
				DWORD result;
				if (result = SHFileOperation(&CacheToRemove))
				{
					CommonWrapper::GetMainFrame()->MessageBox(IsInChinese() ? L"删除文件失败" : L"Failed to delete the out-dated cache", IsInChinese() ? L"错误" : L"Error", MB_ICONERROR);
				}
				Finished = true;
				return true;
			}
			while (std::getline(fin, contact))
			{
				addstr = contact.c_str();
				out.m_wndOutputBuild.AddString(addstr);
			}
		}
		catch (const std::exception& ex)
		{
			std::string err = IsInChinese()?"打开日志时发生异常:":"An exception occoured when reading the decompile log:";
			err += ex.what();
			if (MessageBoxA(MainWnd->m_hWnd, err.c_str(), IsInChinese()?"错误":"Error", MB_ICONERROR) == IDOK)
			{
			}
			out.FillBuildWindow();
			Finished = true;
			return false;
		}
		fin.close();
	}
	Finished = true;
	return true;
}

void UnZipJar(CWnd * MainWnd, const CStringW & JarPath)
{
	static_cast<CMainFrame*>(MainWnd)->m_wndClassView.ClearAll();
	char * buf;
	size_t len;
	if (_dupenv_s(&buf, &len, "USERPROFILE"))
	{
		MainWnd->MessageBox(IsInChinese() ? _T("搜索%USERPROFILE%失败!") : _T("Failed to access %USERPROFILE%"), IsInChinese() ? _T("错误") : _T("Error"), MB_ICONERROR);
		return;
	}
	CStringW ZipCache;
	CStringA Zipwhere;
	Zipwhere += buf;
	Zipwhere += "\\AppData\\Local\\FernFlowerUI\\Cache\\";
	Zipwhere += theApp.Md5ofFile;
	Zipwhere += L"\\JarCache";
	CStringW ZipPlace;
	ZipPlace += Zipwhere;
	if (_access(Zipwhere, 0) == -1)
	{
		_mkdir(Zipwhere);
	}
	{
		SHFILEOPSTRUCT CacheToRemove = { 0 };
		CacheToRemove.hwnd = CommonWrapper::GetMainFrame()->GetSafeHwnd();
		CacheToRemove.wFunc = FO_DELETE;
		CacheToRemove.lpszProgressTitle = IsInChinese()?L"正在删除缓存文件":L"Deleting the out-dated cache";
		CStringW Tmp = ZipPlace + L"\\";
		Tmp += L'\0';
		CacheToRemove.pFrom = Tmp;
		CacheToRemove.pTo = nullptr;
		CacheToRemove.fFlags = FOF_NOCONFIRMATION | FOF_SILENT;
		DWORD result;
		if (result = SHFileOperation(&CacheToRemove))
		{
			MainWnd->MessageBox(IsInChinese() ? L"删除缓存文件失败" : L"Failed to delete the out-dated cache", IsInChinese() ? L"错误" : L"Error", MB_ICONERROR);
		}
	}
	_mkdir(Zipwhere);
	ZipCache += buf;
	free(buf);
	ZipCache += L"\\AppData\\Local\\FernFlowerUI\\Cache\\";
	ZipCache += theApp.Md5ofFile;
	ZipCache += L"\\JarCache.jar";
	if (!CopyFile(JarPath, ZipCache, false))
	{
		MainWnd->MessageBox(IsInChinese()?L"解压jar文件失败":L"Failed to unzip the output file",IsInChinese()? L"错误":L"Error", MB_ICONERROR);
		return;
	}
	HZIP hz = OpenZip(ZipCache, nullptr);
	ZIPENTRY ze;
	GetZipItem(hz, -1, &ze);
	int numitems = ze.index;
	//std::ofstream fout;
	SetUnzipBaseDir(hz, ZipPlace);
	for (int zi = 0; zi < numitems; zi++)
	{
		GetZipItem(hz, zi, &ze);
		DWORD ret = UnzipItem(hz, zi, ze.name);
	}
	CloseZip(hz);
	static_cast<CMainFrame*>(MainWnd)->m_wndClassView.ShowClassView(ZipPlace);
	FinishReading = true;
}


void COutputWnd::UpdateFonts()
{
	m_wndOutputBuild.SetFont(&afxGlobalData.fontRegular);
}

/////////////////////////////////////////////////////////////////////////////
// COutputList1

COutputList::COutputList()
{
}

COutputList::~COutputList()
{
}

BEGIN_MESSAGE_MAP(COutputList, CListBox)
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_EDIT_CLEAR, OnEditClear)
	ON_COMMAND(ID_VIEW_OUTPUTWND, OnViewOutput)
	ON_WM_WINDOWPOSCHANGING()
	ON_WM_CREATE()
	ON_WM_DESTROY()
END_MESSAGE_MAP()
/////////////////////////////////////////////////////////////////////////////
// COutputList 消息处理程序

void COutputList::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	CMenu menu;
	menu.LoadMenu(IDR_OUTPUT_POPUP);

	CMenu* pSumMenu = menu.GetSubMenu(0);

	if (AfxGetMainWnd()->IsKindOf(RUNTIME_CLASS(CMDIFrameWndEx)))
	{
		CMFCPopupMenu* pPopupMenu = new CMFCPopupMenu;

		if (!pPopupMenu->Create(this, point.x, point.y, (HMENU)pSumMenu->m_hMenu, FALSE, TRUE))
			return;

		((CMDIFrameWndEx*)AfxGetMainWnd())->OnShowPopupMenu(pPopupMenu);
		UpdateDialogControls(this, FALSE);
	}

	SetFocus();
}

void COutputList::OnEditCopy()
{
	int nSel = GetCurSel();
	if (LB_ERR == nSel)
	{
		return;
	}
	CStringW Selected;
	GetText(nSel, Selected);
	Clipboarder.SetWindowTextW(Selected);
	Clipboarder.SetSel(0, -1, false);
	Clipboarder.Copy();
}

void COutputList::OnEditClear()
{
	int nSel = GetCurSel();
	if (LB_ERR == nSel)
	{
		return;
	}
	DeleteString(nSel);
}

void COutputList::OnViewOutput()
{
	CDockablePane* pParentBar = DYNAMIC_DOWNCAST(CDockablePane, GetOwner());
	CFrameWndEx* pMainFrame = DYNAMIC_DOWNCAST(CFrameWndEx, GetTopLevelFrame());

	if (pMainFrame != NULL && pParentBar != NULL)
	{
		pMainFrame->SetFocus();
		pMainFrame->ShowPane(pParentBar, FALSE, FALSE, FALSE);
		pMainFrame->RecalcLayout();

	}
}



int COutputList::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CListBox::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO:  在此添加您专用的创建代码
	Clipboarder.Create(WS_CHILD, CRect(0, 0, 0, 0), this, ID_CLIPBOARDER);
	Clipboarder.ShowWindow(SW_HIDE);
	return 0;
}

void COutputList::OnDestroy()
{
	CListBox::OnDestroy();
	Clipboarder.DestroyWindow();

	// TODO: 在此处添加消息处理程序代码
}


void CProgressDlg::OnBnClickedCancel()
{
	// TODO: 在此添加控件通知处理程序代码
	bAbortDecomplie = true;
	KillTimer(ID_TIMER_DOPROGRESS);
	CDialogEx::OnCancel();
	HWND hWndDecomplie = ::FindWindow(L"ConsoleWindowClass", CString(L"6168218c.FernflowerUI.Decomplie.") + theApp.Md5ofFile);
	::SendMessage(hWndDecomplie, WM_CLOSE, 0, 0);
	ExitCode = 1;//Just pretend that there is something wrong with Java
	CommonWrapper::CWaitDlg WaitDlg(AfxGetMainWnd(), []() {return (bool)Finished; }, 10,
		IsInChinese() ? L"正在取消..." : L"Cancelling...");
	WaitDlg.DoModal();
	//CloseHandle(MtxDecomp);
	CommonWrapper::GetMainFrame()->m_wndOutput.ClearAllInfo();
}
