// FcastWinDlg.cpp : fichier d'implémentation
//

#include "stdafx.h"
#include "FcastWin.h"
#include "FcastWinDlg.h"
#include "FcastAboutDlg.h"
#include "afxwin.h"
#include <Shlwapi.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif



// boîte de dialogue CFcastWinDlg
CFcastWinDlg::CFcastWinDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CFcastWinDlg::IDD, pParent)
	, m_ThreadOutput(NULL)
	, m_ThreadOutputErr(NULL)
	, m_FcastCmdLine(_T(""))
	, m_ValueNLvl(_T(""))
	, m_ValueDemux(_T(""))
	, m_ValueTmpDir(_T(""))
	, m_ValueTTL(_T(""))
	, m_ValueFEC(_T(""))
	, m_ValueRepeat(_T(""))
	, m_ValuePathSend(_T(""))
	, m_ValuePathRecv(_T(""))
	, m_EXEFullPath(_T(""))
	, m_ValueDSize(_T(""))
	, m_ValueBW(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CFcastWinDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TABCTRL, m_TabCtrl);
	DDX_Control(pDX, IDC_IPADDR, m_IPAddr);
	DDX_Control(pDX, IDC_ZONE_ADDR, m_ZoneAddr);
	DDX_Control(pDX, IDC_STATIC_ADDR, m_StaticAddr);
	DDX_Control(pDX, IDC_STATIC_PORT, m_StaticPort);
	DDX_Control(pDX, IDC_PORT, m_EditPort);
	DDX_Control(pDX, IDC_ZONE_PATH, m_ZonePath);
	DDX_Control(pDX, IDC_CHECK_RECURSIVE, m_CheckRecursive);
	DDX_Control(pDX, IDC_BUTTON_BROWSE, m_BtnBrowse);
	DDX_Control(pDX, IDC_STATIC_PATH, m_StaticPath);
	DDX_Control(pDX, IDC_ZONE_OPT, m_ZoneOpt);
	DDX_Control(pDX, IDC_CHECK_NLAYER, m_CheckNlayer);
	DDX_Control(pDX, IDC_EDIT_NLVL, m_EditNlvl);
	DDX_Control(pDX, IDC_STATIC_PROFILE, m_StaticProfile);
	DDX_Control(pDX, IDC_COMBO_PROFILE, m_ComboProfile);
	DDX_Control(pDX, IDC_CHECK_DEMUX, m_CheckDemux);
	DDX_Control(pDX, IDC_EDIT_TSI, m_EditTsi);
	DDX_Control(pDX, IDC_STATIC_OPTIMIZE, m_StaticOptimize);
	DDX_Control(pDX, IDC_COMBO_OPTIMIZE, m_ComboOptimize);
	DDX_Control(pDX, IDC_ZONE_OPTRCV, m_ZoneOptRcv);
	DDX_Control(pDX, IDC_CHECK_TTL, m_CheckTTL);
	DDX_Control(pDX, IDC_EDIT_TTL, m_EditTTL);
	DDX_Control(pDX, IDC_CHECK_FEC, m_CheckFEC);
	DDX_Control(pDX, IDC_EDIT_FEC, m_EditFec);
	DDX_Control(pDX, IDC_CHECK_CONT, m_CheckCont);
	DDX_Control(pDX, IDC_CHECK_HUGE, m_CheckHuge);
	DDX_Control(pDX, IDC_CHECK_REPEAT, m_CheckRepeat);
	DDX_Control(pDX, IDC_EDIT_NREPEAT, m_EditRepeat);
	DDX_Control(pDX, IDC_STATIC_REPEAT, m_StaticRepeat);
	DDX_Control(pDX, IDC_ZONE_OUTPUT, m_ZoneOutput);
	DDX_Control(pDX, IDC_STATIC_TRACELVL, m_StaticTraceLvl);
	DDX_Control(pDX, IDC_EDIT_TRACELVL, m_EditTraceLvl);
	DDX_Control(pDX, IDC_STATIC_STATLVL, m_StaticStatLvl);
	DDX_Control(pDX, IDC_EDIT_STATLVL, m_EditStatLvl);
	DDX_Control(pDX, IDC_CHECK_SILENT, m_CheckSilent);
	DDX_Control(pDX, IDC_STATIC_TMPDIR, m_StaticTmpDir);
	DDX_Control(pDX, IDC_EDIT_TMPDIR, m_EditTmpDir);
	DDX_Control(pDX, IDC_STATIC_IF, m_StaticIf);
	DDX_Control(pDX, IDC_COMBO_IF, m_ComboIf);
	DDX_Control(pDX, IDC_START, m_ButtonStart);
	DDX_Control(pDX, IDC_EXPLORER1, m_IExplore);
	DDX_Text(pDX, IDC_EDIT_CMDLINE, m_FcastCmdLine);
	DDV_MaxChars(pDX, m_FcastCmdLine, 1024);
	DDX_Text(pDX, IDC_EDIT_NLVL, m_ValueNLvl);
	DDX_Text(pDX, IDC_EDIT_TSI, m_ValueDemux);
	DDX_Text(pDX, IDC_EDIT_TMPDIR, m_ValueTmpDir);
	DDX_Text(pDX, IDC_EDIT_TTL, m_ValueTTL);
	DDX_Text(pDX, IDC_EDIT_FEC, m_ValueFEC);
	DDX_Text(pDX, IDC_EDIT_NREPEAT, m_ValueRepeat);
	DDX_Control(pDX, IDC_EDIT_PATH_RECV, m_EditPathRecv);
	DDX_Control(pDX, IDC_EDIT_PATH_SEND, m_EditPathSend);
	DDX_Text(pDX, IDC_EDIT_PATH_SEND, m_ValuePathSend);
	DDX_Text(pDX, IDC_EDIT_PATH_RECV, m_ValuePathRecv);
	DDX_Control(pDX, IDC_COMBO_OVERWRITE, m_ComboOverwrite);
	DDX_Control(pDX, IDC_STATIC_OVERWRITE, m_StaticOverwrite);
	DDX_Control(pDX, IDC_EDIT_STATUS, m_EditStatus);
	DDX_Control(pDX, IDC_EDIT_CMDLINE, m_EditCmdLine);
	DDX_Control(pDX, IDC_BTN_UPDATE, m_BtnUpdate);
	DDX_Control(pDX, IDC_STATIC_CMDLINE, m_StatisCmdLine);
	DDX_Control(pDX, IDC_EDIT_DSIZE, m_EditDSize);
	DDX_Control(pDX, IDC_EDIT_BW, m_EditBW);
	DDX_Control(pDX, IDC_STATIC_DSIZE, m_StaticDSize);
	DDX_Control(pDX, IDC_STATIC_BW, m_StaticBW);
	DDX_Text(pDX, IDC_EDIT_DSIZE, m_ValueDSize);
	DDX_Text(pDX, IDC_EDIT_BW, m_ValueBW);
	DDX_Control(pDX, IDC_CHECK_SINGLELAYER, m_CheckSingleLayer);
}

