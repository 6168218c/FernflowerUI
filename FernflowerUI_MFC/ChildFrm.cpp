
// ChildFrm.cpp: CChildFrame 类的实现
//

#include "stdafx.h"
#include "FernflowerUI_MFC.h"
#include "FernflowerUI_MFCView.h"
#include "CommonWrapper.h"
#include "ChildFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CChildFrame

IMPLEMENT_DYNCREATE(CChildFrame, CMDIChildWndEx)

BEGIN_MESSAGE_MAP(CChildFrame, CMDIChildWndEx)
	ON_WM_CREATE()
	ON_WM_MDIACTIVATE()
	ON_UPDATE_COMMAND_UI(ID_ENABLE_RECOVERY, &CChildFrame::OnUpdateEnableRecovery)
	ON_COMMAND(ID_ENABLE_RECOVERY, &CChildFrame::OnEnableRecovery)
	ON_WM_DESTROY()
END_MESSAGE_MAP()

// CChildFrame 构造/析构

CChildFrame::CChildFrame()
{
	EnableActiveAccessibility();
	// TODO: 在此添加成员初始化代码
}

CChildFrame::~CChildFrame()
{
}

BOOL CChildFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext)
{
	return CMDIChildWndEx::OnCreateClient(lpcs, pContext);
}

BOOL CChildFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: 在此处通过修改 CREATESTRUCT cs 来修改窗口类或样式
	if (!CMDIChildWndEx::PreCreateWindow(cs))
		return FALSE;

	return TRUE;
}

// CChildFrame 诊断

#ifdef _DEBUG
void CChildFrame::AssertValid() const
{
	CMDIChildWndEx::AssertValid();
}

void CChildFrame::Dump(CDumpContext& dc) const
{
	CMDIChildWndEx::Dump(dc);
}
#endif //_DEBUG

// CChildFrame 消息处理程序


void CChildFrame::OnUpdateFrameTitle(BOOL bAddToTitle)
{
	// TODO: 在此添加专用代码和/或调用基类

	SetWindowText(m_strTitle);
	//Calling base class' method is useless.
	//CMDIChildWndEx::OnUpdateFrameTitle(bAddToTitle);
}


int CChildFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIChildWndEx::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  在此添加您专用的创建代码

	return 0;
}


void CChildFrame::OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd)
{
	CMDIChildWndEx::OnMDIActivate(bActivate, pActivateWnd, pDeactivateWnd);

	// TODO: 在此处添加消息处理程序代码
}


void CChildFrame::OnUpdateEnableRecovery(CCmdUI *pCmdUI)
{
	// TODO: 在此添加命令更新用户界面处理程序代码
	pCmdUI->SetCheck(theApp.EnableRecovery);
}


void CChildFrame::OnEnableRecovery()
{
	// TODO: 在此添加命令处理程序代码
	theApp.EnableRecovery = !(theApp.EnableRecovery);
}


void CChildFrame::OnDestroy()
{
	for (auto it = CommonWrapper::GetMainFrame()->m_MDIChildWndMap.begin(); it != CommonWrapper::GetMainFrame()->m_MDIChildWndMap.end(); it++)
	{
		if (it->second==this)
		{
			CommonWrapper::GetMainFrame()->m_MDIChildWndMap.erase(it);
			break;
		}
	}
	CMDIChildWndEx::OnDestroy();

	// TODO: 在此处添加消息处理程序代码
}
