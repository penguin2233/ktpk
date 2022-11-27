
// ktpkDlg.h : header file
//

#pragma once


// CktpkDlg dialog
class CktpkDlg : public CDialogEx
{
// Construction
public:
	CktpkDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_KTPK_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CListCtrl hitlist_ctrl;
	CString editbox_var;
	afx_msg void OnBnClickedAdd();
	afx_msg void OnBnClickedSave();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedExit();
	afx_msg LRESULT OnIconEvent(WPARAM wParam, LPARAM lParam);
};
