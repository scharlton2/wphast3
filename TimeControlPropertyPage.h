#pragma once


// CTimeControlPropertyPage dialog
#include <string>
#include "time.h"
#include "TimeControl.h"
#include "Units.h"

class CTimeControlPropertyPage : public CPropertyPage
{
	DECLARE_DYNAMIC(CTimeControlPropertyPage)

public:
	CTimeControlPropertyPage();
	CTimeControlPropertyPage(UINT nIDCaption);
	CTimeControlPropertyPage(UINT nIDCaption, UINT nIDHeaderTitle, UINT nIDHeaderSubTitle = 0);
	virtual ~CTimeControlPropertyPage();

	void SetUnits(const CUnits& units);

	void SetProperties(const CTimeControl& timeControl);
	void GetProperties(CTimeControl& timeControl)const;

	void SetPreviousTimeStep(const CTimeControl& timeControl);
	void GetPreviousTimeStep(CTimeControl& timeControl)const;

// Dialog Data
	enum { IDD = IDD_TC_INT_PROPPAGE };

protected:
	void CommonConstruct(void);
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	CRichEditCtrl     m_wndRichEditCtrl;
	CTimeControl      m_timeControl;
	CTimeControl*     m_pLastTimeControl;

	struct units      m_units;

	std::string       m_sTimeStepRTF;    // IDR_TC_TIME_STEP_RTF
	std::string       m_sTimeChangeRTF;  // IDR_TC_TIME_CHANGE_RTF

public:
	CComboBox m_wndTimeStepCombo;
	CComboBox m_wndTimeChangeCombo;
	virtual BOOL OnInitDialog();
	virtual BOOL OnSetActive();
	afx_msg void OnEnSetfocusTimeStepEdit();
	afx_msg void OnEnSetfocusTimeChangeEdit();
	afx_msg void OnCbnSetfocusTimeStepCombo();
	afx_msg void OnCbnSetfocusTimeChangeCombo();
};
