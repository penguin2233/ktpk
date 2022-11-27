
// ktpk.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CktpkApp:
// See ktpk.cpp for the implementation of this class
//

class CktpkApp : public CWinApp
{
public:
	CktpkApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation


};

extern CktpkApp theApp;