BEGIN_MESSAGE_MAP(CFcastWinDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_START, OnBnClickedStart)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TABCTRL, OnTcnSelchangeTabctrl)
	ON_BN_CLICKED(IDC_CHECK_NLAYER, OnBnClickedCheckNlayer)
	ON_BN_CLICKED(IDC_CHECK_SILENT, OnBnClickedCheckSilent)
	ON_BN_CLICKED(IDC_CHECK_DEMUX, OnBnClickedCheckDemux)
	ON_BN_CLICKED(IDC_CHECK_TTL, OnBnClickedCheckTtl)
	ON_BN_CLICKED(IDC_CHECK_CONT, OnBnClickedCheckCont)
	ON_BN_CLICKED(IDC_CHECK_REPEAT, OnBnClickedCheckRepeat)
	ON_BN_CLICKED(IDC_CHECK_FEC, OnBnClickedCheckFec)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE, OnBnClickedButtonBrowse)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BTN_UPDATE, OnParamsUpdate)
	ON_EN_CHANGE(IDC_PORT, OnEnChangePort)
	ON_NOTIFY(IPN_FIELDCHANGED, IDC_IPADDR, OnIpnFieldchangedIpaddr)
	ON_EN_CHANGE(IDC_EDIT_TRACELVL, OnEnChangeEditTracelvl)
	ON_BN_CLICKED(IDC_CHECK_RECURSIVE, OnBnClickedCheckRecursive)
	ON_EN_CHANGE(IDC_EDIT_STATLVL, OnEnChangeEditStatlvl)
	ON_EN_CHANGE(IDC_EDIT_PATH, OnEnChangeEditPath)
	ON_EN_CHANGE(IDC_EDIT_NLVL, OnEnChangeEditNlvl)
	ON_EN_CHANGE(IDC_EDIT_TSI, OnEnChangeEditTsi)
	ON_CBN_SELCHANGE(IDC_COMBO_PROFILE, OnCbnSelchangeComboProfile)
	ON_CBN_SELCHANGE(IDC_COMBO_OPTIMIZE, OnCbnSelchangeComboOptimize)
	ON_EN_CHANGE(IDC_EDIT_TMPDIR, OnEnChangeEditTmpdir)
	ON_EN_CHANGE(IDC_EDIT_TTL, OnEnChangeEditTtl)
	ON_EN_CHANGE(IDC_EDIT_FEC, OnEnChangeEditFec)
	ON_BN_CLICKED(IDC_CHECK_HUGE, OnBnClickedCheckHuge)
	ON_BN_CLICKED(IDC_CHECK_OVERWRITE, OnBnClickedCheckOverwrite)
	ON_EN_CHANGE(IDC_EDIT_PATH_SEND, OnEnChangeEditPathSend)
	ON_EN_CHANGE(IDC_EDIT_PATH_RECV, OnEnChangeEditPathRecv)
	ON_CBN_SELCHANGE(IDC_COMBO_OVERWRITE, OnCbnSelchangeComboOverwrite)
	ON_EN_CHANGE(IDC_EDIT_DSIZE, OnEnChangeEditDsize)
	ON_EN_CHANGE(IDC_EDIT_BW, OnEnChangeEditBw)
	ON_BN_CLICKED(IDC_CHECK_SINGLELAYER, OnBnClickedCheckSinglelayer)
