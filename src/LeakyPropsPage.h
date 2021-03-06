#pragma once

#include "PropsPropertyPage.h"
#include "BC.h"
#include "gridctrl/ModGridCtrlEx.h"
#include "afxwin.h"

#include "GridTimeSeries.h"

class CUnits;

// CLeakyPropsPage dialog

class CLeakyPropsPage : public CPropsPropertyPage
{
	DECLARE_DYNAMIC(CLeakyPropsPage)

public:
	CLeakyPropsPage();
	virtual ~CLeakyPropsPage();

	void SetProperties(const CBC& rBC);
	void GetProperties(CBC& rBC)const;

	void SetFlowOnly(bool flowOnly)      { this->FlowOnly = flowOnly; }
	bool GetFlowOnly(void)const          { return this->FlowOnly; }

	void SetFreeSurface(bool freeSurface){ this->FreeSurface = freeSurface; }
	bool SetFreeSurface(void)const       { return this->FreeSurface; }

	void SetDefault(bool bDefault)       { this->Default = bDefault; }
	bool GetDefault(void)const           { return this->Default; }

	void SetDesc(LPCTSTR desc)           { this->Description = desc; }
	LPCTSTR GetDesc()                    { return this->Description; }

	void SetUnits(const CUnits &u);

// Dialog Data
	enum { IDD = IDD_PROPS_LEAKY };

	// type enum
	enum ModeType
	{
		NONE     = 0, // also represents chosen property
		CONSTANT = 1,
		LINEAR   = 2,
		POINTS   = 3,
		XYZ      = 4,
	};

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void DDV_SoftValidate();
	virtual void DDX_Series(CDataExchange* pDX);
	virtual void DDX_Single(CDataExchange* pDX);
	virtual void SetPropertyDescription();

	DECLARE_MESSAGE_MAP()

	//{{ COMMON
	// end edit time series grids
	afx_msg void OnEndLabelEditFlux(NMHDR *pNotifyStruct, LRESULT *result);
	afx_msg void OnEndLabelEditSolution(NMHDR *pNotifyStruct, LRESULT *result);

	// selection changed time series grids
	afx_msg void OnSelChangedFlux(NMHDR *pNotifyStruct, LRESULT *result);
	afx_msg void OnSelChangedSolution(NMHDR *pNotifyStruct, LRESULT *result);

	// browse for XYZ and XYZT
	afx_msg void OnBnClickedButtonXYZ();

	// DDV failures
	afx_msg LRESULT OnUM_DDXFailure(WPARAM wParam, LPARAM lParam);

	// should be on superclass
	afx_msg void OnEnSetfocusDescEdit();

	// single property combo
	afx_msg void OnCbnSelchangeComboProptype();

	// mixture checkbox
	afx_msg void OnBnClickedCheckMixture();
	//}} COMMON

	// override
	afx_msg void OnTreeSelChanging(NMHDR *pNotifyStruct, LRESULT *plResult);

	// face checkbox
	afx_msg void OnBnClickedCheckFace();
	afx_msg void OnBnSetfocusCheckFace();

	// property controllers
	CGridTimeSeries ThicknessProperty; // single
	CGridTimeSeries HydCondProperty;   // single
	CGridTimeSeries HeadSeries;
	CGridTimeSeries SolutionSeries;
	CGridTimeSeries ElevationProperty; // single

	// RTF strings
	std::string m_sDescriptionRTF;   // IDR_DESCRIPTION_RTF
	std::string m_sHeadRTF;          // 
	std::string m_sThicknessRTF;     // IDR_BC_LEAKY_THICKNESS_RTF
	std::string m_sHydCondRTF;       // IDR_BC_LEAKY_HYD_COND_RTF
	std::string m_sAssocSolutionRTF; // IDR_BC_LEAKY_ASSOC_SOL_RTF
	std::string m_sElevationRTF;     // IDR_BC_LEAKY_ELEVATION_RTF
	std::string m_sFaceRTF;          // IDR_BC_LEAKY_FACE_RTF
	std::string m_sUseMapZRTF;       // IDR_BC_LEAKY_USE_MAP_Z_RTF

	// should be member of superclass
	CString Description;

	// flags
	bool FlowOnly;
	bool Default;
	bool bSkipFaceValidation;
	bool FreeSurface;

protected:
	// data
	CBC BC;
public:
	afx_msg void OnBnSetfocusUseMapCoorZ();
};
