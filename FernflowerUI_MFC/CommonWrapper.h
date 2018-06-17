#pragma once

#include "resource.h"
#include "MainFrm.h"
#include "FernflowerUI_MFC.h"

namespace CommonWrapper
{
	class CWaitDlg : public CDialogEx
	{
	public:
		CWaitDlg(CWnd * MainWnd, std::function<bool()> c, DWORD Sec, const wchar_t * Text, DWORD MarSec = 20, UINT nIDTemplate = IDD_WAITBAR, bool EnableParent = false);
		afx_msg LRESULT OnInitDialog(WPARAM, LPARAM);
		void OnShowWindow(BOOL bShow, UINT nStatus);
		afx_msg void DoProgress();

		// 对话框数据
#ifdef AFX_DESIGN_TIME
		enum { IDD = IDD_WAITBAR };
#endif

	protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
															// 实现
		CProgressCtrl * m_wndProgressBar;
		int iLow;
		int iHigh;
		std::function<bool()> Condit;
		bool WithoutSleep;
		bool bFlag;
		bool bEnableParent;
		DWORD WaitSeconds;
		DWORD MarqueeSecond;
		CStatic * m_pStaticText;
		CFont m_Font;
		const CStringW StaticText;
	protected:
		afx_msg void OnTimer(UINT_PTR nIDEvent);
		DECLARE_MESSAGE_MAP()
	};

	inline CMainFrame * GetMainFrame()
	{
		return static_cast<CMainFrame*>(theApp.GetMainWnd());
	}

	class CProgressBar : public CDialogEx
	{
	public:
		//The condit must return -1 when it needs to exit the dialog
		CProgressBar(CWnd * MainWnd, std::function<int()> c, DWORD Sec, const wchar_t * Text,int LowBound,int HighBound);
		afx_msg LRESULT OnInitDialog(WPARAM, LPARAM);
		void OnShowWindow(BOOL bShow, UINT nStatus);
		afx_msg void DoProgress();

		// 对话框数据
#ifdef AFX_DESIGN_TIME
		enum { IDD = IDD_WAITBAR };
#endif

	protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
															// 实现
		CProgressCtrl * m_wndProgressBar;
		int iLow;
		int iHigh;
		int Pos;
		std::function<int()> Condit;
		DWORD WaitSeconds;
		CStatic * m_pStaticText;
		CFont m_Font;
		const CStringW StaticText;
	protected:
		afx_msg void OnTimer(UINT_PTR nIDEvent);
		DECLARE_MESSAGE_MAP()
	};

	class CLaunchDialog :public CDialogEx
	{
	public:
		CLaunchDialog() :CDialogEx(IDD_LAUNCHBAR) {};
		~CLaunchDialog() {};
		DECLARE_MESSAGE_MAP()
	protected:
		afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
		afx_msg LRESULT OnInitDialog(WPARAM wParam, LPARAM lParam);
		CStatic * m_pStatic;
		CFont m_Font;
	};
}

inline bool IsInChinese()
{
	return theApp.Language == CFernflowerUIMFCApp::AppLanguage::Chinese;
}