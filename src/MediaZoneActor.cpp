#include "StdAfx.h"
#include "resource.h"
#include "MediaZoneActor.h"
#include "PropertyTreeControlBar.h"
#include "MediaPropertyPage.h"
#include "MediaSpreadPropertyPage.h"
#include "property.h"

#include "WPhastDoc.h"
#include "SetMediaAction.h"
#include "ZoneCreateAction.h"
#include "FlowOnly.h"
#include "Protect.h"

//#include "MediaProps.h"
#include "PropsPage.h"
#include "TreePropSheetEx/TreePropSheetEx.h"


#include <vtkPropAssembly.h>
#include <vtkObjectFactory.h> // reqd by vtkStandardNewMacro

vtkCxxRevisionMacro(CMediaZoneActor, "$Revision$");
vtkStandardNewMacro(CMediaZoneActor);

const char CMediaZoneActor::szHeading[] = "Media";
vtkFloatingPointType CMediaZoneActor::s_color[3] = {0., 0., 0.};
vtkProperty* CMediaZoneActor::s_Property = 0;
vtkProperty* CMediaZoneActor::s_OutlineProperty = 0;


// Note: No header files should follow the following three lines
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CMediaZoneActor::CMediaZoneActor(void)
{
	static StaticInit init; // constructs/destructs s_Property/s_OutlineProperty

	// cube
	this->CubeActor->SetProperty(CMediaZoneActor::s_Property);

	// outline
	this->OutlineActor->SetProperty(CMediaZoneActor::s_OutlineProperty);
}

CMediaZoneActor::~CMediaZoneActor(void)
{
}

void CMediaZoneActor::Create(CWPhastDoc* pWPhastDoc, const CGridElt& gridElt, LPCTSTR desc)
{
	ASSERT(gridElt.polyh && ::AfxIsValidAddress(gridElt.polyh, sizeof(Polyhedron)));
	CZoneCreateAction<CMediaZoneActor>* pAction = new CZoneCreateAction<CMediaZoneActor>(
		pWPhastDoc,
		gridElt.polyh,
		desc
		);
	pAction->GetZoneActor()->SetGridElt(gridElt);
	pWPhastDoc->Execute(pAction);
}

void CMediaZoneActor::Serialize(bool bStoring, hid_t loc_id, const CWPhastDoc* pWPhastDoc)
{
	CZoneActor::Serialize(bStoring, loc_id, pWPhastDoc);

	if (bStoring)
	{
		// store grid_elt
		this->m_grid_elt.Serialize(bStoring, loc_id);
	}
	else
	{
		// load grid_elt
		this->m_grid_elt.Serialize(bStoring, loc_id);

		// color
		if (CMediaZoneActor::s_Property)
		{
			this->CubeActor->SetProperty(CMediaZoneActor::s_Property);
		}
	}
}

void CMediaZoneActor::Insert(CPropertyTreeControlBar* pTreeControlBar, HTREEITEM hInsertAfter)
{
	CTreeCtrl* pTreeCtrl = pTreeControlBar->GetTreeCtrl();
	HTREEITEM htiMedia = pTreeControlBar->GetMediaNode();
	this->InsertAt(pTreeCtrl, htiMedia, hInsertAfter);
	//{{HACK
	CTreeCtrlNode node(this->m_hti, pTreeControlBar->GetTreeCtrlEx());
	pTreeControlBar->SetNodeCheck(node, BST_CHECKED);
	//}}HACK
}

void CMediaZoneActor::InsertAt(CTreeCtrl* pTreeCtrl, HTREEITEM hParent, HTREEITEM hInsertAfter)
{
	this->m_hti = pTreeCtrl->InsertItem(this->GetNameDesc(), hParent, hInsertAfter);
	pTreeCtrl->SetItemData(this->m_hti, (DWORD_PTR)this);
	pTreeCtrl->SelectItem(this->m_hti); // might want to move this -- this causes the tree control to redraw
	this->Update(pTreeCtrl, this->m_hti);
}

