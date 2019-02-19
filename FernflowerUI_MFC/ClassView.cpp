
#include "stdafx.h"
#include "MainFrm.h"
#include "ClassView.h"
#include "Resource.h"
#include "FernFlowerUI_MFC.h"
#include "FernflowerUI_MFCView.h"
#include "CommonWrapper.h"

class CClassViewMenuButton : public CMFCToolBarMenuButton
{
	friend class CClassView;

	DECLARE_SERIAL(CClassViewMenuButton)

public:
	CClassViewMenuButton(HMENU hMenu = NULL) : CMFCToolBarMenuButton((UINT)-1, hMenu, -1)
	{
	}

	virtual void OnDraw(CDC* pDC, const CRect& rect, CMFCToolBarImages* pImages, BOOL bHorz = TRUE,
		BOOL bCustomizeMode = FALSE, BOOL bHighlight = FALSE, BOOL bDrawBorder = TRUE, BOOL bGrayDisabledButtons = TRUE)
	{
		pImages = CMFCToolBar::GetImages();

		CAfxDrawState ds;
		pImages->PrepareDrawImage(ds);

		CMFCToolBarMenuButton::OnDraw(pDC, rect, pImages, bHorz, bCustomizeMode, bHighlight, bDrawBorder, bGrayDisabledButtons);

		pImages->EndDrawImage(ds);
	}
};

IMPLEMENT_SERIAL(CClassViewMenuButton, CMFCToolBarMenuButton, 1)

//////////////////////////////////////////////////////////////////////
// 构造/析构
//////////////////////////////////////////////////////////////////////

CClassView::CClassView()
{
	m_nCurrSort = ID_SORTING_GROUPBYTYPE;
	m_nCurrImageCount = 8;
}

CClassView::~CClassView()
{
}

BEGIN_MESSAGE_MAP(CClassView, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_CLASS_ADD_MEMBER_FUNCTION, OnClassAddMemberFunction)
	ON_COMMAND(ID_CLASS_ADD_MEMBER_VARIABLE, OnClassAddMemberVariable)
	ON_COMMAND(ID_CLASS_DEFINITION, OnClassDefinition)
	ON_COMMAND(ID_CLASS_PROPERTIES, OnClassProperties)
	ON_COMMAND(ID_NEW_FOLDER, OnNewFolder)
	ON_WM_PAINT()
	ON_WM_SETFOCUS()
	ON_COMMAND_RANGE(ID_SORTING_GROUPBYTYPE, ID_SORTING_SORTBYACCESS, OnSort)
	ON_UPDATE_COMMAND_UI_RANGE(ID_SORTING_GROUPBYTYPE, ID_SORTING_SORTBYACCESS, OnUpdateSort)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CClassView 消息处理程序

int CClassView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rectDummy;
	rectDummy.SetRectEmpty();

	// 创建视图: 
	const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | TVS_LINESATROOT | TVS_HASBUTTONS | TVS_SHOWSELALWAYS | TVS_TRACKSELECT | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

	if (!m_wndClassView.Create(dwViewStyle, rectDummy, this, 2))
	{
		TRACE0("未能创建类视图\n");
		return -1;      // 未能创建
	}
	m_wndClassView.SetExtendedStyle(0, TVS_EX_DOUBLEBUFFER);
	SetWindowTheme(m_wndClassView.m_hWnd, _T("Explorer"), nullptr);

	//创建提示文本
	if (!m_wndStaticText.Create(IsInChinese() ? L"反编译Jar文件以显示类视图" : L"Decompile the Java Archive File to show the Class View",
		SS_CENTER | SS_CENTERIMAGE, rectDummy, this))
	{
		TRACE0("未能创建提示文本\n");
		return -1;      // 未能创建
	}
	if (m_TextFont.CreatePointFont(90, L"Microsoft YaHei"))
	{
		m_wndStaticText.SetFont(&m_TextFont);
	}

	// 加载图像: 
	m_wndToolBar.Create(this, AFX_DEFAULT_TOOLBAR_STYLE, IDR_SORT);
	m_wndToolBar.LoadToolBar(IDR_SORT, 0, 0, TRUE /* 已锁定*/);

	OnChangeVisualStyle();

	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);
	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() & ~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));

	m_wndToolBar.SetOwner(this);

	// 所有命令将通过此控件路由，而不是通过主框架路由: 
	m_wndToolBar.SetRouteCommandsViaFrame(FALSE);

	CMenu menuSort;
	menuSort.LoadMenu(IDR_POPUP_SORT);

	m_wndToolBar.ReplaceButton(ID_SORT_MENU, CClassViewMenuButton(menuSort.GetSubMenu(0)->GetSafeHmenu()));

	CClassViewMenuButton* pButton = DYNAMIC_DOWNCAST(CClassViewMenuButton, m_wndToolBar.GetButton(0));

	if (pButton != NULL)
	{
		pButton->m_bText = FALSE;
		pButton->m_bImage = TRUE;
		pButton->SetImage(GetCmdMgr()->GetCmdImage(m_nCurrSort));
		pButton->SetMessageWnd(this);
	}

	// 填入一些静态树视图数据(此处只需填入虚拟代码，而不是复杂的数据)
	//FillClassView();

	return 0;
}

void CClassView::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	AdjustLayout();
}

void CClassView::FillClassView()
{
	HTREEITEM hRoot = m_wndClassView.InsertItem(_T("FakeApp 类"), 0, 0);
	m_wndClassView.SetItemState(hRoot, TVIS_BOLD, TVIS_BOLD);

	HTREEITEM hClass = m_wndClassView.InsertItem(_T("CFakeAboutDlg"), 1, 1, hRoot);
	m_wndClassView.InsertItem(_T("CFakeAboutDlg()"), 3, 3, hClass);

	m_wndClassView.Expand(hRoot, TVE_EXPAND);

	hClass = m_wndClassView.InsertItem(_T("CFakeApp"), 1, 1, hRoot);
	m_wndClassView.InsertItem(_T("CFakeApp()"), 3, 3, hClass);
	m_wndClassView.InsertItem(_T("InitInstance()"), 3, 3, hClass);
	m_wndClassView.InsertItem(_T("OnAppAbout()"), 3, 3, hClass);

	hClass = m_wndClassView.InsertItem(_T("CFakeAppDoc"), 1, 1, hRoot);
	m_wndClassView.InsertItem(_T("CFakeAppDoc()"), 4, 4, hClass);
	m_wndClassView.InsertItem(_T("~CFakeAppDoc()"), 3, 3, hClass);
	m_wndClassView.InsertItem(_T("OnNewDocument()"), 3, 3, hClass);

	hClass = m_wndClassView.InsertItem(_T("CFakeAppView"), 1, 1, hRoot);
	m_wndClassView.InsertItem(_T("CFakeAppView()"), 4, 4, hClass);
	m_wndClassView.InsertItem(_T("~CFakeAppView()"), 3, 3, hClass);
	m_wndClassView.InsertItem(_T("GetDocument()"), 3, 3, hClass);
	m_wndClassView.Expand(hClass, TVE_EXPAND);

	hClass = m_wndClassView.InsertItem(_T("CFakeAppFrame"), 1, 1, hRoot);
	m_wndClassView.InsertItem(_T("CFakeAppFrame()"), 3, 3, hClass);
	m_wndClassView.InsertItem(_T("~CFakeAppFrame()"), 3, 3, hClass);
	m_wndClassView.InsertItem(_T("m_wndMenuBar"), 6, 6, hClass);
	m_wndClassView.InsertItem(_T("m_wndToolBar"), 6, 6, hClass);
	m_wndClassView.InsertItem(_T("m_wndStatusBar"), 6, 6, hClass);

	hClass = m_wndClassView.InsertItem(_T("Globals"), 2, 2, hRoot);
	m_wndClassView.InsertItem(_T("theFakeApp"), 5, 5, hClass);
	m_wndClassView.Expand(hClass, TVE_EXPAND);
}

