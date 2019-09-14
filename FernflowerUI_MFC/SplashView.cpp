#include "stdafx.h"
#include "FernflowerUI_MFC.h"
#include "SplashView.h"
#include "CommonWrapper.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CSplashView,CFormView)

CSplashView::CSplashView():CFormView(IDD_SPLASHFORM)
{
}


CSplashView::~CSplashView()
{
}
BEGIN_MESSAGE_MAP(CSplashView, CFormView)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_COMMAND(IDM_OPEN_CLASS, &CSplashView::OnOpenClass)
	ON_COMMAND(IDM_OPEN_JAR, &CSplashView::OnOpenJar)
	ON_WM_DRAWITEM()
END_MESSAGE_MAP()

CFont linkFont;

int CSplashView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFormView::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  在此添加您专用的创建代码
	
	VERIFY(m_Font.CreatePointFont(240, IsInChinese() ? L"Microsoft YaHei UI Light" : L"Consolas"));
//	VERIFY(linkFont.CreatePointFont(120, IsInChinese() ? L"Microsoft YaHei UI Light" : L"Consolas"));

	return 0;
}


void CSplashView::OnInitialUpdate()
{

	// TODO: 在此添加专用代码和/或调用基类
	{
		CStatic * pStatic;
		VERIFY(pStatic = static_cast<CStatic*>(this->GetDlgItem(IDC_GET_STARTED)));
		pStatic->SetFont(&m_Font);
		VERIFY(pStatic = static_cast<CStatic*>(this->GetDlgItem(IDC_RECENT)));
		pStatic->SetFont(&m_Font);
		VERIFY(pStatic = static_cast<CStatic*>(this->GetDlgItem(IDC_OPENTASK)));
		pStatic->SetFont(&m_Font);
		VERIFY(pStatic = static_cast<CStatic*>(this->GetDlgItem(IDC_VERSIONINFO)));
		pStatic->SetFont(&m_Font);
	}
	/*{
		m_linkHelpCore.Attach(GetDlgItem(IDC_HELP_CORE)->Detach());
		m_linkHelpFeatures.Attach(GetDlgItem(IDC_HELP_FEATURES)->Detach());
		m_linkViewUpdate.Attach(GetDlgItem(IDC_VIEW_UPDATELOG)->Detach());

	}*/
	m_wndlistCtrl.Attach(GetDlgItem(IDC_RECENT_FILES)->Detach());
	CBitmap bmp;
	if (!bmp.LoadBitmap(IDB_STARTLIST_IMAGE))
	{
		TRACE(_T("无法加载位图: %x\n"), IDB_STARTLIST_IMAGE);
		ASSERT(FALSE);
		return;
	}
	BITMAP bmpObj;
	bmp.GetBitmap(&bmpObj);

	UINT nFlags = ILC_MASK;
	nFlags |= ILC_COLOR24;
	m_ImageList.Create(16, bmpObj.bmHeight, nFlags, 0, 0);
	m_ImageList.Add(&bmp, RGB(255, 0, 0));
	m_wndlistCtrl.SetImageList(&m_ImageList, LVSIL_SMALL);
	m_wndlistCtrl.SetExtendedStyle(LVS_EX_SUBITEMIMAGES | LVS_EX_ONECLICKACTIVATE | LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);
	SetWindowTheme(m_wndlistCtrl.m_hWnd, _T("Explorer"), nullptr);

	CFormView::OnInitialUpdate();
	CommonWrapper::GetMainFrame()->SetFocus();
}


void CSplashView::OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/)
{
	// TODO: 在此添加专用代码和/或调用基类
	m_wndlistCtrl.DeleteAllItems();
	for (int i = 0; i < theApp.pMRUList->GetSize(); i++)
	{
		CString str = (*theApp.pMRUList)[i];
		if (str == L"")
			continue;
		if (str.Right(6).CompareNoCase(L".class") == 0)
		{
			CFile Temp;
			VERIFY(Temp.Open(str, CFile::modeRead | CFile::modeNoTruncate | CFile::shareDenyNone));
			m_wndlistCtrl.InsertItem(m_wndlistCtrl.GetItemCount(), Temp.GetFileName(), 1);
			Temp.Close();
		}
		else if ((str.Right(4).CompareNoCase(L".jar") == 0) || (str.Right(4).CompareNoCase(L".zip") == 0))
		{
			CFile Temp;
			VERIFY(Temp.Open(str, CFile::modeRead | CFile::modeNoTruncate | CFile::shareDenyNone));
			m_wndlistCtrl.InsertItem(m_wndlistCtrl.GetItemCount(), Temp.GetFileName(), 0);
			Temp.Close();
		}
	}
	m_wndlistCtrl.Invalidate();
}

BEGIN_MESSAGE_MAP(CMRUFileListCtrl, CListCtrl)
	ON_NOTIFY_REFLECT(NM_CLICK, &CMRUFileListCtrl::OnNMClick)
	ON_NOTIFY_REFLECT(LVN_GETINFOTIP, &CMRUFileListCtrl::OnLvnGetInfoTip)