void CMediaZoneActor::Remove(CPropertyTreeControlBar* pTreeControlBar)
{
	CZoneActor::Remove(pTreeControlBar);
}

void CMediaZoneActor::UnRemove(CPropertyTreeControlBar* pTreeControlBar)
{
	HTREEITEM htiMedia = pTreeControlBar->GetMediaNode();
	ASSERT(this->m_hParent      == htiMedia);
	ASSERT(this->m_hInsertAfter != 0);

	vtkPropAssembly *pPropAssembly = pTreeControlBar->GetDocument()->GetPropAssemblyMedia();
	pPropAssembly->AddPart(this);

	CZoneActor::UnRemove(pTreeControlBar);
}

void CMediaZoneActor::Update(CTreeCtrl* pTreeCtrl, HTREEITEM htiParent)
{
	CZoneActor::Update(pTreeCtrl, htiParent);

	CString format;

// COMMENT: {6/23/2008 1:47:26 PM}	// remove all previous items
// COMMENT: {6/23/2008 1:47:26 PM}	//
// COMMENT: {6/23/2008 1:47:26 PM}	while (HTREEITEM hChild = pTreeCtrl->GetChildItem(htiParent))
// COMMENT: {6/23/2008 1:47:26 PM}	{
// COMMENT: {6/23/2008 1:47:26 PM}		pTreeCtrl->DeleteItem(hChild);
// COMMENT: {6/23/2008 1:47:26 PM}	}
// COMMENT: {6/23/2008 1:47:26 PM}
// COMMENT: {6/23/2008 1:47:26 PM}	// update description
// COMMENT: {6/23/2008 1:47:26 PM}	//
// COMMENT: {6/23/2008 1:47:26 PM}	pTreeCtrl->SetItemText(htiParent, this->GetNameDesc());
// COMMENT: {6/23/2008 1:47:26 PM}
// COMMENT: {6/23/2008 1:47:26 PM}	//{{
// COMMENT: {6/23/2008 1:47:26 PM}	if (this->GetPolyhedronType() == Polyhedron::PRISM)
// COMMENT: {6/23/2008 1:47:26 PM}	{
// COMMENT: {6/23/2008 1:47:26 PM}		HTREEITEM hItem;
// COMMENT: {6/23/2008 1:47:26 PM}		hItem = pTreeCtrl->InsertItem("Top", htiParent);
// COMMENT: {6/23/2008 1:47:26 PM}		pTreeCtrl->SetItemState(hItem, INDEXTOSTATEIMAGEMASK(BST_CHECKED + 1), TVIS_STATEIMAGEMASK);
// COMMENT: {6/23/2008 1:47:26 PM}
// COMMENT: {6/23/2008 1:47:26 PM}		hItem = pTreeCtrl->InsertItem("Perimeter", htiParent);
// COMMENT: {6/23/2008 1:47:26 PM}		pTreeCtrl->SetItemState(hItem, INDEXTOSTATEIMAGEMASK(BST_CHECKED + 1), TVIS_STATEIMAGEMASK);
// COMMENT: {6/23/2008 1:47:26 PM}
// COMMENT: {6/23/2008 1:47:26 PM}		hItem = pTreeCtrl->InsertItem("Bottom", htiParent);
// COMMENT: {6/23/2008 1:47:26 PM}		pTreeCtrl->SetItemState(hItem, INDEXTOSTATEIMAGEMASK(BST_CHECKED + 1), TVIS_STATEIMAGEMASK);
// COMMENT: {6/23/2008 1:47:26 PM}	}
// COMMENT: {6/23/2008 1:47:26 PM}	//}}

	// active
	if (this->m_grid_elt.active)
	{
		static_cast<Cproperty*>(this->m_grid_elt.active)->Insert(pTreeCtrl, htiParent, "active");
	}
	else if (this->GetDefault())
	{
		Cproperty prop(1);
		prop.Insert(pTreeCtrl, htiParent, "active");
	}

	// kx
	if (this->m_grid_elt.kx)
	{
		static_cast<Cproperty*>(this->m_grid_elt.kx)->Insert(pTreeCtrl, htiParent, "Kx");
	}

	// ky
	if (this->m_grid_elt.ky)
	{
		static_cast<Cproperty*>(this->m_grid_elt.ky)->Insert(pTreeCtrl, htiParent, "Ky");
	}

	// kz
	if (this->m_grid_elt.kz)
	{
		static_cast<Cproperty*>(this->m_grid_elt.kz)->Insert(pTreeCtrl, htiParent, "Kz");
	}

	// porosity
	if (this->m_grid_elt.porosity)
	{
		static_cast<Cproperty*>(this->m_grid_elt.porosity)->Insert(pTreeCtrl, htiParent, "porosity");
	}

	// storage
	if (this->m_grid_elt.storage)
	{
		static_cast<Cproperty*>(this->m_grid_elt.storage)->Insert(pTreeCtrl, htiParent, "specific_storage");
	}

	// alpha_long
	if (this->m_grid_elt.alpha_long)
	{
		static_cast<Cproperty*>(this->m_grid_elt.alpha_long)->Insert(pTreeCtrl, htiParent, "long_dispersivity");
	}

	// alpha_horizontal
	if (this->m_grid_elt.alpha_horizontal)
	{
		static_cast<Cproperty*>(this->m_grid_elt.alpha_horizontal)->Insert(pTreeCtrl, htiParent, "horizontal_dispersivity");
	}

	// alpha_vertical
	if (this->m_grid_elt.alpha_vertical)
	{
		static_cast<Cproperty*>(this->m_grid_elt.alpha_vertical)->Insert(pTreeCtrl, htiParent, "vertical_dispersivity");
	}
}