void CClassView::ParseClasses(CString FilePath, HTREEITEM hParentItem)
{
	CFile File;
	File.Open(FilePath, CFile::modeRead | CFile::modeNoTruncate | CFile::shareDenyNone);
	char * buf;
	size_t length = File.GetLength();
	buf = new char[length + 1];
	memset(buf, 0, length + 1);
	File.Read(buf, length);
	for (size_t i = 0; i < length; i++)
	{
		if (buf[i] == '\0')
		{
			buf[i] = ' ';
		}
	}
	buf[length] = '\0';
	int nConLen = MultiByteToWideChar(CP_UTF8, 0, buf, -1, nullptr, 0);
	wchar_t * wbuf = new wchar_t[nConLen + 1]{ 0 };
	MultiByteToWideChar(CP_UTF8, 0, buf, -1, wbuf, nConLen);
	delete buf;
	CString Content;
	Content = wbuf;
	delete wbuf;
	CString NameSpace;
	HTREEITEM hItem = hParentItem;
	std::stack<CString> StringStack;
//	StringStack.push(m_wndClassView.GetItemText(hItem));
	while (hItem = m_wndClassView.GetParentItem(hItem))
	{
		StringStack.push(m_wndClassView.GetItemText(hItem) + L".");
	}
	while (!StringStack.empty())
	{
		NameSpace += StringStack.top();
		StringStack.pop();
	}
	int len = Content.GetLength();
	int StrPos = -1;
	while ((StrPos = Content.Find(L'\"', StrPos + 1)) != -1)
	{
		CHARRANGE Range;
		{
			int nBackslashCount = 0;
			for (int i = StrPos - 1; Content[i] == L'\\'; i--)
				nBackslashCount++;
			if (nBackslashCount % 2)
			{
				continue;
			}
		}
		if ((Content[StrPos - 1] == L'\'') && (Content[StrPos + 1] == L'\''))
		{
			Content.SetAt(StrPos, L' ');
			continue;
		}
		else
		{
			Range.cpMin = StrPos;
			while ((StrPos = Content.Find(L'\"', StrPos + 1)) != -1)
			{
				int nBackslashCount = 0;
				for (int i = StrPos - 1; Content[i] == L'\\'; i--)
					nBackslashCount++;
				if (nBackslashCount % 2)
				{
					continue;
				}
				else if ((Content[StrPos - 1] == L'\'') && (Content[StrPos + 1] == L'\''))
				{
					Content.SetAt(StrPos, L' ');
					continue;
				}
				else
				{
					break;
				}
			}
			Range.cpMax = StrPos;
			for (int i = Range.cpMin; i <= Range.cpMax; i++)
			{
				const_cast<wchar_t*>((LPCWSTR)Content)[i] = L' ';
			}
		}
	}
	StrPos = -1;
	while ((StrPos = Content.Find(L'\'', StrPos + 1)) != -1)
	{
		if (Content[StrPos + 2] == L'\'')
		{
			Content.SetAt(StrPos + 1, L' ');
			StrPos = StrPos + 2;
			continue;
		}
		if (Content[StrPos + 3] == L'\'')
		{
			Content.SetAt(StrPos + 1, L' ');
			Content.SetAt(StrPos + 2, L' ');
			StrPos = StrPos + 3;
			continue;
		}
	}
	Content.Replace(L"\r\n", L"\n");
	StrPos = -1;
	while ((StrPos = Content.Find(L"//", StrPos + 1)) != -1)
	{
		int nEnd = Content.Find(L'\n', StrPos + 1);
		for (int i = StrPos; i <= nEnd; i++)
		{
			const_cast<wchar_t*>((LPCWSTR)Content)[i] = L' ';
		}
	}
	while ((StrPos = Content.Find(L"/*", StrPos + 1)) != -1)
	{
		int nEnd = Content.Find(L"*/", StrPos + 1);
		for (int i = StrPos; i <= nEnd+1; i++)
		{
			const_cast<wchar_t*>((LPCWSTR)Content)[i] = L' ';
		}
		StrPos = nEnd + 1;
	}
	StrPos = -1;
	while ((StrPos = Content.Find(L"static {", StrPos + 1)) != -1)
	{
		int nBraceCount = 0;
		for (int i = StrPos; i < len; i++)
		{
			if (Content[i] == L'{')
			{
				nBraceCount++;
			}
			if (Content[i] == L'}')
			{
				if (nBraceCount == 1)
				{
					int iLast = i;
					for (int iPos = StrPos; iPos <= iLast; iPos++)
					{
						const_cast<wchar_t*>((LPCWSTR)Content)[iPos] = L' ';
					}
					break;
				}
				else
				{
					nBraceCount--;
				}
			}
		}
	}
	m_mapClassFile[FilePath] = hParentItem;
	ParseInnerClass(Content, hParentItem, 0, NameSpace);
	/*
	{
		int iClass = -1;
		int iLast;
		int iFirstFunction = -1;//Fields are always decleared before functions
		int iClassContentBegin = -1;
		while ((iClass = Content.Find(L"class", iClass + 1)) != -1)
		{
			HTREEITEM hClassItem = nullptr;
			{
				CString ClassName;
				int nBraceCount = 0;
				for (int i = iClass; i < len; i++)
				{
					if (Content[i] == L'{')
					{
						if (nBraceCount==0)
						{
							ClassName = Content.Mid(iClass + 6, Content.Find(L' ', iClass + 6) - iClass - 5);
							iClassContentBegin = i;
						}
						nBraceCount++;
					}
					if (Content[i] == L'}')
					{
						if (nBraceCount == 1)
						{
							iLast = i;
							break;
						}
						else
						{
							nBraceCount--;
						}
					}
				}
				//插入树视图节点
				hClassItem = m_wndClassView.InsertItem(ClassName, 1, 1, hParentItem);
			}
			
			int iFunction = -1;
			while (((iFunction = Content.Find(L')', iFunction + 1)) != -1) && (iFunction))
			{
				bool IsDefiningFunc = false;
				for (int i = iFunction + 1; i <= iLast; i++)
				{
					if (!isspace(Content[i]))
					{
						if (Content[i] == L'{'||Content[i]==L't')
						{
							IsDefiningFunc = true;
							int nBraceCount = 0;
							int iLastBrace;
							for (iLastBrace = iFunction+1; iLastBrace <= iLast; iLastBrace++)
							{
								if (Content[iLastBrace] == L'{')
								{
									nBraceCount++;
								}
								if (Content[iLastBrace] == L'}')
								{
									if (nBraceCount == 1)
									{
										break;
									}
									else
									{
										nBraceCount--;
									}
								}
							}
							for (int iChar = i+1; iChar < iLastBrace; iChar++)
							{
								Content.SetAt(iChar, L' ');
							}
							break;
						}
						break;
					}
				}
				if (IsDefiningFunc)
				{
					CString FunctionInfo;
					CString ReturnTypeName;
					bool bIsPublic;
					bool bHasVisitedArgList = false;
					for (int iFuncArgListBegin = iFunction; iFuncArgListBegin > iClass; iFuncArgListBegin--)
					{
						if (Content[iFuncArgListBegin] == L'(')
						{
							bHasVisitedArgList = true;
						}
						if ((bHasVisitedArgList) && (Content[iFuncArgListBegin] == L' '))
						{
							FunctionInfo = Content.Mid(iFuncArgListBegin + 1, iFunction - iFuncArgListBegin);
							for (int i = iFuncArgListBegin - 1; i > iClass; i--)
							{
								if (Content[i] == L' ')
								{
									ReturnTypeName = Content.Mid(i + 1, iFuncArgListBegin - i);
									ReturnTypeName = ReturnTypeName.TrimRight();
									if (ReturnTypeName == L"public")
									{
										ReturnTypeName = IsInChinese() ? L"构造函数" : L"Constructor";
										bIsPublic = true;
									}
									else if (ReturnTypeName == L"protected" || ReturnTypeName == L"private")
									{
										ReturnTypeName = IsInChinese() ? L"构造函数" : L"Constructor";
										bIsPublic = false;
									}
									break;
								}
							}
							int iLineBegin;
							for (iLineBegin = iFuncArgListBegin; Content[iLineBegin] != '\n'; iLineBegin--)
							{
							}
							for (; (iLineBegin < iFuncArgListBegin) && (isspace(Content[iLineBegin])); iLineBegin++)
							{
							}
							if (iFirstFunction==-1)
							{
								iFirstFunction = iLineBegin;
							}
							for (int iAccessRightEnd = iLineBegin; iAccessRightEnd < iFuncArgListBegin; iAccessRightEnd++)
							{
								if (isspace(Content[iAccessRightEnd]))
								{
									CString AccessRight = Content.Mid(iLineBegin, iAccessRightEnd - iLineBegin);
									if (AccessRight == L"public")
									{
										bIsPublic = true;
									}
									else
									{
										bIsPublic = false;
									}
									break;
								}
							}
							break;
						}
					}
					//插入树视图节点
					int ImageIndex = bIsPublic ? 3 : 4;
					m_wndClassView.InsertItem(FunctionInfo + L": " + ReturnTypeName, ImageIndex, ImageIndex, hClassItem);
				}
			}
			int iLastField;
			for (iLastField = iFirstFunction; (iLastField>iClass)&&(Content[iLastField]!=L';'); iLastField--)
			{
			}
			iLastField++;
			for (int i = iClass; i < iLastField; i++)
			{
				if (Content[i] == L';')
				{
					int LineBegin;
					for (LineBegin = i; (LineBegin > iClass) && (Content[LineBegin] != L'\n'); LineBegin--)
					{
					}
					LineBegin++;
					for (; (LineBegin < i)&&(isspace(Content[LineBegin])); LineBegin++)
					{
					}
					CString ThisLine = Content.Mid(LineBegin, i - LineBegin + 1);
					bool bFieldInited = false;
					if (ThisLine.Find(L'=') != -1)
					{
						bFieldInited = true;
					}
					bool bIsPublic = false;
					CString FieldName;
					CString FieldType;
					for (int iAccessRightEnd = LineBegin; iAccessRightEnd < i; iAccessRightEnd++)
					{
						if (isspace(Content[iAccessRightEnd]))
						{
							CString AccessRight = Content.Mid(LineBegin, iAccessRightEnd - LineBegin);
							if (AccessRight == L"public")
							{
								bIsPublic = true;
							}
							else
							{
								bIsPublic = false;
							}
							break;
						}
					}
					if (bFieldInited)
					{
						for (int iFieldNameEnd = i; iFieldNameEnd > LineBegin; iFieldNameEnd--)
						{
							if (Content[iFieldNameEnd] == L'=')
							{
								bool bHasFoundType = false;
								int iTypeIndex = 0;
								for (int iFieldNameBegin = iFieldNameEnd; iFieldNameBegin > LineBegin; iFieldNameBegin++)
								{
									if (isspace(Content[iFieldNameBegin]))
									{
										if (!bHasFoundType)
										{
											bHasFoundType = true;
											iTypeIndex = iFieldNameBegin;
											continue;
										}
										else
										{
											FieldName = Content.Mid(iTypeIndex + 1, iFieldNameEnd - iTypeIndex);
											FieldType = Content.Mid(iFieldNameBegin, iTypeIndex - iFieldNameBegin + 1);
											break;
										}
									}
								}
							}
						}
					}
					else
					{
						bool bHasFoundType = false;
						int iTypeIndex = 0;
						for (int iFieldNameBegin = i; iFieldNameBegin > LineBegin; iFieldNameBegin--)
						{
							if (isspace(Content[iFieldNameBegin]))
							{
								if (!bHasFoundType)
								{
									bHasFoundType = true;
									iTypeIndex = iFieldNameBegin;
									continue;
								}
								else
								{
									FieldName = Content.Mid(iTypeIndex + 1, i - iTypeIndex - 1);
									FieldType = Content.Mid(iFieldNameBegin + 1, iTypeIndex - iFieldNameBegin - 1);
									break;
								}
							}
						}
					}
					//插入树视图节点
					int ImageIndex = bIsPublic ? 5 : 5;
					m_wndClassView.InsertItem(FieldName + L": " + FieldType, ImageIndex, ImageIndex, hClassItem, TVI_FIRST);
				}
			}
		}
	}*/
	
}

