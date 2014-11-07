// FcastWin.cpp : Définit les comportements de classe pour l'application.
//

#include "stdafx.h"
#include "FcastWin.h"
#include "FcastWinDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CFcastWinApp

BEGIN_MESSAGE_MAP(CFcastWinApp, CWinApp)
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()


// construction de CFcastWinApp

CFcastWinApp::CFcastWinApp()
{
	// TODO : ajoutez ici le code de la construction.
	// Placez toutes les initialisations dans InitInstance
}


// Le seul et unique objet CFcastWinApp

CFcastWinApp theApp;


// initialisation de CFcastWinApp

BOOL CFcastWinApp::InitInstance()
{
	// InitCommonControls() est requis sur Windows XP si le manifeste de l'application
	// spécifie l'utilisation de ComCtl32.dll version 6 ou ultérieure pour activer les
	// styles visuels.  Dans le cas contraire, la création de fenêtres échouera.
	InitCommonControls();

	CWinApp::InitInstance();

	AfxEnableControlContainer();
	AfxInitRichEdit2();
	CoInitialize(NULL);


	CFcastWinDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO : Placez ici le code définissant le comportement lorsque la boîte de dialogue est
		//  fermée avec OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO : Placez ici le code définissant le comportement lorsque la boîte de dialogue est
		//  fermée avec Annuler
	}

	dlg.DestroyWindow();

	CoUninitialize();


	// Lorsque la boîte de dialogue est fermée, retourner FALSE afin de quitter
	//  l'application, plutôt que de démarrer la pompe de messages de l'application.
	return FALSE;
}
