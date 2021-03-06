// NewZonePropertyPage.cpp : implementation file
//

#include "stdafx.h"
#include "WPhast.h"
#include "NewZonePropertyPage.h"


// CNewZonePropertyPage dialog

IMPLEMENT_DYNAMIC(CNewZonePropertyPage, baseCNewZonePropertyPage)
CNewZonePropertyPage::CNewZonePropertyPage()
: baseCNewZonePropertyPage(CNewZonePropertyPage::IDD)
, m_type(0)
, m_htiMedia(0)
, m_htiBC(0)
, m_htiBCFlux(0)
, m_htiBCLeaky(0)
, m_htiBCSpec(0)
, m_htiICHead(0)
, m_htiChemIC(0)
, m_htiFlowRate(0)
, m_htiPrintLocs(0)
{
}

CNewZonePropertyPage::~CNewZonePropertyPage()
{
}

void CNewZonePropertyPage::DoDataExchange(CDataExchange* pDX)
{
	baseCNewZonePropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TREE_ZONES, m_wndTree);

	if (this->m_bFirstSetActive)
	{
		this->m_htiMedia     = this->m_wndTree.InsertItem(_T("MEDIA"));
		this->m_htiIC        = this->m_wndTree.InsertItem(_T("INITIAL_CONDITIONS"));
		this->m_htiBC        = this->m_wndTree.InsertItem(_T("BOUNDARY_CONDITIONS"));
		this->m_htiFlowRate  = this->m_wndTree.InsertItem(_T("ZONE_FLOW"));
		this->m_htiPrintLocs = this->m_wndTree.InsertItem(_T("PRINT_LOCATION"));

		// IC
		this->m_htiICHead    = this->m_wndTree.InsertItem(_T("HEAD_IC"), this->m_htiIC);
		this->m_htiChemIC    = this->m_wndTree.InsertItem(_T("CHEMISTRY_IC"), this->m_htiIC);

		// BC
		this->m_htiBCFlux    = this->m_wndTree.InsertItem(_T("FLUX_BC"), this->m_htiBC);
		this->m_htiBCLeaky   = this->m_wndTree.InsertItem(_T("LEAKY_BC"), this->m_htiBC);
		this->m_htiBCSpec    = this->m_wndTree.InsertItem(_T("SPECIFIED_HEAD_BC"), this->m_htiBC);

		// PRINT_LOCATIONS
		this->m_htiPLChem    = this->m_wndTree.InsertItem(_T("CHEMISTRY"), this->m_htiPrintLocs);
		this->m_htiPLXYZChem = this->m_wndTree.InsertItem(_T("XYZ_CHEMISTRY"), this->m_htiPrintLocs);

		this->m_wndTree.EnsureVisible(this->m_htiBCFlux);
		this->m_wndTree.EnsureVisible(this->m_htiICHead);
		this->m_wndTree.EnsureVisible(this->m_htiMedia);
		this->m_wndTree.EnsureVisible(this->m_htiFlowRate);
		this->m_wndTree.EnsureVisible(this->m_htiPLXYZChem);
	}

	if (pDX->m_bSaveAndValidate)
	{
		HTREEITEM hti = this->m_wndTree.GetSelectedItem();
		if (hti == this->m_htiMedia)
		{
			this->m_type = ID_ZONE_TYPE_MEDIA;
		}
		else if (hti == this->m_htiFlowRate)
		{
			this->m_type = ID_ZONE_TYPE_FLOW_RATE;
		}
		else if (hti == this->m_htiBC)
		{
			::AfxMessageBox("Please choose the type of boundary condition (flux_bc, leaky_bc or specified_head_bc).", MB_OK);
			pDX->Fail();
		}
		else if (hti == this->m_htiIC)
		{
			::AfxMessageBox("Please choose the type of initial condition (head_ic or chemistry_ic).", MB_OK);
			pDX->Fail();
		}
		else if (hti == this->m_htiPrintLocs)
		{
			::AfxMessageBox("Please choose the type of print location (chemistry or xyz_chemistry).", MB_OK);
			pDX->Fail();
		}
		else if (hti == this->m_htiPLChem)
		{
			this->m_type = ID_ZONE_TYPE_PL_CHEMISTRY;
		}
		else if (hti == this->m_htiPLXYZChem)
		{
			this->m_type = ID_ZONE_TYPE_PL_XYZ_CHEMISTRY;
		}
		else if (hti == this->m_htiBCFlux)
		{
			this->m_type = ID_ZONE_TYPE_BC_FLUX;
		}
		else if (hti == this->m_htiBCLeaky)
		{
			this->m_type = ID_ZONE_TYPE_BC_LEAKY;
		}
		else if (hti == this->m_htiBCSpec)
		{
			this->m_type = ID_ZONE_TYPE_BC_SPECIFIED;
		}
		else if (hti == this->m_htiICHead)
		{
			this->m_type = ID_ZONE_TYPE_IC_HEAD;
		}
		else if (hti == this->m_htiChemIC)
		{
			this->m_type = ID_ZONE_TYPE_IC_CHEM;
		}
		else 
		{
			::AfxMessageBox("Please choose the type of zone to define.", MB_OK);
			pDX->Fail();
		}
	}
	else
	{
		if (this->m_type == ID_ZONE_TYPE_MEDIA)
		{
			this->m_wndTree.SelectItem(this->m_htiMedia);
		}
		else if (this->m_type == ID_ZONE_TYPE_FLOW_RATE)
		{
			this->m_wndTree.SelectItem(this->m_htiFlowRate);
		}
		else if (this->m_type == ID_ZONE_TYPE_BC_FLUX)
		{
			this->m_wndTree.SelectItem(this->m_htiBCFlux);
		}
		else if (this->m_type == ID_ZONE_TYPE_BC_LEAKY)
		{
			this->m_wndTree.SelectItem(this->m_htiBCLeaky);
		}
		else if (this->m_type == ID_ZONE_TYPE_BC_SPECIFIED)
		{
			this->m_wndTree.SelectItem(this->m_htiBCSpec);
		}
		else if (this->m_type == ID_ZONE_TYPE_IC_HEAD)
		{
			this->m_wndTree.SelectItem(this->m_htiICHead);
		}
		else if (this->m_type == ID_ZONE_TYPE_IC_CHEM)
		{
			this->m_wndTree.SelectItem(this->m_htiChemIC);
		}
		else if (this->m_type == ID_ZONE_TYPE_PL_CHEMISTRY)
		{
			this->m_wndTree.SelectItem(this->m_htiICHead);
		}
		else if (this->m_type == ID_ZONE_TYPE_PL_XYZ_CHEMISTRY)
		{
			this->m_wndTree.SelectItem(this->m_htiPLXYZChem);
		}
		else
		{
			this->m_wndTree.SelectItem(this->m_htiMedia);
		}
	}
}