void CClassView::ParseInnerClass(CString Content, HTREEITEM hParentItem, int nOffset, CString NameSpace)
{
	int len = Content.GetLength();
	CString Backup = Content;
	{
		HTREEITEM hLastClass = TVI_FIRST;
		int iClass = -1;
		int iLast;
		while ((iClass = Content.Find(L"class", iClass + 1)) != -1)
		{
			int iFirstFunction = -1;//Fields are always decleared before functions
			int iClassContentBegin = -1;
			bool bIsAbstract;
			if ((iClass==0?(false):(!(Content[iClass-1]==L'@'||isspace(Content[iClass - 1])))) || !isspace(Content[iClass + 5]))
			{
				continue;
			}
			HTREEITEM hClassItem = nullptr;
			CString ClassName;
			{
				CHARRANGE Range;
				CHARRANGE Scope;
				int nBraceCount = 0;
				int iInnerStartIndex;
				Scope.cpMin = iClass;
				for (int i = iClass; i < len; i++)
				{
					if (Content[i] == L'{')
					{
						if (nBraceCount == 0)
						{
							ClassName = Content.Mid(iClass + 6, Content.Find(L' ', iClass + 6) - iClass - 6);
							Range.cpMin = iClass + 6 + nOffset;
							Range.cpMax = Content.Find(L' ', iClass + 6) + nOffset;
							iClassContentBegin = i;
							//Scope.cpMin = i;
						}
						if (nBraceCount == 1)
						{
							iInnerStartIndex = i + 1;
						}
						nBraceCount++;
					}
					if (Content[i] == L'}')
					{
						if (nBraceCount == 1)
						{
							iLast = i;
							Scope.cpMax = i;
							break;
						}
						if (nBraceCount == 2)
						{
							for (int Index = iInnerStartIndex; Index < i-1; Index++)
							{
//								if(!isspace(Content[Index]))
									Content.SetAt(Index, L' ');
							}
						}
						nBraceCount--;
					}
				}
				int iAbstract = iClass - 1;
				int iAbstractEnd = iClass;
				int Linestart = iClass;
				for (; (Linestart >= 0) && (Content[Linestart] != L'\n'); Linestart--)
				{
				}
				Linestart++;
				for (;isspace(Content[Linestart]); Linestart++)
				{
				}/*
				while (true)
				{
					for (; isspace(Content[iAbstract]); iAbstract--)
					{
					}
					for (; (iAbstract >= Linestart) && !isspace(Content[iAbstract]); iAbstract--)
					{
					}
					CString AbstractiDef = Content.Mid(iAbstract + 1, iAbstractEnd - iAbstract - 1);
					AbstractiDef = AbstractiDef.Trim();
					iAbstractEnd = iAbstract;
					if (AbstractiDef == L"abstract")
					{
						bIsAbstract = true;
						break;
					}
					else
					{
						bIsAbstract = false;
						if (AbstractiDef == L"static" || AbstractiDef == L"final" || AbstractiDef == L"private" || AbstractiDef == L"public" || AbstractiDef == L"protected")
						{
							if (iAbstract == Linestart - 1)
								break;
						}
					}
				}*/
				CString Str = Content.Mid(Linestart, iClass - Linestart + 1);
				if (Str.Find(L"abstract") != -1)
				{
					bIsAbstract = true;
				}
				else
				{
					bIsAbstract = false;
				}
				//插入树视图节点
				hClassItem = m_wndClassView.InsertItem(ClassName, 1, 1, hParentItem, hLastClass);
				hLastClass = hClassItem;
				m_mapItemRange[hClassItem] = Range;
				//m_mapScopeRange[hClassItem][NameSpace + ClassName] = Scope;
				m_mapScopeRange[hClassItem] = { NameSpace + ClassName ,Scope };
			}

			int iFunction = iClass + 1;
			while (((iFunction = Content.Find(L')', iFunction + 1)) != -1) && (iFunction <= iLast))
			{
				bool IsDefiningFunc = false;
				CHARRANGE Scope;
				for (int i = iFunction + 1; i <= iLast; i++)
				{
					if (!isspace(Content[i]))
					{
						if (Content[i] == L'{' || Content[i] == L't')
						{
							IsDefiningFunc = true;
							int nBraceCount = 0;
							int iLastBrace;
							for (iLastBrace = iFunction + 1; iLastBrace < iLast; iLastBrace++)
							{
								if (Content[iLastBrace] == L'{')
								{
									/*if (nBraceCount == 0)
									{
										Scope.cpMin = iLastBrace;
									}*/
									nBraceCount++;
								}
								if (Content[iLastBrace] == L'}')
								{
									if (nBraceCount == 1)
									{
										Scope.cpMax = iLastBrace;
										break;
									}
									else
									{
										nBraceCount--;
									}
								}
							}
							if (nBraceCount == 0)
							{
								VERIFY(bIsAbstract);
								break;
							}
							for (int iChar = i + 1; iChar < iLastBrace; iChar++)
							{
								Content.SetAt(iChar, L' ');
							}
							break;
						}
						if(bIsAbstract&&(Content[i]==L';'))
						{
							int iLineBegin;
							for (iLineBegin = i; Content[iLineBegin] != L'\n'; iLineBegin--)
							{
							}
							iLineBegin++;
							//Scope.cpMin = iLineBegin;
							CString ThisLine = Content.Mid(iLineBegin, i - iLineBegin);
							if (ThisLine.Find(L'=') == -1)
							{
								IsDefiningFunc = true;
							}
							else
							{
								IsDefiningFunc = false;
							}
						}
						break;
					}
				}
				if (IsDefiningFunc)
				{
					CString FunctionInfo;
					CHARRANGE Range;
					CString ReturnTypeName;
					bool bIsPublic;
					bool bHasVisitedArgList = false;
					for (int iFuncArgListBegin = iFunction; iFuncArgListBegin > iClass; iFuncArgListBegin--)
					{
						if (Content[iFuncArgListBegin] == L'(')
						{
							bHasVisitedArgList = true;
							Range.cpMax = iFuncArgListBegin + nOffset;
						}
						if ((bHasVisitedArgList) && (isspace(Content[iFuncArgListBegin])))
						{
							FunctionInfo = Content.Mid(iFuncArgListBegin + 1, iFunction - iFuncArgListBegin);
							Range.cpMin = iFuncArgListBegin + 1 + nOffset;
							for (int i = iFuncArgListBegin - 1; i > iClass; i--)
							{
								if (isspace(Content[i]))
								{
									ReturnTypeName = Content.Mid(i + 1, iFuncArgListBegin - i);
									ReturnTypeName = ReturnTypeName.TrimRight();
									if (ReturnTypeName == L"public")
									{
										ReturnTypeName = IsInChinese() ? L"构造函数" : L"Constructor";
										bIsPublic = true;
									}
									else if (ReturnTypeName == L"protected" || ReturnTypeName == L"private")
									{
										ReturnTypeName = IsInChinese() ? L"构造函数" : L"Constructor";
										bIsPublic = false;
									}
									else
									{
										bIsPublic = false;
									}
									break;
								}
							}
							int iLineBegin;
							for (iLineBegin = iFuncArgListBegin; Content[iLineBegin] != '\n'; iLineBegin--)
							{
							}
							for (; (iLineBegin < iFuncArgListBegin) && (isspace(Content[iLineBegin])); iLineBegin++)
							{
							}
							if (iFirstFunction == -1)
							{
								iFirstFunction = iLineBegin;
							}
							Scope.cpMin = iLineBegin;
							for (int iAccessRightEnd = iLineBegin; iAccessRightEnd < iFuncArgListBegin; iAccessRightEnd++)
							{
								if (isspace(Content[iAccessRightEnd]))
								{
									CString AccessRight = Content.Mid(iLineBegin, iAccessRightEnd - iLineBegin);
									if (AccessRight == L"public")
									{
										bIsPublic = true;
									}
									else
									{
										bIsPublic = false;
									}
									break;
								}
							}
							break;
						}
					}
					if (FunctionInfo[0]==L'(')
					{
						continue;
					}
					//插入树视图节点
					int ImageIndex = bIsPublic ? 4 : 5;
					HTREEITEM hItem = m_wndClassView.InsertItem(FunctionInfo + L" : " + ReturnTypeName, ImageIndex, ImageIndex, hClassItem);
					m_mapItemRange[hItem] = Range;
					//m_mapScopeRange[hItem][NameSpace + ClassName + L'.' + FunctionInfo] = Scope;
					m_mapScopeRange[hItem] = { NameSpace + ClassName + L'.' + FunctionInfo ,Scope };
				}
			}
			if (iFirstFunction == -1)
			{
				iFirstFunction = iLast;
			}
			int iLastField;
			HTREEITEM hLastField = TVI_FIRST;
			for (iLastField = iFirstFunction; (iLastField>iClass) && (Content[iLastField] != L';'); iLastField--)
			{
			}
			iLastField++;
			for (int i = iClass; i < iLastField; i++)
			{
				if (Content[i] == L';')
				{
					int LineBegin;
					for (LineBegin = i; (LineBegin > iClass) && (Content[LineBegin] != L'\n'); LineBegin--)
					{
					}
					LineBegin++;
					for (; (LineBegin < i) && (isspace(Content[LineBegin])); LineBegin++)
					{
					}
					CString ThisLine = Content.Mid(LineBegin, i - LineBegin + 1);
					bool bFieldInited = false;
					if (ThisLine.Find(L'=') != -1)
					{
						bFieldInited = true;
					}
					bool bIsPublic = false;
					CString FieldName;
					CString FieldType;
					CHARRANGE Range;
					for (int iAccessRightEnd = LineBegin; iAccessRightEnd < i; iAccessRightEnd++)
					{
						if (isspace(Content[iAccessRightEnd]))
						{
							CString AccessRight = Content.Mid(LineBegin, iAccessRightEnd - LineBegin);
							if (AccessRight == L"public")
							{
								bIsPublic = true;
							}
							else
							{
								bIsPublic = false;
							}
							break;
						}
					}
					if (bFieldInited)
					{
						for (int iFieldNameEnd = i; iFieldNameEnd > LineBegin; iFieldNameEnd--)
						{
							if (Content[iFieldNameEnd] == L'=')
							{
								bool bHasFoundType = false;
								int iTypeIndex = 0;
								int iFieldNameBegin = iFieldNameEnd - 1;
								for (; (iFieldNameBegin > LineBegin)&&(isspace(Content[iFieldNameBegin])); iFieldNameBegin--)
								{
								}
								iFieldNameEnd = iFieldNameBegin + 1;
								for (; iFieldNameBegin >= LineBegin-1; iFieldNameBegin--)
								{
									if (isspace(Content[iFieldNameBegin]))
									{
										if (!bHasFoundType)
										{
											bHasFoundType = true;
											iTypeIndex = iFieldNameBegin;
											continue;
										}
										else
										{
											FieldName = Content.Mid(iTypeIndex + 1, iFieldNameEnd - iTypeIndex - 1);
											FieldType = Content.Mid(iFieldNameBegin + 1, iTypeIndex - iFieldNameBegin);
											Range.cpMin = iTypeIndex + 1 + nOffset;
											Range.cpMax = iFieldNameEnd + nOffset;
											break;
										}
									}
								}
								break;
							}
						}
					}
					else
					{
						bool bHasFoundType = false;
						int iTypeIndex = 0;
						for (int iFieldNameBegin = i; iFieldNameBegin > iClass; iFieldNameBegin--)
						{
							if (isspace(Content[iFieldNameBegin]))
							{
								if (!bHasFoundType)
								{
									bHasFoundType = true;
									iTypeIndex = iFieldNameBegin;
									continue;
								}
								else
								{
									FieldName = Content.Mid(iTypeIndex + 1, i - iTypeIndex - 1);
									FieldType = Content.Mid(iFieldNameBegin + 1, iTypeIndex - iFieldNameBegin - 1);
									Range.cpMin = iTypeIndex + 1 + nOffset;
									Range.cpMax = i + nOffset;
									break;
								}
							}
						}
					}
					//插入树视图节点
					int ImageIndex = bIsPublic ? 6 : 7;
					HTREEITEM hItem = m_wndClassView.InsertItem(FieldName + L" : " + FieldType, ImageIndex, ImageIndex, hClassItem, hLastField);
					hLastField = hItem;
					m_mapItemRange[hItem] = Range;
				}
			}
			if (iClassContentBegin != -1)
			{
				ParseInnerClass(Backup.Mid(iClass + 1, iLast - iClass - 1), hClassItem, iClass + 1, NameSpace + ClassName + L'.');
			}
			iClass = iLast;
		}



		//Another one

		int iInterface = -1;
		while ((iInterface = Content.Find(L"interface", iInterface + 1)) != -1)
		{
			int iFirstFunction = -1;//Fields are always decleared before functions
			int iInterfaceContentBegin = -1;
			bool bIsAbstract = true;
			if ((iInterface==0?(false):(!(Content[iInterface-1]==L'@'||isspace(Content[iInterface - 1])))) || !isspace(Content[iInterface + 9]))
			{
				continue;
			}
			HTREEITEM hClassItem = nullptr;
			CString ClassName;
			{
				CHARRANGE Range;
				CHARRANGE Scope{ iInterface,0 };
				int nBraceCount = 0;
				int iInnerStartIndex;
				for (int i = iInterface; i < len; i++)
				{
					if (Content[i] == L'{')
					{
						if (nBraceCount == 0)
						{
							ClassName = Content.Mid(iInterface + 10, Content.Find(L' ', iInterface + 10) - iInterface - 9);
							Range.cpMin = iInterface + 10 + nOffset;
							Range.cpMax = Content.Find(L' ', iInterface + 10) + nOffset;
							iInterfaceContentBegin = i;
						}
						if (nBraceCount == 1)
						{
							iInnerStartIndex = i + 1;
						}
						nBraceCount++;
					}
					if (Content[i] == L'}')
					{
						if (nBraceCount == 1)
						{
							iLast = i;
							Scope.cpMin = i;
							break;
						}
						if (nBraceCount == 2)
						{
							for (int Index = iInnerStartIndex; Index < i - 1; Index++)
							{
//								if (!isspace(Content[Index]))
									Content.SetAt(Index, L' ');
							}
						}
						nBraceCount--;
					}
				}
				//插入树视图节点
				hClassItem = m_wndClassView.InsertItem(ClassName, 2, 2, hParentItem, hLastClass);
				hLastClass = hClassItem;
				m_mapItemRange[hClassItem] = Range;
			}

			int iFunction = iInterface + 1;
			while (((iFunction = Content.Find(L')', iFunction + 1)) != -1) && (iFunction <= iLast))
			{
				bool IsDefiningFunc = false;
				for (int i = iFunction + 1; i <= iLast; i++)
				{
					if (!isspace(Content[i]))
					{
						if (bIsAbstract && (Content[i] == L';'|| Content[i] == L't'))
						{
							int iLineBegin;
							for (iLineBegin = i; Content[iLineBegin] != L'\n'; iLineBegin--)
							{
							}
							iLineBegin++;
							CString ThisLine = Content.Mid(iLineBegin, i - iLineBegin);
							if (ThisLine.Find(L'=') == -1)
							{
								IsDefiningFunc = true;
							}
							else
							{
								IsDefiningFunc = false;
							}
						}
						break;
					}
				}
				if (IsDefiningFunc)
				{
					CString FunctionInfo;
					CHARRANGE Range;
					CString ReturnTypeName;
					bool bIsPublic = true;
					bool bHasVisitedArgList = false;
					for (int iFuncArgListBegin = iFunction; iFuncArgListBegin > iInterface; iFuncArgListBegin--)
					{
						if (Content[iFuncArgListBegin] == L'(')
						{
							bHasVisitedArgList = true;
							Range.cpMax = iFuncArgListBegin + nOffset;
						}
						if ((bHasVisitedArgList) && (isspace(Content[iFuncArgListBegin])))
						{
							FunctionInfo = Content.Mid(iFuncArgListBegin + 1, iFunction - iFuncArgListBegin);
							Range.cpMin = iFuncArgListBegin + 1 + nOffset;
							for (int i = iFuncArgListBegin - 1; i > iInterface; i--)
							{
								if (isspace(Content[i]))
								{
									ReturnTypeName = Content.Mid(i + 1, iFuncArgListBegin - i);
									ReturnTypeName = ReturnTypeName.TrimRight();
									if (ReturnTypeName == L"public")
									{
										ReturnTypeName = IsInChinese() ? L"构造函数" : L"Constructor";
										bIsPublic = true;
									}
									else if (ReturnTypeName == L"protected" || ReturnTypeName == L"private")
									{
										ReturnTypeName = IsInChinese() ? L"构造函数" : L"Constructor";
										bIsPublic = false;
									}
									else
									{
										bIsPublic = false;
									}
									break;
								}
							}
							int iLineBegin;
							for (iLineBegin = iFuncArgListBegin; Content[iLineBegin] != '\n'; iLineBegin--)
							{
							}
							for (; (iLineBegin < iFuncArgListBegin) && (isspace(Content[iLineBegin])); iLineBegin++)
							{
							}
							if (iFirstFunction == -1)
							{
								iFirstFunction = iLineBegin;
							}
							for (int iAccessRightEnd = iLineBegin; iAccessRightEnd < iFuncArgListBegin; iAccessRightEnd++)
							{
								if (isspace(Content[iAccessRightEnd]))
								{
									CString AccessRight = Content.Mid(iLineBegin, iAccessRightEnd - iLineBegin);
									if (AccessRight == L"private"||AccessRight==L"protected")
									{
										bIsPublic = false;
									}
									else
									{
										bIsPublic = true;
									}
									break;
								}
							}
							break;
						}
					}
					if (FunctionInfo[0] == L'(')
					{
						continue;
					}
					//插入树视图节点
					int ImageIndex = bIsPublic ? 4 : 5;
					HTREEITEM hItem = m_wndClassView.InsertItem(FunctionInfo + L" : " + ReturnTypeName, ImageIndex, ImageIndex, hClassItem);
					m_mapItemRange[hItem] = Range;
				}
			}
			if (iFirstFunction == -1)
			{
				iFirstFunction = iLast;
			}
			int iLastField;
			HTREEITEM hLastField = TVI_FIRST;
			for (iLastField = iFirstFunction; (iLastField>iInterface) && (Content[iLastField] != L';'); iLastField--)
			{
			}
			iLastField++;
			for (int i = iInterface; i < iLastField; i++)
			{
				if (Content[i] == L';')
				{
					int LineBegin;
					for (LineBegin = i; (LineBegin > iInterface) && (Content[LineBegin] != L'\n'); LineBegin--)
					{
					}
					LineBegin++;
					for (; (LineBegin < i) && (isspace(Content[LineBegin])); LineBegin++)
					{
					}
					CString ThisLine = Content.Mid(LineBegin, i - LineBegin + 1);
					bool bFieldInited = false;
					if (ThisLine.Find(L'=') != -1)
					{
						bFieldInited = true;
					}
					bool bIsPublic = false;
					CString FieldName;
					CString FieldType;
					CHARRANGE Range;
					for (int iAccessRightEnd = LineBegin; iAccessRightEnd < i; iAccessRightEnd++)
					{
						if (isspace(Content[iAccessRightEnd]))
						{
							CString AccessRight = Content.Mid(LineBegin, iAccessRightEnd - LineBegin);
							if (AccessRight == L"public")
							{
								bIsPublic = true;
							}
							else
							{
								bIsPublic = false;
							}
							break;
						}
					}
					if (bFieldInited)
					{
						for (int iFieldNameEnd = i; iFieldNameEnd > LineBegin; iFieldNameEnd--)
						{
							if (Content[iFieldNameEnd] == L'=')
							{
								bool bHasFoundType = false;
								int iTypeIndex = 0;
								int iFieldNameBegin = iFieldNameEnd - 1;
								for (; (iFieldNameBegin > LineBegin) && (isspace(Content[iFieldNameBegin])); iFieldNameBegin--)
								{
								}
								iFieldNameEnd = iFieldNameBegin + 1;
								for (; iFieldNameBegin >= LineBegin - 1; iFieldNameBegin--)
								{
									if (isspace(Content[iFieldNameBegin]))
									{
										if (!bHasFoundType)
										{
											bHasFoundType = true;
											iTypeIndex = iFieldNameBegin;
											continue;
										}
										else
										{
											FieldName = Content.Mid(iTypeIndex + 1, iFieldNameEnd - iTypeIndex - 1);
											FieldType = Content.Mid(iFieldNameBegin + 1, iTypeIndex - iFieldNameBegin);
											Range.cpMin = iTypeIndex + 1 + nOffset;
											Range.cpMax = iFieldNameEnd + nOffset;
											break;
										}
									}
								}
								break;
							}
						}
					}
					else
					{
						bool bHasFoundType = false;
						int iTypeIndex = 0;
						for (int iFieldNameBegin = i; iFieldNameBegin > LineBegin; iFieldNameBegin--)
						{
							if (isspace(Content[iFieldNameBegin]))
							{
								if (!bHasFoundType)
								{
									bHasFoundType = true;
									iTypeIndex = iFieldNameBegin;
									continue;
								}
								else
								{
									FieldName = Content.Mid(iTypeIndex + 1, i - iTypeIndex - 1);
									FieldType = Content.Mid(iFieldNameBegin + 1, iTypeIndex - iFieldNameBegin - 1);
									Range.cpMin = iTypeIndex + 1 + nOffset;
									Range.cpMax = i + nOffset;
									break;
								}
							}
						}
					}
					//插入树视图节点
					int ImageIndex = bIsPublic ? 6 : 7;
					HTREEITEM hItem = m_wndClassView.InsertItem(FieldName + L" : " + FieldType, ImageIndex, ImageIndex, hClassItem, hLastField);
					hLastField = hItem;
					m_mapItemRange[hItem] = Range;
				}
			}
			if (iInterfaceContentBegin != -1)
			{
				ParseInnerClass(Backup.Mid(iInterface + 1, iLast - iInterface - 1), hClassItem, iInterface + 1, NameSpace + ClassName + L'.');
			}
			iInterface = iLast;
		}
	}
}

