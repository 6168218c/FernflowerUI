
#include "stdafx.h"
#include "ViewTree.h"
#include "CommonWrapper.h"
#include "FernflowerUI_MFCView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CViewTree

CViewTree::CViewTree()
{
}

CViewTree::~CViewTree()
{
}

BEGIN_MESSAGE_MAP(CViewTree, CTreeCtrl)
	ON_NOTIFY_REFLECT(NM_CLICK, &CViewTree::OnNMClick)
	ON_NOTIFY_REFLECT(NM_DBLCLK, &CViewTree::OnNMDblclk)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CViewTree 消息处理程序

BOOL CViewTree::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	BOOL bRes = CTreeCtrl::OnNotify(wParam, lParam, pResult);

	NMHDR* pNMHDR = (NMHDR*)lParam;
	ASSERT(pNMHDR != NULL);

	if (pNMHDR && pNMHDR->code == TTN_SHOW && GetToolTips() != NULL)
	{
		GetToolTips()->SetWindowPos(&wndTop, -1, -1, -1, -1, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOSIZE);
	}

	return bRes;
}


void CViewTree::OnNMClick(NMHDR *pNMHDR, LRESULT *pResult)
{
	CPoint Point;
	UINT uFlags;
	GetCursorPos(&Point);
	ScreenToClient(&Point);
	HTREEITEM hItem = HitTest(Point, &uFlags);
	if ((!hItem) || (!(uFlags&TVHT_ONITEM)))
	{
		pResult = 0;
		return;
	}
	int ImgIndex, SelectedImgIndex;
	if (this->GetItemImage(hItem, ImgIndex, SelectedImgIndex))
	{
		if (ImgIndex == 0)
		{
			AfxGetMainWnd()->BeginWaitCursor();
			std::stack<CStringW> StringStack;
			CStringW JavaPath;
			CStringW FileName = GetItemText(hItem);
			HTREEITEM hClassItem = hItem;
			FileName += L".java";
			char * UserProFile;
			size_t size;
			if (_dupenv_s(&UserProFile, &size, "USERPROFILE"))
			{
				AfxGetMainWnd()->MessageBox(IsInChinese() ? _T("搜索%USERPROFILE%失败!") : _T("Failed to access %USERPROFILE%"), IsInChinese() ? _T("错误") : _T("Error"), MB_ICONERROR);
				return;
			}
			JavaPath = UserProFile;
			JavaPath += L"\\AppData\\Local\\FernFlowerUI\\Cache\\";
			JavaPath += theApp.Md5ofFile;
			JavaPath += L"\\JarCache\\";
			free(UserProFile);
			StringStack.push(GetItemText(hItem) + L".java");
			while (hItem = GetParentItem(hItem))
			{
				StringStack.push(GetItemText(hItem) + L"\\");
			}
			while (!StringStack.empty())
			{
				JavaPath += StringStack.top();
				StringStack.pop();
			}
			CFile File;
			File.Open(JavaPath, CStdioFile::modeNoTruncate | CStdioFile::modeRead | CStdioFile::shareDenyNone);
			size_t Len = File.GetLength();
			char * buf = new char[Len + 1];
			memset(buf, 0, Len + 1);
			File.Read(buf, Len);
			for (size_t i = 0; i < Len; i++)
			{
				if (buf[i] == '\0')
				{
					buf[i] = ' ';
				}
			}
			buf[Len] = '\0';
			int nConLen = MultiByteToWideChar(CP_UTF8, 0, buf, -1, nullptr, 0);
			wchar_t * wbuf = new wchar_t[nConLen + 1]{ 0 };
			MultiByteToWideChar(CP_UTF8, 0, buf, Len, wbuf, nConLen);
			delete buf;
			CStringW Contact = wbuf;
			delete wbuf;
			/*CStringW Contact, buf;
			while (File.ReadString(buf))
			{

			Contact += (buf + L"\r\n");
			}*/
			//Contact.Replace(L"   ", L"      ");
			if (CommonWrapper::GetMainFrame()->m_MDIChildWndMap.count(JavaPath) == 1)
			{
				CommonWrapper::GetMainFrame()->m_MDIChildWndMap[JavaPath]->MDIActivate();
				return;
			}
			std::pair<HTREEITEM, CString> Pair(hClassItem, JavaPath);
			CMDIChildWndEx * MDIChild = CommonWrapper::GetMainFrame()->CreateDocumentWindow(File.GetFileName(), (CObject*)&Pair);
			VERIFY(MDIChild != nullptr);
			{
				//std::lock_guard<std::mutex> locker(CommonWrapper::GetMainFrame()->m_mtxChildWndMap);
				CommonWrapper::GetMainFrame()->m_MDIChildWndMap[JavaPath] = MDIChild;
			}
			MDIChild->MDIActivate();
			CFernflowerUIMFCView * pView = static_cast<CFernflowerUIMFCView*>(CommonWrapper::GetMainFrame()->MDIGetActive()->GetActiveView());
			pView->FinishHighLight = false;
			std::future<void> SetHighLight = std::async(std::launch::async,[&](const CStringW & Str, CWnd * MainWnd) {
				pView->SetViewText(Str);
			}, Contact, AfxGetMainWnd());
			/*CommonWrapper::CWaitDlg Wait(AfxGetMainWnd(),
			[]()->bool {AfxGetMainWnd()->RestoreWaitCursor(); return static_cast<CFernflowerUIMFCView*>(static_cast<CFrameWnd*>(AfxGetMainWnd())->GetActiveView())->FinishHighLight; },
			5, L"正在打开文件");
			Wait.DoModal();*/
			CommonWrapper::CProgressBar Progress(AfxGetMainWnd(),
				[&]()->int {AfxGetMainWnd()->RestoreWaitCursor(); return pView->FinishHighLight; },
				5, IsInChinese() ? L"正在打开文件" : L"Opening the file", 0, 78);
			Progress.DoModal();
			AfxGetMainWnd()->EndWaitCursor();
			CommonWrapper::GetMainFrame()->SetWindowText(theApp.JarFilePath + L" => " + FileName + _T(" - FernFlowerUI"));
			File.Close();
		}
	}
	*pResult = 0;
}


