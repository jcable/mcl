

// boîte de dialogue CAboutDlg utilisée pour la boîte de dialogue 'À propos de' pour votre application
class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Données de la boîte de dialogue
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // prise en charge de DDX/DDV

// Implémentation
protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	CEdit m_Credits;
};
