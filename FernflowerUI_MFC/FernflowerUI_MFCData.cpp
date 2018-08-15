#include "stdafx.h"
#include "FernflowerUI_MFCData.h"
#include "CommonWrapper.h"

void DecomplieOptions::InitTooltip()
{
	TooltipMap=
	{
		{ L"rbr" ,IsInChinese() ? L"隐藏Java桥梁方法" : L"Hide bridge methods" },
		{ L"rsy" ,IsInChinese() ? L"隐藏synthetic成员" : L"Hide synthetic class members" },
		{ L"din" ,IsInChinese() ? L"反编译嵌套类" : L"Decompile inner classes" },
		{ L"dc4" ,IsInChinese() ? L"取消对Java1.4类的引用" : L"Collapse 1.4 class references" },
		{ L"das" ,IsInChinese() ? L"启用反编译断言" : L"Decompile assertions" },
		{ L"hes" ,IsInChinese() ? L"隐藏空白基类调用" : L"Hide empty super invocation" },
		{ L"hdc" ,IsInChinese() ? L"隐藏空白默认构造函数" : L"Hide empty default constructor" },
		{ L"dgs" ,IsInChinese() ? L"反编译通用签名" : L"Decompile generic signatures" },
		{ L"ner" ,IsInChinese() ? L"假设函数的返回不抛出异常" : L"Assume return not throwing exceptions" },
		{ L"den" ,IsInChinese() ? L"反编译枚举类型" : L"Decompile enumerations" },
		{ L"rgn" ,IsInChinese() ? L"当在new语句中,隐藏getClass()的调用" : L"Remove getClass() invocation, when it is part of a qualified new statement" },
		{ L"lit" ,IsInChinese() ? L"\"原样\"地输出数字文字" : L"Output numeric literals \"as - is\"" },
		{ L"asc" ,IsInChinese() ? L"将非ASCII编码字符反编译为Unicode字\nP.S.若反编译代码中出现乱码,可勾选该选项并重新编译" :
		L"Encode non-ASCII characters in string and character literals as Unicode escapes\nP.S.If you have trouble reading the decompiled code,try enabling this and decompile again." },
		{ L"bto" ,IsInChinese() ? L"将int值1解释为布尔值true(为了解决Java编译器的bug)" : L"Interpret int 1 as boolean true(workaround to a compiler bug)" },
		{ L"nns" ,IsInChinese() ? L"允许不设置synthetic属性(为了解决Java编译器的bug)" : L"Allow for not set synthetic attribute" },
		{ L"uto" ,IsInChinese() ? L"将无名称类型视作java.lang.Object(为了解决Java编译器的体系缺陷)" : L"Consider nameless types as java.lang.Object (workaround to a compiler architecture flaw)" },
		{ L"udv" ,IsInChinese() ? L"如果允许,按照调试信息重新为变量命名" : L"Reconstruct variable names from debug information, if present" },
		{ L"rer" ,IsInChinese() ? L"忽略空的异常范围" : L"Remove empty exception ranges" },
		{ L"fdi" ,IsInChinese() ? L"de-inline最终结构" : L"De-inline finally structures" },
		//{ L"mpm" ,IsInChinese() ? L"" : L"" },
		{ L"ren" ,IsInChinese() ? L"重命名有二义性的类和类成员" : L"Rename ambiguous (resp. obfuscated) classes and class elements" },
		{ L"inn" ,IsInChinese() ? L"检查Intellij IDEA的@NotNull注释并尽可能移除插入的代码" : L"Check for IntelliJ IDEA-specific @NotNull annotation and remove inserted code if found" },
		{ L"lac" ,IsInChinese() ? L"将lambda表达式反编译为匿名类" : L"Decompile lambda expressions to anonymous classes" },
		{ L"nls" ,IsInChinese() ? L"选择换行符类别,不勾选表示Windows下\"\\r\\n\",勾选表示Unix下\"\\n\"" : L"define new line character to be used for output. 0 - '\\r\\n' (Windows), 1 - '\\n' (Unix)" },
		{ L"ind" ,IsInChinese() ? L"缩进字符串长度" : L"Length of the indentation string" },
		{ L"log" ,IsInChinese() ? L"日志记录等级,分为TRACE, INFO, WARN, ERROR" : L"A logging level, possible values are TRACE, INFO, WARN, ERROR" }
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
	// TODO: 在此处插入 return 语句
	for (auto it = DecompOption.OptionMap.begin(); it != DecompOption.OptionMap.end(); it++)
	{
		ar << it->first << it->second;
	}
	return ar;
}

CArchive & operator>>(CArchive & ar, DecomplieOptions & DecompOption)
{
	// TODO: 在此处插入 return 语句
	for (auto it = DecompOption.OptionMap.begin(); it != DecompOption.OptionMap.end(); it++)
	{
		CString Option, Data;
		ar >> Option >> Data;
		DecompOption.OptionMap[Option] = Data;
	}
	return ar;
}
