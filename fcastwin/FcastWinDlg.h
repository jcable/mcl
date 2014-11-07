// FcastWinDlg.h : fichier d'en-tête
//

#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include "IExplore.h"


#define STATUS_NBLINE 30

#define IDTAB_SEND 0
#define IDTAB_RECV 1
#define IDTAB_HELP 2

// boîte de dialogue CFcastWinDlg
class CFcastWinDlg : public CDialog
{
// Construction
public:
	CFcastWinDlg(CWnd* pParent = NULL);	// constructeur standard

// Données de la boîte de dialogue
	enum { IDD = IDD_FCASTWIN_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// prise en charge de DDX/DDV


// Implémentation
protected:
	HICON m_hIcon;

	// Fonctions générées de la table des messages
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
	// Control RichEdit status
	CRichEditCtrl m_StatusWin;
public:
	afx_msg void OnBnClickedStart();
	static UINT ThreadChildStdOut(LPVOID param);
	void ThreadChildStdOut(void);
	static UINT ThreadChildStdErr(LPVOID param);
	void ThreadChildStdErr(void);

private:
	CWinThread* m_ThreadOutput;
public:
	afx_msg void OnBnClickedButtonCommand();

private:
	CWinThread* m_ThreadOutputErr;
public:
	CTabCtrl m_TabCtrl;
	afx_msg void OnTcnSelchangeTabctrl(NMHDR *pNMHDR, LRESULT *pResult);
	CIPAddressCtrl m_IPAddr;
private:
	void ShowControlForTab(int nTab);
public:
	CStatic m_ZoneAddr;
	CStatic m_StaticAddr;
	CStatic m_StaticPort;
	CEdit m_EditPort;
	CStatic m_ZonePath;
	CButton m_BtnBrowse;
	CButton m_CheckRecursive;
	CStatic m_StaticPath;
	CStatic m_ZoneOpt;
	CButton m_CheckNlayer;
	CEdit m_EditNlvl;
	CStatic m_StaticProfile;
	CComboBox m_ComboProfile;
	CButton m_CheckDemux;
	CEdit m_EditTsi;
	CStatic m_StaticOptimize;
	CComboBox m_ComboOptimize;
	CStatic m_ZoneOptRcv;
	CButton m_CheckTTL;
	CEdit m_EditTTL;
	CButton m_CheckFEC;
	CEdit m_EditFec;
	CButton m_CheckCont;
	CButton m_CheckHuge;
	CButton m_CheckRepeat;
	CEdit m_EditRepeat;
	CStatic m_StaticRepeat;
	CStatic m_ZoneOutput;
	CStatic m_StaticTraceLvl;
	CEdit m_EditTraceLvl;
	CStatic m_StaticStatLvl;
	CEdit m_EditStatLvl;
	CButton m_CheckSilent;
	CStatic m_StaticTmpDir;
	CEdit m_EditTmpDir;
	CStatic m_StaticIf;
	CComboBox m_ComboIf;
	afx_msg void OnBnClickedCheckNlayer();
	afx_msg void OnBnClickedCheckSilent();
	afx_msg void OnBnClickedCheckDemux();
	afx_msg void OnBnClickedCheckTtl();
	afx_msg void OnBnClickedCheckCont();
	afx_msg void OnBnClickedCheckRepeat();
	afx_msg void OnBnClickedCheckFec();
	afx_msg void OnBnClickedButtonBrowse();
	CButton m_ButtonStart;
	CExplorer1 m_IExplore;
	afx_msg void OnClose();
	afx_msg void OnParamsUpdate();
	afx_msg void OnEnChangePort();
	afx_msg void OnIpnFieldchangedIpaddr(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnEnChangeEditTracelvl();
	afx_msg void OnBnClickedCheckRecursive();
public:
	CString m_FcastCmdLine;
	afx_msg void OnEnChangeEditStatlvl();
	afx_msg void OnEnChangeEditPath();
	CString m_ValueNLvl;
	afx_msg void OnEnChangeEditNlvl();
	CString m_ValueDemux;
	afx_msg void OnEnChangeEditTsi();
	afx_msg void OnCbnSelchangeComboProfile();
	afx_msg void OnCbnSelchangeComboOptimize();
	afx_msg void OnEnChangeEditTmpdir();
	CString m_ValueTmpDir;
	afx_msg void OnEnChangeEditTtl();
	CString m_ValueTTL;
	CString m_ValueFEC;
	afx_msg void OnEnChangeEditFec();
	afx_msg void OnBnClickedCheckHuge();
	CString m_ValueRepeat;
	afx_msg void OnBnClickedCheckOverwrite();
	CEdit m_EditPathRecv;
	CEdit m_EditPathSend;
	CString m_ValuePathSend;
	CString m_ValuePathRecv;
	afx_msg void OnEnChangeEditPathSend();
	afx_msg void OnEnChangeEditPathRecv();
private:
	CString m_EXEFullPath;
public:
	CComboBox m_ComboOverwrite;
	CStatic m_StaticOverwrite;
	afx_msg void OnCbnSelchangeComboOverwrite();
	CEdit m_EditStatus;
	CEdit m_EditCmdLine;
	CButton m_BtnUpdate;
	CStatic m_StatisCmdLine;
	CEdit m_EditDSize;
	CEdit m_EditBW;
	CStatic m_StaticDSize;
	CStatic m_StaticBW;
	CString m_ValueDSize;
	CString m_ValueBW;
	afx_msg void OnEnChangeEditDsize();
	afx_msg void OnEnChangeEditBw();
	CButton m_CheckSingleLayer;
	afx_msg void OnBnClickedCheckSinglelayer();
};