void CClassView::OnContextMenu(CWnd* pWnd, CPoint point)
{
	CTreeCtrl* pWndTree = (CTreeCtrl*)&m_wndClassView;
	ASSERT_VALID(pWndTree);

	if (pWnd != pWndTree)
	{
		CDockablePane::OnContextMenu(pWnd, point);
		return;
	}

	if (point != CPoint(-1, -1))
	{
		// 选择已单击的项: 
		CPoint ptTree = point;
		pWndTree->ScreenToClient(&ptTree);

		UINT flags = 0;
		HTREEITEM hTreeItem = pWndTree->HitTest(ptTree, &flags);
		if (hTreeItem != NULL)
		{
			pWndTree->SelectItem(hTreeItem);
		}
	}

	pWndTree->SetFocus();
	CMenu menu;
	menu.LoadMenu(IDR_POPUP_SORT);

	CMenu* pSumMenu = menu.GetSubMenu(0);

	if (AfxGetMainWnd()->IsKindOf(RUNTIME_CLASS(CMDIFrameWndEx)))
	{
		CMFCPopupMenu* pPopupMenu = new CMFCPopupMenu;

		if (!pPopupMenu->Create(this, point.x, point.y, (HMENU)pSumMenu->m_hMenu, FALSE, TRUE))
			return;

		((CMDIFrameWndEx*)AfxGetMainWnd())->OnShowPopupMenu(pPopupMenu);
		UpdateDialogControls(this, FALSE);
	}
}

