#pragma once

#include "stdafx.h"

struct DecomplieOptions
{
	std::map<CString, CString> OptionMap = {
		{ L"rbr" ,L"1" },
		{ L"rsy" ,L"0" },
		{ L"din" ,L"1" },
		{ L"dc4" ,L"1" },
		{ L"das" ,L"1" },
		{ L"hes" ,L"1" },
		{ L"hdc" ,L"1" },
		{ L"dgs" ,L"0" },
		{ L"ner" ,L"1" },
		{ L"den" ,L"1" },
		{ L"rgn" ,L"1" },
		{ L"lit" ,L"0" },
		{ L"asc" ,L"0" },
		{ L"bto" ,L"1" },
		{ L"nns" ,L"0" },
		{ L"uto" ,L"1" },
		{ L"udv" ,L"1" },
		{ L"rer" ,L"1" },
		{ L"fdi" ,L"1" },
		//{ L"mpm" ,L"0" },
		{ L"ren" ,L"0" },
		{ L"inn" ,L"1" },
		{ L"lac" ,L"0" },
		{ L"nls" ,L"0" },
		{ L"ind" ,L"\"     \"" },
		{ L"log" ,L"INFO" }
	};
	std::map<CString, CString> TooltipMap;
	void InitTooltip();
	CString GetOptionString();
};

CArchive & operator<<(CArchive & ar, DecomplieOptions & DecompOption);

CArchive & operator>>(CArchive & ar, DecomplieOptions & DecompOption);