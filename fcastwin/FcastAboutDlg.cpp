#include "stdafx.h"
#include "FcastWin.h"
#include "FcastWinDlg.h"
#include "FcastAboutDlg.h"
#include "afxwin.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_CREDITS, m_Credits);
}

BOOL CAboutDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	m_Credits.SetWindowText("MCL comes with ABSOLUTELY NO WARRANTY; This is free software,\r\nand you are welcome to redistribute it under certain conditions;\r\nSee the GNU General Public License as published by the Free Software\r\nFoundation, version 2 or later, for more details.\r\n\r\nCredits:\r\n  * Vincent Roca (since Oct 2000: INRIA R.A., before: Univ. Paris 6)\r\n  * Julien Laboure (since May 2000: INRIA R.A.)\r\n  * Benoit Mordelet (since December 2000: Activia Networks)\r\n  * fec.c -- forward error corection based on Vandermonde matrices\r\n    (C)1997-98 Luigi Rizzo (luigi@iet.unipi.it) (980624)\r\n    Portions derived from code by Phil Karn (karn@ka9q.ampr.org),\r\n    Robert Morelos-Zaragoza (robert@spectra.eng.hawaii.edu) and\r\n    Hari Thirumoorthy (harit@spectra.eng.hawaii.edu), Aug 1995");
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}