void CClassView::AdjustLayout()
{
	if (GetSafeHwnd() == NULL)
	{
		return;
	}

	CRect rectClient;
	GetClientRect(rectClient);

	int cyTlb = m_wndToolBar.CalcFixedLayout(FALSE, TRUE).cy;

	m_wndToolBar.SetWindowPos(NULL, rectClient.left, rectClient.top, rectClient.Width(), cyTlb, SWP_NOACTIVATE | SWP_NOZORDER);
	m_wndClassView.SetWindowPos(NULL, rectClient.left + 1, rectClient.top + cyTlb + 1, rectClient.Width() - 2, rectClient.Height() - cyTlb - 2, SWP_NOACTIVATE | SWP_NOZORDER);
	m_wndStaticText.SetWindowPos(NULL, rectClient.left + 1, rectClient.top + cyTlb + 1, rectClient.Width() - 2, rectClient.Height() - cyTlb - 2, SWP_NOACTIVATE | SWP_NOZORDER);
	if (theApp.Md5ofFile==L"")
	{
		m_wndClassView.ShowWindow(SW_HIDE);
		m_wndStaticText.ShowWindow(SW_SHOW);
	}
	else
	{
		m_wndStaticText.ShowWindow(SW_HIDE);
		m_wndClassView.ShowWindow(SW_SHOW);
	}
}