END_MESSAGE_MAP()


// gestionnaires de messages pour CFcastWinDlg
BOOL CFcastWinDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);
	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}
	SetIcon(m_hIcon, TRUE);			// Définir une grande icône
	SetIcon(m_hIcon, FALSE);		// Définir une petite icône

	// TODO : ajoutez ici une initialisation supplémentaire
	m_TabCtrl.InsertItem(IDTAB_SEND, "Send");
	m_TabCtrl.InsertItem(IDTAB_RECV, "Receive");
	m_TabCtrl.InsertItem(IDTAB_HELP, "Help");
	ShowControlForTab(0);
	m_CheckNlayer.SetCheck(BST_UNCHECKED);
	m_EditNlvl.EnableWindow(FALSE);
	m_EditNlvl.SetWindowText("5");
	m_EditTraceLvl.EnableWindow();
	m_EditTraceLvl.SetWindowText("");
	m_EditStatLvl.EnableWindow();
	m_EditStatLvl.SetWindowText("");
	m_CheckDemux.SetCheck(BST_UNCHECKED);
	m_EditTsi.EnableWindow(FALSE);
	m_EditTsi.SetWindowText("0");
	m_EditTmpDir.SetWindowText("");
	m_CheckTTL.SetCheck(BST_UNCHECKED);
	m_EditTTL.EnableWindow(FALSE);
	m_EditTTL.SetWindowText("1");
	m_CheckRepeat.SetCheck(BST_UNCHECKED);
	m_EditRepeat.EnableWindow(FALSE);
	m_EditRepeat.SetWindowText("3");
	m_CheckFEC.SetCheck(BST_UNCHECKED);
	m_EditFec.EnableWindow(FALSE);
	m_EditFec.SetWindowText("0");
	m_ComboOverwrite.InsertString(0, "Ask");
	m_ComboOverwrite.InsertString(1, "Never");
	m_ComboOverwrite.InsertString(2, "Always");
	m_ComboOverwrite.SetCurSel(0);
	m_ComboOptimize.InsertString(0, "DEFAULT");
	m_ComboOptimize.InsertString(1, "Speed");
	m_ComboOptimize.InsertString(2, "Space");
	m_ComboOptimize.InsertString(3, "CPU");
	m_ComboOptimize.SetCurSel(0);
	m_ComboProfile.InsertString(0, "DEFAULT");
	m_ComboProfile.InsertString(1, "Low Internet");
	m_ComboProfile.InsertString(2, "Medium Internet");
	m_ComboProfile.InsertString(3, "High Internet");
	m_ComboProfile.InsertString(4, "High LAN");
	m_ComboProfile.InsertString(5, "Custom...");
	m_ComboProfile.SetCurSel(0);
	m_EditDSize.EnableWindow(FALSE);
	m_EditBW.EnableWindow(FALSE);
	m_IPAddr.SetAddress(224, 100, 101, 102);
	m_EditPort.SetWindowText("2002");
	CString MyCmdLine = GetCommandLine();
	MyCmdLine.Trim('\"');

	int ind2, ind = MyCmdLine.Find('\\', 0);
	while( ind != -1)
	{
		ind2 = ind;
		ind = MyCmdLine.Find('\\', ind2+1);
	}
	m_EXEFullPath = MyCmdLine.Left(ind2);
	m_ValuePathSend = "c:\\foo.bar";
	m_ValuePathRecv = m_EXEFullPath;

	m_IExplore.Navigate(m_EXEFullPath + "\\fcast.html", 0, (struct tagVARIANT *)"_SELF", NULL, NULL);
	UpdateData(FALSE);
	OnParamsUpdate();
	return TRUE;
}



