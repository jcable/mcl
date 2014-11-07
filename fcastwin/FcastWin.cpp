// FcastWin.cpp : D�finit les comportements de classe pour l'application.
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
	// InitCommonControls() est requis sur Windows�XP si le manifeste de l'application
	// sp�cifie l'utilisation de ComCtl32.dll version�6 ou ult�rieure pour activer les
	// styles visuels.  Dans le cas contraire, la cr�ation de fen�tres �chouera.
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
		// TODO : Placez ici le code d�finissant le comportement lorsque la bo�te de dialogue est
		//  ferm�e avec OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO : Placez ici le code d�finissant le comportement lorsque la bo�te de dialogue est
		//  ferm�e avec Annuler
	}

	dlg.DestroyWindow();

	CoUninitialize();


	// Lorsque la bo�te de dialogue est ferm�e, retourner FALSE afin de quitter
	//  l'application, plut�t que de d�marrer la pompe de messages de l'application.
	return FALSE;
}