BOOL CClassView::PreTranslateMessage(MSG* pMsg)
{
	return CDockablePane::PreTranslateMessage(pMsg);
}

void CClassView::OnSort(UINT id)
{
	if (m_nCurrSort == id)
	{
		return;
	}

	m_nCurrSort = id;

	CClassViewMenuButton* pButton = DYNAMIC_DOWNCAST(CClassViewMenuButton, m_wndToolBar.GetButton(0));

	if (pButton != NULL)
	{
		pButton->SetImage(GetCmdMgr()->GetCmdImage(id));
		m_wndToolBar.Invalidate();
		m_wndToolBar.UpdateWindow();
	}
}

void CClassView::OnUpdateSort(CCmdUI* pCmdUI)
{
	//pCmdUI->SetCheck(pCmdUI->m_nID == m_nCurrSort);
	pCmdUI->Enable(FALSE);
}

void CClassView::OnClassAddMemberFunction()
{
	AfxMessageBox(_T("添加成员函数..."));
}

void CClassView::OnClassAddMemberVariable()
{
	// TODO: 在此处添加命令处理程序代码
}

void CClassView::OnClassDefinition()
{
	// TODO: 在此处添加命令处理程序代码
}

void CClassView::OnClassProperties()
{
	// TODO: 在此处添加命令处理程序代码
}