void CFcastWinDlg::OnBnClickedStart()
{
	UpdateData(TRUE);

	if(m_TabCtrl.GetCurSel() == IDTAB_RECV)
	{
		if(!m_ValuePathRecv.IsEmpty() && PathFileExists(m_ValuePathRecv) && PathIsDirectory(m_ValuePathRecv))
		{
			m_EditStatus.SetWindowText("Executing fcast " + m_FcastCmdLine + "\r\n(working dir: " + m_ValuePathRecv + ").");
			ShellExecute(NULL, NULL, m_EXEFullPath + "\\fcast.exe", m_FcastCmdLine, m_ValuePathRecv, SW_SHOW );
		}
		else
		{
			MessageBox("Invalid destination directory: " + m_ValuePathRecv, "Error", MB_OK+MB_ICONERROR);
		}
	}
	else if(m_TabCtrl.GetCurSel() == IDTAB_SEND)
	{
		if(!m_ValuePathSend.IsEmpty() && PathFileExists(m_ValuePathSend))
		{
			CString PathSendFrom, ObjSendName;
			int ind2=-1; int ind;

			ObjSendName.Empty();

			if(PathIsDirectory(m_ValuePathSend))
			{
				if(m_ValuePathSend[m_ValuePathSend.GetLength()-1] == '\\')
				{
					m_ValuePathSend = m_ValuePathSend.Left(m_ValuePathSend.GetLength()-1);
				}
		        ind = m_ValuePathSend.Find('\\', 0);
				while( ind != -1)
				{
					ind2 = ind;
					ind = m_ValuePathSend.Find('\\', ind2+1);
				}
				if(ind2>=0)
				{
					PathSendFrom = m_ValuePathSend.Left(ind2+1);
					if(ind2+1 < m_ValuePathSend.GetLength())
					{
						ObjSendName =  m_ValuePathSend.Right(m_ValuePathSend.GetLength() - ind2 - 1);
					}
				}
				else
				{
					PathSendFrom = m_ValuePathSend;
					ObjSendName = ".";
				}
			}
			else
			{
		        ind = m_ValuePathSend.Find('\\', 0);
				while( ind != -1)
				{
					ind2 = ind;
					ind = m_ValuePathSend.Find('\\', ind2+1);
				}
				if(ind2>=0)
				{
					PathSendFrom = m_ValuePathSend.Left(ind2+1);
					if(ind2+1 < m_ValuePathSend.GetLength())
					{
						ObjSendName =  m_ValuePathSend.Right(m_ValuePathSend.GetLength() - ind2 - 1);
					}
				}
				else
				{
					PathSendFrom.Empty();
					ObjSendName = m_ValuePathSend;
				}
			}

			m_EditStatus.SetWindowText("Executing fcast " + m_FcastCmdLine + " " + ObjSendName + "\r\n(working dir: " + PathSendFrom + ").");
			ShellExecute(NULL, NULL, m_EXEFullPath + "\\fcast.exe", m_FcastCmdLine + " " + ObjSendName, PathSendFrom, SW_SHOW );
		}
		else
		{
			MessageBox("No such file or directory: " + m_ValuePathSend, "Error", MB_OK+MB_ICONERROR);
		}
	}

}


void CFcastWinDlg::OnTcnSelchangeTabctrl(NMHDR *pNMHDR, LRESULT *pResult)
{
	int nTab = m_TabCtrl.GetCurSel();
	ShowControlForTab(nTab);
	*pResult = 0;
}


void CFcastWinDlg::OnBnClickedCheckNlayer()
{
	if(m_CheckNlayer.GetCheck() == BST_CHECKED)
	{
		m_EditNlvl.EnableWindow();
		m_CheckSingleLayer.SetCheck( BST_UNCHECKED);
	}
	else
	{
		m_EditNlvl.EnableWindow(FALSE);
	}

	OnParamsUpdate();
}

void CFcastWinDlg::OnBnClickedCheckSilent()
{
	if(m_CheckSilent.GetCheck())
	{
		m_EditTraceLvl.EnableWindow(FALSE);
		m_EditStatLvl.EnableWindow(FALSE);
	}
	else
	{
		m_EditTraceLvl.EnableWindow();
		m_EditStatLvl.EnableWindow();
	}

	OnParamsUpdate();
}

void CFcastWinDlg::OnBnClickedCheckDemux()
{
	if(m_CheckDemux.GetCheck())
	{
		m_EditTsi.EnableWindow();
	}
	else
	{
		m_EditTsi.EnableWindow(FALSE);
	}

	OnParamsUpdate();
}

void CFcastWinDlg::OnBnClickedCheckTtl()
{
	if(m_CheckTTL.GetCheck())
	{
		m_EditTTL.EnableWindow();
	}
	else
	{
		m_EditTTL.EnableWindow(FALSE);
	}

	OnParamsUpdate();
}

void CFcastWinDlg::OnBnClickedCheckCont()
{
	if(m_CheckCont.GetCheck())
	{
		m_CheckRepeat.EnableWindow(FALSE);
		m_EditRepeat.EnableWindow(FALSE);
		m_StaticRepeat.EnableWindow(FALSE);
	}
	else
	{
		m_CheckRepeat.EnableWindow();
		if(m_CheckRepeat.GetCheck()) {
			m_EditRepeat.EnableWindow();
		}
		else {
			m_EditRepeat.EnableWindow(FALSE);
		}
		m_StaticRepeat.EnableWindow();
	}

	OnParamsUpdate();
}

void CFcastWinDlg::OnBnClickedCheckRepeat()
{
	if(m_CheckRepeat.GetCheck())
	{
		m_EditRepeat.EnableWindow();
	}
	else
	{
		m_EditRepeat.EnableWindow(FALSE);
	}

	OnParamsUpdate();
}

void CFcastWinDlg::OnBnClickedCheckFec()
{
	if(m_CheckFEC.GetCheck())
	{
		m_EditFec.EnableWindow();
	}
	else
	{
		m_EditFec.EnableWindow(FALSE);
	}

	OnParamsUpdate();
}

