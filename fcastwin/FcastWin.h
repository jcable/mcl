// FcastWin.h : fichier d'en-t�te principal pour l'application PROJECT_NAME
//

#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "res/resource.h"		// symboles principaux


// CFcastWinApp�:
// Consultez FcastWin.cpp pour l'impl�mentation de cette classe
//

class CFcastWinApp : public CWinApp
{
public:
	CFcastWinApp();

// Substitutions
	public:
	virtual BOOL InitInstance();

// Impl�mentation

	DECLARE_MESSAGE_MAP()
};

extern CFcastWinApp theApp;
