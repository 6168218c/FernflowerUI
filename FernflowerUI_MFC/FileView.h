#pragma once

#include "ShellTreeView.h"

// CFileView

static const UINT WM_SHELLTREE_SELPATH = RegisterWindowMessage(L"6168218c.FernflowerUI_MFC.ShellTreeSelectPath");

class CShellTreeFrame :public CMiniFrameWnd
{
public:
	CShellTreeView m_ShellTreeView;
	CString strPathToSelect;
public:
	DECLARE_MESSAGE_MAP()
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
protected:
	afx_msg LRESULT OnShelltreeSelpath(WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
};

class CFileView : public CDockablePane
{
	DECLARE_DYNAMIC(CFileView)

public:
	CFileView();
	virtual ~CFileView();
	friend class COutputWnd;
	friend class CFernflowerUIMFCApp;
	friend class CSubThread;
	friend class CShellTreeView;
	friend class CMainFrame;

protected:
	DECLARE_MESSAGE_MAP()
	CShellTreeView m_ShellTreeView;
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
protected:
	void AdjustLayout();
public:
	virtual void ShowPane(BOOL bShow, BOOL bDelay, BOOL bActivate/* = TRUE*/);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
};

class CSubThread :public CWinThread
{
public:
	friend class CFileView;
	CSubThread();
	~CSubThread();
	CSubThread(AFX_THREADPROC pfnThreadProc, LPVOID pParam);
protected:
	BOOL InitInstance();
	HHOOK m_CreationHook;
	void HookWindowCreate();
	bool UnHookWindowCreate();
	DECLARE_DYNCREATE(CSubThread)
protected:
	CFileView * pParentView;
	HWND MainWnd;
	bool Created;
};