END_MESSAGE_MAP()


void CMRUFileListCtrl::OnNMClick(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	CPoint Point;
	UINT uFlags;
	GetCursorPos(&Point);
	ScreenToClient(&Point);
	int nIndex = HitTest(Point, &uFlags);
	if ((uFlags&LVHT_ONITEMICON) || (uFlags&LVHT_ONITEMLABEL))
	{
		theApp.DoDecomplie((*theApp.pMRUList)[nIndex]);
	}
	*pResult = 0;
}


void CSplashView::OnDestroy()
{
	m_wndlistCtrl.Detach();
	CFormView::OnDestroy();

	// TODO: 在此处添加消息处理程序代码
}


void CSplashView::OnOpenClass()
{
	// TODO: 在此添加命令处理程序代码
	theApp.OpenFlag = CFernflowerUIMFCApp::OpenFlags::OpenClass;
	theApp.OpenFile();
}


void CSplashView::OnOpenJar()
{
	// TODO: 在此添加命令处理程序代码
	theApp.OpenFlag = CFernflowerUIMFCApp::OpenFlags::OpenJar;
	theApp.OpenFile();
}


void CStartPageLink::OnDraw(CDC* pDC, const CRect& rect, UINT uiState)
{
	// TODO: 在此添加专用代码和/或调用基类
	if (!(HFONT)m_linkFont)
	{
		LOGFONT logFont;
		GetGlobalData()->fontDefaultGUIUnderline.GetLogFont(&logFont);
		logFont.lfHeight = 12;
		logFont.lfUnderline = FALSE;
		lstrcpy(logFont.lfFaceName, IsInChinese() ? L"Microsoft YaHei" : L"Consolas");
		m_linkFont.CreateFontIndirect(&logFont);
	}
	ASSERT_VALID(pDC);

	// Set font:
	CFont* pOldFont = NULL;

	if (m_bAlwaysUnderlineText || m_bHover)
	{
		//pOldFont = pDC->SelectObject(&(GetGlobalData()->fontDefaultGUIUnderline));
		pOldFont = pDC->SelectObject(&m_linkFont);
	}
	else
	{
		pOldFont = CMFCButton::SelectFont(pDC);
	}

	ENSURE(pOldFont != NULL);

	// Set text parameters:
	pDC->SetTextColor(m_bHover ? GetGlobalData()->clrHotLinkHoveredText : (m_bVisited ? GetGlobalData()->clrHotLinkVisitedText : GetGlobalData()->clrHotLinkNormalText));
	pDC->SetBkMode(TRANSPARENT);

	// Obtain label:
	CString strLabel;
	GetWindowText(strLabel);

	CRect rectText = rect;
	pDC->DrawText(strLabel, rectText, m_bMultilineText ? DT_WORDBREAK : DT_SINGLELINE);

	pDC->SelectObject(pOldFont);
}


void CSplashView::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (!(HFONT)m_linkFont)
	{
		LOGFONT logFont;
		GetGlobalData()->fontDefaultGUIUnderline.GetLogFont(&logFont);
		HDC hDC = ::GetDC(HWND_DESKTOP);
		logFont.lfHeight = -MulDiv(12, GetDeviceCaps(hDC, LOGPIXELSY), 72);
		::ReleaseDC(HWND_DESKTOP, hDC);
		logFont.lfUnderline = FALSE;
		lstrcpy(logFont.lfFaceName, IsInChinese() ? L"Microsoft YaHei" : L"Consolas");
		m_linkFont.CreateFontIndirect(&logFont);
	}
	HGDIOBJ hOldFont, hLinkFont;
	hLinkFont = m_linkFont.Detach();
	hOldFont = GetGlobalData()->fontDefaultGUIUnderline.Detach();
	GetGlobalData()->fontDefaultGUIUnderline.Attach(hLinkFont);
	CFormView::OnDrawItem(nIDCtl, lpDrawItemStruct);
	GetGlobalData()->fontDefaultGUIUnderline.Detach();
	GetGlobalData()->fontDefaultGUIUnderline.Attach(hOldFont);
	m_linkFont.Attach(hLinkFont);
}


void CMRUFileListCtrl::OnLvnGetInfoTip(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLVGETINFOTIP pGetInfoTip = reinterpret_cast<LPNMLVGETINFOTIP>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	//pGetInfoTip->pszText = nullptr;
	/*LVSETINFOTIP InfoTip = { 0 };
	CString str = (*theApp.pMRUList)[pGetInfoTip->iItem];
	InfoTip.iItem = pGetInfoTip->iItem;
	InfoTip.iSubItem = pGetInfoTip->iSubItem;
	InfoTip.pszText = str.GetBuffer();
	SetInfoTip(&InfoTip);*/
	CString str = (*theApp.pMRUList)[pGetInfoTip->iItem];
	lstrcpy(pGetInfoTip->pszText, str);
	*pResult = 0;
}
