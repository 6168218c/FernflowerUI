#include "stdafx.h"
#include "FernflowerUI_MFCData.h"
#include "CommonWrapper.h"

void DecomplieOptions::InitTooltip()
{
	TooltipMap=
	{
		{ L"rbr" ,IsInChinese() ? L"����Java��������" : L"Hide bridge methods" },
		{ L"rsy" ,IsInChinese() ? L"����synthetic��Ա" : L"Hide synthetic class members" },
		{ L"din" ,IsInChinese() ? L"������Ƕ����" : L"Decompile inner classes" },
		{ L"dc4" ,IsInChinese() ? L"ȡ����Java1.4�������" : L"Collapse 1.4 class references" },
		{ L"das" ,IsInChinese() ? L"���÷��������" : L"Decompile assertions" },
		{ L"hes" ,IsInChinese() ? L"���ؿհ׻������" : L"Hide empty super invocation" },
		{ L"hdc" ,IsInChinese() ? L"���ؿհ�Ĭ�Ϲ��캯��" : L"Hide empty default constructor" },
		{ L"dgs" ,IsInChinese() ? L"������ͨ��ǩ��" : L"Decompile generic signatures" },
		{ L"ner" ,IsInChinese() ? L"���躯���ķ��ز��׳��쳣" : L"Assume return not throwing exceptions" },
		{ L"den" ,IsInChinese() ? L"������ö������" : L"Decompile enumerations" },
		{ L"rgn" ,IsInChinese() ? L"����new�����,����getClass()�ĵ���" : L"Remove getClass() invocation, when it is part of a qualified new statement" },
		{ L"lit" ,IsInChinese() ? L"\"ԭ��\"�������������" : L"Output numeric literals \"as - is\"" },
		{ L"asc" ,IsInChinese() ? L"����ASCII�����ַ�������ΪUnicode��\nP.S.������������г�������,�ɹ�ѡ��ѡ����±���" :
		L"Encode non-ASCII characters in string and character literals as Unicode escapes\nP.S.If you have trouble reading the decompiled code,try enabling this and decompile again." },
		{ L"bto" ,IsInChinese() ? L"��intֵ1����Ϊ����ֵtrue(Ϊ�˽��Java��������bug)" : L"Interpret int 1 as boolean true(workaround to a compiler bug)" },
		{ L"nns" ,IsInChinese() ? L"��������synthetic����(Ϊ�˽��Java��������bug)" : L"Allow for not set synthetic attribute" },
		{ L"uto" ,IsInChinese() ? L"����������������java.lang.Object(Ϊ�˽��Java����������ϵȱ��)" : L"Consider nameless types as java.lang.Object (workaround to a compiler architecture flaw)" },
		{ L"udv" ,IsInChinese() ? L"�������,���յ�����Ϣ����Ϊ��������" : L"Reconstruct variable names from debug information, if present" },
		{ L"rer" ,IsInChinese() ? L"���Կյ��쳣��Χ" : L"Remove empty exception ranges" },
		{ L"fdi" ,IsInChinese() ? L"de-inline���սṹ" : L"De-inline finally structures" },
		//{ L"mpm" ,IsInChinese() ? L"" : L"" },
		{ L"ren" ,IsInChinese() ? L"�������ж����Ե�������Ա" : L"Rename ambiguous (resp. obfuscated) classes and class elements" },
		{ L"inn" ,IsInChinese() ? L"���Intellij IDEA��@NotNullע�Ͳ��������Ƴ�����Ĵ���" : L"Check for IntelliJ IDEA-specific @NotNull annotation and remove inserted code if found" },
		{ L"lac" ,IsInChinese() ? L"��lambda���ʽ������Ϊ������" : L"Decompile lambda expressions to anonymous classes" },
		{ L"nls" ,IsInChinese() ? L"ѡ���з����,����ѡ��ʾWindows��\"\\r\\n\",��ѡ��ʾUnix��\"\\n\"" : L"define new line character to be used for output. 0 - '\\r\\n' (Windows), 1 - '\\n' (Unix)" },
		{ L"ind" ,IsInChinese() ? L"�����ַ�������" : L"Length of the indentation string" },
		{ L"log" ,IsInChinese() ? L"��־��¼�ȼ�,��ΪTRACE, INFO, WARN, ERROR" : L"A logging level, possible values are TRACE, INFO, WARN, ERROR" }
	};
}

CString DecomplieOptions::GetOptionString()
{
	CString Result;
	for (auto it = OptionMap.begin(); it != OptionMap.end(); it++)
	{
		Result = Result + L"-" + it->first + L"=" + it->second + L" ";
	}
	return Result;
}

CArchive & operator<<(CArchive & ar, DecomplieOptions & DecompOption)
{
	// TODO: �ڴ˴����� return ���
	for (auto it = DecompOption.OptionMap.begin(); it != DecompOption.OptionMap.end(); it++)
	{
		ar << it->first << it->second;
	}
	return ar;
}

CArchive & operator>>(CArchive & ar, DecomplieOptions & DecompOption)
{
	// TODO: �ڴ˴����� return ���
	for (auto it = DecompOption.OptionMap.begin(); it != DecompOption.OptionMap.end(); it++)
	{
		CString Option, Data;
		ar >> Option >> Data;
		DecompOption.OptionMap[Option] = Data;
	}
	return ar;
}
