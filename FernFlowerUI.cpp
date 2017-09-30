
#include "stdafx.h"
#include "FernFlowerUI.h"

#define MAX_LOADSTRING 100

HINSTANCE hInst;
WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];
std::ofstream fout;
DWORD ChosenFileSize;
HWND hedit;
HWND hProgressBar;
bool CanUseProgress;

ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
bool				AccessJar(HWND hwnd);
bool				DownloadJar();
void				Decomplie(HWND hWnd, const char * szFile);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);


	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_FERNFLOWERUI, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_FERNFLOWERUI));

	MSG msg;

	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;
}



//
//  函数: MyRegisterClass()
//
//  目的: 注册窗口类。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_FERNFLOWERUI));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_FERNFLOWERUI);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}


BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance;

	HWND hWnd = CreateWindowExW(WS_EX_ACCEPTFILES,szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		case IDM_OPEN:
		{
			if (!AccessJar(hWnd))
				break;
			char szFilePath[MAX_PATH] = { 0 };
			OPENFILENAMEA ofn = { 0 };
			ofn.hwndOwner = hWnd;
			ofn.lStructSize = sizeof(ofn);
			ofn.Flags = OFN_EXPLORER | OFN_FORCESHOWHIDDEN;
			ofn.lpstrFilter = "Jar文件(*.jar)\0*.jar\0\0";
			ofn.lpstrInitialDir = "%USERPROFILE%";
			ofn.lpstrFile = szFilePath;
			ofn.nMaxFile = MAX_PATH;
			if (GetOpenFileNameA(&ofn))
			{
				Decomplie(hWnd, szFilePath);
			}
		}
		break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_CREATE:
		{
			InitCommonControls();
			RECT rect;
			GetClientRect(hWnd, &rect);
			hedit = CreateWindow(TEXT("edit"), TEXT(""), WS_CHILD | WS_VISIBLE|ES_READONLY,
			0, 0, rect.right - rect.left, rect.bottom - rect.top, hWnd, (HMENU)IDW_TEXT, nullptr, nullptr);
			SendDlgItemMessage(hWnd, IDW_TEXT, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));
			SetWindowText(hedit, L"请在%USERPROFILE%\\AppData\\Local\\FernFlowerUI中放置fernflower.jar后使用本程序");
			hProgressBar = CreateWindowA(PROGRESS_CLASSA, "", WS_CHILD|PBS_SMOOTH|PBS_SMOOTHREVERSE,
				CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
				hWnd, (HMENU)0, LPCREATESTRUCTA(lParam)->hInstance, nullptr
			);
			ShowWindow(hProgressBar, SW_HIDE);
			MoveWindow(hProgressBar, rect.left, (rect.bottom - rect.top) / 4, rect.right - rect.left, (rect.bottom - rect.top) / 10, true);
		}
		break;
	case WM_SIZE: 
		{
			int height = HIWORD(lParam);
			int width = LOWORD(lParam);
			RECT rect;
			GetClientRect(hWnd, &rect);
			MoveWindow(hedit, rect.left, rect.top, width, height, TRUE);
			MoveWindow(hProgressBar, rect.left, (rect.bottom - rect.top) / 4, rect.right - rect.left, (rect.bottom - rect.top) / 10, true);
		}
		break;
	case WM_DROPFILES:
		{
			if (!AccessJar(hWnd))
			{
				break;
			}
			HDROP hdrop = (HDROP)wParam;
			int DropFileNums = 0;
			DropFileNums = DragQueryFileW(hdrop, 0xFFFFFFFF, nullptr, 0);
			if (DropFileNums>1)
			{
				MessageBoxA(hWnd, "不支持拖拽打开多个文件\n已自动取消操作", "错误", MB_ICONERROR);
				break;
			}
			char szFilePath[MAX_PATH];
			DragQueryFileA(hdrop, 0, szFilePath, sizeof(szFilePath));
			Decomplie(hWnd, szFilePath);
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}


bool AccessJar(HWND hwnd)
{
	char * userprofile;
	size_t len;
	if (_dupenv_s(&userprofile,&len,"USERPROFILE"))
	{
		return false;
	}
	std::string saccess = userprofile;
	saccess += "\\AppData\\Local\\FernFlowerUI";
	if (_access(saccess.c_str(),0)==-1)
	{
		_mkdir(saccess.c_str());
		if (MessageBoxA(hwnd, "检测到是第一次使用，是否要下载fernflower.jar?", "", MB_OKCANCEL) == IDOK)
			DownloadJar();
		else
		{
			return false;
		}
	}
	else if (_access((saccess+"\\fernflower.jar").c_str(),0)==-1)
	{
		if (MessageBoxA(hwnd, "找不到%USERPROFILE%\\AppData\\Local\\FernFlowerUI\\fernflower.jar,是否要下载?", "", MB_OKCANCEL) == IDOK)
			DownloadJar();
		else
		{
			return false;
		}
	}
	return true;
}