BEGIN_MESSAGE_MAP(CNewZonePropertyPage, baseCNewZonePropertyPage)
	ON_NOTIFY(NM_DBLCLK, IDC_TREE_ZONES, OnNMDblclkTreeZones)
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE_ZONES, OnTVNSelChanged)
END_MESSAGE_MAP()

// CNewZonePropertyPage message handlers

BOOL CNewZonePropertyPage::OnInitDialog()
{
	baseCNewZonePropertyPage::OnInitDialog();

	// Add extra initialization here

	// Store pointers to all the pages in m_PropPageArray.
	// 0 : this page            CNewZonePropertyPage
	// 1 : media page           CMediaPropertyPage
	// 2 : flux page            CBCFluxPropertyPage/CBCFluxPropertyPage2
	// 3 : leaky page           CBCLeakyPropertyPage/CBCLeakyPropertyPage2
	// 4 : specified page       CBCSpecifiedPropertyPage/CBCSpecifiedHeadPropertyPage
	// 5 : head_ic page         CICHeadPropertyPage
	// 6 : chem_ic page         CChemICPropertyPage/CChemICSpreadPropertyPage
	// 7 : zone_flow page       CZoneFlowRatePropertyPage
	// 8 : print_locations page CPrintLocsPropsPage
	
	CPropertySheet* pSheet = (CPropertySheet*) this->GetParent();   
	if (pSheet->IsWizard())
	{
		int nCount = pSheet->GetPageCount();
		ASSERT(nCount == 9);
		for (int i = 0; i < nCount; ++i)
		{
			this->m_PropPageArray.Add(pSheet->GetPage(i));
		}

		// remove all but this page
		for (int i = 1; i < nCount; ++i)
		{
			pSheet->RemovePage(1);
		}
	}

#if defined(USE_LAYOUT)
	// Layout controls
	this->CreateRoot(VERTICAL)
		<< item(IDC_TREE_ZONES, GREEDY)
		;
	this->UpdateLayout();
#endif

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CNewZonePropertyPage::OnSetActive()
{
	BOOL bRet = baseCNewZonePropertyPage::OnSetActive();
	CPropertySheet* pSheet = static_cast<CPropertySheet*>(this->GetParent());
	ASSERT_KINDOF(CPropertySheet, pSheet);
	if (pSheet->IsWizard())
	{
#ifdef SAVE
		HTREEITEM hti = this->m_wndTree.GetSelectedItem();
		if (hti == this->m_htiFlowRate)
		{
			pSheet->SetWizardButtons(PSWIZB_FINISH);
		}
		else
#endif
		{
			pSheet->SetWizardButtons(PSWIZB_NEXT);
		}
	}
	return bRet;
}

UINT CNewZonePropertyPage::GetType(void)
{
	return m_type;
}

LRESULT CNewZonePropertyPage::OnWizardNext()
{
	// Add your specialized code here and/or call the base class
	CPropertySheet* pSheet = (CPropertySheet*) this->GetParent();

	// 0 : this page            CNewZonePropertyPage
	// 1 : media page           CMediaPropertyPage
	// 2 : flux page            CBCFluxPropertyPage/CBCFluxPropertyPage2
	// 3 : leaky page           CBCLeakyPropertyPage/CBCLeakyPropertyPage2
	// 4 : specified page       CBCSpecifiedPropertyPage/CBCSpecifiedHeadPropertyPage
	// 5 : head_ic page         CICHeadPropertyPage
	// 6 : chem_ic page         CChemICPropertyPage/CChemICSpreadPropertyPage
	// 7 : zone_flow page       CZoneFlowRatePropertyPage
	// 8 : print_locations page CPrintLocsPropsPage

	// remove all but this page
	int nCount = pSheet->GetPageCount();
	for (int i = 1; i < nCount; ++i)
	{
		pSheet->RemovePage(1);
	}

	// add only the next page
	if (this->UpdateData(TRUE))
	{
		switch (this->m_type)
		{
		case ID_ZONE_TYPE_MEDIA:
			pSheet->AddPage(this->m_PropPageArray[1]);
			break;
		case ID_ZONE_TYPE_BC_FLUX:
			pSheet->AddPage(this->m_PropPageArray[2]);
			break;
		case ID_ZONE_TYPE_BC_LEAKY:
			pSheet->AddPage(this->m_PropPageArray[3]);
			break;
		case ID_ZONE_TYPE_BC_SPECIFIED:
			pSheet->AddPage(this->m_PropPageArray[4]);
			break;
		case ID_ZONE_TYPE_IC_HEAD:
			pSheet->AddPage(this->m_PropPageArray[5]);
			break;
		case ID_ZONE_TYPE_IC_CHEM:
			pSheet->AddPage(this->m_PropPageArray[6]);
			break;
		case ID_ZONE_TYPE_FLOW_RATE:
			pSheet->AddPage(this->m_PropPageArray[7]);
			break;
		case ID_ZONE_TYPE_PL_CHEMISTRY:
		case ID_ZONE_TYPE_PL_XYZ_CHEMISTRY:
			pSheet->AddPage(this->m_PropPageArray[8]);
			break;
		default:
			ASSERT(FALSE);
			break;
		}
		return baseCNewZonePropertyPage::OnWizardNext();
	}
	return 1;
}

void CNewZonePropertyPage::OnNMDblclkTreeZones(NMHDR *pNMHDR, LRESULT *pResult)
{
	// Add your control notification handler code here
	HTREEITEM hti = this->m_wndTree.GetSelectedItem();
	if ((hti != NULL) && (hti != this->m_htiBC) && (hti != this->m_htiIC) /* && (hti != this->m_htiFlowRate) */)
	{	
		// simulate user pressing next
		CPropertySheet *pSheet = static_cast<CPropertySheet*>(this->GetParent());
		pSheet->PressButton(PSBTN_NEXT);
	}
#ifdef SAVE
	else if (hti == m_htiFlowRate)
	{
		// simulate user pressing finish
		CPropertySheet *pSheet = static_cast<CPropertySheet*>(this->GetParent());
		pSheet->PressButton(PSBTN_FINISH);
	}
#endif
	*pResult = 0;
}

BOOL CNewZonePropertyPage::OnWizardFinish()
{
	// add only the next page
#ifdef SAVE
	if (this->UpdateData(TRUE))
	{
		ASSERT(this->m_type == ID_ZONE_TYPE_FLOW_RATE);
	}
#endif
	return baseCNewZonePropertyPage::OnWizardFinish();
}

void CNewZonePropertyPage::OnTVNSelChanged(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	// Add your control notification handler code here

#ifdef SAVE
	CPropertySheet* pSheet = static_cast<CPropertySheet*>(this->GetParent());
	ASSERT_KINDOF(CPropertySheet, pSheet);

	if (pSheet->IsWizard())
	{
		HTREEITEM hti = this->m_wndTree.GetSelectedItem();
		if (hti == this->m_htiFlowRate)
		{
			pSheet->SetWizardButtons(PSWIZB_FINISH);
		}
		else
		{
			pSheet->SetWizardButtons(PSWIZB_NEXT);
		}
	}
#endif

	*pResult = 0;
}