void CMediaZoneActor::Edit(CTreeCtrl* pTreeCtrl)
{
	CString str;
	str.Format(_T("%s Properties"), this->GetName());
// COMMENT: {3/31/2009 4:55:31 PM}	CPropertySheet props(str);
	TreePropSheet::CTreePropSheetEx props(str);

	CFrameWnd *pFrame = (CFrameWnd*)::AfxGetApp()->m_pMainWnd;
	ASSERT_VALID(pFrame);
	CWPhastDoc* pDoc = reinterpret_cast<CWPhastDoc*>(pFrame->GetActiveDocument());
	ASSERT_VALID(pDoc);

	ASSERT(this->GetData().polyh);

	// set default as active
	//
	CGridElt elt(this->m_grid_elt);
	if (this->GetDefault())
	{
		if (!elt.active || elt.active->type == PROP_UNDEFINED)
		{
			Cproperty prop(1);
			Cproperty::CopyProperty(&elt.active, &prop);
		}
	}

// COMMENT: {3/31/2009 5:04:31 PM}	CMediaSpreadPropertyPage mediaSpreadProps;
// COMMENT: {3/31/2009 5:04:31 PM}	mediaSpreadProps.SetProperties(elt);
// COMMENT: {3/31/2009 5:04:31 PM}	mediaSpreadProps.SetDesc(this->GetDesc());
// COMMENT: {3/31/2009 5:04:31 PM}	if (this->GetDefault())
// COMMENT: {3/31/2009 5:04:31 PM}	{
// COMMENT: {3/31/2009 5:04:31 PM}		mediaSpreadProps.SetDefault(true);
// COMMENT: {3/31/2009 5:04:31 PM}		mediaSpreadProps.SetFlowOnly(bool(pDoc->GetFlowOnly()));
// COMMENT: {3/31/2009 5:04:31 PM}	}
// COMMENT: {3/31/2009 5:04:31 PM}	props.AddPage(&mediaSpreadProps);

	//{{
// COMMENT: {3/31/2009 5:10:56 PM}	CMediaProps mediaProps;
	CPropsPage mediaProps;


	mediaProps.SetProperties(elt);
	mediaProps.SetDesc(this->GetDesc());
	if (this->GetDefault())
	{
		mediaProps.SetDefault(true);
		mediaProps.SetFlowOnly(bool(pDoc->GetFlowOnly()));
	}
	props.AddPage(&mediaProps);
	//}}

	if (props.DoModal() == IDOK)
	{
// COMMENT: {3/31/2009 5:05:38 PM}		CGridElt grid_elt;
// COMMENT: {3/31/2009 5:05:38 PM}		mediaSpreadProps.GetProperties(grid_elt);
// COMMENT: {3/31/2009 5:05:38 PM}		ASSERT(grid_elt.polyh);
// COMMENT: {3/31/2009 5:05:38 PM}		pDoc->Execute(new CSetMediaAction(this, pTreeCtrl, grid_elt, mediaSpreadProps.GetDesc()));

		CGridElt GridElt;
		mediaProps.GetProperties(GridElt);
		ASSERT(GridElt.polyh);
		pDoc->Execute(new CSetMediaAction(this, pTreeCtrl, GridElt, mediaProps.GetDesc()));

	}
}