void CClassView::OnNewFolder()
{
	//AfxMessageBox(_T("新建文件夹..."));
	int ImgIndex, SelectedImgIndex;
	if (m_wndClassView.GetItemImage(m_wndClassView.GetSelectedItem(), ImgIndex, SelectedImgIndex))
	{
		CStringW AddStr;
		if (ImgIndex == 1)
		{
			AddStr = L".java";
		}
		CStringW Path;
		std::stack<CStringW> StringStack;
		char * UserProFile;
		size_t size;
		if (_dupenv_s(&UserProFile, &size, "USERPROFILE"))
		{
			AfxGetMainWnd()->MessageBox(IsInChinese() ? _T("搜索%USERPROFILE%失败!") : _T("Failed to access %USERPROFILE%"), IsInChinese() ? _T("错误") : _T("Error"), MB_ICONERROR);
			return;
		}
		Path = UserProFile;
		Path += "\\AppData\\Local\\FernFlowerUI\\Cache\\";
		Path += theApp.Md5ofFile;
		Path += "\\JarCache\\";
		free(UserProFile);
		HTREEITEM hItem = m_wndClassView.GetSelectedItem();
		StringStack.push(m_wndClassView.GetItemText(hItem) + AddStr);
		while (hItem = m_wndClassView.GetParentItem(hItem))
		{
			StringStack.push(m_wndClassView.GetItemText(hItem) + L"\\");
		}
		while (!StringStack.empty())
		{
			Path += StringStack.top();
			StringStack.pop();
		}
		/*CStringA Cmd = "cmd /c explorer /select,\"";
		Cmd += Path;
		Cmd += "\"";
		WinExec(Cmd, SW_HIDE);**/
		LPITEMIDLIST idlChild;
		theApp.GetShellManager()->ItemFromPath(Path, idlChild);
		auto OpenThread=[](LPITEMIDLIST pidl){
			SHOpenFolderAndSelectItems(pidl, 0, nullptr, 0);
			theApp.GetShellManager()->FreeItem(pidl);
		};
		std::thread thrd = std::thread(OpenThread, idlChild);
		thrd.detach();
	}
	else
	{

	}
}