bool DownloadJar()
{
	char Buffer[MAXBLOCKSIZE];
	unsigned long Number = 1;
	FILE* stream;
	HINTERNET hSession = InternetOpenA("FernFlowerUI2.0", INTERNET_OPEN_TYPE_PRECONFIG, nullptr, nullptr, 0);
	if (hSession)
	{
		HINTERNET handle = InternetOpenUrlA(hSession, "https://raw.githubusercontent.com/6168218c/Fernflower---forge/master/fernflower.jar",
			nullptr, 0, INTERNET_FLAG_DONT_CACHE, 0);
		if (handle)
		{
			char * filestr;
			size_t length;
			errno_t err = _dupenv_s(&filestr, &length, "USERPROFILE");
			if (err)
			{
				MessageBoxA(nullptr, "无法打开%USERPROFILE%", "错误", MB_ICONERROR);
				return false;
			}
			std::string filename = filestr;
			filename += "\\AppData\\Local\\FernFlowerUI\\fernflower.jar";
			if (!fopen_s(&stream,filename.c_str(), "wb"))
			{
				  while (Number > 0)
			      {
			        InternetReadFile(handle,Buffer, MAXBLOCKSIZE - 1, &Number);
				    fwrite(Buffer, sizeof(char), Number, stream);
				  }
			    fclose(stream);
		    }
			InternetCloseHandle(handle);
			handle = nullptr;
		}
		InternetCloseHandle(hSession);
		hSession = nullptr;
	}
	return true;
}

void Decomplie(HWND hWnd ,const char * szFile)
{
	char szOutput[MAX_PATH];
	strcpy_s(szOutput, szFile);
	for (int i = MAX_PATH - 1; szOutput[i] != '\\'; i--)
		szOutput[i] = '\0';
	std::string str = "/c java -jar %USERPROFILE%\\AppData\\Local\\FernFlowerUI\\fernflower.jar";
	std::string s = szOutput;
	s += "decomplie";
	_mkdir(s.c_str());
	s += "\\DecomplieLog.txt";
	fout.open(s.c_str());
	fout.close();
	str += "  ";
	str += szFile;
	str += "  ";
	str += szOutput;
	str += "decomplie\\";
	str += "  >>";
	str += s.c_str();
	{
		SHFILEINFOA FileInfo = { 0 };
		SHGetFileInfoA(szFile, 0, &FileInfo, sizeof(FileInfo), SHGFI_DISPLAYNAME);
		std::string newfile = szOutput;
		newfile += "decomplie\\";
		newfile += FileInfo.szDisplayName;
		HANDLE hfile_old = CreateFileA(szFile, GENERIC_READ, FILE_SHARE_WRITE | FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, nullptr);
		if (hfile_old==INVALID_HANDLE_VALUE)
		{
			MessageBoxA(hWnd, "出现未知错误，无法显示进度条", "错误", MB_ICONERROR);
			CanUseProgress = false;
		}
		else
		{
			CanUseProgress = true;
			ChosenFileSize = GetFileSize(hfile_old, nullptr);
		}
		CloseHandle(hfile_old);
		HANDLE hfile_new = CreateFileA(newfile.c_str(), GENERIC_READ, FILE_SHARE_WRITE | FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, nullptr);
		if (hfile_new == INVALID_HANDLE_VALUE)
		{
			MessageBoxA(hWnd, "出现未知错误，无法显示进度条", "错误", MB_ICONERROR);
			CanUseProgress = false;
		}
		STARTUPINFOA startinfo = { 0 };
		startinfo.cb = sizeof(STARTUPINFOA);
		startinfo.dwFlags = STARTF_USESHOWWINDOW;
		startinfo.lpTitle = "Decomplie Thread";
		startinfo.wShowWindow = SW_HIDE;
		PROCESS_INFORMATION pi;
		bool b = CreateProcessA("C:\\Windows\\System32\\cmd.exe", const_cast<char*>(str.c_str()), nullptr, nullptr, false,
			CREATE_DEFAULT_ERROR_MODE | CREATE_NEW_PROCESS_GROUP | CREATE_NEW_CONSOLE,
			nullptr, nullptr, &startinfo, &pi);
		if (!b)
		{
			MessageBoxA(hWnd, "创建线程失败!", "错误", MB_ICONERROR);
			return;
		}
		else
		{
			SetWindowTextA(hWnd, "FernFlowerUI -正在反编译，请耐心等待");
			if (CanUseProgress)
			{
				ShowWindow(hProgressBar, SW_SHOW);
			}
			PBRANGE range;
			SendMessage(hProgressBar, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
			SendMessage(hProgressBar, PBM_GETRANGE, true, (LPARAM)&range);
			DWORD state,newfilesize;
			do
			{
				if (CanUseProgress)
				{
					newfilesize = GetFileSize(hfile_new, nullptr);
					SendMessage(hProgressBar, PBM_DELTAPOS, (int)(range.iHigh - range.iLow)*(double(newfilesize) / double(ChosenFileSize)), 0);
				}
				GetExitCodeProcess(pi.hProcess, &state);
				Sleep(1000);
			} while (state == STILL_ACTIVE);
			ShowWindow(hProgressBar, SW_HIDE);
			CloseHandle(hfile_old);
		}
	}
	SetWindowTextA(hWnd, "FernFlowerUI");
	std::string ss = "notepad.exe  ";
	ss += s;
	if (IDOK == MessageBoxA(hWnd, "反编译线程执行完毕,未知是否成功,确定要打开日志?", "反编译完成", MB_OKCANCEL | MB_ICONINFORMATION))
	{
		WinExec(ss.c_str(), SW_SHOW);
	}
}