void CFcastWinDlg::OnBnClickedButtonBrowse()
{
	BROWSEINFO MyBrowseInfo;
	TCHAR MyShPath[MAX_PATH];

	ZeroMemory(&MyBrowseInfo, sizeof(MyBrowseInfo));
	if(m_TabCtrl.GetCurSel() == IDTAB_SEND)
	{
		MyBrowseInfo.lpszTitle	= "Select a file or directory to send";
		MyBrowseInfo.ulFlags	= BIF_BROWSEINCLUDEFILES | BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
	}
	else if(m_TabCtrl.GetCurSel() == IDTAB_RECV)
	{
		MyBrowseInfo.lpszTitle	= "Select a directory to store received files to";
		MyBrowseInfo.ulFlags	= BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
	}

	MyBrowseInfo.hwndOwner	= this->m_hWnd;
	MyBrowseInfo.pidlRoot	= NULL;
	MyBrowseInfo.pszDisplayName = NULL;
	LPITEMIDLIST MyItem  = SHBrowseForFolder(&MyBrowseInfo);
	if( MyItem!=NULL )
	{
		SHGetPathFromIDList(MyItem, MyShPath);
		if(m_TabCtrl.GetCurSel() == IDTAB_SEND)
		{
			m_ValuePathSend = MyShPath;
		}
		else if(m_TabCtrl.GetCurSel() == IDTAB_RECV)
		{
			if(PathFileExists(MyShPath) && PathIsDirectory(MyShPath))
			{
				m_ValuePathRecv = MyShPath;
			}
		}
		UpdateData(FALSE);
		OnParamsUpdate();
	}

}

void CFcastWinDlg::OnClose()
{
	CDialog::OnClose();
}

void CFcastWinDlg::OnParamsUpdate()
{
	CString Args, ValuePort;
	BYTE IPField1, IPField2, IPField3, IPField4;
	UpdateData(TRUE);
	
	if(m_TabCtrl.GetCurSel() == IDTAB_SEND)
	{
		m_FcastCmdLine = "-send -P";
	}
	else if(m_TabCtrl.GetCurSel() == IDTAB_RECV)
	{
		m_FcastCmdLine = "-recv -P";
	}
	else
	{
		return;
	}

	m_EditPort.GetWindowText(ValuePort);

	m_IPAddr.GetAddress(IPField1, IPField2, IPField3, IPField4);
	Args.Format(" -a%d.%d.%d.%d/%s",IPField1 , IPField2 , IPField3 , IPField4, ValuePort);

	switch( m_ComboOptimize.GetCurSel())
	{
	case 0:
		break;
	case 1:
		Args += " -ospeed";
		break;
	case 2:
		Args += " -ospace";
		break;
	case 3:
		Args += " -ocpu";
		break;
	default:
		break;
	}

	switch( m_ComboProfile.GetCurSel())
	{
	case 0:
		break;
	case 1:
		Args += " -plow";
		break;
	case 2:
		Args += " -pmed";
		break;
	case 3:
		Args += " -phig";
		break;
	case 4:
		Args += " -plan";
		break;
	case 5:
		Args += " -p" + m_ValueDSize;
		if(m_TabCtrl.GetCurSel()==IDTAB_SEND && m_ValueBW!="")
		{
			Args += "/" + m_ValueBW;
		}
	default:
		break;
	}

	if( m_CheckNlayer.GetCheck() == BST_CHECKED && m_ValueNLvl!="" )
	{
        Args += " -l" + m_ValueNLvl;
	}
	else if ( m_CheckSingleLayer.GetCheck() == BST_CHECKED )
	{
		Args += " -singlelayer";
	}


	if(m_CheckSilent.GetCheck() == BST_CHECKED)
	{
		Args += " -silent";
	}
	else
	{
		CString ValueTraceLvl;
		CString ValueStatLvl;
		m_EditTraceLvl.GetWindowText(ValueTraceLvl);
		m_EditStatLvl.GetWindowText(ValueStatLvl);

		if( ValueTraceLvl != "" )
		{
			Args += " -v" + ValueTraceLvl;
		}
		if( ValueStatLvl != "" )
		{
			Args += " -stat" + ValueStatLvl;
		}
	}

	if(m_CheckDemux.GetCheck() == BST_CHECKED && m_ValueDemux!="")
	{
        Args += " -demux" + m_ValueDemux;
	}

	if(m_ValueTmpDir!="")
	{
		Args += " -tmpdir" + m_ValueTmpDir;
	}

	if(m_TabCtrl.GetCurSel() == IDTAB_SEND)
	{
		if(!m_ValuePathSend.IsEmpty())
		{
			if(PathFileExists(m_ValuePathSend) && PathIsDirectory(m_ValuePathSend))
			{
				m_CheckRecursive.EnableWindow(TRUE);
				m_CheckRecursive.SetCheck(BST_CHECKED);
			}
			else if(PathFileExists(m_ValuePathSend))
			{
				m_CheckRecursive.EnableWindow(TRUE);
				m_CheckRecursive.SetCheck(BST_UNCHECKED);
			}
			else
			{
				m_CheckRecursive.SetCheck(BST_UNCHECKED);
				m_CheckRecursive.EnableWindow(FALSE);
			}
		}

		if(m_CheckRecursive.GetCheck() == BST_CHECKED)
		{
			Args += " -R";
		}
		if(	m_CheckTTL.GetCheck() && m_ValueTTL!="")
		{
			Args += " -t" + m_ValueTTL;
		}
		if(	m_CheckFEC.GetCheck() && m_ValueFEC!="")
		{
			Args += " -fec" + m_ValueFEC;
		}
		if(	m_CheckHuge.GetCheck() )
		{
			Args += " -huge";
		}
		if(m_CheckCont.GetCheck())
		{
			Args += " -cont";
		}
		else if( m_CheckRepeat.GetCheck() && m_ValueRepeat!="")
		{
			Args += " -repeat" + m_ValueRepeat;
		}
	}
	else if(m_TabCtrl.GetCurSel() == IDTAB_RECV)
	{
		switch( m_ComboOverwrite.GetCurSel())
		{
			case 0:
				Args += " -int";
				break;
			case 1:
				Args += " -never";
				break;
			case 2:
				Args += " -force";
				break;
			default:
				break;
		}
	}

	m_FcastCmdLine += Args;
	UpdateData(FALSE);

}