void CClassView::OnPaint()
{
	CPaintDC dc(this); // 用于绘制的设备上下文

	CRect rectTree;
	m_wndClassView.GetWindowRect(rectTree);
	ScreenToClient(rectTree);

	rectTree.InflateRect(1, 1);
	dc.Draw3dRect(rectTree, ::GetSysColor(COLOR_3DSHADOW), ::GetSysColor(COLOR_3DSHADOW));
}

void CClassView::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);

	m_wndClassView.SetFocus();
}

void CClassView::OnChangeVisualStyle()
{
	m_ClassViewImages.DeleteImageList();

	UINT uiBmpId = theApp.m_bHiColorIcons ? IDB_CLASS_VIEW_24 : IDB_CLASS_VIEW;

	CBitmap bmp;
	if (!bmp.LoadBitmap(uiBmpId))
	{
		TRACE(_T("无法加载位图: %x\n"), uiBmpId);
		ASSERT(FALSE);
		return;
	}

	BITMAP bmpObj;
	bmp.GetBitmap(&bmpObj);

	UINT nFlags = ILC_MASK;

	nFlags |= (theApp.m_bHiColorIcons) ? ILC_COLOR24 : ILC_COLOR4;

	m_ClassViewImages.Create(16, bmpObj.bmHeight, nFlags, 0, 0);
	m_ClassViewImages.Add(&bmp, RGB(255, 0, 0));

	m_wndClassView.SetImageList(&m_ClassViewImages, TVSIL_NORMAL);

	m_wndToolBar.CleanUpLockedImages();
	m_wndToolBar.LoadBitmap(theApp.m_bHiColorIcons ? IDB_SORT_24 : IDR_SORT, 0, 0, TRUE /* 锁定*/);
}

void CClassView::SearchClass(CStringW Path, HTREEITEM hParent)
{
	CStringW ThisPath = Path + L"\\*.*";
	CFileFind Find;
	bool b = Find.FindFile(ThisPath);
	while (b)
	{
		b = Find.FindNextFileW();
		Find.MatchesMask(FILE_ATTRIBUTE_DIRECTORY);
		if (!Find.IsDots())
		{
			CStringW Class = Find.GetFilePath();
			if (Find.IsDirectory())
			{
				SearchClass(Class, m_wndClassView.InsertItem(Find.GetFileTitle(), 3, 3, hParent));
			}
		}
	}
	Find.Close();
	b = Find.FindFile(ThisPath);
	while (b)
	{
		b = Find.FindNextFileW();
		if (!Find.IsDots())
		{
			CStringW Class = Find.GetFilePath();
			if (!Find.IsDirectory())
			{
				CStringW strTmp = Class.Right(5);
				strTmp.MakeLower();
				if (strTmp.CompareNoCase(L".java") == 0)
				{
					ParseClasses(Find.GetFilePath(), m_wndClassView.InsertItem(Find.GetFileTitle(), 0, 0, hParent));
					lstjavaFileName.push_back(Find.GetFilePath());
				}
				else
				{
					SHFILEINFO sfi;
					if (SHGetFileInfo(Find.GetFilePath(), 0, &sfi, sizeof sfi, SHGFI_ICON | SHGFI_SMALLICON | SHGFI_SYSICONINDEX))
					{
						m_mapSysImageId[sfi.iIcon] = m_ClassViewImages.Add(sfi.hIcon);
						m_wndClassView.InsertItem(Find.GetFileName(), m_mapSysImageId[sfi.iIcon], m_mapSysImageId[sfi.iIcon],hParent);
					}
				}
			}
		}
	}
	Find.Close();
}

void CClassView::ShowClassView(const CStringW & Path)
{
	m_wndClassView.DeleteAllItems();
	m_mapItemRange.clear();
	CStringW ThisPath = Path + L"\\*.*";
	CFileFind Find;
	bool b = Find.FindFile(ThisPath);
	while (b)
	{
		b = Find.FindNextFileW();
		Find.MatchesMask(FILE_ATTRIBUTE_DIRECTORY);
		if (!Find.IsDots())
		{
			CStringW Class = Find.GetFilePath();
			if (Find.IsDirectory())
			{
				SearchClass(Class, m_wndClassView.InsertItem(Find.GetFileTitle(), 3, 3));
			}
		}
	}
	Find.Close();
	b = Find.FindFile(ThisPath);
	while (b)
	{
		b = Find.FindNextFileW();
		if (!Find.IsDots())
		{
			CStringW Class = Find.GetFilePath();
			if (!Find.IsDirectory())
			{
				CStringW strTmp = Class.Right(5);
				strTmp.MakeLower();
				if (strTmp.CompareNoCase(L".java") == 0)
				{
					ParseClasses(Find.GetFilePath(), m_wndClassView.InsertItem(Find.GetFileTitle(), 0, 0));
					lstjavaFileName.push_back(Find.GetFilePath());
				}
				else
				{
					SHFILEINFO sfi;
					if (SHGetFileInfo(Find.GetFilePath(), 0, &sfi, sizeof sfi, SHGFI_ICON | SHGFI_SMALLICON | SHGFI_SYSICONINDEX))
					{
						m_mapSysImageId[sfi.iIcon] = m_ClassViewImages.Add(sfi.hIcon);
						m_wndClassView.InsertItem(Find.GetFileName(), m_mapSysImageId[sfi.iIcon], m_mapSysImageId[sfi.iIcon]);
					}
				}
			}
		}
	}
	Find.Close();
}
