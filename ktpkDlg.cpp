
// ktpkDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "ktpk.h"
#include "ktpkDlg.h"
#include "afxdialogex.h"

#include "killer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CktpkDlg dialog
std::vector<std::string> hitlist;
std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
bool changed = false;
DWORD killerThreadID = 0;

// APPLICATION DEFINED MESSAGES
// 0x0420 = start killer
// 0x0421 = exit killer
// 0x0422 = pause killer
// 0x0423 = callback message for events regarding the notification icon

CktpkDlg::CktpkDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_KTPK_DIALOG, pParent)
	, editbox_var(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CktpkDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_HITLIST, hitlist_ctrl);
	DDX_Text(pDX, IDC_EDIT1, editbox_var);
}

BEGIN_MESSAGE_MAP(CktpkDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_ADD, &CktpkDlg::OnBnClickedAdd)
	ON_BN_CLICKED(IDC_SAVE, &CktpkDlg::OnBnClickedSave)
	ON_BN_CLICKED(IDC_OK, &CktpkDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDOK, &CktpkDlg::OnBnClickedExit)
	ON_MESSAGE(0x0423, &CktpkDlg::OnIconEvent)
END_MESSAGE_MAP()


// CktpkDlg message handlers

BOOL CktpkDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	// load configuration file
	std::fstream configuration("config.txt");
	if (configuration.good()) {
		while (!configuration.eof()) {
			std::string process;
			getline(configuration, process);
			if (process == "") { continue; }
			hitlist.push_back(process);
		}
		configuration.close();
	}
	hitlist_ctrl.InsertColumn(0, L"Process Name", LVCFMT_LEFT, 225);
	for (size_t i = 0; i < hitlist.size(); i++) {
		std::wstring process = converter.from_bytes(hitlist[i]);
		if (process == L"") { continue; }
		hitlist_ctrl.InsertItem(0, process.c_str());
	}
	
	// start killer thread
	HANDLE killerT = CreateThread(NULL, 0, killerMain, &hitlist, 0, &killerThreadID);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CktpkDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CktpkDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CktpkDlg::OnBnClickedAdd()
{
	UpdateData(1);
	std::wstring processName = editbox_var.GetString();
	if (processName == L"") { return; } // edit box was empty
	hitlist_ctrl.InsertItem(1, processName.c_str());
	editbox_var.SetString(L"");
	UpdateData(0);
	changed = true;
	return;
}


void CktpkDlg::OnBnClickedSave()
{
	if (changed == false) { return; } // no changes to save
	UpdateData(1);
	std::ofstream configuration("config.txt");
	for (int i = 0; i < hitlist_ctrl.GetItemCount(); i++) {
		configuration << converter.to_bytes(hitlist_ctrl.GetItemText(i, NULL)) << '\n';
	}
	configuration.close();
	changed = false;
	return;
}


void CktpkDlg::OnBnClickedOk()
{
	if (changed == true) {
		int msgboxRet = MessageBox(L"Changes made to configuration have not been saved yet. Save?",
			L"Unsaved Changes",
			MB_YESNOCANCEL | MB_ICONINFORMATION | MB_APPLMODAL | MB_SETFOREGROUND);
		switch (msgboxRet) {
		case 2: { return; } // cancel
		case 6: { // yes
			std::ofstream configuration("config.txt");
			if (configuration.good()) {
				UpdateData(1);
				for (int i = 0; i < hitlist_ctrl.GetItemCount(); i++) {
					configuration << converter.to_bytes(hitlist_ctrl.GetItemText(i, NULL)) << '\n';
				}
				configuration.close();
				changed = false;
			}
			else {
				MessageBox(L"Failed to save.", NULL, MB_ICONERROR | MB_OK | MB_APPLMODAL | MB_SETFOREGROUND);
				PostThreadMessage(killerThreadID, 0x0421, NULL, NULL);
				CDialogEx::OnOK();
				return;
			}
		}
		case 7: { break; } // no
		}
	
	}

	// display notification area icon
	NOTIFYICONDATA notifyicondata;
	notifyicondata.cbSize = sizeof(notifyicondata);
	notifyicondata.uVersion = NOTIFYICON_VERSION_4;
	Shell_NotifyIcon(NIM_SETVERSION, &notifyicondata);
	HMODULE moduleHandle = GetModuleHandle(NULL);
	HANDLE iconHandle = LoadImage(moduleHandle, MAKEINTRESOURCE(IDI_ICON1), 1, 0, 0, LR_DEFAULTSIZE);
	notifyicondata.hWnd = m_hWnd;
	notifyicondata.uID = 1;
	notifyicondata.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
	notifyicondata.uCallbackMessage = 0x0423;
	notifyicondata.hIcon = static_cast<HICON>(iconHandle);
	LoadString(moduleHandle, IDS_ARMED, notifyicondata.szTip, 64);
	Shell_NotifyIcon(NIM_ADD, &notifyicondata);
	

	// start killer thread
	PostThreadMessage(killerThreadID, 0x0420, NULL, NULL); 

	// hide configuration window
	this->ShowWindow(SW_HIDE);
}


void CktpkDlg::OnBnClickedExit()
{
	// remove notification icon
	NOTIFYICONDATA notifyicondata;
	notifyicondata.cbSize = sizeof(notifyicondata);
	notifyicondata.uID = 1;
	Shell_NotifyIcon(NIM_DELETE, &notifyicondata);
	// FIX THIS
	
	// post exit message to killer thread
	PostThreadMessage(killerThreadID, 0x0421, NULL, NULL);

	CDialogEx::OnOK();
}


LRESULT CktpkDlg::OnIconEvent(WPARAM wParam, LPARAM lParam)
{
	if (lParam == WM_LBUTTONDOWN) {
		PostThreadMessage(killerThreadID, 0x0422, NULL, NULL);
		this->ShowWindow(SW_SHOW);
	}
	return 0;
}
