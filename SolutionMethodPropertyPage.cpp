// SolutionMethodPropertyPage.cpp : implementation file
//

#include "stdafx.h"
#include "WPhast.h"
#include "SolutionMethodPropertyPage.h"

#include "Global.h"


// CSolutionMethodPropertyPage dialog

IMPLEMENT_DYNAMIC(CSolutionMethodPropertyPage, CPropertyPage)
CSolutionMethodPropertyPage::CSolutionMethodPropertyPage()
	: CPropertyPage(CSolutionMethodPropertyPage::IDD)
{
}

CSolutionMethodPropertyPage::~CSolutionMethodPropertyPage()
{
}

void CSolutionMethodPropertyPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);

	if (pDX->m_bSaveAndValidate)
	{
		switch (this->GetCheckedRadioButton(IDC_RADIO_ITER, IDC_RADIO_DIRECT))
		{
		case (IDC_RADIO_DIRECT):
			this->solutionMethod.solver_method = CSolutionMethod::SMT_DIRECT;
			break;
		default:
			this->solutionMethod.solver_method = CSolutionMethod::SMT_ITERATIVE;
			break;
		}
	}
	else
	{
		switch (this->solutionMethod.solver_method)
		{
		case (CSolutionMethod::SMT_DIRECT):
			this->CheckRadioButton(IDC_RADIO_ITER, IDC_RADIO_DIRECT, IDC_RADIO_DIRECT);
			this->EnableIterative(FALSE);
			break;
		default:
			this->CheckRadioButton(IDC_RADIO_ITER, IDC_RADIO_DIRECT, IDC_RADIO_ITER);
			this->EnableIterative(TRUE);
			break;
		}
	}

	DDX_Text(pDX, IDC_EDIT_TOL, this->solutionMethod.solver_tolerance);

	DDX_Text(pDX, IDC_EDIT_SAVE, this->solutionMethod.solver_save_directions);

	DDX_Text(pDX, IDC_EDIT_MAX, this->solutionMethod.solver_maximum);

	DDX_Text(pDX, IDC_EDIT_SPACE, this->solutionMethod.solver_space);
	if (pDX->m_bSaveAndValidate && (this->solutionMethod.solver_space < 0.0 || this->solutionMethod.solver_space > 0.5))
	{
		::AfxMessageBox("The weighting factor for spatial differencing must be between 0.0 and 0.5.");
		pDX->Fail();
	}

	DDX_Text(pDX, IDC_EDIT_TIME, this->solutionMethod.solver_time);
	if (pDX->m_bSaveAndValidate && (this->solutionMethod.solver_time < 0.5 || this->solutionMethod.solver_time > 1.0))
	{
		::AfxMessageBox("The weighting factor for time differencing must be between 0.5 and 1.0.");
		pDX->Fail();
	}

	CGlobal::DDX_Check(pDX, IDC_CHECK_CROSS_DISP, this->solutionMethod.cross_dispersion);
	DDX_Control(pDX, IDC_SPIN_SAVE, spinSave);
	DDX_Control(pDX, IDC_SPIN_MAX, spinMax);

	DDX_Text(pDX, IDC_EDIT_REBAL, this->solutionMethod.rebalance_fraction);
	if (pDX->m_bSaveAndValidate && (this->solutionMethod.rebalance_fraction < 0.0 || this->solutionMethod.rebalance_fraction > 1.0))
	{
		::AfxMessageBox("The rebalancing fraction must be between 0.0 and 1.0.");
		pDX->Fail();
	}

}


BEGIN_MESSAGE_MAP(CSolutionMethodPropertyPage, CPropertyPage)
	ON_BN_CLICKED(IDC_RADIO_ITER, OnBnClickedRadioIter)
	ON_BN_CLICKED(IDC_RADIO_DIRECT, OnBnClickedRadioDirect)
END_MESSAGE_MAP()

// CSolutionMethodPropertyPage message handlers

void CSolutionMethodPropertyPage::EnableIterative(BOOL value)
{
	int Ids[] = 
	{
		IDC_STATIC_TOL,
		IDC_STATIC_SAVE,
		IDC_STATIC_MAX,
		IDC_EDIT_TOL,
		IDC_EDIT_SAVE,
		IDC_EDIT_MAX,
	};

	for (int i = 0; i < sizeof(Ids) / sizeof(Ids[0]); ++i)
	{
		if (CWnd *pWnd = this->GetDlgItem(Ids[i]))
		{
			pWnd->EnableWindow(value);
		}
	}
}

void CSolutionMethodPropertyPage::OnBnClickedRadioIter()
{
	switch (this->GetCheckedRadioButton(IDC_RADIO_ITER, IDC_RADIO_DIRECT))
	{
	case (IDC_RADIO_ITER):
		this->EnableIterative(TRUE);
		break;
	case (IDC_RADIO_DIRECT):
		this->EnableIterative(FALSE);
		break;
	default:
		ASSERT(FALSE);
		break;
	}
}

void CSolutionMethodPropertyPage::OnBnClickedRadioDirect()
{
	switch (this->GetCheckedRadioButton(IDC_RADIO_ITER, IDC_RADIO_DIRECT))
	{
	case (IDC_RADIO_ITER):
		this->EnableIterative(TRUE);
		break;
	case (IDC_RADIO_DIRECT):
		this->EnableIterative(FALSE);
		break;
	default:
		ASSERT(FALSE);
		break;
	}
}

BOOL CSolutionMethodPropertyPage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	this->spinSave.SetRange32(1, INT_MAX);
	this->spinMax.SetRange32(1, INT_MAX);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}