#pragma once
#include <afxext.h>

class CMRUFileListCtrl :public CListCtrl
{
public:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnNMClick(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnGetInfoTip(NMHDR *pNMHDR, LRESULT *pResult);
};

class CStartPageLink :public CMFCLinkCtrl
{
protected:
	CFont m_linkFont;
public:
	virtual void OnDraw(CDC* pDC, const CRect& rect, UINT uiState);
};

class CSplashView :
	public CFormView
{
protected:
	CFont m_Font;
	CFont m_linkFont;
	CMRUFileListCtrl m_wndlistCtrl;
	CStartPageLink   m_linkHelpCore;
	CStartPageLink   m_linkHelpFeatures;
	CStartPageLink   m_linkViewUpdate;
	std::map<int, int> m_mapSysImageId;
	CImageList m_ImageList;
protected:
	CSplashView();
	~CSplashView();
	DECLARE_DYNCREATE(CSplashView)
	DECLARE_MESSAGE_MAP()
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
public:
	virtual void OnInitialUpdate();
	virtual void OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/);
	afx_msg void OnDestroy();
	afx_msg void OnOpenClass();
	afx_msg void OnOpenJar();
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
};