void CFcastWinDlg::OnEnChangePort()
{
	OnParamsUpdate();
}

void CFcastWinDlg::OnIpnFieldchangedIpaddr(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMIPADDRESS pIPAddr = reinterpret_cast<LPNMIPADDRESS>(pNMHDR);
	OnParamsUpdate();
	*pResult = 0;
}

void CFcastWinDlg::OnEnChangeEditTracelvl()
{
	OnParamsUpdate();
}

void CFcastWinDlg::OnBnClickedCheckRecursive()
{
	OnParamsUpdate();
}

void CFcastWinDlg::OnEnChangeEditStatlvl()
{
	OnParamsUpdate();
}

void CFcastWinDlg::OnEnChangeEditPath()
{
	OnParamsUpdate();
}

void CFcastWinDlg::OnEnChangeEditNlvl()
{
	OnParamsUpdate();
}

void CFcastWinDlg::OnEnChangeEditTsi()
{
	OnParamsUpdate();
}

void CFcastWinDlg::OnCbnSelchangeComboProfile()
{
	if(m_ComboProfile.GetCurSel()==5)
	{
		m_EditDSize.EnableWindow(TRUE);
		m_EditBW.EnableWindow(TRUE);
	}
	else
	{
		m_EditDSize.EnableWindow(FALSE);
		m_EditBW.EnableWindow(FALSE);
	}


	OnParamsUpdate();
}

void CFcastWinDlg::OnCbnSelchangeComboOptimize()
{
	OnParamsUpdate();
}

void CFcastWinDlg::OnEnChangeEditTmpdir()
{
	OnParamsUpdate();
}

void CFcastWinDlg::OnEnChangeEditTtl()
{
	OnParamsUpdate();
}

void CFcastWinDlg::OnEnChangeEditFec()
{
	OnParamsUpdate();
}

void CFcastWinDlg::OnBnClickedCheckHuge()
{
	OnParamsUpdate();
}

void CFcastWinDlg::OnBnClickedCheckOverwrite()
{
	OnParamsUpdate();
}

