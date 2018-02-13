
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
	ON_NOTIFY_REFLECT(TVN_SELCHANGED, &CViewTree::OnTvnSelchanged)
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


void CViewTree::OnTvnSelchanged(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码

	int ImgIndex, SelectedImgIndex;
	if (this->GetItemImage(pNMTreeView->itemNew.hItem,ImgIndex,SelectedImgIndex))
	{
		if (ImgIndex == 1)
		{
			AfxGetMainWnd()->BeginWaitCursor();
			std::stack<CStringW> StringStack;
			HTREEITEM hItem = pNMTreeView->itemNew.hItem;
			CStringW JavaPath;
			char * UserProFile;
			size_t size;
			if (_dupenv_s(&UserProFile,&size,"USERPROFILE"))
			{
				AfxGetMainWnd()->MessageBox(L"搜索%USERPROFILE%失败!", L"错误", MB_ICONERROR);
				return;
			}
			JavaPath = UserProFile;
			JavaPath += L"\\AppData\\Local\\FernFlowerUI\\JarCache\\";
			free(UserProFile);
			StringStack.push(GetItemText(hItem)+L".java");
			while (hItem=GetParentItem(hItem))
			{
				StringStack.push(GetItemText(hItem) + L"\\");
			}
			while (!StringStack.empty())
			{
				JavaPath += StringStack.top();
				StringStack.pop();
			}
			CStdioFile File;
			File.Open(JavaPath, CStdioFile::modeNoTruncate | CStdioFile::modeRead | CStdioFile::shareDenyRead);
			size_t Len = File.GetLength();
			char * buf = new char[Len + 1];
			memset(buf, 0, Len+1);
			File.Read(buf, Len);
			for (size_t i = 0; i < Len; i++)
			{
				if (buf[i]==L'\0')
				{
					buf[i] = L' ';
				}
			}
			CStringW Contact;
			Contact = buf;
			delete buf;
			/*CStringW Contact, buf;
			while (File.ReadString(buf))
			{
				
				Contact += (buf + L"\r\n");
			}*/
			Contact.Replace(L"   ", L"        ");
			static_cast<CFernflowerUIMFCView*>(static_cast<CFrameWnd*>(AfxGetMainWnd())->GetActiveView())->FinishHighLight = false;
			std::future<void> SetHighLight = std::async([](const CStringW & Str,CWnd * MainWnd) {
				static_cast<CFernflowerUIMFCView*>(static_cast<CFrameWnd*>(MainWnd)->GetActiveView())->SetViewText(Str);
			}, Contact,AfxGetMainWnd());
			/*CommonWrapper::CWaitDlg Wait(AfxGetMainWnd(),
				[]()->bool {AfxGetMainWnd()->RestoreWaitCursor(); return static_cast<CFernflowerUIMFCView*>(static_cast<CFrameWnd*>(AfxGetMainWnd())->GetActiveView())->FinishHighLight; },
				5, L"正在打开文件");
			Wait.DoModal();*/
			CommonWrapper::CProgressBar Progress(AfxGetMainWnd(),
				[]()->int {AfxGetMainWnd()->RestoreWaitCursor(); return static_cast<CFernflowerUIMFCView*>(static_cast<CFrameWnd*>(AfxGetMainWnd())->GetActiveView())->FinishHighLight; },
				5, L"正在打开文件", 0, 77);
			Progress.DoModal();
			AfxGetMainWnd()->EndWaitCursor();
		}
	}
	*pResult = 0;
}