void CViewTree::OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	HTREEITEM hItem = this->GetSelectedItem();
	HTREEITEM hSelectedItem = hItem;
	int ImgIndex, SelectedImgIndex;
	if (this->GetItemImage(hItem, ImgIndex, SelectedImgIndex))
	{
		if ((ImgIndex != 3) && (ImgIndex < 8))
		{
			if (ImgIndex != 0)
			{
				while (hItem = GetParentItem(hItem))
				{
					VERIFY(this->GetItemImage(hItem, ImgIndex, SelectedImgIndex));
					if (ImgIndex == 0)
					{
						break;
					}
				}
			}
			AfxGetMainWnd()->BeginWaitCursor();
			std::stack<CStringW> StringStack;
			CStringW JavaPath;
			CStringW FileName = GetItemText(hItem);
			HTREEITEM hClassItem = hItem;
			FileName += L".java";
			char * UserProFile;
			size_t size;
			if (_dupenv_s(&UserProFile, &size, "USERPROFILE"))
			{
				AfxGetMainWnd()->MessageBox(IsInChinese() ? _T("搜索%USERPROFILE%失败!") : _T("Failed to access %USERPROFILE%"), IsInChinese() ? _T("错误") : _T("Error"), MB_ICONERROR);
				return;
			}
			JavaPath = UserProFile;
			JavaPath += L"\\AppData\\Local\\FernFlowerUI\\Cache\\";
			JavaPath += theApp.Md5ofFile;
			JavaPath += L"\\JarCache\\";
			free(UserProFile);
			StringStack.push(GetItemText(hItem) + L".java");
			while (hItem = GetParentItem(hItem))
			{
				StringStack.push(GetItemText(hItem) + L"\\");
			}
			while (!StringStack.empty())
			{
				JavaPath += StringStack.top();
				StringStack.pop();
			}
			CHARRANGE chRange = { 0 };
			if (CommonWrapper::GetMainFrame()->m_MDIChildWndMap.count(JavaPath) == 1)
			{
				if (hClassItem != hSelectedItem)
				{
					chRange = CommonWrapper::GetMainFrame()->GetClassView()->m_mapItemRange[hSelectedItem];
				}
			}
			else
			{
				chRange = CommonWrapper::GetMainFrame()->GetClassView()->m_mapItemRange[hSelectedItem];;
			}
			CViewTree::OpenJavaFile(JavaPath, hClassItem, chRange);
		}
		else if (ImgIndex != 3)
		{
			std::stack<CStringW> StringStack;
			CStringW FilePath;
			CStringW FileName = GetItemText(hItem);
			char * UserProFile;
			size_t size;
			if (_dupenv_s(&UserProFile, &size, "USERPROFILE"))
			{
				AfxGetMainWnd()->MessageBox(IsInChinese() ? _T("搜索%USERPROFILE%失败!") : _T("Failed to access %USERPROFILE%"), IsInChinese() ? _T("错误") : _T("Error"), MB_ICONERROR);
				return;
			}
			FilePath = UserProFile;
			FilePath += L"\\AppData\\Local\\FernFlowerUI\\Cache\\";
			FilePath += theApp.Md5ofFile;
			FilePath += L"\\JarCache\\";
			free(UserProFile);
			StringStack.push(FileName);
			while (hItem = GetParentItem(hItem))
			{
				StringStack.push(GetItemText(hItem) + L"\\");
			}
			while (!StringStack.empty())
			{
				FilePath += StringStack.top();
				StringStack.pop();
			}
			ShellExecute(CommonWrapper::GetMainFrame()->GetSafeHwnd(), L"open", FilePath, nullptr, nullptr, SW_SHOW);
		}
	}
	*pResult = 0;
}