void CFcastWinDlg::ShowControlForTab(int nTab)
{
	switch(nTab)
	{
		case IDTAB_SEND:
			m_ButtonStart.ShowWindow(SW_SHOW);
			m_ButtonStart.SetWindowText("&Send Now!");
			m_IPAddr.ShowWindow(SW_SHOW);
			m_StaticAddr.ShowWindow(SW_SHOW);
			m_ZoneAddr.ShowWindow(SW_SHOW);
			m_StaticPort.ShowWindow(SW_SHOW);
			m_EditPort.ShowWindow(SW_SHOW);
			m_ZonePath.SetWindowText("File(s) to send");
			m_ZonePath.ShowWindow(SW_SHOW);
			m_IExplore.put_Height(0);
			m_IExplore.put_Width(0);

			m_CheckRecursive.ShowWindow(SW_SHOW);
			m_BtnBrowse.ShowWindow(SW_SHOW);
			m_EditPathRecv.ShowWindow(SW_HIDE);
			m_EditPathSend.ShowWindow(SW_SHOW);
			m_StaticPath.ShowWindow(SW_SHOW);
			m_ComboOverwrite.ShowWindow(SW_HIDE);
			m_StaticOverwrite.ShowWindow(SW_HIDE);
			m_ZoneOpt.ShowWindow(SW_SHOW);
			m_ZoneOptRcv.ShowWindow(SW_HIDE);
			m_CheckNlayer.ShowWindow(SW_SHOW);
			m_EditNlvl.ShowWindow(SW_SHOW);
			m_CheckSingleLayer.ShowWindow(SW_SHOW);
			m_StaticProfile.ShowWindow(SW_SHOW);
			m_ComboProfile.ShowWindow(SW_SHOW);
			m_EditDSize.ShowWindow(SW_SHOW);
			m_StaticDSize.ShowWindow(SW_SHOW);
			m_EditBW.ShowWindow(SW_SHOW);
			m_StaticBW.ShowWindow(SW_SHOW);
			m_CheckDemux.ShowWindow(SW_SHOW);
			m_EditTsi.ShowWindow(SW_SHOW);
			m_StaticOptimize.ShowWindow(SW_SHOW);
			m_ComboOptimize.ShowWindow(SW_SHOW);
			m_EditStatus.ShowWindow(SW_SHOW);
			m_EditStatus.SetWindowText("Ready.");
			m_EditCmdLine.ShowWindow(SW_SHOW);
			m_BtnUpdate.ShowWindow(SW_SHOW);
			m_StatisCmdLine.ShowWindow(SW_SHOW);

			m_CheckTTL.ShowWindow(SW_SHOW);
			m_EditTTL.ShowWindow(SW_SHOW);
			m_CheckFEC.ShowWindow(SW_SHOW);
			m_EditFec.ShowWindow(SW_SHOW);
			m_CheckCont.ShowWindow(SW_SHOW);
			m_CheckHuge.ShowWindow(SW_SHOW);
			m_CheckRepeat.ShowWindow(SW_SHOW);
			m_EditRepeat.ShowWindow(SW_SHOW);
			m_StaticRepeat.ShowWindow(SW_SHOW);

			m_ZoneOutput.ShowWindow(SW_SHOW);
			m_StaticTraceLvl.ShowWindow(SW_SHOW);
			m_EditTraceLvl.ShowWindow(SW_SHOW);
			m_StaticStatLvl.ShowWindow(SW_SHOW);
			m_EditStatLvl.ShowWindow(SW_SHOW);
			m_CheckSilent.ShowWindow(SW_SHOW);

			m_StaticTmpDir.ShowWindow(SW_SHOW);
			m_EditTmpDir.ShowWindow(SW_SHOW);
			m_StaticIf.ShowWindow(SW_SHOW);
			m_ComboIf.ShowWindow(SW_SHOW);

			OnParamsUpdate();

			break;

		case IDTAB_RECV:
			m_ButtonStart.ShowWindow(SW_SHOW);
			m_ButtonStart.SetWindowText("&Receive Now!");
			m_IPAddr.ShowWindow(SW_SHOW);
			m_StaticAddr.ShowWindow(SW_SHOW);
			m_ZoneAddr.ShowWindow(SW_SHOW);
			m_StaticPort.ShowWindow(SW_SHOW);
			m_EditPort.ShowWindow(SW_SHOW);
			m_ZonePath.SetWindowText("Destination Folder");
			m_ZonePath.ShowWindow(SW_SHOW);
			m_IExplore.put_Height(0);
			m_IExplore.put_Width(0);

			m_CheckRecursive.ShowWindow(SW_HIDE);
			m_BtnBrowse.ShowWindow(SW_SHOW);
			m_EditPathRecv.ShowWindow(SW_SHOW);
			m_EditPathSend.ShowWindow(SW_HIDE);
			m_StaticPath.ShowWindow(SW_SHOW);
			m_ComboOverwrite.ShowWindow(SW_SHOW);
			m_StaticOverwrite.ShowWindow(SW_SHOW);
			m_ZoneOpt.ShowWindow(SW_HIDE);
			m_ZoneOptRcv.ShowWindow(SW_SHOW);
			m_CheckNlayer.ShowWindow(SW_SHOW);
			m_EditNlvl.ShowWindow(SW_SHOW);
			m_CheckSingleLayer.ShowWindow(SW_SHOW);
			m_StaticProfile.ShowWindow(SW_SHOW);
			m_ComboProfile.ShowWindow(SW_SHOW);
			m_EditDSize.ShowWindow(SW_SHOW);
			m_StaticDSize.ShowWindow(SW_SHOW);
			m_EditBW.ShowWindow(SW_HIDE);
			m_StaticBW.ShowWindow(SW_HIDE);
			m_CheckDemux.ShowWindow(SW_SHOW);
			m_EditTsi.ShowWindow(SW_SHOW);
			m_StaticOptimize.ShowWindow(SW_SHOW);
			m_ComboOptimize.ShowWindow(SW_SHOW);
			m_CheckTTL.ShowWindow(SW_HIDE);
			m_EditTTL.ShowWindow(SW_HIDE);
			m_CheckFEC.ShowWindow(SW_HIDE);
			m_EditFec.ShowWindow(SW_HIDE);
			m_CheckCont.ShowWindow(SW_HIDE);
			m_CheckHuge.ShowWindow(SW_HIDE);
			m_CheckRepeat.ShowWindow(SW_HIDE);
			m_EditRepeat.ShowWindow(SW_HIDE);
			m_StaticRepeat.ShowWindow(SW_HIDE);
			m_EditStatus.ShowWindow(SW_SHOW);
			m_EditStatus.SetWindowText("Ready.");
			m_EditCmdLine.ShowWindow(SW_SHOW);
			m_BtnUpdate.ShowWindow(SW_SHOW);
			m_StatisCmdLine.ShowWindow(SW_SHOW);

			m_ZoneOutput.ShowWindow(SW_SHOW);
			m_StaticTraceLvl.ShowWindow(SW_SHOW);
			m_EditTraceLvl.ShowWindow(SW_SHOW);
			m_StaticStatLvl.ShowWindow(SW_SHOW);
			m_EditStatLvl.ShowWindow(SW_SHOW);
			m_CheckSilent.ShowWindow(SW_SHOW);

			m_StaticTmpDir.ShowWindow(SW_SHOW);
			m_EditTmpDir.ShowWindow(SW_SHOW);
			m_StaticIf.ShowWindow(SW_SHOW);
			m_ComboIf.ShowWindow(SW_SHOW);

			OnParamsUpdate();

			break;

		case IDTAB_HELP:
			m_ButtonStart.ShowWindow(SW_HIDE);
			m_IPAddr.ShowWindow(SW_HIDE);
			m_StaticAddr.ShowWindow(SW_HIDE);
			m_ZoneAddr.ShowWindow(SW_HIDE);
			m_StaticPort.ShowWindow(SW_HIDE);
			m_EditPort.ShowWindow(SW_HIDE);
			m_ZonePath.ShowWindow(SW_HIDE);
			m_IExplore.put_Width(474);
			m_IExplore.put_Height(480);

			m_CheckRecursive.ShowWindow(SW_HIDE);
			m_BtnBrowse.ShowWindow(SW_HIDE);
			m_EditPathRecv.ShowWindow(SW_HIDE);
			m_EditPathSend.ShowWindow(SW_HIDE);
			m_StaticPath.ShowWindow(SW_HIDE);
			m_ComboOverwrite.ShowWindow(SW_HIDE);
			m_StaticOverwrite.ShowWindow(SW_HIDE);
			m_ZoneOpt.ShowWindow(SW_HIDE);
			m_ZoneOptRcv.ShowWindow(SW_HIDE);
			m_CheckNlayer.ShowWindow(SW_HIDE);
			m_EditNlvl.ShowWindow(SW_HIDE);
			m_CheckSingleLayer.ShowWindow(SW_HIDE);
			m_StaticProfile.ShowWindow(SW_HIDE);
			m_ComboProfile.ShowWindow(SW_HIDE);
			m_EditDSize.ShowWindow(SW_HIDE);
			m_StaticDSize.ShowWindow(SW_HIDE);
			m_EditBW.ShowWindow(SW_HIDE);
			m_StaticBW.ShowWindow(SW_HIDE);
			m_CheckDemux.ShowWindow(SW_HIDE);
			m_EditTsi.ShowWindow(SW_HIDE);
			m_StaticOptimize.ShowWindow(SW_HIDE);
			m_ComboOptimize.ShowWindow(SW_HIDE);
			m_CheckTTL.ShowWindow(SW_HIDE);
			m_EditTTL.ShowWindow(SW_HIDE);
			m_CheckFEC.ShowWindow(SW_HIDE);
			m_EditFec.ShowWindow(SW_HIDE);
			m_CheckCont.ShowWindow(SW_HIDE);
			m_CheckHuge.ShowWindow(SW_HIDE);
			m_CheckRepeat.ShowWindow(SW_HIDE);
			m_EditRepeat.ShowWindow(SW_HIDE);
			m_StaticRepeat.ShowWindow(SW_HIDE);
			m_EditStatus.ShowWindow(SW_HIDE);
			m_EditCmdLine.ShowWindow(SW_HIDE);
			m_BtnUpdate.ShowWindow(SW_HIDE);
			m_StatisCmdLine.ShowWindow(SW_HIDE);

			m_ZoneOutput.ShowWindow(SW_HIDE);
			m_StaticTraceLvl.ShowWindow(SW_HIDE);
			m_EditTraceLvl.ShowWindow(SW_HIDE);
			m_StaticStatLvl.ShowWindow(SW_HIDE);
			m_EditStatLvl.ShowWindow(SW_HIDE);
			m_CheckSilent.ShowWindow(SW_HIDE);

			m_StaticTmpDir.ShowWindow(SW_HIDE);
			m_EditTmpDir.ShowWindow(SW_HIDE);
			m_StaticIf.ShowWindow(SW_HIDE);
			m_ComboIf.ShowWindow(SW_HIDE);
			break;
		default:
			MessageBox("Erreur de tabulation!", "Erreur!", MB_ICONERROR);
	}
}


void CFcastWinDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

void CFcastWinDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // contexte de périphérique pour la peinture

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Centrer l'icône dans le rectangle client
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Dessiner l'icône
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// Le système appelle cette fonction pour obtenir le curseur à afficher lorsque l'utilisateur fait glisser
//  la fenêtre réduite.
HCURSOR CFcastWinDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}
void CFcastWinDlg::OnEnChangeEditPathSend()
{
	m_CheckRecursive.EnableWindow(TRUE);
	m_CheckRecursive.SetCheck(BST_UNCHECKED);
	OnParamsUpdate();
}

void CFcastWinDlg::OnEnChangeEditPathRecv()
{
	OnParamsUpdate();
}

void CFcastWinDlg::OnCbnSelchangeComboOverwrite()
{
	OnParamsUpdate();
}

void CFcastWinDlg::OnEnChangeEditDsize()
{
	OnParamsUpdate();
}

void CFcastWinDlg::OnEnChangeEditBw()
{
	OnParamsUpdate();
}

void CFcastWinDlg::OnBnClickedCheckSinglelayer()
{
	if(m_CheckSingleLayer.GetCheck() == BST_CHECKED)
	{
		m_CheckNlayer.SetCheck(BST_UNCHECKED);
		m_EditNlvl.EnableWindow(FALSE);
	}
	OnParamsUpdate();
}