CGridElt CMediaZoneActor::GetGridElt(void)const
{
	return this->m_grid_elt;
}

CGridElt CMediaZoneActor::GetData(void)const
{
	return this->GetGridElt();
}

void CMediaZoneActor::SetGridElt(const CGridElt& rGridElt)
{
	// NOTE: Use SetBounds to set the zone
	//
	CProtect p(this->m_grid_elt.polyh);
	this->m_grid_elt = rGridElt;
}


void CMediaZoneActor::SetData(const CGridElt& rGridElt)
{
	this->SetGridElt(rGridElt);
}

HTREEITEM CMediaZoneActor::GetHTreeItem(void)const
{
	return this->m_hti;
}

void CMediaZoneActor::Add(CWPhastDoc *pWPhastDoc)
{
	if (!pWPhastDoc)
	{
		ASSERT(FALSE);
		return;
	}
	if (vtkPropAssembly *pPropAssembly = pWPhastDoc->GetPropAssemblyMedia())
	{
		pPropAssembly->AddPart(this);
		if (!pWPhastDoc->GetPropCollection()->IsItemPresent(pPropAssembly))
		{
			pWPhastDoc->GetPropCollection()->AddItem(pPropAssembly);
		}
	}
#ifdef _DEBUG
	else ASSERT(FALSE);
#endif
}

void CMediaZoneActor::Remove(CWPhastDoc *pWPhastDoc)
{
	if (!pWPhastDoc)
	{
		ASSERT(FALSE);
		return;
	}
	if (vtkPropAssembly *pPropAssembly = pWPhastDoc->GetPropAssemblyMedia())
	{
		pPropAssembly->RemovePart(this);
		// VTK HACK
		// This is req'd because ReleaseGraphicsResources is not called when
		// vtkPropAssembly::RemovePart(vtkProp *prop) is called
		pWPhastDoc->ReleaseGraphicsResources(this);
	}
#ifdef _DEBUG
	else ASSERT(FALSE);
#endif
}

void CMediaZoneActor::SetStaticColor(COLORREF cr)
{
	CMediaZoneActor::s_color[0] = (double)GetRValue(cr)/255.;
	CMediaZoneActor::s_color[1] = (double)GetGValue(cr)/255.;
	CMediaZoneActor::s_color[2] = (double)GetBValue(cr)/255.;	
	if (CMediaZoneActor::s_Property)
	{
		CMediaZoneActor::s_Property->SetColor(CMediaZoneActor::s_color);
	}
	if (CMediaZoneActor::s_OutlineProperty)
	{
		CMediaZoneActor::s_OutlineProperty->SetColor(CMediaZoneActor::s_color);
		CMediaZoneActor::s_OutlineProperty->SetEdgeColor(CMediaZoneActor::s_color);
		CMediaZoneActor::s_OutlineProperty->SetAmbientColor(CMediaZoneActor::s_color);
	}
}

Polyhedron*& CMediaZoneActor::GetPolyhedron(void)
{
	return this->m_grid_elt.polyh;
}