// Helper to open .java file
void CViewTree::OpenJavaFile(const CString& JavaPath, HTREEITEM hClassItem, CHARRANGE chRange)
{
	// TODO: 在此处添加实现代码.
	CFile File;
	File.Open(JavaPath, CStdioFile::modeNoTruncate | CStdioFile::modeRead | CStdioFile::shareDenyNone);
	size_t Len = File.GetLength();
	char * buf = new char[Len + 1];
	memset(buf, 0, Len + 1);
	File.Read(buf, Len);
	for (size_t i = 0; i < Len; i++)
	{
		if (buf[i] == '\0')
		{
			buf[i] = ' ';
		}
	}
	buf[Len] = '\0';
	int nConLen = MultiByteToWideChar(CP_UTF8, 0, buf, -1, nullptr, 0);
	wchar_t * wbuf = new wchar_t[nConLen + 1]{ 0 };
	MultiByteToWideChar(CP_UTF8, 0, buf, Len, wbuf, nConLen);
	delete buf;
	CStringW Contact = wbuf;
	delete wbuf;
	/*CStringW Contact, buf;
	while (File.ReadString(buf))
	{

	Contact += (buf + L"\r\n");
	}*/
	//Contact.Replace(L"   ", L"        ");
	if (CommonWrapper::GetMainFrame()->m_MDIChildWndMap.count(JavaPath) == 1)
	{
		CommonWrapper::GetMainFrame()->m_MDIChildWndMap[JavaPath]->MDIActivate();
		if (chRange.cpMin*chRange.cpMax)
		{
			CFernflowerUIMFCView * pView = static_cast<CFernflowerUIMFCView*>(CommonWrapper::GetMainFrame()->MDIGetActive()->GetActiveView());
			pView->m_wndEdit.SetSel(chRange.cpMin, chRange.cpMin);
			pView->m_wndEdit.SetSel(chRange);
		}
		return;
	}
	std::pair<HTREEITEM, CString> Pair(hClassItem, JavaPath);
	CMDIChildWndEx * MDIChild = CommonWrapper::GetMainFrame()->CreateDocumentWindow(File.GetFileName(), (CObject*)&Pair);
	VERIFY(MDIChild != nullptr);
	{
		//std::lock_guard<std::mutex> locker(CommonWrapper::GetMainFrame()->m_mtxChildWndMap);
		CommonWrapper::GetMainFrame()->m_MDIChildWndMap[JavaPath] = MDIChild;
	}
	MDIChild->MDIActivate();
	CFernflowerUIMFCView * pView = static_cast<CFernflowerUIMFCView*>(CommonWrapper::GetMainFrame()->MDIGetActive()->GetActiveView());
	pView->FinishHighLight = false;
	std::future<void> SetHighLight = std::async(std::launch::async, [&](const CStringW & Str, CWnd * MainWnd, CHARRANGE chRange) {
		pView->SetViewText(Str, chRange);
	}, Contact, AfxGetMainWnd(), chRange);
	/*CommonWrapper::CWaitDlg Wait(AfxGetMainWnd(),
	[]()->bool {AfxGetMainWnd()->RestoreWaitCursor(); return static_cast<CFernflowerUIMFCView*>(static_cast<CFrameWnd*>(AfxGetMainWnd())->GetActiveView())->FinishHighLight; },
	5, L"正在打开文件");
	Wait.DoModal();*/
	CommonWrapper::CProgressBar Progress(AfxGetMainWnd(),
		[&]()->int {AfxGetMainWnd()->RestoreWaitCursor(); return pView->FinishHighLight; },
		5, IsInChinese() ? L"正在打开文件" : L"Opening the file", 0, 78);
	Progress.DoModal();
	AfxGetMainWnd()->EndWaitCursor();
	CommonWrapper::GetMainFrame()->SetWindowText(theApp.JarFilePath + L" => " + File.GetFileName() + _T(" - FernFlowerUI"));
	File.Close();
	return;
}
