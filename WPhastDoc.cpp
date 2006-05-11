// WPhastDoc.cpp : implementation of the CWPhastDoc class
//
#include "stdafx.h"
#include "WPhast.h"

#include "WPhastDoc.h"
#include "WPhastView.h"

#include "srcimpl.h"       // SrcFullPath SrcResolveShortcut
#include "HDFMirrorFile.h"
#include "PhastInput.h"
#include "PrintFreqPropertyPage.h"

// #include <vtkPropCollection.h>
#include <vtkCubeSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkLODActor.h>
#include <vtkProperty.h>

#include <vtkAxes.h>
#include <vtkTubeFilter.h>
#include <vtkAssembly.h>
#include <vtkPropAssembly.h>
#include <vtkProp3DCollection.h>

#include <vector>
#include <list>
#include <fstream>  // std::ifstream
#include <sstream>  // std::ostringstream std::istringstream

#include "Action.h"
#include "ZoneActor.h"
#include "MediaZoneActor.h"
#include "BCZoneActor.h"
#include "ICZoneActor.h"
#include "ICHeadZoneActor.h"
#include "ICChemZoneActor.h"

#include "ISerial.h"
#include "DelayRedraw.h"

////#include "ZoneLODActor.h"
#include "GridActor.h"
#include "AxesActor.h"
#include "Global.h"

#include "ImportErrorDialog.h"
#include "RunDialog.h"

#include "PropertyTreeControlBar.h"
#include "BoxPropertiesDialogBar.h"
#include "TimeControlPropertyPage.h"

#include "ModelessPropertySheet.h"
#include "ETSLayoutModelessPropertySheet.h"
#include "ScalePropertyPage.h"

#include "GridPropertyPage2.h"
#include "GridRefinePage.h"
#include "GridCoarsenPage.h"
#include "GridElementsSelector.h"

#include "NewZoneWidget.h"

#include <vtkBoxWidget.h>

#include <vtkAbstractPropPicker.h>
#include <vtkWin32RenderWindowInteractor.h>

// #include "CreateZoneAction.h"
#include "ZoneCreateAction.h"
#include "AddStressPeriodAction.h"
#include "WellCreateAction.h"
#include "RiverCreateAction.h"
#include "RiverMovePointAction.h"
#include "RiverInsertPointAction.h"
#include "GridDeleteLineAction.h"
#include "GridInsertLineAction.h"
#include "GridMoveLineAction.h"

#include "Unit.h"
#include "Units.h"
#include "NewModel.h"
#include "FlowOnly.h"
#include "SteadyFlow.h"
#include "FreeSurface.h"
#include "RedrawOnDtor.h"

#include "HeadIC.h"
#include "TimeControl2.h"
#include "MapActor.h"
#include "MapImageActor.h"
#include "WorldTransform.h"
#include "SiteMap.h"
#include "NewModel.h"

#include "WellActor.h"
#include "RiverActor.h"

extern void GetDefaultMedia(struct grid_elt* p_grid_elt);
extern void GetDefaultHeadIC(struct head_ic* p_head_ic);
extern void GetDefaultChemIC(struct chem_ic* p_chem_ic);

static const TCHAR szZoneFormat[]    = _T("Zone %d");
static const TCHAR szZoneFind[]      = _T("Zone ");


extern "C" {
int error_msg (const char *err_str, const int stop);
}

// Note: No header files should follow the following three lines
#ifdef _DEBUG
#define new DEBUG_NEW
#endif


struct WPhastDocImpl {
	// Note: see http://www.gotw.ca/publications/mill04.htm
	std::vector<CAction*> m_vectorActions;
	std::vector<CAction*>::size_type m_vectorActionsIndex;
	std::vector<CAction*>::size_type m_lastSaveIndex;
	std::vector<CZone*> m_vectorDefaultZones;
}; 



// CWPhastDoc

IMPLEMENT_DYNCREATE(CWPhastDoc, CDocument)

BEGIN_MESSAGE_MAP(CWPhastDoc, CDocument)
	ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, OnUpdateEditUndo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_REDO, OnUpdateEditRedo)
	ON_COMMAND(ID_EDIT_REDO, OnEditRedo)
	ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)
	ON_UPDATE_COMMAND_UI(ID_TOOLS_GEOMETRY, OnUpdateToolsGeometry)
	ON_COMMAND(ID_TOOLS_GEOMETRY, OnToolsGeometry)
	ON_COMMAND(ID_FILE_IMPORT, OnFileImport)
	ON_COMMAND(ID_FILE_EXPORT, OnFileExport)
	ON_COMMAND(ID_FILE_RUN, OnFileRun)
	ON_COMMAND(ID_TOOLS_NEWSTRESSPERIOD, OnToolsNewStressPeriod)
	ON_UPDATE_COMMAND_UI(ID_VIEW_AXES, OnUpdateViewAxes)
	ON_COMMAND(ID_VIEW_AXES, OnViewAxes)
	ON_UPDATE_COMMAND_UI(ID_SETPROJECTIONTO_PARALLEL, OnUpdateSetprojectiontoParallel)
	ON_COMMAND(ID_SETPROJECTIONTO_PARALLEL, OnSetprojectiontoParallel)
	ON_UPDATE_COMMAND_UI(ID_SETPROJECTIONTO_PERSPECTIVE, OnUpdateSetprojectiontoPerspective)
	ON_COMMAND(ID_SETPROJECTIONTO_PERSPECTIVE, OnSetprojectiontoPerspective)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SITEMAP, OnUpdateViewSitemap)
	ON_COMMAND(ID_VIEW_SITEMAP, OnViewSitemap)
	ON_COMMAND(ID_VIEW_GRID, OnViewGrid)
	ON_UPDATE_COMMAND_UI(ID_VIEW_GRID, OnUpdateViewGrid)

	ON_UPDATE_COMMAND_UI(ID_MEDIAZONES_HIDEALL, OnUpdateMediaZonesHideAll)
	ON_COMMAND(ID_MEDIAZONES_HIDEALL, OnMediaZonesHideAll)	
	ON_UPDATE_COMMAND_UI(ID_MEDIAZONES_SHOWSELECTED, OnUpdateMediaZonesShowSelected)
	ON_COMMAND(ID_MEDIAZONES_SHOWSELECTED, OnMediaZonesShowSelected)	
	ON_COMMAND(ID_MEDIAZONES_SELECTALL, OnMediaZonesSelectAll)
	ON_COMMAND(ID_MEDIAZONES_UNSELECTALL, OnMediaZonesUnselectAll)

	ON_UPDATE_COMMAND_UI(ID_ICZONES_HIDEALL, OnUpdateICZonesHideAll)
	ON_COMMAND(ID_ICZONES_HIDEALL, OnICZonesHideAll)
	ON_UPDATE_COMMAND_UI(ID_ICZONES_SHOWSELECTED, OnUpdateICZonesShowSelected)
	ON_COMMAND(ID_ICZONES_SHOWSELECTED, OnICZonesShowSelected)	
	ON_COMMAND(ID_ICZONES_SELECTALL, OnICZonesSelectAll)
	ON_COMMAND(ID_ICZONES_UNSELECTALL, OnICZonesUnselectAll)

	ON_UPDATE_COMMAND_UI(ID_BCZONES_HIDEALL, OnUpdateBCZonesHideAll)
	ON_COMMAND(ID_BCZONES_HIDEALL, OnBCZonesHideAll)
	ON_UPDATE_COMMAND_UI(ID_BCZONES_SHOWSELECTED, OnUpdateBCZonesShowSelected)
	ON_COMMAND(ID_BCZONES_SHOWSELECTED, OnBCZonesShowSelected)	
	ON_COMMAND(ID_BCZONES_SELECTALL, OnBCZonesSelectAll)
	ON_COMMAND(ID_BCZONES_UNSELECTALL, OnBCZonesUnselectAll)

	// ID_WELLS_HIDEALL
	ON_UPDATE_COMMAND_UI(ID_WELLS_HIDEALL, OnUpdateWellsHideAll)
	ON_COMMAND(ID_WELLS_HIDEALL, OnWellsHideAll)

	// ID_WELLS_SHOWSELECTED
	ON_UPDATE_COMMAND_UI(ID_WELLS_SHOWSELECTED, OnUpdateWellsShowSelected)
	ON_COMMAND(ID_WELLS_SHOWSELECTED, OnWellsShowSelected)

	// ID_WELLS_SELECTALL
	ON_COMMAND(ID_WELLS_SELECTALL, OnWellsSelectAll)

	// ID_WELLS_UNSELECTALL
	ON_COMMAND(ID_WELLS_UNSELECTALL, OnWellsUnselectAll)

	// ID_RIVERS_HIDEALL
	ON_UPDATE_COMMAND_UI(ID_RIVERS_HIDEALL, OnUpdateRiversHideAll)
	ON_COMMAND(ID_RIVERS_HIDEALL, OnRiversHideAll)

	// ID_RIVERS_SHOWSELECTED
	ON_UPDATE_COMMAND_UI(ID_RIVERS_SHOWSELECTED, OnUpdateRiversShowSelected)
	ON_COMMAND(ID_RIVERS_SHOWSELECTED, OnRiversShowSelected)

	// ID_RIVERS_SELECTALL
	ON_COMMAND(ID_RIVERS_SELECTALL, OnRiversSelectAll)

	// ID_RIVERS_UNSELECTALL
	ON_COMMAND(ID_RIVERS_UNSELECTALL, OnRiversUnselectAll)

	ON_COMMAND(ID_VIEW_HIDEALL, OnViewHideAll)
	ON_COMMAND(ID_VIEW_SHOWALL, OnViewShowAll)

	// ID_TOOLS_MODIFYGRID
	ON_UPDATE_COMMAND_UI(ID_TOOLS_MODIFYGRID, OnUpdateToolsModifyGrid)
	ON_COMMAND(ID_TOOLS_MODIFYGRID, OnToolsModifyGrid)

	// ID_TOOLS_NEWZONE
	ON_UPDATE_COMMAND_UI(ID_TOOLS_NEWZONE, OnUpdateToolsNewZone)
	ON_COMMAND(ID_TOOLS_NEWZONE, OnToolsNewZone)
END_MESSAGE_MAP()

#if defined(WPHAST_AUTOMATION)
BEGIN_DISPATCH_MAP(CWPhastDoc, CDocument)
	DISP_PROPERTY_EX_ID(CWPhastDoc, "Visible", dispidVisible, GetVisible, SetVisible, VT_VARIANT)
	DISP_FUNCTION_ID(CWPhastDoc, "SaveAs", dispidSaveAs, SaveAs, VT_VARIANT, VTS_VARIANT)
	DISP_FUNCTION_ID(CWPhastDoc, "Import", dispidImport, Import, VT_VARIANT, VTS_VARIANT)
	DISP_FUNCTION_ID(CWPhastDoc, "Run", dispidRun, Run, VT_VARIANT, VTS_NONE)
END_DISPATCH_MAP()

// Note: we add support for IID_IWPhast to support typesafe binding
//  from VBA.  This IID must match the GUID that is attached to the 
//  dispinterface in the .IDL file.

// {7AF963C5-1815-4D7A-81DC-1987723AB9CE}
static const IID IID_IWPhast =
{ 0x7AF963C5, 0x1815, 0x4D7A, { 0x81, 0xDC, 0x19, 0x87, 0x72, 0x3A, 0xB9, 0xCE } };

BEGIN_INTERFACE_MAP(CWPhastDoc, CDocument)
	INTERFACE_PART(CWPhastDoc, IID_IWPhast, Dispatch)
END_INTERFACE_MAP()
#endif

CWPhastDoc::CWPhastDoc()
: m_pimpl(0)
, m_pPropCollection(0)
, m_pRemovedPropCollection(0)
, m_pGridActor(0)
, m_pAxesActor(0)
, m_pGeometrySheet(0)
, m_pScalePage(0)
, m_pUnits(0)
, m_pModel(0) // , m_pFlowOnly(0)
, m_ProjectionMode(PT_PERSPECTIVE)
, m_pMapActor(0)
, m_pPropAssemblyMedia(0)
, m_pPropAssemblyBC(0)
, m_pPropAssemblyIC(0)
, m_pPropAssemblyWells(0)
, m_pPropAssemblyRivers(0)
, RiverCallbackCommand(0)
, GridCallbackCommand(0)
, RiverMovePointAction(0)
, m_pGridSheet(0)
, m_pGridPage(0)
, ModifyGridSheet(0)
, GridRefinePage(0)
, GridCoarsenPage(0)
, GridElementsSelector(0)
, NewZoneWidget(0)
, NewZoneCallbackCommand(0)
{
#if defined(WPHAST_AUTOMATION)
	EnableAutomation();

	AfxOleLockApp();
#endif

	ASSERT(this->m_pimpl == 0);
	this->m_pimpl = new WPhastDocImpl();
	this->m_pimpl->m_vectorActionsIndex = 0;
	this->m_pimpl->m_lastSaveIndex = -1;

// COMMENT: {7/28/2005 2:48:16 PM}	// create the grid
// COMMENT: {7/28/2005 2:48:16 PM}	//
// COMMENT: {7/28/2005 2:48:16 PM}	ASSERT(this->m_pGridActor == 0);
// COMMENT: {7/28/2005 2:48:16 PM}	this->m_pGridActor = CGridActor::New();
// COMMENT: {7/28/2005 2:48:16 PM}	this->m_pGridActor->GetProperty()->SetColor(1.0, 0.8, 0.6);

	// create the prop list
	//
	ASSERT(this->m_pPropCollection == 0);
	this->m_pPropCollection = vtkPropCollection::New();

	// create the removed prop list
	//
	ASSERT(this->m_pRemovedPropCollection == 0);
	this->m_pRemovedPropCollection = vtkPropCollection::New();	

	// create the prop-assemblies
	//
	this->m_pPropAssemblyMedia  = vtkPropAssembly::New();
	this->m_pPropAssemblyIC     = vtkPropAssembly::New();
	this->m_pPropAssemblyBC     = vtkPropAssembly::New();
	this->m_pPropAssemblyWells  = vtkPropAssembly::New();
	this->m_pPropAssemblyRivers = vtkPropAssembly::New();
	this->m_pPropCollection->AddItem(this->m_pPropAssemblyMedia);
	this->m_pPropCollection->AddItem(this->m_pPropAssemblyIC);
	this->m_pPropCollection->AddItem(this->m_pPropAssemblyBC);
	this->m_pPropCollection->AddItem(this->m_pPropAssemblyWells);
	this->m_pPropCollection->AddItem(this->m_pPropAssemblyRivers);

	// create the axes
	//
	this->m_pAxesActor = CAxesActor::New();

	// create Geometry property sheet
	//
	this->m_pGeometrySheet = new CModelessPropertySheet("Geometry");
	this->m_pScalePage = new CScalePropertyPage();
	this->m_pScalePage->m_pDoc = this;
	this->m_pGeometrySheet->AddPage(this->m_pScalePage);

	// create units
	//
	this->m_pUnits = new CUnits;

	// create model
	//
	this->m_pModel = new CNewModel;
	this->m_pModel->m_printFreq = CPrintFreq::NewDefaults();
// COMMENT: {7/28/2005 2:54:07 PM}	this->New(CNewModel::Default());

	// river event listener
	//
	this->RiverCallbackCommand = vtkCallbackCommand::New();
	this->RiverCallbackCommand->SetClientData(this);
	this->RiverCallbackCommand->SetCallback(CWPhastDoc::RiverListener);

	// grid event listener
	//
	this->GridCallbackCommand = vtkCallbackCommand::New();
	this->GridCallbackCommand->SetClientData(this);
	this->GridCallbackCommand->SetCallback(CWPhastDoc::GridListener);
}

#define CLEANUP_ASSEMBLY_MACRO(PTR_ASSEMBLY) \
	ASSERT(PTR_ASSEMBLY); \
	if (PTR_ASSEMBLY) { \
		if (vtkPropCollection *pPropCollection = PTR_ASSEMBLY->GetParts()) { \
			ASSERT(pPropCollection->GetNumberOfItems() == 0); \
			pPropCollection->RemoveAllItems(); \
		} \
		PTR_ASSEMBLY->Delete(); \
		PTR_ASSEMBLY = 0; \
	}
#define ASSERT_DELETE_SET_NULL_MACRO(POINTER) \
	ASSERT(POINTER); \
	if (POINTER) { \
		delete POINTER; \
		POINTER = 0; \
	}

CWPhastDoc::~CWPhastDoc()
{
#if defined(WPHAST_AUTOMATION)
	AfxOleUnlockApp();
#endif

	ASSERT(this->m_pimpl);
	if (this->m_pimpl)
	{
		ASSERT(this->m_pimpl->m_vectorActions.empty()); // should be emptied in DeleteContents
		delete this->m_pimpl;
		this->m_pimpl = 0;
	}

	ASSERT(this->m_pPropCollection);
	if (this->m_pPropCollection)
	{
		ASSERT(this->m_pPropCollection->GetNumberOfItems() == 0); // should be emptied in DeleteContents
		this->m_pPropCollection->Delete();
		this->m_pPropCollection = 0;
	}

	ASSERT(this->m_pRemovedPropCollection);
	if (this->m_pRemovedPropCollection)
	{
		ASSERT(this->m_pRemovedPropCollection->GetNumberOfItems() == 0); // should be emptied in DeleteContents
		this->m_pRemovedPropCollection->Delete();
		this->m_pRemovedPropCollection = 0;
	}

 	ASSERT(this->m_pGridActor);
	if (this->m_pGridActor)
	{
		this->m_pGridActor->Delete();
		this->m_pGridActor = 0;
	}

	CLEANUP_ASSEMBLY_MACRO(this->m_pPropAssemblyMedia);
	CLEANUP_ASSEMBLY_MACRO(this->m_pPropAssemblyIC);
	CLEANUP_ASSEMBLY_MACRO(this->m_pPropAssemblyBC);
	CLEANUP_ASSEMBLY_MACRO(this->m_pPropAssemblyWells);
	CLEANUP_ASSEMBLY_MACRO(this->m_pPropAssemblyRivers);	

	ASSERT(this->m_pAxesActor);
	if (this->m_pAxesActor)
	{
		this->m_pAxesActor->Delete();
		this->m_pAxesActor = 0;
	}

	ASSERT(this->m_pScalePage == 0);     // should be deleted in CWPhastDoc::OnCloseDocument()
	ASSERT(this->m_pGeometrySheet == 0); // should be deleted in CWPhastDoc::OnCloseDocument()

	ASSERT_DELETE_SET_NULL_MACRO(this->m_pUnits);
	ASSERT_DELETE_SET_NULL_MACRO(this->m_pModel);
// COMMENT: {5/5/2005 3:51:23 PM}	ASSERT_DELETE_SET_NULL_MACRO(this->m_pPrintFreq);

	if (this->m_pMapActor)
	{
		this->m_pMapActor->Delete();
		this->m_pMapActor = 0;
	}

	// callbacks
	//
	if (this->RiverCallbackCommand)
	{
		this->RiverCallbackCommand->Delete();
	}
	if (this->GridCallbackCommand)
	{
		this->GridCallbackCommand->Delete();
	}

	// Grid
	//
	if (this->m_pGridPage)
	{
		this->m_pGridPage->DestroyWindow();
		delete this->m_pGridPage;
		this->m_pGridPage = 0;
	}
	if (this->m_pGridSheet)
	{
		this->m_pGridSheet->DestroyWindow();
		delete this->m_pGridSheet;
		this->m_pGridSheet = 0;
	}

	// Grid modifiers
	//
	if (this->ModifyGridSheet)
	{
		this->ModifyGridSheet->DestroyWindow();
		delete this->ModifyGridSheet;
		this->ModifyGridSheet = 0;
	}
	if (this->GridRefinePage)
	{
		this->GridRefinePage->DestroyWindow();
		delete this->GridRefinePage;
		this->GridRefinePage = 0;
	}
	if (this->GridCoarsenPage)
	{
		this->GridCoarsenPage->DestroyWindow();
		delete this->GridCoarsenPage;
		this->GridCoarsenPage = 0;
	}
	ASSERT(this->NewZoneWidget == 0);           // should be deleted in pView->CancelMode
	ASSERT(this->NewZoneCallbackCommand == 0);  // should be deleted in pView->CancelMode
	ASSERT(this->GridElementsSelector == 0);    // should be deleted in pView->CancelMode
}

BOOL CWPhastDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// add reinitialization code here
	// (SDI documents will reuse this document)
	TRACE("CWPhastDoc::OnNewDocument()\n");
	this->SetModifiedFlag(TRUE);

	return TRUE;
}

// CWPhastDoc serialization

void CWPhastDoc::Serialize(CArchive& ar)
{
	static const char szWPhast[] = "WPhast";
	herr_t status;

	CWaitCursor wait;

	bool bStoring = (ar.IsStoring() == TRUE);

	if (ar.IsStoring())
	{
		// Update StatusBar
		//
		if (CWnd* pWnd = ((CFrameWnd*)::AfxGetMainWnd())->GetMessageBar())
		{
			CString status(_T("Saving..."));
			pWnd->SetWindowText(status);
		}

		// get hdf file handle
		//
		CHDFMirrorFile* pFile = (CHDFMirrorFile*)ar.GetFile();
		ASSERT(pFile->GetHID() > 0);

		// Create the "/WPhast" group
		//
		hid_t wphast_id = ::H5Gcreate(pFile->GetHID(), szWPhast, 0);
		ASSERT(wphast_id > 0);
		if (wphast_id > 0)
		{
			// store flowonly
			ASSERT(this->m_pModel);
			this->m_pModel->m_flowOnly.Serialize(bStoring, wphast_id);
			this->m_pModel->m_freeSurface.Serialize(bStoring, wphast_id);
			this->m_pModel->m_steadyFlow.Serialize(bStoring, wphast_id);
			this->m_pModel->m_solutionMethod.Serialize(bStoring, wphast_id);

			// store units
			ASSERT(this->m_pUnits);
			this->m_pUnits->Serialize(bStoring, wphast_id);

			// store grid
			this->m_pGridActor->Serialize(bStoring, wphast_id);

			// store axes
			//this->m_pAxesActor->Serialize(bStoring, wphast_id);  // not implemented

			// store site map
			if (this->m_pMapActor) this->m_pMapActor->Serialize(bStoring, wphast_id);

			// store media
			this->SerializeMedia(bStoring, wphast_id);

			// store ic
			this->SerializeIC(bStoring, wphast_id);

			// store bc
			this->SerializeBC(bStoring, wphast_id);

			// store wells
			this->SerializeWells(bStoring, wphast_id);

			// store rivers
			this->SerializeRivers(bStoring, wphast_id);

			// store PRINT_INITIAL
			this->m_pModel->m_printInput.Serialize(bStoring, wphast_id);

			// store PRINT_FREQUENCY
			this->m_pModel->m_printFreq.Serialize(bStoring, wphast_id);

			// store time control
			this->m_pModel->m_timeControl2.Serialize(bStoring, wphast_id);

			// close WPhast group
			status = ::H5Gclose(wphast_id);
			ASSERT(status >= 0);
		}
	}
	else
	{
		// add loading code here

		// Update StatusBar
		//
		if (CWnd* pWnd = ((CFrameWnd*)::AfxGetMainWnd())->GetMessageBar())
		{
			CString status(_T("Loading..."));
			pWnd->SetWindowText(status);
		}

		// delay redrawing treectrl
		//
		CWnd *pWndTreeCtrl = 0;
		if (CPropertyTreeControlBar *pPropertyTreeControlBar = this->GetPropertyTreeControlBar())
		{
			pWndTreeCtrl = pPropertyTreeControlBar->GetTreeCtrl();
		}
		CDelayRedraw delayTree(pWndTreeCtrl);

		// delay redrawing render window
		//
		CDelayRedraw delayRender(::AfxGetMainWnd()->GetActiveWindow(), this);

		CHDFMirrorFile* pFile = (CHDFMirrorFile*)ar.GetFile();
		ASSERT(pFile->GetHID() > 0);
		if (pFile->GetHID() < 0) return;

		// Open the "/WPhast" group
		//
		hid_t wphast_id = ::H5Gopen(pFile->GetHID(), szWPhast);
		ASSERT(wphast_id > 0);
		if (wphast_id > 0)
		{
			// Note: can't call this->New(...) here since the defaults are unknown
			// until this->SerializeMedia and this->SerializeIC are called

			// load model (flowonly freeSurface steadyFlow)
			ASSERT(this->m_pModel);
			this->m_pModel->m_flowOnly.Serialize(bStoring, wphast_id);
			this->m_pModel->m_freeSurface.Serialize(bStoring, wphast_id);
			this->m_pModel->m_steadyFlow.Serialize(bStoring, wphast_id);
			this->m_pModel->m_solutionMethod.Serialize(bStoring, wphast_id);

			// update properties bar
			this->SetModel(*this->m_pModel);

			// load units
			ASSERT(this->m_pUnits);
			this->m_pUnits->Serialize(bStoring, wphast_id);
			// update properties bar
			if (CPropertyTreeControlBar* pTree = this->GetPropertyTreeControlBar())
			{
				pTree->SetUnits(this->m_pUnits);
			}

			// load grid
			this->m_pGridActor->Serialize(bStoring, wphast_id);

			// set grid
			CGrid x, y, z;
			this->m_pGridActor->GetGrid(x, y, z);
			this->ResizeGrid(x, y, z);

			// load axes
			//this->m_pAxesActor->Serialize(bStoring, wphast_id); // not implemented
			this->GetPropCollection()->AddItem(this->m_pAxesActor);

			// load site map
			ASSERT(this->m_pMapActor == NULL);
			this->m_pMapActor = CMapActor::New();
			this->m_pMapActor->Serialize(bStoring, wphast_id);
			CSiteMap siteMap = this->m_pMapActor->GetSiteMap();
			if (siteMap.m_fileName.empty())
			{
				this->m_pMapActor->Delete();
				this->m_pMapActor = 0;
			}
			else
			{
				this->m_pMapActor->SetPickable(0);
				this->GetPropCollection()->AddItem(this->m_pMapActor);
			}

			// load media
			this->SerializeMedia(bStoring, wphast_id);

			// load ICs
			this->SerializeIC(bStoring, wphast_id);

			// load BCs
			this->SerializeBC(bStoring, wphast_id);

			// load wells
			this->SerializeWells(bStoring, wphast_id);

			// load rivers
			this->SerializeRivers(bStoring, wphast_id);

			// load PRINT_INITIAL
			this->m_pModel->m_printInput.Serialize(bStoring, wphast_id);

			// load PRINT_FREQUENCY
#ifdef _DEBUG
			CTimeSeries<Ctime>::iterator it0 = this->m_pModel->m_printFreq.print_force_chem.begin();
			ASSERT(it0 != this->m_pModel->m_printFreq.print_force_chem.end());
			TRACE("first force_chem type = %d\n", it0->first.type);
			TRACE("first force_chem type = %d\n", it0->first.value_defined);
			TRACE("first force_chem type = %g\n", it0->first.value);

			TRACE("force_chem type = %d\n", it0->second.type);
			TRACE("force_chem type = %d\n", it0->second.value_defined);
			TRACE("force_chem type = %g\n", it0->second.value);

#endif
			this->m_pModel->m_printFreq.Serialize(bStoring, wphast_id);
#ifdef _DEBUG
			CTimeSeries<Ctime>::iterator it = this->m_pModel->m_printFreq.print_force_chem.begin();
			ASSERT(it != this->m_pModel->m_printFreq.print_force_chem.end());

			TRACE("first force_chem type = %d\n", it->first.type);
			TRACE("first force_chem type = %d\n", it->first.value_defined);
			TRACE("first force_chem type = %g\n", it->first.value);

			TRACE("force_chem type = %d\n", it->second.type);
			TRACE("force_chem type = %d\n", it->second.value_defined);
			TRACE("force_chem type = %g\n", it->second.value);
#endif

			// update properties bar
			if (CPropertyTreeControlBar* pTree = this->GetPropertyTreeControlBar())
			{
				pTree->SetPrintFrequency(&this->m_pModel->m_printFreq);
			}

			// load time control
			this->m_pModel->m_timeControl2.Serialize(bStoring, wphast_id);			

			// update properties bar
			if (CPropertyTreeControlBar* pTree = this->GetPropertyTreeControlBar())
			{
				pTree->SetTimeControl2(&this->m_pModel->m_timeControl2);
			}

			// close WPhast group
			status = ::H5Gclose(wphast_id);
			ASSERT(status >= 0);
		}

		// set scale for all zones, wells ...
		vtkFloatingPointType* scale = this->m_pGridActor->GetScale();
		this->SetScale(scale[0], scale[1], scale[2]);

		this->ResetCamera();

		// update properties bar
		if (CPropertyTreeControlBar* pTree = this->GetPropertyTreeControlBar())
		{
			pTree->GetGridNode().Expand(TVE_COLLAPSE);
			pTree->GetMediaNode().Expand(TVE_COLLAPSE);
			pTree->GetICNode().Expand(TVE_COLLAPSE);
			pTree->GetBCNode().Expand(TVE_COLLAPSE);
			pTree->GetWellsNode().Expand(TVE_COLLAPSE);
			pTree->GetRiversNode().Expand(TVE_COLLAPSE);
			this->ClearSelection();
		}
	}

	// Update StatusBar
	//
	if (CWnd* pWnd = ((CFrameWnd*)::AfxGetMainWnd())->GetMessageBar())
	{
		CString status;
		status.LoadString(AFX_IDS_IDLEMESSAGE);
		pWnd->SetWindowText(status);
	}
}

void CWPhastDoc::SerializeMedia(bool bStoring, hid_t loc_id)
{
	if (CPropertyTreeControlBar *pBar = this->GetPropertyTreeControlBar())
	{
		this->SerializeActors<CMediaZoneActor>(bStoring, loc_id, pBar->GetMediaNode(), "Zones");
	}
}

void CWPhastDoc::SerializeBC(bool bStoring, hid_t loc_id)
{
	if (CPropertyTreeControlBar *pBar = this->GetPropertyTreeControlBar())
	{
		this->SerializeActors<CBCZoneActor>(bStoring, loc_id, pBar->GetBCNode(), "Zones");
	}
}

void CWPhastDoc::SerializeWells(bool bStoring, hid_t loc_id)
{
	if (CPropertyTreeControlBar *pBar = this->GetPropertyTreeControlBar())
	{
		this->SerializeActors<CWellActor>(bStoring, loc_id, pBar->GetWellsNode(), "Wells");
	}
}

void CWPhastDoc::SerializeRivers(bool bStoring, hid_t loc_id)
{
	if (CPropertyTreeControlBar *pBar = this->GetPropertyTreeControlBar())
	{
		this->SerializeActors<CRiverActor>(bStoring, loc_id, pBar->GetRiversNode(), "Rivers");
	}
}

void CWPhastDoc::SerializeIC(bool bStoring, hid_t loc_id)
{
	// Revisions prior to and including r669 stored ALL the IC zones 
	// in the IC group.  Now each IC zone type has its own group heading.
	// This code should be backwards compatible for reading of .wphast
	// files.
	//
	// through r669:
	//
	//      GROUP "IC" {
	//         GROUP "Default:IC_CHEM" {
	//            ...
	//         } 
	//         GROUP "Default:IC_HEAD" {
	//            ...
	//         } 
	//         GROUP "Zone 10:IC_CHEM" {
	//            ...
	//         } 
	//         GROUP "Zone 9:IC_HEAD" {
	//            ...
	//         } 
	//         DATASET "Zones" {
	//            ...
	//            DATA {
	//               "Default:IC_HEAD", "Default:IC_CHEM", "Zone 9:IC_HEAD",
	//               "Zone 10:IC_CHEM"
	//            } 
	//         } 
	//      } 
	//	
	// from now on this is the layout:
	//
	//      GROUP "IC" {
	//         GROUP "ICChem" {
	//            GROUP "Default" {
	//               ...
	//            } 
	//            GROUP "Zone 10" {
	//               ...
	//            } 
	//            DATASET "Zones" {
	//               ...
	//               DATA {
	//                  "Default", "Zone 10"
	//               } 
	//            } 
	//         } 
	//         GROUP "ICHead" {
	//            GROUP "Default" {
	//               ...
	//            } 
	//            GROUP "Zone 9" {
	//               ...
	//            } 
	//            DATASET "Zones" {
	//               ...
	//               DATA {
	//                  "Default", "Zone 9"
	//               } 
	//            } 
	//         } 
	//      } 
	//

	static const char szIC[]    = "IC";
	static const char szZones[] = "Zones";

	hid_t ic_id;
	hid_t zone_id;
	herr_t status;

	if (bStoring)
	{
		// Create the szIC group
		ic_id = ::H5Gcreate(loc_id, szIC, 0); // always created even if empty
		ASSERT(ic_id > 0);
		if (ic_id > 0)
		{
			// write new formats
			//
			if (CPropertyTreeControlBar *pBar = this->GetPropertyTreeControlBar())
			{
				this->SerializeActors<CICHeadZoneActor>(bStoring, ic_id, pBar->GetICHeadNode(), szZones);
				this->SerializeActors<CICChemZoneActor>(bStoring, ic_id, pBar->GetICChemNode(), szZones);
			}

			// close the szIC group
			status = ::H5Gclose(ic_id);
			ASSERT(status >= 0);
		}
	}
	else
	{
		// add loading code here

		// Open the szIC group
		ic_id = ::H5Gopen(loc_id, szIC);
		ASSERT(ic_id > 0);
		if (ic_id > 0)
		{
			hid_t old_id = ::H5Dopen(ic_id, szZones);
			if (old_id <= 0)
			{
				// read new format
				//
				if (CPropertyTreeControlBar *pBar = this->GetPropertyTreeControlBar())
				{
					this->SerializeActors<CICHeadZoneActor>(bStoring, ic_id, pBar->GetICHeadNode(), szZones);
					this->SerializeActors<CICChemZoneActor>(bStoring, ic_id, pBar->GetICChemNode(), szZones);
				}
			}
			else
			{
				// read old format (this .wphast file was written by a version older than
				// or equal to 669)
				//
				status = ::H5Dclose(old_id);
				ASSERT(status >= 0);

				std::list<std::string> listNames;
				CGlobal::ReadList(ic_id, szZones, listNames);
				std::list<std::string>::iterator iter = listNames.begin();
				for (; iter != listNames.end(); ++iter)
				{
					// open zone group
					zone_id = ::H5Gopen(ic_id, (*iter).c_str());
					ASSERT(zone_id > 0);
					if (zone_id > 0)
					{
						std::string name = (*iter);
						std::string::size_type pos;

						if ((pos = name.find(":IC_HEAD")) != std::string::npos)
						{
							CICHeadZoneActor* pZone = CICHeadZoneActor::New();
							name.resize(pos);
							pZone->Serialize(false, zone_id, this->GetUnits());
							pZone->SetName(name.c_str());
							this->Add(pZone);
							pZone->Delete(); // ok ref counted
						}
						else if ((pos = name.find(":IC_CHEM")) != std::string::npos)
						{
							CICChemZoneActor* pZone = CICChemZoneActor::New();
							name.resize(pos);
							pZone->Serialize(false, zone_id, this->GetUnits());
							pZone->SetName(name.c_str());
							this->Add(pZone);
							pZone->Delete(); // ok ref counted
						}
						else
						{
							ASSERT(FALSE);
						}

						// close the szZones group
						status = ::H5Gclose(zone_id);
						ASSERT(status >= 0);
					}
				}

			}
			// close the szIC group
			status = ::H5Gclose(ic_id);
			ASSERT(status >= 0);
		}
	}
}

template<typename ACTOR>
void CWPhastDoc::SerializeActors(bool bStoring, hid_t loc_id, CTreeCtrlNode parentNode, const char* szNamesListHeading)
{
	hid_t group_id;
	hid_t actor_id;
	herr_t status;

	if (bStoring)
	{
		// STORING

		// create the ACTOR::szHeading group
		group_id = ::H5Gcreate(loc_id, ACTOR::szHeading, 0); // always created even if empty
		ASSERT(group_id > 0);
		if (group_id > 0)
		{
			int nCount;
			std::list<LPCTSTR> listNames;
			std::list<ACTOR*> listActors;

			// get actors and actors names
			//
			nCount = parentNode.GetChildCount();
			for (int i = 0; i < nCount; ++i)
			{
				if (ACTOR *pActor = ACTOR::SafeDownCast((vtkObject*)parentNode.GetChildAt(i).GetData()))
				{
					listActors.push_back(pActor);
					ASSERT(pActor->GetName() && ::strlen(pActor->GetName()));
					listNames.push_back(pActor->GetName());
				}
				else ASSERT(FALSE);
			}

			if (listNames.size() > 0)
			{
				// store actor names
				//
				CGlobal::WriteList(group_id, szNamesListHeading, listNames);

				// store each actor
				//
				std::list<ACTOR*>::iterator iter = listActors.begin();
				for (; iter != listActors.end(); ++iter)
				{
					// create actor group
					//
					actor_id = ::H5Gcreate(group_id, (*iter)->GetName(), 0);
					ASSERT(actor_id > 0);
					if (actor_id > 0)
					{
						// store actor
						//
						(*iter)->Serialize(true, actor_id, this->GetUnits());

						// close the actor group
						//
						status = ::H5Gclose(actor_id);
						ASSERT(status >= 0);
					}
				}
			}

			// close the ACTOR::szHeading group
			//
			status = ::H5Gclose(group_id);
			ASSERT(status >= 0);
		}
	}
	else
	{
		// LOADING

		// open the ACTOR::szHeading group
		//
		group_id = ::H5Gopen(loc_id, ACTOR::szHeading);
		ASSERT(group_id > 0);
		if (group_id > 0)
		{
			std::list<std::string> listNames;
			CGlobal::ReadList(group_id, szNamesListHeading, listNames);
			std::list<std::string>::iterator iter = listNames.begin();
			for (; iter != listNames.end(); ++iter)
			{
				// open actor group
				//
				actor_id = ::H5Gopen(group_id, (*iter).c_str());
				ASSERT(actor_id > 0);
				if (actor_id > 0)
				{
					ACTOR* pActor = ACTOR::New();
					if (pActor)
					{
						// load actor
						//
						pActor->Serialize(false, actor_id, this->GetUnits());
						pActor->SetName((*iter).c_str());
						this->Add(pActor);
						pActor->Delete(); // ok ref counted
					}

					// close the actor group
					//
					status = ::H5Gclose(actor_id);
					ASSERT(status >= 0);
				}
			}

			// close the ACTOR::szHeading group
			//
			status = ::H5Gclose(group_id);
			ASSERT(status >= 0);
		}
	}
}

// CWPhastDoc diagnostics

#ifdef _DEBUG
void CWPhastDoc::AssertValid() const
{
	CDocument::AssertValid();
	ASSERT(this->m_pAxesActor != 0);
	ASSERT(this->m_pimpl != 0);
	ASSERT(this->m_pPropCollection != 0);
	ASSERT(this->m_pRemovedPropCollection != 0);
}

void CWPhastDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}

#endif //_DEBUG

CPropertyTreeControlBar* CWPhastDoc::GetPropertyTreeControlBar() const
{
	CFrameWnd* pFrame = (CFrameWnd*)::AfxGetMainWnd();
	if (!pFrame) return 0;

	if (CPropertyTreeControlBar* pTree = static_cast<CPropertyTreeControlBar*>(pFrame->GetControlBar(IDW_CONTROLBAR_TREE))) {
		ASSERT_KINDOF(CPropertyTreeControlBar, pTree);
		ASSERT_VALID(pTree);
		return pTree;
	}
	///ASSERT(FALSE);
	return 0;
}

CBoxPropertiesDialogBar* CWPhastDoc::GetBoxPropertiesDialogBar() const
{
	CFrameWnd* pFrame = (CFrameWnd*)::AfxGetMainWnd();
	if (!pFrame) return 0;

	if (CBoxPropertiesDialogBar* pBar = static_cast<CBoxPropertiesDialogBar*>(pFrame->GetControlBar(IDW_CONTROLBAR_BOXPROPS))) {
		ASSERT_KINDOF(CBoxPropertiesDialogBar, pBar);
		ASSERT_VALID(pBar);
		return pBar;
	}
	ASSERT(FALSE);
	return 0;
}

vtkPropCollection* CWPhastDoc::GetPropCollection() const
{
	return this->m_pPropCollection;
}

vtkPropCollection* CWPhastDoc::GetRemovedPropCollection() const
{
	return this->m_pRemovedPropCollection;
}

// CWPhastDoc commands

BOOL CWPhastDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;

	return TRUE;
}

CFile* CWPhastDoc::GetFile(LPCTSTR lpszFileName, UINT nOpenFlags,
	CFileException* pError)
{
	/// return CDocument::GetFile(lpszFileName, nOpenFlags, pError);

	CHDFMirrorFile* pFile = new CHDFMirrorFile;
	ASSERT(pFile != NULL);
	if (!pFile->Open(lpszFileName, nOpenFlags, pError))
	{
		delete pFile;
		pFile = NULL;
	}
	return pFile;
}

void CWPhastDoc::InitDocument()
{
	TRACE("CWPhastDoc::InitDocument()\n");

	//// create pimpl
	////
	//ASSERT(this->m_pimpl == 0);
	//this->m_pimpl = new WPhastDocImpl();
	//this->m_pimpl->m_vectorActionsIndex = 0;

	// create the list
	//
	// ASSERT(this->m_pPropCollection == 0);
	// this->m_pPropCollection = vtkPropCollection::New();

	////// create the grid
	//////
	////ASSERT(this->m_pGridActor == 0);
	////this->m_pGridActor = CGridActor::New();
	////this->m_pGridActor->GetProperty()->SetColor(1.0, 0.8, 0.6);

	//////// create the axes
	////////
	//////ASSERT(this->m_pAxes == 0);
	//////ASSERT(this->m_pAxesTubeFilter == 0);
	//////ASSERT(this->m_pAxesPolyDataMapper == 0);
	//////ASSERT(this->m_pAxesActor == 0);
	//////this->m_pAxes = vtkAxes::New();
	//////this->m_pAxes->SetOrigin(0, 0, 0);
	//////this->m_pAxesTubeFilter = vtkTubeFilter::New();
	//////this->m_pAxesTubeFilter->SetInput(this->m_pAxes->GetOutput());
	//////this->m_pAxesTubeFilter->SetNumberOfSides(10);
	//////this->m_pAxesPolyDataMapper = vtkPolyDataMapper::New();
	//////this->m_pAxesPolyDataMapper->SetInput(this->m_pAxesTubeFilter->GetOutput());
	//////this->m_pAxesActor = vtkActor::New();
	//////this->m_pAxesActor->SetMapper(this->m_pAxesPolyDataMapper);
	//////this->m_pAxesActor->SetPickable(0);

	//if (CPropertyTreeControlBar* pTree = this->GetPropertyTreeControlBar()) {
	//	pTree->InitDocument();
	//}
	//else {
	//	ASSERT(FALSE);
	//}

	/****
	// set scale
	//
	for (int i = 0; i < 3; ++i) {
		this->m_Scale[i] = 1.0;
	}

	// Update scale page
	//
	this->m_pScalePage->m_XScale = m_Scale[0];
	this->m_pScalePage->m_YScale = m_Scale[1];
	this->m_pScalePage->m_ZScale = m_Scale[2];
	if (this->m_pScalePage->GetSafeHwnd()) {
		this->m_pScalePage->UpdateData(FALSE);
	}
	****/

}

///////////////////////////////////////////////////////////////////////////////////////
#define CLEAR_PROP_ASSEMBLY_MACRO(PROP_ASSEMBLY_PTR) \
do { \
	ASSERT(PROP_ASSEMBLY_PTR); \
	if (PROP_ASSEMBLY_PTR) { \
		if (vtkPropCollection *pPropCollection = PROP_ASSEMBLY_PTR->GetParts()) { \
			pPropCollection->RemoveAllItems(); \
		} \
	} \
} while(0)
///////////////////////////////////////////////////////////////////////////////////////

void CWPhastDoc::DeleteContents()
{
	// Add your specialized code here and/or call the base class
	TRACE("CWPhastDoc::DeleteContents()\n");

	ASSERT(this->m_pAxesActor);

	// reset model
	ASSERT(this->m_pModel);
	delete this->m_pModel;
	this->m_pModel = new CNewModel();
	this->m_pModel->m_printFreq = CPrintFreq::NewDefaults();

	// reset units
	ASSERT(this->m_pUnits);
	delete this->m_pUnits;
	this->m_pUnits = new CUnits();

	// clear undo/redo
	//
	ASSERT(this->m_pimpl);
	if (this->m_pimpl)
	{
		// delete all actions
		std::vector<CAction*>::iterator iter = this->m_pimpl->m_vectorActions.begin();
		for (; iter != this->m_pimpl->m_vectorActions.end(); ++iter) {
			delete (*iter);
		}
		this->m_pimpl->m_vectorActions.clear();
		this->m_pimpl->m_vectorActionsIndex = 0;
		this->m_pimpl->m_lastSaveIndex = -1;
	}

	// create the grid
	//
	if (this->m_pGridActor)
	{
		if (this->m_pGridActor->GetEnabled())
		{
			this->m_pGridActor->SetEnabled(0);
		}
		this->m_pGridActor->Delete();
	}
	this->m_pGridActor = CGridActor::New();
	ASSERT(this->m_pGridActor);
	this->m_pGridActor->AddObserver(CGridActor::DeleteGridLineEvent, this->GridCallbackCommand);
	this->m_pGridActor->AddObserver(CGridActor::InsertGridLineEvent, this->GridCallbackCommand);
	this->m_pGridActor->AddObserver(CGridActor::MoveGridLineEvent,   this->GridCallbackCommand);
// COMMENT: {8/9/2005 7:57:14 PM}	this->m_pGridActor->GetProperty()->SetColor(1.0, 0.8, 0.6);
	this->m_pGridActor->SetScale(1, 1, 1);
	this->m_pGridActor->SetPickable(0);

	// update views
	//
	POSITION pos = this->GetFirstViewPosition();
	while (pos != NULL)
	{
		CWPhastView *pView = (CWPhastView*) GetNextView(pos);
		ASSERT_VALID(pView);
		ASSERT(this->m_pPropCollection);
		if (this->m_pPropCollection)
		{
			// remove all props
			this->m_pPropCollection->InitTraversal();
			for (int i = 0; i < this->m_pPropCollection->GetNumberOfItems(); ++i)
			{
				vtkProp* prop = this->m_pPropCollection->GetNextProp();
				pView->GetRenderer()->GetProps()->RemoveItem(prop);
			}
		}

		// all vtkProps must release before the Renderer is destroyed
		// ie when an import fails
		this->m_pGridActor->ReleaseGraphicsResources(pView->GetRenderer()->GetVTKWindow());
		this->m_pAxesActor->ReleaseGraphicsResources(pView->GetRenderer()->GetVTKWindow());		

		pView->DeleteContents();
	}

	// clear prop collection
	//
	ASSERT(this->m_pPropCollection);
	if (this->m_pPropCollection)
	{
		// Note: all CZone are deleted when the CZoneCreateAction<CZone> is deleted
		this->m_pPropCollection->RemoveAllItems();
	}

	// clear removed prop collection
	//
	ASSERT(this->m_pRemovedPropCollection);
	if (this->m_pRemovedPropCollection)
	{
		this->m_pRemovedPropCollection->RemoveAllItems();
	}

	// update tree
	//
	if (CPropertyTreeControlBar* pTree = this->GetPropertyTreeControlBar())
	{
		ASSERT_VALID(pTree);
		pTree->DeleteContents();
	}

	// clear the zone prop-assemblies
	//
	CLEAR_PROP_ASSEMBLY_MACRO(this->m_pPropAssemblyMedia);
	CLEAR_PROP_ASSEMBLY_MACRO(this->m_pPropAssemblyIC);
	CLEAR_PROP_ASSEMBLY_MACRO(this->m_pPropAssemblyBC);
	CLEAR_PROP_ASSEMBLY_MACRO(this->m_pPropAssemblyWells);
	CLEAR_PROP_ASSEMBLY_MACRO(this->m_pPropAssemblyRivers);

	// Turn-all on by default
	//
	this->m_pPropAssemblyMedia->SetVisibility(1);
	this->m_pPropAssemblyIC->SetVisibility(1);
	this->m_pPropAssemblyBC->SetVisibility(1);
	this->m_pPropAssemblyWells->SetVisibility(1);
	this->m_pPropAssemblyRivers->SetVisibility(1);

	// reset scale
	//
	
// COMMENT: {7/28/2005 4:38:34 PM}	//{{
// COMMENT: {7/28/2005 4:38:34 PM}	// create the grid
// COMMENT: {7/28/2005 4:38:34 PM}	//
// COMMENT: {7/28/2005 4:38:34 PM}	if (this->m_pGridActor)
// COMMENT: {7/28/2005 4:38:34 PM}	{
// COMMENT: {7/28/2005 4:38:34 PM}		this->m_pGridActor->Delete();
// COMMENT: {7/28/2005 4:38:34 PM}	}
// COMMENT: {7/28/2005 4:38:34 PM}	this->m_pGridActor = CGridActor::New();
// COMMENT: {7/28/2005 4:38:34 PM}	this->m_pGridActor->GetProperty()->SetColor(1.0, 0.8, 0.6);
// COMMENT: {7/28/2005 4:38:34 PM}	//}}
// COMMENT: {7/28/2005 4:38:34 PM}	ASSERT(this->m_pGridActor);
// COMMENT: {7/28/2005 4:38:34 PM}	this->m_pGridActor->SetScale(1, 1, 1);

	if (this->m_pMapActor)
	{
		this->m_pMapActor->Delete();
		this->m_pMapActor = 0;
	}

	// update geometry property sheet
	// Note: can't call this->SetScale(1.0f, 1.0f, 1.0f);
	// Since the this->m_pGridActor contains no data for
	// this->m_pGridActor->GetBounds
	if (this->m_pScalePage)
	{
		this->m_pScalePage->m_XScale = 1;
		this->m_pScalePage->m_YScale = 1;
		this->m_pScalePage->m_ZScale = 1;
		if (this->m_pScalePage->GetSafeHwnd())
		{
			this->m_pScalePage->UpdateData(FALSE);
		}
	}

	// Update BoxPropertiesDialogBar
	//
	if (CBoxPropertiesDialogBar* pBar = this->GetBoxPropertiesDialogBar())
	{
		pBar->Set(0, 0, this->GetUnits());
	}

	this->UpdateAllViews(0);

	CDocument::DeleteContents();
}

void CWPhastDoc::Execute(CAction* pAction)
{
	pAction->Execute();

	while (this->m_pimpl->m_vectorActionsIndex < this->m_pimpl->m_vectorActions.size()) {
		delete this->m_pimpl->m_vectorActions.back();
		this->m_pimpl->m_vectorActions.pop_back();
	}
	this->m_pimpl->m_vectorActions.push_back(pAction);
	this->m_pimpl->m_vectorActionsIndex = this->m_pimpl->m_vectorActions.size();
	this->SetModifiedFlag(TRUE);
}

void CWPhastDoc::OnUpdateEditUndo(CCmdUI *pCmdUI)
{
	BOOL b = (this->m_pimpl->m_vectorActionsIndex > 0);
	b = b && (this->m_pGridSheet->GetSafeHwnd() == 0);
	b = b && (this->m_pGeometrySheet->GetSafeHwnd() == 0);
	pCmdUI->Enable(b);
}

void CWPhastDoc::OnUpdateEditRedo(CCmdUI *pCmdUI)
{
	BOOL b = (this->m_pimpl->m_vectorActionsIndex < this->m_pimpl->m_vectorActions.size());
	b = b && (this->m_pGridSheet->GetSafeHwnd() == 0);
	b = b && (this->m_pGeometrySheet->GetSafeHwnd() == 0);
	pCmdUI->Enable(b);
}

void CWPhastDoc::OnEditUndo()
{
	CWaitCursor wait;
	--(this->m_pimpl->m_vectorActionsIndex);
	ASSERT(this->m_pimpl->m_vectorActionsIndex >= 0);
	this->m_pimpl->m_vectorActions.at(this->m_pimpl->m_vectorActionsIndex)->UnExecute();
	this->SetModifiedFlag(this->m_pimpl->m_vectorActionsIndex != this->m_pimpl->m_lastSaveIndex);
}

void CWPhastDoc::OnEditRedo() // 57644
{
	CWaitCursor wait;
	ASSERT(this->m_pimpl->m_vectorActionsIndex < this->m_pimpl->m_vectorActions.size());
	this->m_pimpl->m_vectorActions.at(this->m_pimpl->m_vectorActionsIndex)->Execute();
	++(this->m_pimpl->m_vectorActionsIndex);
	ASSERT(this->m_pimpl->m_vectorActionsIndex != 0);
	this->SetModifiedFlag(this->m_pimpl->m_vectorActionsIndex != this->m_pimpl->m_lastSaveIndex);
}

void CWPhastDoc::SetScale(vtkFloatingPointType x, vtkFloatingPointType y, vtkFloatingPointType z)
{
	ASSERT(x != 0);
	ASSERT(y != 0);
	ASSERT(z != 0);

	vtkFloatingPointType scale[3];
	scale[0] = x;
	scale[1] = y;
	scale[2] = z;

	// Update scale page
	//
	ASSERT(this->m_pScalePage);
	if (this->m_pScalePage)
	{
		this->m_pScalePage->m_XScale = scale[0];
		this->m_pScalePage->m_YScale = scale[1];
		this->m_pScalePage->m_ZScale = scale[2];
		if (this->m_pScalePage->GetSafeHwnd())
		{
			this->m_pScalePage->UpdateData(FALSE);
		}
	}

	// set scale for the grid
	//
	this->m_pGridActor->SetScale(scale);

	// reset the axes
	//
	vtkFloatingPointType bounds[6];
	this->m_pGridActor->GetBounds(bounds);
	float defaultAxesSize = (bounds[1]-bounds[0] + bounds[3]-bounds[2] + bounds[5]-bounds[4])/12;
	this->m_pAxesActor->SetDefaultPositions(bounds);
	this->m_pAxesActor->SetDefaultSize(defaultAxesSize);
	this->m_pAxesActor->SetDefaultTubeDiameter(defaultAxesSize * 0.1);

	// set scale for the map
	//
	if (this->m_pMapActor)
	{
		this->m_pMapActor->SetScale(x, y, z);
	}

	// set scale for all zones
	//
	if (vtkPropCollection* pCollection = this->GetPropCollection())
	{
		pCollection->InitTraversal();
		for (int i = 0; i < pCollection->GetNumberOfItems(); ++i)
		{
			vtkProp* prop = pCollection->GetNextProp();
			if (CZoneActor *pZone = CZoneActor::SafeDownCast(prop))
			{
				pZone->SetScale(scale);
			}
			if (vtkPropAssembly *pPropAssembly = vtkPropAssembly::SafeDownCast(prop))
			{
				if (vtkPropCollection *pPropCollection = pPropAssembly->GetParts())
				{
					vtkProp *pProp;
					pPropCollection->InitTraversal();
					for (; (pProp = pPropCollection->GetNextProp()); )
					{
						if (vtkProp3D *prop3D = vtkProp3D::SafeDownCast(pProp))
						{
							prop3D->SetScale(scale);
						}
					}
				}
			}
		}
	}

	// if modifying grid update GridElementsSelector
	//
	if (this->GridElementsSelector)
	{
		int ibounds[6];
		this->GridElementsSelector->GetIBounds(ibounds);
		this->GridElementsSelector->SetIBounds(ibounds);
	}

	this->Notify(0, WPN_SCALE_CHANGED, 0, 0);
// COMMENT: {5/8/2006 4:32:55 PM}	this->UpdateAllViews(0);
}

void CWPhastDoc::SetFlowOnly(const CFlowOnly& flowOnly)
{
	this->m_pModel->m_flowOnly = flowOnly;
	if (CPropertyTreeControlBar* pTree = this->GetPropertyTreeControlBar())
	{
		pTree->SetFlowOnly(&this->m_pModel->m_flowOnly);
	}
}

CFlowOnly CWPhastDoc::GetFlowOnly(void)const
{
	return this->m_pModel->m_flowOnly;
}

void CWPhastDoc::SetSteadyFlow(const CSteadyFlow& steadyFlow)
{
	this->m_pModel->m_steadyFlow = steadyFlow;
	if (CPropertyTreeControlBar* pTree = this->GetPropertyTreeControlBar())
	{
		pTree->SetSteadyFlow(&this->m_pModel->m_steadyFlow);
	}
}

CSteadyFlow CWPhastDoc::GetSteadyFlow(void)const
{
	return this->m_pModel->m_steadyFlow;
}

void CWPhastDoc::SetFreeSurface(const CFreeSurface &freeSurface)
{
	this->m_pModel->m_freeSurface = freeSurface;
	if (CPropertyTreeControlBar* pTree = this->GetPropertyTreeControlBar())
	{
		pTree->SetFreeSurface(&this->m_pModel->m_freeSurface);
	}
}

CFreeSurface CWPhastDoc::GetFreeSurface(void)const
{
	return this->m_pModel->m_freeSurface;
}

CNewModel* CWPhastDoc::GetModel(void)
{
	return this->m_pModel;
}

void CWPhastDoc::SetModel(const CNewModel& model)
{
	ASSERT(this->m_pModel);
	(*this->m_pModel) = model;
	if (CPropertyTreeControlBar* pTree = this->GetPropertyTreeControlBar())
	{
		pTree->SetModel(this->m_pModel);
	}
}

void CWPhastDoc::ResizeGrid(const CGrid& x, const CGrid&  y, const CGrid&  z)
{
	ASSERT(this->m_pGridActor);
	ASSERT(this->m_pAxesActor);
	ASSERT(this->m_pPropCollection);
	ASSERT(this->m_pimpl);
	ASSERT(this->m_pUnits);
	ASSERT(this->m_pModel);

	// reset the grid
	//
	this->m_pGridActor->SetGrid(x, y, z, *this->m_pUnits);
	this->m_pGridActor->SetPickable(0);
	this->m_pPropCollection->AddItem(this->m_pGridActor);
	if (CPropertyTreeControlBar* pTree = this->GetPropertyTreeControlBar())
	{
		pTree->SetGridActor(this->m_pGridActor);
	}

	// Update default zones etc.
	//
	this->UpdateGridDomain();
}


void CWPhastDoc::AddDefaultZone(CZone* pZone)
{
	// TODO : Is this still used?
	if (pZone)
	{
		this->m_pimpl->m_vectorDefaultZones.push_back(pZone);
	}
}

vtkFloatingPointType* CWPhastDoc::GetScale()
{
	return this->m_pGridActor->GetScale();
}

void CWPhastDoc::GetScale(vtkFloatingPointType data[3])
{
	this->m_pGridActor->GetScale(data);
}

vtkFloatingPointType* CWPhastDoc::GetGridBounds()
{
	if (this->m_pGridActor)
	{
		return this->m_pGridActor->GetBounds();
	}
	ASSERT(FALSE);
	return 0;
}

void CWPhastDoc::OnUpdateToolsGeometry(CCmdUI *pCmdUI)
{
	if (this->m_pGeometrySheet->GetSafeHwnd())
	{
		pCmdUI->SetCheck(1);
	}
	else
	{
		pCmdUI->SetCheck(0);
	}
}

void CWPhastDoc::OnToolsGeometry()
{
	if (this->m_pGeometrySheet->GetSafeHwnd())
	{
		this->m_pGeometrySheet->DestroyWindow();
	}
	else
	{
		// this allows main window to cover the propertysheet but
		// also causes the propertysheet to display on the taskbar
		///this->m_pGeometrySheet->Create();

		// this forces the propertysheet to always be on top of
		// the main window but the propertysheet doesn't show up
		// on the taskbar
		this->m_pGeometrySheet->Create(::AfxGetApp()->m_pMainWnd);
	}
}

void CWPhastDoc::ResetCamera(void)
{
	POSITION pos = this->GetFirstViewPosition();
	while (pos != NULL) {
		CWPhastView *pView = (CWPhastView*) this->GetNextView(pos);
		ASSERT_VALID(pView);
		pView->ResetCamera();
	}
}

CString CWPhastDoc::GetNextZoneName(void)
{
	static CString str;
	str.Format(szZoneFormat, this->GetNextZoneNumber());
	return str;
}

int CWPhastDoc::GetNextWellNumber(void)
{
	std::set<int> wellNums;
	this->GetUsedWellNumbers(wellNums);
	if (wellNums.rbegin() != wellNums.rend())
	{
		return (*wellNums.rbegin()) + 1;
	}
	else
	{
		return 1;
	}
}

void CWPhastDoc::GetUsedWellNumbers(std::set<int>& usedNums)
{
	usedNums.clear();
	if (vtkPropCollection *pPropCollection = this->GetPropAssemblyWells()->GetParts())
	{
		vtkProp *pProp = 0;
		pPropCollection->InitTraversal();
		for (; (pProp = pPropCollection->GetNextProp()); )
		{
			if (CWellActor *pWellActor = CWellActor::SafeDownCast(pProp))
			{
				// store used n_user numbers
				//
				std::pair< std::set<int>::iterator, bool > pr;
				pr = usedNums.insert( pWellActor->GetWell().n_user );
				ASSERT(pr.second); // duplicate?
			}
		}
	}
}

int CWPhastDoc::GetNextRiverNumber(void)
{
	std::set<int> riverNums;
	this->GetUsedRiverNumbers(riverNums);
	if (riverNums.rbegin() != riverNums.rend())
	{
		return (*riverNums.rbegin()) + 1;
	}
	else
	{
		return 1;
	}
}

void CWPhastDoc::GetUsedRiverNumbers(std::set<int>& usedNums)
{
	usedNums.clear();
	if (vtkPropCollection *pPropCollection = this->GetPropAssemblyRivers()->GetParts())
	{
		vtkProp *pProp = 0;
		pPropCollection->InitTraversal();
		for (; (pProp = pPropCollection->GetNextProp()); )
		{
			if (CRiverActor *pRiverActor = CRiverActor::SafeDownCast(pProp))
			{
				// store used n_user numbers
				//
				std::pair< std::set<int>::iterator, bool > pr;
				pr = usedNums.insert( pRiverActor->GetRiver().n_user );
				ASSERT(pr.second); // duplicate?
			}
		}
	}
}

int CWPhastDoc::GetNextZoneNumber(void)const
{
	std::set<int> zoneNums;
	this->GetUsedZoneNumbers(zoneNums);
	if (zoneNums.rbegin() != zoneNums.rend())
	{
		return (*zoneNums.rbegin()) + 1;
	}
	else
	{
		return 1;
	}
}

void CWPhastDoc::GetUsedZoneNumbers(std::set<int>& usedNums)const
{
	char *ptr;
	usedNums.clear();

	std::list<vtkPropCollection*> collections;
	collections.push_back(this->GetPropAssemblyMedia()->GetParts());
	collections.push_back(this->GetPropAssemblyBC()->GetParts());
	collections.push_back(this->GetPropAssemblyIC()->GetParts());

	std::list<vtkPropCollection*>::iterator iter = collections.begin();
	for (; iter != collections.end(); ++iter)
	{
		if (vtkPropCollection *pPropCollection = (*iter))
		{
			vtkProp *pProp = 0;
			pPropCollection->InitTraversal();
			for (; (pProp = pPropCollection->GetNextProp()); )
			{
				if (CZoneActor *pZoneActor = CZoneActor::SafeDownCast(pProp))
				{
					// store used n_user numbers
					//
					std::pair< std::set<int>::iterator, bool > pr;
					CString str = pZoneActor->GetName();
					if (str.Find(szZoneFind) == 0) {
						int n = ::strtol((const char*)str + 4, &ptr, 10);
						pr = usedNums.insert(n);
						ASSERT(pr.second); // duplicate?
					}
				}
			}
		}
	}
}

BOOL CWPhastDoc::OnSaveDocument(LPCTSTR lpszPathName)
{
	// Add your specialized code here and/or call the base class
	this->m_pimpl->m_lastSaveIndex = this->m_pimpl->m_vectorActionsIndex;

	return CDocument::OnSaveDocument(lpszPathName);
}

void CWPhastDoc::OnCloseDocument()
{
	// Add your specialized code here and/or call the base class

	// must destroy all modeless dialogs here
	// otherwise GetPropertyTreeControlBar will cause
	// an ASSERT when called from CWPhastDoc::DeleteContents
	// which is called by CDocument::OnCloseDocument

	ASSERT(this->m_pScalePage);
	this->m_pScalePage->DestroyWindow();
	delete this->m_pScalePage;
	this->m_pScalePage = 0;

	ASSERT(this->m_pGeometrySheet);
	this->m_pGeometrySheet->DestroyWindow();
	delete this->m_pGeometrySheet;
	this->m_pGeometrySheet = 0;

// COMMENT: {7/19/2005 8:13:02 PM}	// Grid
// COMMENT: {7/19/2005 8:13:02 PM}	//
// COMMENT: {7/19/2005 8:13:02 PM}	if (this->m_pGridPage)
// COMMENT: {7/19/2005 8:13:02 PM}	{
// COMMENT: {7/19/2005 8:13:02 PM}		this->m_pGridPage->DestroyWindow();
// COMMENT: {7/19/2005 8:13:02 PM}		delete this->m_pGridPage;
// COMMENT: {7/19/2005 8:13:02 PM}		this->m_pGridPage = 0;
// COMMENT: {7/19/2005 8:13:02 PM}	}
// COMMENT: {7/19/2005 8:13:02 PM}	if (this->m_pGridSheet)
// COMMENT: {7/19/2005 8:13:02 PM}	{
// COMMENT: {7/19/2005 8:13:02 PM}		this->m_pGridSheet->DestroyWindow();
// COMMENT: {7/19/2005 8:13:02 PM}		delete this->m_pGridSheet;
// COMMENT: {7/19/2005 8:13:02 PM}		this->m_pGridSheet = 0;
// COMMENT: {7/19/2005 8:13:02 PM}	}

	CDocument::OnCloseDocument();
}

void CWPhastDoc::OnFileImport()
{
	CString newName;
	if (!DoPromptFileName(newName, IDS_IMPORT_PHAST_TRANS_133, //AFX_IDS_OPENFILE,
	  OFN_HIDEREADONLY | OFN_FILEMUSTEXIST, TRUE))
		return; // open cancelled

	// modified from: CDocument* CDocManager::OpenDocumentFile
	//
	TCHAR szPath[_MAX_PATH];
	ASSERT(lstrlen(newName) < sizeof(szPath)/sizeof(szPath[0]));
	TCHAR szTemp[_MAX_PATH];
	if (newName[0] == '\"')
		newName = newName.Mid(1);
	lstrcpyn(szTemp, newName, _MAX_PATH);
	LPTSTR lpszLast = _tcsrchr(szTemp, '\"');
	if (lpszLast != NULL)
		*lpszLast = 0;
	SrcFullPath(szPath, szTemp);
	TCHAR szLinkName[_MAX_PATH];
	if (SrcResolveShortcut(AfxGetMainWnd(), szPath, szLinkName, _MAX_PATH))
		lstrcpy(szPath, szLinkName);

	CDocument* pDoc = this->GetDocTemplate()->OpenDocumentFile(NULL);
	if (pDoc)
	{
		ASSERT(pDoc == this);
		CDelayRedraw delay(::AfxGetMainWnd(), pDoc);
		if (!this->DoImport(szPath))
		{
			::AfxMessageBox("An error occured during the import", MB_OK);
			this->SetModifiedFlag(FALSE);
		}
		this->ResetCamera();
	}
	if (CPropertyTreeControlBar *pPropertyTreeControlBar = this->GetPropertyTreeControlBar())
	{
		if (CWnd *pTree = pPropertyTreeControlBar->GetTreeCtrl())
		{
			pTree->RedrawWindow();
		}
	}
}

BOOL CWPhastDoc::DoImport(LPCTSTR lpszPathName)
{
	// Return Value
	// Nonzero if the file was successfully loaded; otherwise 0.
	CWaitCursor wait;

	TCHAR szDrive[_MAX_DRIVE];
	TCHAR szDir[_MAX_DIR];
	TCHAR szFName[_MAX_FNAME];
	TCHAR szExt[_MAX_EXT];

	::_tsplitpath(lpszPathName, szDrive, szDir, szFName, szExt);

	// remove .trans if exists
	//
	CString strPrefix(szFName);
	strPrefix.MakeLower();
	strPrefix.Replace(_T(".trans"), _T(""));

	std::ifstream ifs;
	ifs.open(lpszPathName);
	if (!ifs.is_open())
	{
		return FALSE;
	}

	CPhastInput* pInput = CPhastInput::New(ifs, strPrefix);
	if (!pInput) return FALSE;

	BOOL bReturnValue = TRUE;
	try
	{
		CPrintFreq printFreq;
		printFreq.InitSync(pInput); // must be called before Load()

		pInput->Load();
		if (pInput->GetErrorCount() != 0)
		{
			// goto ImportError;
			// throw "Input file contains errors.";
			throw pInput->GetErrorCount();
		}

		// start loading data

		// FLOW_ONLY
		//
		CFlowOnly flowOnly(::flow_only != 0);
		if (!flowOnly)
		{
			flowOnly.SetDiffusivity(::fluid_diffusivity);
		}

		// STEADY_FLOW
		//
		CSteadyFlow steadyFlow;
		steadyFlow.SyncWithSrcInput();

		// MEDIA
		//
		CGridElt gridElt = CGridElt::Full();
		::GetDefaultMedia(&gridElt);

		// HEAD_IC
		//
		CHeadIC headIC = CHeadIC::Full();
		::GetDefaultHeadIC(&headIC);

		// CHEMISTRY_IC
		//
		CChemIC chemIC = CChemIC::Full();
		::GetDefaultChemIC(&chemIC);

		// PRINT_INITIAL
		//
		printFreq.SyncWithSrcInput();

		// PRINT_FREQUENCY
		//
		CPrintInput printInput;
		printInput.SyncWithSrcInput();

		// SOLUTION_METHOD
		//
		CSolutionMethod solutionMethod;
		solutionMethod.SyncWithSrcInput();

		// TIME_CONTROL
		//
		CTimeControl2 timeControl2(::time_step, ::time_end, ::count_time_end);

		// create new document
		//
		CNewModel model;
		model.m_flowOnly       = flowOnly;
		model.m_freeSurface    = (::free_surface != 0);
		model.m_steadyFlow     = steadyFlow;
		model.m_units          = ::units;
		model.m_gridKeyword    = CGridKeyword(::grid, ::snap, ::axes, ::print_input_xy);
		model.m_media          = gridElt;
		model.m_headIC         = headIC;
		model.m_chemIC         = chemIC;
		model.m_printInput     = printInput;
		model.m_printFreq      = printFreq;
		model.m_timeControl2   = timeControl2;
		model.m_solutionMethod = solutionMethod;

		this->New(model);

		// MEDIA zones
		//
		for (int i = 0; i < ::count_grid_elt_zones; ++i)
		{
			// not undoable
			const struct grid_elt* grid_elt_ptr = ::grid_elt_zones[i];

			if (i == 0)
			{
				// if the first zone is equivalent to the default gridElt
				// don't add it

				// TODO: check for equivalance to default gridElt
			}

			CZoneCreateAction<CMediaZoneActor>* pAction = new CZoneCreateAction<CMediaZoneActor>(
				this,
				this->GetNextZoneName(),
				grid_elt_ptr->zone->x1,
				grid_elt_ptr->zone->x2,
				grid_elt_ptr->zone->y1,
				grid_elt_ptr->zone->y2,
				grid_elt_ptr->zone->z1,
				grid_elt_ptr->zone->z2,
				NULL
				);
			pAction->GetZoneActor()->SetGridElt(*grid_elt_ptr);
			pAction->Execute();
			delete pAction;
		}

		// BC Zones
		//
		std::list<CBCZoneActor*> listBCZoneActors;
		for (int i = 0; i < ::count_bc; ++i)
		{
			const struct bc* bc_ptr = ::bc[i];

			// not undoable
			//
			CZoneCreateAction<CBCZoneActor>* pAction = new CZoneCreateAction<CBCZoneActor>(
				this,
				this->GetNextZoneName(),
				bc_ptr->zone->x1,
				bc_ptr->zone->x2,
				bc_ptr->zone->y1,
				bc_ptr->zone->y2,
				bc_ptr->zone->z1,
				bc_ptr->zone->z2,
				NULL
				);
			pAction->GetZoneActor()->SetBC(*bc_ptr);
			pAction->Execute();
			listBCZoneActors.push_back(pAction->GetZoneActor());
			delete pAction;
		}

		// Wells
		//
		for (int i = 0; i < ::count_wells; ++i)
		{
			const Well* well_ptr = &::wells[i];			
			CWellSchedule well(*well_ptr);

			// not undoable
			CWellCreateAction *pAction = new CWellCreateAction(this, well);
			pAction->Execute();
			delete pAction;
		}

		// RIVER
		//
		for (int i = 0; i < ::count_rivers; ++i)
		{
			const River* river_ptr = &::rivers[i];
			CRiver river(*river_ptr);
			
			// not undoable
			CRiverCreateAction *pAction = new CRiverCreateAction(this, river);
			pAction->Execute();
			delete pAction;
		}

		// IC
		//
		for (int i = 0; i < ::count_head_ic; ++i)
		{
			const struct head_ic* head_ic_ptr = ::head_ic[i];

			if (i == 0)
			{
				// if the first zone is equivalent to the default headIC
				// don't add it

				// TODO: check for equivalance to default headIC
			}

			CZoneCreateAction<CICHeadZoneActor>* pAction = new CZoneCreateAction<CICHeadZoneActor>(
				this,
				this->GetNextZoneName(),
				head_ic_ptr->zone->x1,
				head_ic_ptr->zone->x2,
				head_ic_ptr->zone->y1,
				head_ic_ptr->zone->y2,
				head_ic_ptr->zone->z1,
				head_ic_ptr->zone->z2,
				NULL
				);
			pAction->GetZoneActor()->SetData(*head_ic_ptr);
			pAction->Execute();
			delete pAction;
		}

		// CHEMISTRY_IC
		//
		for (int i = 0; i < ::count_chem_ic; ++i)
		{
			const struct chem_ic* chem_ic_ptr = ::chem_ic[i];

			if (i == 0)
			{
				// if the first zone is equivalent to the default hechemICadIC
				// don't add it

				// TODO: check for equivalance to default chemIC
			}

			CZoneCreateAction<CICChemZoneActor>* pAction = new CZoneCreateAction<CICChemZoneActor>(
				this,
				this->GetNextZoneName(),
				chem_ic_ptr->zone->x1,
				chem_ic_ptr->zone->x2,
				chem_ic_ptr->zone->y1,
				chem_ic_ptr->zone->y2,
				chem_ic_ptr->zone->z1,
				chem_ic_ptr->zone->z2,
				NULL
				);
			pAction->GetZoneActor()->SetData(*chem_ic_ptr);
			pAction->Execute();
			delete pAction;
		}
	}
	catch (int)
	{
		CImportErrorDialog dlg;
		dlg.m_lpszErrorMessages = pInput->GetErrorMsg();
		dlg.DoModal();
		bReturnValue = FALSE;
	}
	catch (const char * error)
	{
		::AfxMessageBox(error, MB_OK|MB_ICONEXCLAMATION);
		bReturnValue = FALSE;
	}
	catch (...)
	{
		::AfxMessageBox("An unknown error occured during import", MB_OK|MB_ICONEXCLAMATION);
		bReturnValue = FALSE;
	}

	// reset tree control
	//
	if (CPropertyTreeControlBar* pTree = this->GetPropertyTreeControlBar())
	{
		pTree->GetGridNode().Expand(TVE_COLLAPSE);
		pTree->GetMediaNode().Expand(TVE_COLLAPSE);
		pTree->GetICNode().Expand(TVE_COLLAPSE);
		pTree->GetBCNode().Expand(TVE_COLLAPSE);
		pTree->GetWellsNode().Expand(TVE_COLLAPSE);
		pTree->GetRiversNode().Expand(TVE_COLLAPSE);
		this->ClearSelection();
	}

	pInput->Delete();
	this->SetTitle(strPrefix);
	this->SetModifiedFlag(TRUE);

	return bReturnValue;

//ImportError:
//	CImportErrorDialog dlg;
//	dlg.m_lpszErrorMessages = pInput->GetErrorMsg();
//	dlg.DoModal();
//	pInput->Delete();
//	return FALSE;
}

void CWPhastDoc::OnFileExport()
{
	CString newName;
	newName = this->GetTitle() + ".trans.dat";
	if (!DoPromptFileName(newName, IDS_EXPORT_PHAST_TRANS_136,
	  OFN_HIDEREADONLY, FALSE))
		return; // save cancelled

	// modified from: CDocument* CDocManager::OpenDocumentFile
	TCHAR szPath[_MAX_PATH];
	ASSERT(lstrlen(newName) < sizeof(szPath)/sizeof(szPath[0]));
	TCHAR szTemp[_MAX_PATH];
	if (newName[0] == '\"')
		newName = newName.Mid(1);
	lstrcpyn(szTemp, newName, _MAX_PATH);
	LPTSTR lpszLast = _tcsrchr(szTemp, '\"');
	if (lpszLast != NULL)
		*lpszLast = 0;
	SrcFullPath(szPath, szTemp);
	TCHAR szLinkName[_MAX_PATH];
	if (SrcResolveShortcut(AfxGetMainWnd(), szPath, szLinkName, _MAX_PATH))
		lstrcpy(szPath, szLinkName);

	CString strPath(szPath);
	if (strPath.Find(_T(".trans.dat")) == -1) {
		strPath += ".trans.dat";
	}

	if (!this->DoExport(strPath)) {
		::AfxMessageBox("An error occured during the export", MB_OK);
		this->SetModifiedFlag(FALSE);
	}

}

BOOL CWPhastDoc::DoExport(LPCTSTR lpszPathName)
{
	std::ofstream ofs;
	ofs.open(lpszPathName);
	if (!ofs.is_open())
	{
		CString str("Unable to open \"");
		str += lpszPathName;
		str += "\" for writing.";
		::AfxMessageBox(str, MB_OK|MB_ICONEXCLAMATION);
		return FALSE;
	}

	CString path = this->GetPathName();
	if (path.IsEmpty())
	{
		ofs << "# Exported from WPhast(Unsaved File)\n";
	}
	else
	{
		ofs << "# Exported from WPhast(" << (LPCTSTR)path <<  ")\n";
	}

	return this->WriteTransDat(ofs);
}

BOOL CWPhastDoc::WriteTransDat(std::ostream& os)
{
	int nCount;

	// FLOW_ONLY
	os << this->m_pModel->m_flowOnly;

	// STEADY_FLOW
	os << this->m_pModel->m_steadyFlow;

	// FREE_SURFACE
	os << this->m_pModel->m_freeSurface;

	// SOLUTION_METHOD
	os << this->m_pModel->m_solutionMethod;

	// UNITS
	os << this->GetUnits();

	// GRID
	CGridKeyword gridKeyword;
	this->m_pGridActor->GetGridKeyword(gridKeyword);
	os << gridKeyword;

	// MEDIA
	CTreeCtrlNode nodeMedia = this->GetPropertyTreeControlBar()->GetMediaNode();
	nCount = nodeMedia.GetChildCount();
	if (nCount > 0) {
		os << "MEDIA\n";
	}
	for (int i = 0; i < nCount; ++i) {
		if (CMediaZoneActor *pZone = CMediaZoneActor::SafeDownCast((vtkObject*)nodeMedia.GetChildAt(i).GetData())) {
			CGridElt elt = pZone->GetGridElt();
			os << elt;
		}
	}

	// BCs SPECIFIED_VALUE_BC FLUX_BC LEAKY_BC
	CTreeCtrlNode nodeBC = this->GetPropertyTreeControlBar()->GetBCNode();
	nCount = nodeBC.GetChildCount();
	for (int i = 0; i < nCount; ++i) {
		if (CBCZoneActor *pZone = CBCZoneActor::SafeDownCast((vtkObject*)nodeBC.GetChildAt(i).GetData())) {
			CBC b = pZone->GetBC();
			os << b;
		}
	}

	// HEAD_IC 
	CTreeCtrlNode nodeICHead = this->GetPropertyTreeControlBar()->GetICHeadNode();
	nCount = nodeICHead.GetChildCount();
	for (int i = 0; i < nCount; ++i)
	{
		if (CICHeadZoneActor *pZone = CICHeadZoneActor::SafeDownCast((vtkObject*)nodeICHead.GetChildAt(i).GetData()))
		{
			os << pZone->GetData();
		}
	}

	// CHEMISTRY_IC 
	CTreeCtrlNode nodeICChem = this->GetPropertyTreeControlBar()->GetICChemNode();
	nCount = nodeICChem.GetChildCount();
	for (int i = 0; i < nCount; ++i)
	{
		if (CICChemZoneActor *pZone = CICChemZoneActor::SafeDownCast((vtkObject*)nodeICChem.GetChildAt(i).GetData()))
		{
			os << pZone->GetData();
		}
	}

	// Wells
	CTreeCtrlNode nodeWells = this->GetPropertyTreeControlBar()->GetWellsNode();
	nCount = nodeWells.GetChildCount();
	for (int i = 0; i < nCount; ++i)
	{
		if (CWellActor *pWellActor = CWellActor::SafeDownCast((vtkObject*)nodeWells.GetChildAt(i).GetData()))
		{
			os << pWellActor->GetWell();
		}
	}

	// RIVERS
	CTreeCtrlNode nodeRivers = this->GetPropertyTreeControlBar()->GetRiversNode();
	nCount = nodeRivers.GetChildCount();
	for (int i = 0; i < nCount; ++i)
	{
		if (CRiverActor *pRiverActor = CRiverActor::SafeDownCast((vtkObject*)nodeRivers.GetChildAt(i).GetData()))
		{
			os << *pRiverActor;
		}
	}

	os << this->GetPrintInput();

	os << this->GetPrintFrequency();

	os << this->GetTimeControl2();

	return TRUE;
}

BOOL CWPhastDoc::DoPromptFileName(CString& fileName, UINT nIDSTitle, DWORD lFlags, BOOL bOpenFileDialog)
{
	// modified from: BOOL CDocManager::DoPromptFileName
	CFileDialog dlgFile(bOpenFileDialog, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, NULL, 0);

	CString title;
	VERIFY(title.LoadString(nIDSTitle));

	dlgFile.m_ofn.Flags |= lFlags;

	CString strFilter;

	// add *.trans.dat filter
	// modified from: _AfxAppendFilterSuffix
	strFilter += _T("Phast Transport Files (*.trans.dat)");
	strFilter += (TCHAR)_T('\0');
	strFilter += _T("*.trans.dat");
	strFilter += (TCHAR)_T(';');
	strFilter += (TCHAR)_T('\0');
	dlgFile.m_ofn.nFilterIndex = dlgFile.m_ofn.nMaxCustFilter + 1;  // 1 based number
	dlgFile.m_ofn.nMaxCustFilter++;

	// append the "*.*" all files filter
	CString allFilter;
	VERIFY(allFilter.LoadString(AFX_IDS_ALLFILTER));
	strFilter += allFilter;
	strFilter += (TCHAR)'\0';   // next string please
	strFilter += _T("*.*");
	strFilter += (TCHAR)'\0';   // last string
	dlgFile.m_ofn.nMaxCustFilter++;

	dlgFile.m_ofn.lpstrFilter = strFilter;
	dlgFile.m_ofn.lpstrTitle = title;
	dlgFile.m_ofn.lpstrFile = fileName.GetBuffer(_MAX_PATH);

	INT_PTR nResult = dlgFile.DoModal();
	fileName.ReleaseBuffer();
	return nResult == IDOK;
}

const CUnits& CWPhastDoc::GetUnits(void)const
{
	return *this->m_pUnits;
}

void CWPhastDoc::GetUnits(CUnits& units)const
{
	units = (*this->m_pUnits);
}

void CWPhastDoc::SetUnits(const CUnits& units)
{
	ASSERT(this->m_pUnits);
	(*this->m_pUnits) = units;

	// minimize length units for display purposes
	//
	if (this->m_pUnits->horizontal.defined == TRUE)
	{
		CString strLength(this->m_pUnits->horizontal.input);
		CGlobal::MinimizeLengthUnits(strLength);
		this->m_pUnits->horizontal.set_input(strLength);
	}
	if (this->m_pUnits->vertical.defined == TRUE)
	{
		CString strLength(this->m_pUnits->vertical.input);
		CGlobal::MinimizeLengthUnits(strLength);
		this->m_pUnits->vertical.set_input(strLength);
	}

	// update properties bar
	//
	if (CPropertyTreeControlBar* pTree = this->GetPropertyTreeControlBar())
	{
		pTree->SetUnits(this->m_pUnits);
	}

	// update grid
	//
	CGrid x, y, z;
	this->m_pGridActor->GetGrid(x, y, z);
	this->ResizeGrid(x, y, z);

	// update zones
	//
	if (vtkPropCollection* pCollection = this->GetPropCollection())
	{
		pCollection->InitTraversal();
		for (int i = 0; i < pCollection->GetNumberOfItems(); ++i)
		{
			vtkProp* prop = pCollection->GetNextProp();
			if (CZoneActor *pZone = CZoneActor::SafeDownCast(prop))
			{
				ASSERT(FALSE); // TODO 8/24/2004 Does this need to change (similar to SetScale)
				pZone->SetUnits(units);
			}
			if (CWellActor *pWell = CWellActor::SafeDownCast(prop))
			{
				ASSERT(FALSE);
			}
		}
	}

	// set scale for all zones
	//
	if (vtkPropCollection* pCollection = this->GetPropCollection())
	{
		pCollection->InitTraversal();
		for (int i = 0; i < pCollection->GetNumberOfItems(); ++i)
		{
			vtkProp* prop = pCollection->GetNextProp();
			if (CZoneActor *pZone = CZoneActor::SafeDownCast(prop))
			{
				ASSERT(FALSE);
				pZone->SetUnits(units);
			}
			if (vtkPropAssembly *pPropAssembly = vtkPropAssembly::SafeDownCast(prop))
			{
				if (vtkPropCollection *pPropCollection = pPropAssembly->GetParts())
				{
					vtkProp *pProp;
					pPropCollection->InitTraversal();
					for (; (pProp = pPropCollection->GetNextProp()); )
					{
						if (vtkProp3D *prop3D = vtkProp3D::SafeDownCast(pProp))
						{
							if (CZoneActor *pZone = CZoneActor::SafeDownCast(prop3D))
							{
								pZone->SetUnits(units);
							}
							if (CWellActor *pWellActor = CWellActor::SafeDownCast(prop3D))
							{
								pWellActor->SetUnits(units);
							}
							if (CRiverActor *pRiverActor = CRiverActor::SafeDownCast(prop3D))
							{
								pRiverActor->SetUnits(units);
							}
						}
					}
				}
			}
		}
	}

	// for all views
	//
	POSITION pos = this->GetFirstViewPosition();
	while (pos != NULL)
	{
		CWPhastView *pView = (CWPhastView*) GetNextView(pos);

		// resize the selection bounding box
		//
		if (vtkAbstractPropPicker *picker = vtkAbstractPropPicker::SafeDownCast( pView->GetRenderWindowInteractor()->GetPicker() ))
		{
			if (vtkProp3D* prop = picker->GetProp3D())
			{
				if (CZoneActor *pZone = CZoneActor::SafeDownCast(prop))
				{
					pZone->Select(pView);
				}
			}
		}

		// resize the Box Widget
		//
		if (pView->GetBoxWidget()->GetProp3D())
		{
			pView->GetBoxWidget()->PlaceWidget();
		}
	}
	this->UpdateAllViews(0);


	// Update BoxPropertiesDialogBar
	//
	if (CBoxPropertiesDialogBar* pBar = this->GetBoxPropertiesDialogBar())
	{
		POSITION pos = this->GetFirstViewPosition();
		CWPhastView *pView = (CWPhastView*) GetNextView(pos);
		pBar->Set(pView, pBar->GetProp3D(), this->GetUnits());
	}
}

void CWPhastDoc::New(const CNewModel& model)
{
	ASSERT(this->m_pGridActor);
	ASSERT(this->m_pAxesActor);
	ASSERT(this->m_pPropCollection);
	ASSERT(this->m_pimpl);
	ASSERT(this->m_pUnits);

	// set FlowOnly
	// set SteadyFlow
	// set FreeSurface
	// set PrintInput
	// set PrintFreq
	this->SetModel(model);

	// set the units
	//
	ASSERT(this->m_pUnits);
	*this->m_pUnits = model.m_units;
	// update properties bar
	//
	if (CPropertyTreeControlBar* pTree = this->GetPropertyTreeControlBar())
	{
		pTree->SetUnits(this->m_pUnits);
	}

	// set the grid
	//
	this->m_pGridActor->SetGridKeyword(model.m_gridKeyword, this->GetUnits());
	this->m_pGridActor->SetPickable(0);
	this->m_pPropCollection->AddItem(this->m_pGridActor);
	if (CPropertyTreeControlBar* pTree = this->GetPropertyTreeControlBar()) {
		pTree->SetGridActor(this->m_pGridActor);
	}

	// set the axes
	//
	vtkFloatingPointType *bounds = this->GetGridBounds();
	vtkFloatingPointType defaultAxesSize = (bounds[1]-bounds[0] + bounds[3]-bounds[2] + bounds[5]-bounds[4])/12;
	this->m_pAxesActor->SetDefaultPositions(bounds);
	this->m_pAxesActor->SetDefaultSize(defaultAxesSize);
	this->m_pAxesActor->SetDefaultTubeDiameter(defaultAxesSize * 0.1);
	this->m_pPropCollection->AddItem(this->m_pAxesActor);

	// create the default zones
	//
	CZone zone;
	this->m_pGridActor->GetDefaultZone(zone);

	// default media
	//
	CZoneCreateAction<CMediaZoneActor>* pMediaAction = new CZoneCreateAction<CMediaZoneActor>(
		this,
		"Default",
		zone.x1,
		zone.x2,
		zone.y1,
		zone.y2,
		zone.z1,
		zone.z2,
		NULL
		);
	pMediaAction->GetZoneActor()->SetGridElt(model.m_media);
	pMediaAction->GetZoneActor()->SetDefault(true);
	pMediaAction->Execute();
	delete pMediaAction;


	// default head_ic
	CZoneCreateAction<CICHeadZoneActor>* pICHeadAction = new CZoneCreateAction<CICHeadZoneActor>(
		this,
		"Default",
		zone.x1,
		zone.x2,
		zone.y1,
		zone.y2,
		zone.z1,
		zone.z2,
		NULL
		);
	pICHeadAction->GetZoneActor()->SetData(model.m_headIC);
	pICHeadAction->GetZoneActor()->SetDefault(true);
	pICHeadAction->Execute();
	delete pICHeadAction;

	// CHEMISTRY_IC (default)
	CZoneCreateAction<CICChemZoneActor>* pChemICAction = new CZoneCreateAction<CICChemZoneActor>(
		this,
		"Default",
		zone.x1,
		zone.x2,
		zone.y1,
		zone.y2,
		zone.z1,
		zone.z2,
		NULL
		);
	pChemICAction->GetZoneActor()->SetData(model.m_chemIC);
	pChemICAction->GetZoneActor()->SetDefault(true);
	pChemICAction->Execute();
	delete pChemICAction;

	// update properties bar
	//
	if (CPropertyTreeControlBar* pTree = this->GetPropertyTreeControlBar())
	{
		pTree->SetTimeControl2(&this->m_pModel->m_timeControl2);
	}

	if (model.HasSiteMap())
	{
		ASSERT(this->m_pMapActor == NULL);
		this->m_pMapActor = CMapActor::New();   // Note pixel(0,0) is the same size as all other pixels
		this->m_pMapActor->SetSiteMap(model.GetSiteMap());
		this->m_pMapActor->SetPickable(0);
		this->GetPropCollection()->AddItem(this->m_pMapActor);
	}

	// reset tree control
	//
	if (CPropertyTreeControlBar* pTree = this->GetPropertyTreeControlBar())
	{
		pTree->GetGridNode().Expand(TVE_COLLAPSE);
		pTree->GetMediaNode().Expand(TVE_COLLAPSE);
		pTree->GetICNode().Expand(TVE_COLLAPSE);
		pTree->GetBCNode().Expand(TVE_COLLAPSE);
		pTree->GetWellsNode().Expand(TVE_COLLAPSE);
		pTree->GetRiversNode().Expand(TVE_COLLAPSE);
		pTree->ClearSelection();
	}

	// refresh screen
	//
	this->ResetCamera();
	this->UpdateAllViews(0);
}

void CWPhastDoc::OnFileRun()
{
	CWaitCursor wait;
	// this is required to get the prefix
	//
	if (this->GetPathName().IsEmpty())
	{
		if (!this->DoSave(NULL))
		{
			return;
		}
	}

	std::ostringstream oss;
	this->WriteTransDat(oss);

	std::string str = oss.str();
	std::istringstream iss(str);

	TCHAR szDrive[_MAX_DRIVE];
	TCHAR szDir[_MAX_DIR];
	TCHAR szFName[_MAX_FNAME];
	TCHAR szExt[_MAX_EXT];

	ASSERT(!this->GetPathName().IsEmpty());
	::_tsplitpath(this->GetPathName(), szDrive, szDir, szFName, szExt);

	TCHAR szPhastTmp[_MAX_PATH];
	::_tmakepath(szPhastTmp, szDrive, szDir, "Phast", ".tmp");

	CString strPrefix(szFName);


	CPhastInput* pInput = CPhastInput::New(iss, strPrefix);
	if (!pInput) return;

	// Update StatusBar
	//
	if (CWnd* pWnd = ((CFrameWnd*)::AfxGetMainWnd())->GetMessageBar())
	{
		CString status(_T("Writing Phast.tmp..."));
		pWnd->SetWindowText(status);
	}

	bool bWritePhastTmpOK = false;
	try
	{
		pInput->WritePhastTmp(szPhastTmp);
		if (pInput->GetErrorCount() != 0)
		{
			// goto ImportError;
			// throw "Input file contains errors.";
			throw pInput->GetErrorCount();
		}
		bWritePhastTmpOK = true;
	}
	catch (int)
	{
		CImportErrorDialog dlg;
		dlg.m_lpszErrorMessages = pInput->GetErrorMsg();
		dlg.DoModal();
	}
	catch (const char * error)
	{
		::AfxMessageBox(error, MB_OK|MB_ICONEXCLAMATION);
	}
	catch (...)
	{
		::AfxMessageBox("An unknown error occured during the run.", MB_OK|MB_ICONEXCLAMATION);
	}
	pInput->Delete();

	if (bWritePhastTmpOK)
	{
		wait.Restore();

		TCHAR szPhastTmpDir[_MAX_PATH];
		::_tmakepath(szPhastTmpDir, szDrive, szDir, "", "");

		// Update StatusBar
		//
		if (CWnd* pWnd = ((CFrameWnd*)::AfxGetMainWnd())->GetMessageBar())
		{
			CString status(_T("Running..."));
			pWnd->SetWindowText(status);
		}

		CRunDialog run;
		run.SetWorkingDirectory(szPhastTmpDir);
		run.DoModal();
	}
}

// COMMENT: {4/8/2005 6:51:09 PM}CTreeCtrlNode CWPhastDoc::AddStressPeriod(const CTimeControl& timeControl)
// COMMENT: {4/8/2005 6:51:09 PM}{
// COMMENT: {4/8/2005 6:51:09 PM}    CPropertyTreeControlBar* pTree = this->GetPropertyTreeControlBar();
// COMMENT: {4/8/2005 6:51:09 PM}	ASSERT(pTree);
// COMMENT: {4/8/2005 6:51:09 PM}	CTreeCtrlNode node = pTree->AddStressPeriod(timeControl);
// COMMENT: {4/8/2005 6:51:09 PM}	return node;
// COMMENT: {4/8/2005 6:51:09 PM}}

// COMMENT: {4/8/2005 6:51:14 PM}void CWPhastDoc::RemoveStressPeriod(int nStressPeriod)
// COMMENT: {4/8/2005 6:51:14 PM}{
// COMMENT: {4/8/2005 6:51:14 PM}    CPropertyTreeControlBar* pTree = this->GetPropertyTreeControlBar();
// COMMENT: {4/8/2005 6:51:14 PM}	ASSERT(pTree);
// COMMENT: {4/8/2005 6:51:14 PM}	pTree->RemoveStressPeriod(nStressPeriod);
// COMMENT: {4/8/2005 6:51:14 PM}}

// COMMENT: {4/8/2005 6:51:20 PM}int CWPhastDoc::GetStressPeriodCount(void)const
// COMMENT: {4/8/2005 6:51:20 PM}{
// COMMENT: {4/8/2005 6:51:20 PM}    CPropertyTreeControlBar* pTree = this->GetPropertyTreeControlBar();
// COMMENT: {4/8/2005 6:51:20 PM}	ASSERT(pTree);
// COMMENT: {4/8/2005 6:51:20 PM}	return pTree->GetStressPeriodCount();
// COMMENT: {4/8/2005 6:51:20 PM}}

// COMMENT: {4/11/2005 1:22:32 PM}void CWPhastDoc::SetTimeControl(const CTimeControl& timeControl, int nStressPeriod)
// COMMENT: {4/11/2005 1:22:32 PM}{
// COMMENT: {4/11/2005 1:22:32 PM}	if (nStressPeriod == 1) {
// COMMENT: {4/11/2005 1:22:32 PM}		(*this->m_pTimeControl) = timeControl;
// COMMENT: {4/11/2005 1:22:32 PM}
// COMMENT: {4/11/2005 1:22:32 PM}		// update properties bar
// COMMENT: {4/11/2005 1:22:32 PM}		//
// COMMENT: {4/11/2005 1:22:32 PM}		if (CPropertyTreeControlBar* pTree = this->GetPropertyTreeControlBar()) {
// COMMENT: {4/11/2005 1:22:32 PM}			pTree->SetTimeControl(this->m_pTimeControl);
// COMMENT: {4/11/2005 1:22:32 PM}		}
// COMMENT: {4/11/2005 1:22:32 PM}	}
// COMMENT: {4/11/2005 1:22:32 PM}	else {
// COMMENT: {4/11/2005 1:22:32 PM}		CTimeControl copy(timeControl);
// COMMENT: {4/11/2005 1:22:32 PM}		if (copy.GetTimeEndInput() == 0) {
// COMMENT: {4/11/2005 1:22:32 PM}			copy.SetTimeEndInput(this->GetUnits().time.c_str());
// COMMENT: {4/11/2005 1:22:32 PM}		}
// COMMENT: {4/11/2005 1:22:32 PM}		if (copy.GetTimeStepInput() == 0) {
// COMMENT: {4/11/2005 1:22:32 PM}			copy.SetTimeStepInput(this->GetUnits().time.c_str());
// COMMENT: {4/11/2005 1:22:32 PM}		}
// COMMENT: {4/11/2005 1:22:32 PM}
// COMMENT: {4/11/2005 1:22:32 PM}		// update properties bar
// COMMENT: {4/11/2005 1:22:32 PM}		//
// COMMENT: {4/11/2005 1:22:32 PM}		if (CPropertyTreeControlBar* pTree = this->GetPropertyTreeControlBar()) {
// COMMENT: {4/11/2005 1:22:32 PM}			pTree->SetTimeControl(copy, nStressPeriod);
// COMMENT: {4/11/2005 1:22:32 PM}		}
// COMMENT: {4/11/2005 1:22:32 PM}	}
// COMMENT: {4/11/2005 1:22:32 PM}}

void CWPhastDoc::SetTimeControl2(const CTimeControl2& timeControl2)
{
	ASSERT(this->m_pModel);
	this->m_pModel->m_timeControl2 = timeControl2;

	// update properties bar
	//
	if (CPropertyTreeControlBar* pTree = this->GetPropertyTreeControlBar())
	{
		pTree->SetTimeControl2(&this->m_pModel->m_timeControl2);
	}
}

const CTimeControl2& CWPhastDoc::GetTimeControl2(void)const
{
	return this->m_pModel->m_timeControl2;
}

void CWPhastDoc::SetPrintFrequency(const CPrintFreq& printFreq)
{
	this->m_pModel->m_printFreq = printFreq;

	// update properties bar
	//
	if (CPropertyTreeControlBar* pTree = this->GetPropertyTreeControlBar())
	{
		pTree->SetPrintFrequency(&this->m_pModel->m_printFreq);
	}
}

const CPrintFreq& CWPhastDoc::GetPrintFrequency(void)const
{
	return this->m_pModel->m_printFreq;
}

void CWPhastDoc::SetPrintInput(const CPrintInput& printInput)
{
	this->m_pModel->m_printInput = printInput;

	// update properties bar
	//
	if (CPropertyTreeControlBar* pTree = this->GetPropertyTreeControlBar())
	{
		pTree->SetPrintInput(&this->m_pModel->m_printInput);
	}
}

const CPrintInput& CWPhastDoc::GetPrintInput(void)const
{
	return this->m_pModel->m_printInput;
}

void CWPhastDoc::OnToolsNewStressPeriod(void)
{
// COMMENT: {4/8/2005 6:57:20 PM}	if (CPropertyTreeControlBar* pTree = this->GetPropertyTreeControlBar()) {
// COMMENT: {4/8/2005 6:57:20 PM}		CPropertySheet props(_T("New Simulation Period"));
// COMMENT: {4/8/2005 6:57:20 PM}		CTimeControlPropertyPage tcPage;
// COMMENT: {4/8/2005 6:57:20 PM}		tcPage.SetPreviousTimeStep(*pTree->GetTimeControl(pTree->GetStressPeriodCount()));
// COMMENT: {4/8/2005 6:57:20 PM}		props.AddPage(&tcPage);
// COMMENT: {4/8/2005 6:57:20 PM}
// COMMENT: {4/8/2005 6:57:20 PM}		if (props.DoModal() == IDOK) {
// COMMENT: {4/8/2005 6:57:20 PM}			CTimeControl tc;
// COMMENT: {4/8/2005 6:57:20 PM}			tcPage.GetProperties(tc);
// COMMENT: {4/8/2005 6:57:20 PM}			this->Execute(new CAddStressPeriodAction(this, tc));
// COMMENT: {4/8/2005 6:57:20 PM}		}
// COMMENT: {4/8/2005 6:57:20 PM}	}
}

void CWPhastDoc::ReleaseGraphicsResources(vtkProp* pProp)
{
	// foreach view
	//
	POSITION pos = this->GetFirstViewPosition();
	while (pos != NULL)
	{
		CView *pView = this->GetNextView(pos);
		if (CWPhastView *pWPhastView = static_cast<CWPhastView*>(pView))
		{
			ASSERT_KINDOF(CWPhastView, pWPhastView);
			ASSERT_VALID(pWPhastView);
			pProp->ReleaseGraphicsResources(pWPhastView->GetRenderer()->GetVTKWindow());
		}
	}
}

void CWPhastDoc::OnUpdateViewAxes(CCmdUI *pCmdUI)
{
	if (this->m_pAxesActor->GetVisibility())
	{
		pCmdUI->SetCheck(1);
	}
	else
	{
		pCmdUI->SetCheck(0);
	}
}

void CWPhastDoc::OnViewAxes()
{
	if (this->m_pAxesActor->GetVisibility())
	{
		this->m_pAxesActor->SetVisibility(0);
	}
	else
	{
		this->m_pAxesActor->SetVisibility(1);
	}
	this->UpdateAllViews(0);
}

void CWPhastDoc::OnUpdateSetprojectiontoParallel(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(this->m_ProjectionMode == PT_PARALLEL);
}

void CWPhastDoc::OnSetprojectiontoParallel()
{
	this->m_ProjectionMode = PT_PARALLEL;
	// update views
	//
	POSITION pos = this->GetFirstViewPosition();
	while (pos != NULL) {
		CWPhastView *pView = (CWPhastView*) GetNextView(pos);
		ASSERT_VALID(pView);
		pView->ParallelProjectionOn();
	}
	UpdateAllViews(NULL);
}

void CWPhastDoc::OnUpdateSetprojectiontoPerspective(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(this->m_ProjectionMode == PT_PERSPECTIVE);
}

void CWPhastDoc::OnSetprojectiontoPerspective()
{
	this->m_ProjectionMode = PT_PERSPECTIVE;

	// update views
	//
	POSITION pos = this->GetFirstViewPosition();
	while (pos != NULL) {
		CWPhastView *pView = (CWPhastView*) GetNextView(pos);
		ASSERT_VALID(pView);
		pView->ParallelProjectionOff();
	}
	UpdateAllViews(NULL);
}

void CWPhastDoc::OnUpdateViewSitemap(CCmdUI *pCmdUI)
{
	if (this->m_pMapActor == NULL)
	{
		pCmdUI->Enable(FALSE);
		return;
	}

	if (this->m_pMapActor->GetVisibility())
	{
		pCmdUI->SetCheck(1);
	}
	else
	{
		pCmdUI->SetCheck(0);
	}
}

void CWPhastDoc::OnViewSitemap()
{
	if (this->m_pMapActor->GetVisibility())
	{
		this->m_pMapActor->SetVisibility(0);
	}
	else
	{
		this->m_pMapActor->SetVisibility(1);
	}
	this->UpdateAllViews(0);
}

void CWPhastDoc::OnUpdateViewGrid(CCmdUI *pCmdUI)
{
	if (this->m_pGridActor->GetVisibility())
	{
		pCmdUI->SetCheck(1);
	}
	else
	{
		pCmdUI->SetCheck(0);
	}
}

void CWPhastDoc::OnViewGrid()
{
	CPropertyTreeControlBar *pTree = this->GetPropertyTreeControlBar();

	if (this->m_pGridActor->GetVisibility())
	{
		this->m_pGridActor->SetVisibility(0);
		// if (pTree) pTree->SetNodeCheck(pTree->GetGridNode(), BST_UNCHECKED);
		if (pTree) pTree->GetGridNode().SetState(INDEXTOSTATEIMAGEMASK(BST_UNCHECKED + 1), TVIS_STATEIMAGEMASK);

	}
	else
	{
		this->m_pGridActor->SetVisibility(1);
		// if (pTree) pTree->SetNodeCheck(pTree->GetGridNode(), BST_CHECKED);
		if (pTree) pTree->GetGridNode().SetState(INDEXTOSTATEIMAGEMASK(BST_CHECKED + 1), TVIS_STATEIMAGEMASK);
	}
	this->UpdateAllViews(0);
}

void CWPhastDoc::OnUpdateMediaZonesHideAll(CCmdUI *pCmdUI)
{
	if (CPropertyTreeControlBar *pTree = this->GetPropertyTreeControlBar())
	{
		switch (pTree->GetMediaCheck())
		{
		case BST_UNCHECKED:
			// currently unchecked
			pCmdUI->SetRadio(TRUE);
			break;
		case BST_CHECKED:
			// currently checked
			pCmdUI->SetRadio(FALSE);
			break;
		default:
			ASSERT(FALSE);
			pCmdUI->Enable(FALSE);
			break;
		}
	}
}

void CWPhastDoc::OnMediaZonesHideAll()
{
	if (CPropertyTreeControlBar *pTree = this->GetPropertyTreeControlBar())
	{	
		pTree->SetMediaCheck(BST_UNCHECKED);
	}
	this->UpdateAllViews(0);
}

void CWPhastDoc::OnUpdateMediaZonesShowSelected(CCmdUI *pCmdUI)
{
	if (CPropertyTreeControlBar *pTree = this->GetPropertyTreeControlBar())
	{	
		switch (pTree->GetMediaCheck())
		{
		case BST_UNCHECKED:
			// currently unchecked
			pCmdUI->SetRadio(FALSE);
			break;
		case BST_CHECKED:
			// currently checked
			pCmdUI->SetRadio(TRUE);
			break;
		default:
			ASSERT(FALSE);
			pCmdUI->Enable(FALSE);
			break;
		}
	}
}

void CWPhastDoc::OnMediaZonesShowSelected()
{
	if (CPropertyTreeControlBar *pTree = this->GetPropertyTreeControlBar())
	{	
		pTree->SetMediaCheck(BST_CHECKED);
	}
	this->UpdateAllViews(0);
}

void CWPhastDoc::OnMediaZonesSelectAll()
{
	if (CPropertyTreeControlBar* pTree = this->GetPropertyTreeControlBar())
	{
		CTreeCtrlNode nodeMedia = pTree->GetMediaNode();
		int nCount = nodeMedia.GetChildCount();
		for (int i = 0; i < nCount; ++i)
		{
			pTree->SetNodeCheck(nodeMedia.GetChildAt(i), BST_CHECKED);
		}
	}
	this->UpdateAllViews(0);
}

void CWPhastDoc::OnMediaZonesUnselectAll()
{
	if (CPropertyTreeControlBar* pTree = this->GetPropertyTreeControlBar())
	{
		CTreeCtrlNode nodeMedia = pTree->GetMediaNode();
		int nCount = nodeMedia.GetChildCount();
		for (int i = 0; i < nCount; ++i)
		{
			pTree->SetNodeCheck(nodeMedia.GetChildAt(i), BST_UNCHECKED);
		}
	}
	this->UpdateAllViews(0);
}

void CWPhastDoc::OnUpdateICZonesHideAll(CCmdUI *pCmdUI)
{
	if (CPropertyTreeControlBar *pTree = this->GetPropertyTreeControlBar())
	{
		switch (pTree->GetICCheck())
		{
		case BST_UNCHECKED:
			// currently unchecked
			pCmdUI->SetRadio(TRUE);
			break;
		case BST_CHECKED:
			// currently checked
			pCmdUI->SetRadio(FALSE);
			break;
		default:
			ASSERT(FALSE);
			pCmdUI->Enable(FALSE);
			break;
		}
	}
}

void CWPhastDoc::OnICZonesHideAll()
{
	if (CPropertyTreeControlBar *pTree = this->GetPropertyTreeControlBar())
	{	
		pTree->SetICCheck(BST_UNCHECKED);
	}
	this->UpdateAllViews(0);
}

void CWPhastDoc::OnUpdateBCZonesHideAll(CCmdUI *pCmdUI)
{
	if (CPropertyTreeControlBar *pTree = this->GetPropertyTreeControlBar())
	{
		switch (pTree->GetBCCheck())
		{
		case BST_UNCHECKED:
			// currently unchecked
			pCmdUI->SetRadio(TRUE);
			break;
		case BST_CHECKED:
			// currently checked
			pCmdUI->SetRadio(FALSE);
			break;
		default:
			ASSERT(FALSE);
			pCmdUI->Enable(FALSE);
			break;
		}
	}
}

void CWPhastDoc::OnBCZonesHideAll()
{
	if (CPropertyTreeControlBar *pTree = this->GetPropertyTreeControlBar())
	{	
		pTree->SetBCCheck(BST_UNCHECKED);
	}
	this->UpdateAllViews(0);
}

void CWPhastDoc::OnUpdateICZonesShowSelected(CCmdUI *pCmdUI)
{
	if (CPropertyTreeControlBar *pTree = this->GetPropertyTreeControlBar())
	{	
		switch (pTree->GetICCheck())
		{
		case BST_UNCHECKED:
			// currently unchecked
			pCmdUI->SetRadio(FALSE);
			break;
		case BST_CHECKED:
			// currently checked
			pCmdUI->SetRadio(TRUE);
			break;
		default:
			ASSERT(FALSE);
			pCmdUI->Enable(FALSE);
			break;
		}
	}
}

void CWPhastDoc::OnICZonesShowSelected()
{
	if (CPropertyTreeControlBar *pTree = this->GetPropertyTreeControlBar())
	{	
		pTree->SetICCheck(BST_CHECKED);
	}
	this->UpdateAllViews(0);
}

void CWPhastDoc::OnUpdateBCZonesShowSelected(CCmdUI *pCmdUI)
{
	if (CPropertyTreeControlBar *pTree = this->GetPropertyTreeControlBar())
	{	
		switch (pTree->GetBCCheck())
		{
		case BST_UNCHECKED:
			// currently unchecked
			pCmdUI->SetRadio(FALSE);
			break;
		case BST_CHECKED:
			// currently checked
			pCmdUI->SetRadio(TRUE);
			break;
		default:
			ASSERT(FALSE);
			pCmdUI->Enable(FALSE);
			break;
		}
	}
}

void CWPhastDoc::OnBCZonesShowSelected()
{
	if (CPropertyTreeControlBar *pTree = this->GetPropertyTreeControlBar())
	{	
		pTree->SetBCCheck(BST_CHECKED);
	}
	this->UpdateAllViews(0);
}

void CWPhastDoc::OnICZonesSelectAll()
{
	if (CPropertyTreeControlBar* pTree = this->GetPropertyTreeControlBar())
	{
		int nCount;

		// head
		CTreeCtrlNode nodeICHead = pTree->GetICHeadNode();
		nCount = nodeICHead.GetChildCount();
		for (int i = 0; i < nCount; ++i)
		{
			pTree->SetNodeCheck(nodeICHead.GetChildAt(i), BST_CHECKED);
		}

		// chem
		CTreeCtrlNode nodeICChem = pTree->GetICChemNode();
		nCount = nodeICChem.GetChildCount();
		for (int i = 0; i < nCount; ++i)
		{
			pTree->SetNodeCheck(nodeICChem.GetChildAt(i), BST_CHECKED);
		}
	}
	this->UpdateAllViews(0);
}

void CWPhastDoc::OnICZonesUnselectAll()
{
	if (CPropertyTreeControlBar* pTree = this->GetPropertyTreeControlBar())
	{
		int nCount;

		// head
		CTreeCtrlNode nodeICHead = pTree->GetICHeadNode();
		nCount = nodeICHead.GetChildCount();
		for (int i = 0; i < nCount; ++i)
		{
			pTree->SetNodeCheck(nodeICHead.GetChildAt(i), BST_UNCHECKED);
		}

		// chem
		CTreeCtrlNode nodeICChem = pTree->GetICChemNode();
		nCount = nodeICChem.GetChildCount();
		for (int i = 0; i < nCount; ++i)
		{
			pTree->SetNodeCheck(nodeICChem.GetChildAt(i), BST_UNCHECKED);
		}
	}
	this->UpdateAllViews(0);
}

void CWPhastDoc::OnBCZonesSelectAll()
{
	if (CPropertyTreeControlBar* pTree = this->GetPropertyTreeControlBar())
	{
		CTreeCtrlNode node = pTree->GetBCNode();
		int nCount = node.GetChildCount();
		for (int i = 0; i < nCount; ++i)
		{
			pTree->SetNodeCheck(node.GetChildAt(i), BST_CHECKED);
		}
	}
	this->UpdateAllViews(0);
}

void CWPhastDoc::OnBCZonesUnselectAll()
{
	if (CPropertyTreeControlBar* pTree = this->GetPropertyTreeControlBar())
	{
		CTreeCtrlNode node = pTree->GetBCNode();
		int nCount = node.GetChildCount();
		for (int i = 0; i < nCount; ++i)
		{
			pTree->SetNodeCheck(node.GetChildAt(i), BST_UNCHECKED);
		}
	}
	this->UpdateAllViews(0);
}

void CWPhastDoc::ClearSelection(void)
{
// COMMENT: {9/9/2005 7:30:46 PM}	// foreach view
// COMMENT: {9/9/2005 7:30:46 PM}	//
// COMMENT: {9/9/2005 7:30:46 PM}	POSITION pos = this->GetFirstViewPosition();
// COMMENT: {9/9/2005 7:30:46 PM}	while (pos != NULL)
// COMMENT: {9/9/2005 7:30:46 PM}	{
// COMMENT: {9/9/2005 7:30:46 PM}		CView *pView = this->GetNextView(pos);
// COMMENT: {9/9/2005 7:30:46 PM}		if (CWPhastView *pWPhastView = static_cast<CWPhastView*>(pView))
// COMMENT: {9/9/2005 7:30:46 PM}		{
// COMMENT: {9/9/2005 7:30:46 PM}			ASSERT_KINDOF(CWPhastView, pWPhastView);
// COMMENT: {9/9/2005 7:30:46 PM}			ASSERT_VALID(pWPhastView);
// COMMENT: {9/9/2005 7:30:46 PM}			pWPhastView->ClearSelection();
// COMMENT: {9/9/2005 7:30:46 PM}		}
// COMMENT: {9/9/2005 7:30:46 PM}	}

	// clear selected river points
	//
	vtkPropCollection *pPropCollection = this->GetPropAssemblyRivers()->GetParts();
	pPropCollection->InitTraversal();
	vtkProp* pProp = pPropCollection->GetNextProp();
	for (; pProp; pProp = pPropCollection->GetNextProp())
	{
		if (CRiverActor *pActor = CRiverActor::SafeDownCast(pProp))
		{
			pActor->ClearSelection();
			pActor->SetEnabled(0);
		}
	}

	// Notify
	//
	this->Notify(0, WPN_SELCHANGED, 0, 0);
}

void CWPhastDoc::Add(CZoneActor *pZoneActor, HTREEITEM hInsertAfter)
{
	this->InternalAdd(pZoneActor, true, hInsertAfter);
}

void CWPhastDoc::UnAdd(CZoneActor *pZoneActor)
{
	this->InternalDelete(pZoneActor, false);
}

void CWPhastDoc::Delete(CZoneActor *pZoneActor)
{
	this->InternalDelete(pZoneActor, true);
}

void CWPhastDoc::UnDelete(CZoneActor *pZoneActor)
{
	this->InternalAdd(pZoneActor, false);
}

void CWPhastDoc::InternalAdd(CZoneActor *pZoneActor, bool bAdd, HTREEITEM hInsertAfter)
{
	ASSERT(pZoneActor);
	if (pZoneActor == NULL) return;

	// set scale
	//
	vtkFloatingPointType *scale = this->GetScale();
	pZoneActor->SetScale(scale[0], scale[1], scale[2]);

	// add to document
	//
	pZoneActor->Add(this);

	if (!bAdd)
	{
		// see InternalDelete
		//
		ASSERT(this->m_pRemovedPropCollection);
		ASSERT(this->m_pRemovedPropCollection->IsItemPresent(pZoneActor));
		this->m_pRemovedPropCollection->RemoveItem(pZoneActor);
	}

	// add to property tree
	//
	if (CPropertyTreeControlBar *pPropertyTreeControlBar = this->GetPropertyTreeControlBar())
	{
		if (bAdd)
		{
			pPropertyTreeControlBar->AddZone(pZoneActor, hInsertAfter);
		}
		else
		{
			pZoneActor->UnRemove(pPropertyTreeControlBar);
		}
	}

	// foreach view
	//
	POSITION pos = this->GetFirstViewPosition();
	while (pos != NULL)
	{
		CView *pView = this->GetNextView(pos);
		if (CWPhastView *pWPhastView = static_cast<CWPhastView*>(pView))
		{
			ASSERT_KINDOF(CWPhastView, pWPhastView);
			ASSERT_VALID(pWPhastView);

			// set selection bounding box
			//
			pZoneActor->Select(pWPhastView);

			// Update BoxPropertiesDialogBar
			//
			if (CBoxPropertiesDialogBar* pBar = static_cast<CBoxPropertiesDialogBar*>(  ((CFrameWnd*)::AfxGetMainWnd())->GetControlBar(IDW_CONTROLBAR_BOXPROPS) ) )
			{
				pBar->Set(pWPhastView, pZoneActor, this->GetUnits());
			}
		}
	}

	// render
	//
	this->UpdateAllViews(0);
}

void CWPhastDoc::InternalDelete(CZoneActor *pZoneActor, bool bDelete)
{
	ASSERT(pZoneActor);
	if (pZoneActor == NULL) return;

	if (bDelete) {
		// make sure pZoneActor ref count doesn't go to zero
		//
		ASSERT(this->m_pRemovedPropCollection);
		ASSERT(!this->m_pRemovedPropCollection->IsItemPresent(pZoneActor));
		this->m_pRemovedPropCollection->AddItem(pZoneActor);
	}

	// remove from document
	//
	pZoneActor->Remove(this);

	// foreach view
	//
	POSITION pos = this->GetFirstViewPosition();
	while (pos != NULL)
	{
		CView *pView = this->GetNextView(pos);
		if (CWPhastView *pWPhastView = static_cast<CWPhastView*>(pView))
		{
			ASSERT_KINDOF(CWPhastView, pWPhastView);
			ASSERT_VALID(pWPhastView);

			// clear selection bounding box
			//
			pWPhastView->ClearSelection();

			// Update BoxPropertiesDialogBar
			//
			if (CBoxPropertiesDialogBar* pBar = static_cast<CBoxPropertiesDialogBar*>(  ((CFrameWnd*)::AfxGetMainWnd())->GetControlBar(IDW_CONTROLBAR_BOXPROPS) ) ) {
				pBar->Set(pWPhastView, NULL, this->GetUnits());
			}
		}
	}

	// remove from property tree
	//
	if (CPropertyTreeControlBar* pTree = this->GetPropertyTreeControlBar()) {
		pTree->RemoveZone(pZoneActor);
	}

	// render
	//
	this->UpdateAllViews(0);
}

void CWPhastDoc::Add(CWellActor *pWellActor, HTREEITEM hInsertAfter)
{
	ASSERT(pWellActor);
	if (!pWellActor) return;
	ASSERT(pWellActor->GetPickable());

	// set scale
	//
	vtkFloatingPointType *scale = this->GetScale();
	pWellActor->SetScale(scale[0], scale[1], scale[2]);

	// set height
	//
	CGrid xyz[3];
	this->GetGrid(xyz[0], xyz[1], xyz[2]);
	pWellActor->SetZAxis(xyz[2], this->GetUnits());

	// set radius
	//
	vtkFloatingPointType *bounds = this->GetGridBounds();
	float defaultAxesSize = (bounds[1]-bounds[0] + bounds[3]-bounds[2] + bounds[5]-bounds[4])/12;
	pWellActor->SetDefaultTubeDiameter(defaultAxesSize * 0.17 / sqrt(scale[0] * scale[1]));

	// add to well assembly
	//
	if (vtkPropAssembly *pPropAssembly = this->GetPropAssemblyWells())
	{
		ASSERT(!pPropAssembly->GetParts()->IsItemPresent(pWellActor));
		pPropAssembly->AddPart(pWellActor);
		if (!this->GetPropCollection()->IsItemPresent(pPropAssembly))
		{
			this->GetPropCollection()->AddItem(pPropAssembly);
		}
	}

	// add to property tree
	//
	if (CPropertyTreeControlBar *pTree = this->GetPropertyTreeControlBar())
	{
		pWellActor->Add(pTree, hInsertAfter);
	}

	// render
	//
	this->UpdateAllViews(0);
}

void CWPhastDoc::UnAdd(CWellActor *pWellActor)
{
	ASSERT(pWellActor);
	if (!pWellActor) return;

	// remove from wells assembly
	//
	if (vtkPropAssembly *pPropAssembly = this->GetPropAssemblyWells())
	{
		ASSERT(pPropAssembly->GetParts()->IsItemPresent(pWellActor));
		pPropAssembly->RemovePart(pWellActor);
		// VTK HACK
		// This is req'd because ReleaseGraphicsResources is not called when
		// vtkPropAssembly::RemovePart(vtkProp *prop) is called
		this->ReleaseGraphicsResources(pWellActor);
	}

	// remove from property tree
	//
	if (CPropertyTreeControlBar *pTree = this->GetPropertyTreeControlBar())
	{
		pWellActor->UnAdd(pTree);
	}

	// render
	//
	this->UpdateAllViews(0);
}

void CWPhastDoc::Remove(CWellActor *pWellActor)
{
	ASSERT(pWellActor && pWellActor->IsA("CWellActor"));
	if (pWellActor == 0) return;

	// validate actor
	//
	vtkFloatingPointType *scale = this->GetScale();
	vtkFloatingPointType *wellscale = pWellActor->GetScale();
	ASSERT(wellscale[0] == scale[0]);
	ASSERT(wellscale[1] == scale[1]);
	ASSERT(wellscale[2] == scale[2]);

	// make sure pWellActor ref count doesn't go to zero
	//
	ASSERT(this->m_pRemovedPropCollection);
	ASSERT(!this->m_pRemovedPropCollection->IsItemPresent(pWellActor));
	this->m_pRemovedPropCollection->AddItem(pWellActor);

	// remove from wells assembly
	//
	if (vtkPropAssembly *pPropAssembly = this->GetPropAssemblyWells())
	{
		ASSERT(pPropAssembly->GetParts()->IsItemPresent(pWellActor));
		pPropAssembly->RemovePart(pWellActor);
		// VTK HACK
		// This is req'd because ReleaseGraphicsResources is not called when
		// vtkPropAssembly::RemovePart(vtkProp *prop) is called
		this->ReleaseGraphicsResources(pWellActor);
	}

	// remove from property tree
	//
	if (CPropertyTreeControlBar *pTree = this->GetPropertyTreeControlBar())
	{
		pWellActor->Remove(pTree);
	}

	// render scene
	//
	this->UpdateAllViews(0);
}

void CWPhastDoc::UnRemove(CWellActor *pWellActor)
{
	ASSERT(pWellActor && pWellActor->IsA("CWellActor"));
	if (pWellActor == 0) return;

	// validate pWellActor
	//
	ASSERT(pWellActor->GetReferenceCount() > 0);

	// verify scale
	//
	vtkFloatingPointType *scale = this->GetScale();
	vtkFloatingPointType *wellscale = pWellActor->GetScale();
	ASSERT(wellscale[0] == scale[0]);
	ASSERT(wellscale[1] == scale[1]);
	ASSERT(wellscale[2] == scale[2]);

	// add to wells assembly
	//
	if (vtkPropAssembly *pPropAssembly = this->GetPropAssemblyWells())
	{
		ASSERT(!pPropAssembly->GetParts()->IsItemPresent(pWellActor));
		pPropAssembly->AddPart(pWellActor);
		if (!this->GetPropCollection()->IsItemPresent(pPropAssembly))
		{
			this->GetPropCollection()->AddItem(pPropAssembly);
			ASSERT(pWellActor->GetReferenceCount() > 1);
		}
	}

	// see CWPhastDoc::Delete
	//
	ASSERT(this->m_pRemovedPropCollection);
	ASSERT(this->m_pRemovedPropCollection->IsItemPresent(pWellActor));
	this->m_pRemovedPropCollection->RemoveItem(pWellActor);
	ASSERT(pWellActor->GetReferenceCount() > 0);


	// add to property tree
	//
	if (CPropertyTreeControlBar *pTree = this->GetPropertyTreeControlBar())
	{
		pWellActor->UnRemove(pTree);
	}

	// render scene
	//
	this->UpdateAllViews(0);
}

void CWPhastDoc::Select(CZoneActor *pZoneActor)
{	
	ASSERT(pZoneActor && pZoneActor->IsA("CZoneActor"));
	this->Notify(0, WPN_SELCHANGED, 0, pZoneActor);
}

void CWPhastDoc::Select(CWellActor *pWellActor)
{	
	ASSERT(pWellActor && pWellActor->IsA("CWellActor"));
	this->Notify(0, WPN_SELCHANGED, 0, pWellActor);
}

void CWPhastDoc::Select(CRiverActor *pRiverActor)
{
	ASSERT(pRiverActor && pRiverActor->IsA("CRiverActor"));
	this->Notify(0, WPN_SELCHANGED, 0, pRiverActor);
}

void CWPhastDoc::GetGrid(CGrid& x, CGrid& y, CGrid& z)const
{
	this->m_pGridActor->GetGrid(x, y, z);
}

void CWPhastDoc::OnUpdateWellsHideAll(CCmdUI *pCmdUI)
{
	if (vtkPropAssembly *pPropAssembly = this->GetPropAssemblyWells())
	{
		if (pPropAssembly->GetVisibility())
		{
			pCmdUI->SetRadio(FALSE);
		}
		else
		{
			pCmdUI->SetRadio(TRUE);
		}
	}
}

void CWPhastDoc::OnWellsHideAll()
{
	if (CPropertyTreeControlBar *pTree = this->GetPropertyTreeControlBar())
	{	
		pTree->SetNodeCheck(pTree->GetWellsNode(), BST_UNCHECKED);
	}
	this->UpdateAllViews(0);
}

void CWPhastDoc::OnUpdateWellsShowSelected(CCmdUI *pCmdUI)
{
	if (CPropertyTreeControlBar *pTree = this->GetPropertyTreeControlBar())
	{	
		switch (pTree->GetNodeCheck(pTree->GetWellsNode()))
		{
		case BST_UNCHECKED:
			// currently unchecked
			pCmdUI->SetRadio(FALSE);
			break;
		case BST_CHECKED:
			// currently checked
			pCmdUI->SetRadio(TRUE);
			break;
		default:
			ASSERT(FALSE);
			pCmdUI->Enable(FALSE);
			break;
		}
	}
}

void CWPhastDoc::OnWellsShowSelected()
{
	if (CPropertyTreeControlBar *pTree = this->GetPropertyTreeControlBar())
	{	
		pTree->SetNodeCheck(pTree->GetWellsNode(), BST_CHECKED);
	}
	this->UpdateAllViews(0);
}

void CWPhastDoc::OnWellsSelectAll()
{
	if (CPropertyTreeControlBar* pTree = this->GetPropertyTreeControlBar())
	{
		CTreeCtrlNode node = pTree->GetWellsNode();
		int nCount = node.GetChildCount();
		for (int i = 0; i < nCount; ++i)
		{
			pTree->SetNodeCheck(node.GetChildAt(i), BST_CHECKED);
		}
	}
	this->UpdateAllViews(0);
}

void CWPhastDoc::OnWellsUnselectAll()
{
	if (CPropertyTreeControlBar* pTree = this->GetPropertyTreeControlBar())
	{
		CTreeCtrlNode node = pTree->GetWellsNode();
		int nCount = node.GetChildCount();
		for (int i = 0; i < nCount; ++i)
		{
			pTree->SetNodeCheck(node.GetChildAt(i), BST_UNCHECKED);
		}
	}
	this->UpdateAllViews(0);
}

VARIANT CWPhastDoc::GetVisible(void)
{
	AFX_MANAGE_STATE(AfxGetAppModuleState());

	VARIANT vaResult;
	VariantInit(&vaResult);
	vaResult.vt = VT_BOOL;

	if (::IsWindowVisible(::AfxGetApp()->m_pMainWnd->GetSafeHwnd()))
	{
		vaResult.boolVal = VARIANT_TRUE;		
	}
	else
	{
		vaResult.boolVal = VARIANT_FALSE;
	}
	return vaResult;
}

//void CWPhastDoc::SetVis(VARIANT newVal)
void CWPhastDoc::SetVisible(const VARIANT& newVal)
{
	AFX_MANAGE_STATE(AfxGetAppModuleState());

	// TODO: Add your property handler code here
#if defined(_DEBUG)
	afxDump << (COleVariant)newVal;
	ASSERT(newVal.vt == VT_BOOL);
#endif

	if (newVal.boolVal)
	{
		::AfxGetMainWnd()->ShowWindow(SW_SHOW);
	}
	else
	{
		::AfxGetMainWnd()->ShowWindow(SW_HIDE);
	}
}

VARIANT CWPhastDoc::SaveAs(const VARIANT& FileName)
{
	AFX_MANAGE_STATE(AfxGetAppModuleState());

	VARIANT vaResult;
	VariantInit(&vaResult);
	vaResult.vt      = VT_BOOL;
	vaResult.boolVal = VARIANT_FALSE;

#if defined(_DEBUG)
	afxDump << (COleVariant)FileName;
	ASSERT(FileName.vt == VT_BSTR);
#endif

	CString name(FileName.bstrVal);
	if (this->OnSaveDocument(name))
	{
		vaResult.boolVal = VARIANT_TRUE;		
	}

	// TODO: Add your dispatch handler code here

	return vaResult;
}

VARIANT CWPhastDoc::Import(const VARIANT& FileName)
{
	AFX_MANAGE_STATE(AfxGetAppModuleState());

	VARIANT vaResult;
	VariantInit(&vaResult);
	vaResult.vt      = VT_BOOL;
	vaResult.boolVal = VARIANT_FALSE;

#if defined(_DEBUG)
	afxDump << (COleVariant)FileName;
	ASSERT(FileName.vt == VT_BSTR);
#endif

	CString newName(FileName.bstrVal);

	// modified from: CDocument* CDocManager::OpenDocumentFile
	//
	TCHAR szPath[_MAX_PATH];
	ASSERT(lstrlen(newName) < sizeof(szPath)/sizeof(szPath[0]));
	TCHAR szTemp[_MAX_PATH];
	if (newName[0] == '\"')
		newName = newName.Mid(1);
	lstrcpyn(szTemp, newName, _MAX_PATH);
	LPTSTR lpszLast = _tcsrchr(szTemp, '\"');
	if (lpszLast != NULL)
		*lpszLast = 0;
	SrcFullPath(szPath, szTemp);
	TCHAR szLinkName[_MAX_PATH];
	if (SrcResolveShortcut(AfxGetMainWnd(), szPath, szLinkName, _MAX_PATH))
		lstrcpy(szPath, szLinkName);

	//
	// avoid SaveAs Dialog
	this->SetModifiedFlag(FALSE);

	CDocument* pDoc = this->GetDocTemplate()->OpenDocumentFile(NULL, FALSE);
	if (pDoc)
	{
		ASSERT(pDoc == this);
		if (this->DoImport(szPath))
		{
			vaResult.boolVal = VARIANT_TRUE;		
			this->SetModifiedFlag(FALSE);
		}
		this->ResetCamera();
	}

	return vaResult;
}

VARIANT CWPhastDoc::Run(void)
{
	AFX_MANAGE_STATE(AfxGetAppModuleState());

	VARIANT vaResult;
	VariantInit(&vaResult);

	///OnFileRun();
	::AfxMessageBox("This method has not been implemented yet");

	return vaResult;
}

void CWPhastDoc::Add(CRiverActor *pRiverActor, HTREEITEM hInsertAfter)
{
	ASSERT(pRiverActor);
	if (!pRiverActor) return;

	// set scale
	//
	vtkFloatingPointType *scale = this->GetScale();
	pRiverActor->SetScale(scale[0], scale[1], scale[2]);

	// set radius
	//
	pRiverActor->ScaleFromBounds(this->GetGridBounds());

	// set z
	//
// COMMENT: {6/22/2005 5:50:09 PM}	vtkFloatingPointType *bounds = this->GetGridBounds();
	CGrid x, y, z;
	this->GetGrid(x, y, z);
	z.Setup();
	pRiverActor->SetZ(z.coord[z.count_coord - 1]);

	// for all views disable interaction
	//
	POSITION pos = this->GetFirstViewPosition();
	while (pos != NULL)
	{
		CWPhastView *pView = (CWPhastView*) GetNextView(pos);
		ASSERT_VALID(pView);
		pRiverActor->SetInteractor(pView->GetRenderWindowInteractor());
		pRiverActor->SetEnabled(0);
	}	

	// add to river assembly
	//
	if (vtkPropAssembly *pPropAssembly = this->GetPropAssemblyRivers())
	{
		ASSERT(!pPropAssembly->GetParts()->IsItemPresent(pRiverActor));
		pPropAssembly->AddPart(pRiverActor);
		if (!this->GetPropCollection()->IsItemPresent(pPropAssembly))
		{
			this->GetPropCollection()->AddItem(pPropAssembly);
		}
	}

	// add to property tree
	//
	if (CPropertyTreeControlBar *pTree = this->GetPropertyTreeControlBar())
	{
		pRiverActor->Add(pTree, hInsertAfter);
	}

	// add listeners
	//
	pRiverActor->AddObserver(CRiverActor::StartMovePointEvent, this->RiverCallbackCommand);
	pRiverActor->AddObserver(CRiverActor::MovingPointEvent,    this->RiverCallbackCommand);
	pRiverActor->AddObserver(CRiverActor::EndMovePointEvent,   this->RiverCallbackCommand);
	pRiverActor->AddObserver(CRiverActor::InsertPointEvent,    this->RiverCallbackCommand);	

	// render
	//
	this->UpdateAllViews(0);
}

void CWPhastDoc::UnAdd(CRiverActor *pRiverActor)
{
	ASSERT(pRiverActor);
	if (!pRiverActor) return;

	// remove from rivers assembly
	//
	if (vtkPropAssembly *pPropAssembly = this->GetPropAssemblyRivers())
	{
		ASSERT(pPropAssembly->GetParts()->IsItemPresent(pRiverActor));
		pPropAssembly->RemovePart(pRiverActor);
		// VTK HACK
		// This is req'd because ReleaseGraphicsResources is not called when
		// vtkPropAssembly::RemovePart(vtkProp *prop) is called
		this->ReleaseGraphicsResources(pRiverActor);
	}

	// remove from property tree
	//
	if (CPropertyTreeControlBar *pTree = this->GetPropertyTreeControlBar())
	{
		pRiverActor->UnAdd(pTree);
	}

	// render
	//
	this->UpdateAllViews(0);
}

void CWPhastDoc::Remove(CRiverActor *pRiverActor)
{
	ASSERT(pRiverActor && pRiverActor->IsA("CRiverActor"));
	if (pRiverActor == 0) return;

	// validate actor
	//
	vtkFloatingPointType *scale = this->GetScale();
	vtkFloatingPointType *riverscale = pRiverActor->GetScale();
// COMMENT: {7/12/2005 3:42:06 PM}	ASSERT(riverscale[0] == scale[0]);
// COMMENT: {7/12/2005 3:42:06 PM}	ASSERT(riverscale[1] == scale[1]);
// COMMENT: {7/12/2005 3:42:06 PM}	ASSERT(riverscale[2] == scale[2]);

	// make sure pWellActor ref count doesn't go to zero
	//
	ASSERT(this->m_pRemovedPropCollection);
	ASSERT(!this->m_pRemovedPropCollection->IsItemPresent(pRiverActor));
	this->m_pRemovedPropCollection->AddItem(pRiverActor);

	// remove from rivers assembly
	//
	if (vtkPropAssembly *pPropAssembly = this->GetPropAssemblyRivers())
	{
		ASSERT(pPropAssembly->GetParts()->IsItemPresent(pRiverActor));
		pPropAssembly->RemovePart(pRiverActor);
		// VTK HACK
		// This is req'd because ReleaseGraphicsResources is not called when
		// vtkPropAssembly::RemovePart(vtkProp *prop) is called
		this->ReleaseGraphicsResources(pRiverActor);
	}

	// remove from property tree
	//
	if (CPropertyTreeControlBar *pTree = this->GetPropertyTreeControlBar())
	{
		pRiverActor->Remove(pTree);
	}

	// render scene
	//
	this->UpdateAllViews(0);
}

void CWPhastDoc::UnRemove(CRiverActor *pRiverActor)
{
	ASSERT(pRiverActor && pRiverActor->IsA("CRiverActor"));
	if (pRiverActor == 0) return;

	// validate pWellActor
	//
	ASSERT(pRiverActor->GetReferenceCount() > 0);

	// verify scale
	//
	vtkFloatingPointType *scale = this->GetScale();
	vtkFloatingPointType *riverscale = pRiverActor->GetScale();
// COMMENT: {7/12/2005 3:41:44 PM}	ASSERT(riverscale[0] == scale[0]);
// COMMENT: {7/12/2005 3:41:44 PM}	ASSERT(riverscale[1] == scale[1]);
// COMMENT: {7/12/2005 3:41:44 PM}	ASSERT(riverscale[2] == scale[2]);

	// add to rivers assembly
	//
	if (vtkPropAssembly *pPropAssembly = this->GetPropAssemblyRivers())
	{
		ASSERT(!pPropAssembly->GetParts()->IsItemPresent(pRiverActor));
		pPropAssembly->AddPart(pRiverActor);
		if (!this->GetPropCollection()->IsItemPresent(pPropAssembly))
		{
			this->GetPropCollection()->AddItem(pPropAssembly);
			ASSERT(pRiverActor->GetReferenceCount() > 1);
		}
	}

	// see CWPhastDoc::Delete
	//
	ASSERT(this->m_pRemovedPropCollection);
	ASSERT(this->m_pRemovedPropCollection->IsItemPresent(pRiverActor));
	this->m_pRemovedPropCollection->RemoveItem(pRiverActor);
	ASSERT(pRiverActor->GetReferenceCount() > 0);


	// add to property tree
	//
	if (CPropertyTreeControlBar *pTree = this->GetPropertyTreeControlBar())
	{
		pRiverActor->UnRemove(pTree);
	}

	// render scene
	//
	this->UpdateAllViews(0);
}


void CWPhastDoc::RiverListener(vtkObject* caller, unsigned long eid, void* clientdata, void *calldata)
{
	ASSERT(caller->IsA("CRiverActor"));
	ASSERT(clientdata);

	if (CRiverActor* river = CRiverActor::SafeDownCast(caller))
	{
		CWPhastDoc* self = reinterpret_cast<CWPhastDoc*>(clientdata);

		switch (eid)
		{
		case CRiverActor::StartMovePointEvent:
			ASSERT(self->RiverMovePointAction == 0);
			self->RiverMovePointAction = new CRiverMovePointAction(river, self, river->GetCurrentPointId(), river->GetCurrentPointPosition()[0], river->GetCurrentPointPosition()[1]);
			break;

		case CRiverActor::MovingPointEvent:
			ASSERT(self->RiverMovePointAction != 0);
			if (CWnd* pWnd = ((CFrameWnd*)::AfxGetMainWnd())->GetMessageBar())
			{
				// update message bar
				//
				static TCHAR buffer[80];
				const CUnits& units = self->GetUnits();
				::_sntprintf(buffer, 80, "%g[%s] x %g[%s]",
					self->RiverMovePointAction->GetRiverActor()->GetCurrentPointPosition()[0] / units.horizontal.input_to_si,
					units.horizontal.defined ? units.horizontal.input : units.horizontal.si,
					self->RiverMovePointAction->GetRiverActor()->GetCurrentPointPosition()[1] / units.horizontal.input_to_si,
					units.horizontal.defined ? units.horizontal.input : units.horizontal.si
					);
				pWnd->SetWindowText(buffer);
			}
			break;

		case CRiverActor::EndMovePointEvent:
			ASSERT(self->RiverMovePointAction != 0);
			self->RiverMovePointAction->SetPoint(river->GetCurrentPointPosition()[0], river->GetCurrentPointPosition()[1]);
			self->Execute(self->RiverMovePointAction);
			self->RiverMovePointAction = 0;
			break;

		case CRiverActor::InsertPointEvent:
			{
				double* pos = river->GetCurrentPointPosition();
				vtkIdType id = river->GetCurrentPointId() - 1;
				CRiverInsertPointAction* pRiverInsertPointAction = new CRiverInsertPointAction(river, id, pos[0], pos[1], pos[2], true);
				self->Execute(pRiverInsertPointAction);
			}
			break;
		}
	}
}

void CWPhastDoc::GetGridKeyword(CGridKeyword& gridKeyword)const
{
	this->m_pGridActor->GetGridKeyword(gridKeyword);
}

void CWPhastDoc::SetGridKeyword(const CGridKeyword& gridKeyword)
{
	this->m_pGridActor->SetGridKeyword(gridKeyword, this->GetUnits());
	this->ResizeGrid(gridKeyword.m_grid[0], gridKeyword.m_grid[1], gridKeyword.m_grid[2]);
}

void CWPhastDoc::Edit(CGridActor* pGridActor)
{
	ASSERT(pGridActor == this->m_pGridActor);
	if (!this->m_pGridSheet)
	{
		this->m_pGridSheet = new CModelessPropertySheet("Grid");
		///this->m_pGridSheet = new ETSLayoutModelessPropertySheet("Time Control", ::AfxGetMainWnd());
		///((ETSLayoutPropertySheet*)this->m_pGridSheet)->ModelessWithButtons();

	}
	if (!this->m_pGridPage)
	{
		this->m_pGridPage  = new CGridPropertyPage2();
		this->m_pGridSheet->AddPage(this->m_pGridPage);
	}
	
	CGridKeyword gridKeyword;
	pGridActor->GetGridKeyword(gridKeyword);
	this->m_pGridPage->SetProperties(gridKeyword);
	this->m_pGridPage->SetUnits(this->GetUnits());

	this->m_pGridPage->m_pDoc = this;
	this->m_pGridPage->m_pActor = pGridActor;

	this->m_pGridSheet->Create(::AfxGetApp()->m_pMainWnd);
}

void CWPhastDoc::ModifyGrid(CGridActor* gridActor, CGridElementsSelector* gridElementsSelector)
{
	ASSERT(gridActor == this->m_pGridActor);

	if (this->ModifyGridSheet)
	{
		this->ModifyGridSheet->DestroyWindow();
		delete this->ModifyGridSheet;
	}
	if (this->GridRefinePage)
	{
		this->GridRefinePage->DestroyWindow();
		delete this->GridRefinePage;
	}
	if (this->GridCoarsenPage)
	{
		this->GridCoarsenPage->DestroyWindow();
		delete this->GridCoarsenPage;
	}

	this->ModifyGridSheet = new CModelessPropertySheet("");
	this->GridRefinePage  = new CGridRefinePage();
	this->GridCoarsenPage = new CGridCoarsenPage();
	this->ModifyGridSheet->AddPage(this->GridRefinePage);
	this->ModifyGridSheet->AddPage(this->GridCoarsenPage);
	
	CGridKeyword gridKeyword;
	gridActor->GetGridKeyword(gridKeyword);

	this->GridRefinePage->SetProperties(gridKeyword);
	this->GridRefinePage->SetUnits(this->GetUnits());
	this->GridRefinePage->SetDocument(this);
	this->GridRefinePage->SetActor(gridActor);
	this->GridRefinePage->SetWidget(gridElementsSelector);

	this->GridCoarsenPage->SetProperties(gridKeyword);
	this->GridCoarsenPage->SetUnits(this->GetUnits());
	this->GridCoarsenPage->SetDocument(this);
	this->GridCoarsenPage->SetActor(gridActor);
	this->GridCoarsenPage->SetWidget(gridElementsSelector);

	int min[3];
	gridElementsSelector->GetMin(min);
	this->GridRefinePage->SetMin(min);
	this->GridCoarsenPage->SetMin(min);

	int max[3];
	gridElementsSelector->GetMax(max);
	this->GridRefinePage->SetMax(max);
	this->GridCoarsenPage->SetMax(max);

	this->ModifyGridSheet->Create(::AfxGetApp()->m_pMainWnd);
}

void CWPhastDoc::Update(IObserver* pSender, LPARAM lHint, CObject* pHint, vtkObject* pObject)
{
}

vtkProp3D* CWPhastDoc::GetGridActor(void)
{
	return this->m_pGridActor;
}

void CWPhastDoc::GridListener(vtkObject* caller, unsigned long eid, void* clientdata, void *calldata)
{
	ASSERT(caller->IsA("CGridActor"));
	ASSERT(clientdata);

	if (CGridActor* grid = CGridActor::SafeDownCast(caller))
	{
		CWPhastDoc* self = reinterpret_cast<CWPhastDoc*>(clientdata);
		switch (eid)
		{
		case CGridActor::DeleteGridLineEvent:
			{
				int axis =  grid->GetAxisIndex();
				int index =  grid->GetPlaneIndex();
				double value = *(double*)calldata;
				CGridDeleteLineAction* pGridDeleteLineAction = new CGridDeleteLineAction(grid, self, axis, index, value, true);
				self->Execute(pGridDeleteLineAction);
			}
			break;
		case CGridActor::InsertGridLineEvent:
			{
				int axis =  grid->GetAxisIndex();
				int index =  grid->GetPlaneIndex();
				double value = *(double*)calldata;
				CGridInsertLineAction* pGridInsertLineAction = new CGridInsertLineAction(grid, self, axis, index, value, true);
				self->Execute(pGridInsertLineAction);
			}
			break;
		case CGridActor::MoveGridLineEvent:
			{
				GridLineMoveMemento memento = *(GridLineMoveMemento*)calldata;
				CGridMoveLineAction* pGridMoveLineAction = new CGridMoveLineAction(grid, self, memento, true);
				self->Execute(pGridMoveLineAction);
			}
			break;
		}
	}
}

void CWPhastDoc::UpdateGridDomain(void)
{
	// get scale
	//
	vtkFloatingPointType *scale = this->GetScale();

	// get bounds
	//
	vtkFloatingPointType bounds[6];
	this->m_pGridActor->GetBounds(bounds);
	float defaultAxesSize = (bounds[1]-bounds[0] + bounds[3]-bounds[2] + bounds[5]-bounds[4])/12;

	// reset the axes
	//
	this->m_pAxesActor->SetDefaultPositions(bounds);
	this->m_pAxesActor->SetDefaultSize(defaultAxesSize);
	this->m_pAxesActor->SetDefaultTubeDiameter(defaultAxesSize * 0.1);

	// update default zones
	//
	if (vtkPropCollection* pCollection = this->GetPropCollection())
	{
		CZone bounds;
		this->m_pGridActor->GetDefaultZone(bounds);
		pCollection->InitTraversal();
		for (int i = 0; i < pCollection->GetNumberOfItems(); ++i)
		{
			vtkProp* prop = pCollection->GetNextProp();
			if (vtkPropAssembly *pPropAssembly = vtkPropAssembly::SafeDownCast(prop))
			{
				if (vtkPropCollection *pPropCollection = pPropAssembly->GetParts())
				{
					pPropCollection->InitTraversal();
					for (int i = 0; i < pPropCollection->GetNumberOfItems(); ++i)
					{
						vtkProp* prop = pPropCollection->GetNextProp();
						if (CZoneActor *pZone = CZoneActor::SafeDownCast(prop))
						{
							if (pZone->GetDefault())
							{
								pZone->SetBounds(bounds, this->GetUnits());
							}
						}
					}
				}
			}
			if (CZoneActor *pZone = CZoneActor::SafeDownCast(prop))
			{
				if (pZone->GetDefault())
				{
					pZone->SetBounds(bounds, this->GetUnits());
				}
			}
		}
	}

	// update rivers
	//
	if (vtkPropAssembly *pPropAssembly = this->GetPropAssemblyRivers())
	{
		if (vtkPropCollection *pPropCollection = pPropAssembly->GetParts())
		{
			vtkProp *pProp;
			pPropCollection->InitTraversal();
			for (; (pProp = pPropCollection->GetNextProp()); )
			{
				if (CRiverActor* pRiverActor = CRiverActor::SafeDownCast(pProp))
				{
					// update radius
					//
					pRiverActor->ScaleFromBounds(this->GetGridBounds());

					//
					//
					CGrid x, y, z;
					this->GetGrid(x, y, z);
					z.Setup();
					pRiverActor->SetZ(z.coord[z.count_coord - 1]);
				}
			}
		}
	}

	// update wells
	//
	if (vtkPropAssembly *pPropAssembly = this->GetPropAssemblyWells())
	{
		CGrid xyz[3];
		this->GetGrid(xyz[0], xyz[1], xyz[2]);
		if (vtkPropCollection *pPropCollection = pPropAssembly->GetParts())
		{
			vtkProp *pProp;
			pPropCollection->InitTraversal();
			for (; (pProp = pPropCollection->GetNextProp()); )
			{
				if (CWellActor* pWellActor = CWellActor::SafeDownCast(pProp))
				{
					// update height
					//
					pWellActor->SetZAxis(xyz[2], this->GetUnits());

					// update radius
					//
// COMMENT: {8/16/2005 6:59:13 PM}					pWellActor->SetDefaultTubeDiameter(defaultAxesSize * 0.17 / sqrt(scale[0] * scale[1]));
					pWellActor->SetDefaultTubeDiameter(defaultAxesSize * 0.17);
				}
			}
		}
	}


	// refresh screen
	//
	this->UpdateAllViews(0);
}

void CWPhastDoc::OnViewHideAll()
{
	if (CPropertyTreeControlBar *pTree = this->GetPropertyTreeControlBar())
	{	
		pTree->SetMediaCheck(BST_UNCHECKED);
		pTree->SetBCCheck(BST_UNCHECKED);
		pTree->SetICCheck(BST_UNCHECKED);
		pTree->SetNodeCheck(pTree->GetWellsNode(), BST_UNCHECKED);
		pTree->SetNodeCheck(pTree->GetRiversNode(), BST_UNCHECKED);
		pTree->GetGridNode().Select();
	}
	this->UpdateAllViews(0);
}

void CWPhastDoc::OnUpdateRiversHideAll(CCmdUI *pCmdUI)
{
	if (vtkPropAssembly *pPropAssembly = this->GetPropAssemblyRivers())
	{
		if (pPropAssembly->GetVisibility())
		{
			pCmdUI->SetRadio(FALSE);
		}
		else
		{
			pCmdUI->SetRadio(TRUE);
		}
	}
}

void CWPhastDoc::OnRiversHideAll()
{
	if (CPropertyTreeControlBar *pTree = this->GetPropertyTreeControlBar())
	{	
		pTree->SetNodeCheck(pTree->GetRiversNode(), BST_UNCHECKED);
	}
	this->UpdateAllViews(0);
}

void CWPhastDoc::OnUpdateRiversShowSelected(CCmdUI *pCmdUI)
{
	if (CPropertyTreeControlBar *pTree = this->GetPropertyTreeControlBar())
	{	
		switch (pTree->GetNodeCheck(pTree->GetRiversNode()))
		{
		case BST_UNCHECKED:
			// currently unchecked
			pCmdUI->SetRadio(FALSE);
			break;
		case BST_CHECKED:
			// currently checked
			pCmdUI->SetRadio(TRUE);
			break;
		default:
			ASSERT(FALSE);
			pCmdUI->Enable(FALSE);
			break;
		}
	}
}

void CWPhastDoc::OnRiversShowSelected()
{
	if (CPropertyTreeControlBar *pTree = this->GetPropertyTreeControlBar())
	{	
		pTree->SetNodeCheck(pTree->GetRiversNode(), BST_CHECKED);
	}
	this->UpdateAllViews(0);
}

void CWPhastDoc::OnRiversSelectAll()
{
	if (CPropertyTreeControlBar* pTree = this->GetPropertyTreeControlBar())
	{
		CTreeCtrlNode node = pTree->GetRiversNode();
		int nCount = node.GetChildCount();
		for (int i = 0; i < nCount; ++i)
		{
			pTree->SetNodeCheck(node.GetChildAt(i), BST_CHECKED);
		}
	}
	this->UpdateAllViews(0);
}

void CWPhastDoc::OnRiversUnselectAll()
{
	if (CPropertyTreeControlBar* pTree = this->GetPropertyTreeControlBar())
	{
		CTreeCtrlNode node = pTree->GetRiversNode();
		int nCount = node.GetChildCount();
		for (int i = 0; i < nCount; ++i)
		{
			pTree->SetNodeCheck(node.GetChildAt(i), BST_UNCHECKED);
		}
	}
	this->UpdateAllViews(0);
}

void CWPhastDoc::OnUpdateToolsModifyGrid(CCmdUI *pCmdUI)
{
	if (CGridActor* pGridActor = CGridActor::SafeDownCast(this->GetGridActor()))
	{
		if (pGridActor->GetVisibility())
		{
			pCmdUI->Enable(TRUE);
		}
		else
		{
			pCmdUI->Enable(FALSE);
		}
	}
	else
	{
		pCmdUI->Enable(FALSE);
	}

	if (this->GridElementsSelector)
	{
		pCmdUI->SetCheck(1);
	}
	else
	{
		pCmdUI->SetCheck(0);
	}
}

void CWPhastDoc::OnToolsModifyGrid()
{
	if (this->GridElementsSelector)
	{
		this->EndModifyGrid();
	}
	else
	{
		POSITION pos = this->GetFirstViewPosition();
		if (pos != NULL)
		{
			CWPhastView *pView = (CWPhastView*) GetNextView(pos);
			ASSERT_VALID(pView);
			ASSERT(pView->GetRenderWindowInteractor());
			if (pView->GetRenderWindowInteractor())
			{
				pView->CancelMode();

				this->GridElementsSelector = CGridElementsSelector::New();
				this->GridElementsSelector->SetInteractor(pView->GetRenderWindowInteractor());
				this->GridElementsSelector->SetGridActor(reinterpret_cast<CGridActor *>(this->GetGridActor()));
				this->GridElementsSelector->SetDocument(this);
				this->GridElementsSelector->SetEnabled(1);
			}
		}
	}
}

void CWPhastDoc::EndModifyGrid()
{
	if (this->GridElementsSelector)
	{
		this->GridElementsSelector->Delete();
		this->GridElementsSelector = 0;
	}
}

CSolutionMethod CWPhastDoc::GetSolutionMethod(void)const
{
	return this->m_pModel->m_solutionMethod;
}

void CWPhastDoc::SetSolutionMethod(const CSolutionMethod &solutionMethod)
{
	this->m_pModel->m_solutionMethod = solutionMethod;
	if (CPropertyTreeControlBar* pTree = this->GetPropertyTreeControlBar())
	{
		pTree->SetSolutionMethod(&this->m_pModel->m_solutionMethod);
	}
}

void CWPhastDoc::OnViewShowAll()
{
	if (CPropertyTreeControlBar *pTree = this->GetPropertyTreeControlBar())
	{	
		pTree->SetMediaCheck(BST_CHECKED);
		pTree->SetBCCheck(BST_CHECKED);
		pTree->SetICCheck(BST_CHECKED);
		pTree->SetNodeCheck(pTree->GetWellsNode(), BST_CHECKED);
		pTree->SetNodeCheck(pTree->GetRiversNode(), BST_CHECKED);
		pTree->GetGridNode().Select();
	}
	this->UpdateAllViews(0);
}

void CWPhastDoc::SizeHandles(double size)
{
	if (size != size) return;
	TRACE("SizeHandles = %g\n", size);

	// set scale for all zones
	//
	if (vtkPropCollection* pCollection = this->GetPropCollection())
	{
		pCollection->InitTraversal();
		for (int i = 0; i < pCollection->GetNumberOfItems(); ++i)
		{
			vtkProp* prop = pCollection->GetNextProp();
			if (vtkPropAssembly *pPropAssembly = vtkPropAssembly::SafeDownCast(prop))
			{
				if (vtkPropCollection *pPropCollection = pPropAssembly->GetParts())
				{
					vtkProp *pProp;
					pPropCollection->InitTraversal();
					for (; (pProp = pPropCollection->GetNextProp()); )
					{
						if (vtkProp3D *prop3D = vtkProp3D::SafeDownCast(pProp))
						{
							if (CWellActor *pWellActor = CWellActor::SafeDownCast(prop3D))
							{
								pWellActor->SetRadius(0.008 * size);
							}
							if (CRiverActor *pRiverActor = CRiverActor::SafeDownCast(prop3D))
							{
								pRiverActor->SetRadius(0.008 * size);
							}
						}
					}
				}
			}
		}
	}
}

void CWPhastDoc::OnUpdateToolsNewZone(CCmdUI *pCmdUI)
{
	if (this->NewZoneWidget && this->NewZoneWidget->GetEnabled())
	{
		pCmdUI->SetCheck(1);
	}
	else
	{
		pCmdUI->SetCheck(0);
	}
}

void CWPhastDoc::OnToolsNewZone()
{
	if (this->NewZoneWidget && this->NewZoneWidget->GetEnabled())
	{
		this->EndNewZone();
	}
	else
	{
		if (this->NewZoneWidget) this->EndNewZone();
		this->BeginNewZone();
	}
}

void CWPhastDoc::BeginNewZone()
{
	ASSERT(this->NewZoneWidget == 0);
	ASSERT(this->NewZoneCallbackCommand == 0);

	POSITION pos = this->GetFirstViewPosition();
	if (pos != NULL)
	{
		CWPhastView *pView = (CWPhastView*) GetNextView(pos);
		ASSERT_VALID(pView);
		ASSERT(pView->GetRenderWindowInteractor());
		if (pView->GetRenderWindowInteractor())
		{
			pView->CancelMode();

			// create widget
			this->NewZoneWidget = CNewZoneWidget::New();
			this->NewZoneWidget->SetInteractor(pView->GetRenderWindowInteractor());
			this->NewZoneWidget->SetProp3D(this->GetGridActor());

			// add listener callback
			this->NewZoneCallbackCommand = vtkCallbackCommand::New();
			this->NewZoneCallbackCommand->SetClientData(this);
			this->NewZoneCallbackCommand->SetCallback(CWPhastDoc::NewZoneListener);
			this->NewZoneWidget->AddObserver(vtkCommand::EndInteractionEvent, this->NewZoneCallbackCommand);

			// enable widget
			this->NewZoneWidget->SetEnabled(1);
		}
	}
}

void CWPhastDoc::EndNewZone()
{
	if (this->NewZoneCallbackCommand)
	{
		ASSERT(this->NewZoneCallbackCommand->IsA("vtkObjectBase"));
		this->NewZoneCallbackCommand->Delete();
		this->NewZoneCallbackCommand = 0;
	}
	if (this->NewZoneWidget)
	{
		ASSERT(this->NewZoneWidget->IsA("CNewZoneWidget"));
		this->NewZoneWidget->Delete();
		this->NewZoneWidget = 0;
	}
}

#include "NewZonePropertyPage.h"
#include "MediaSpreadPropertyPage.h"
#include "BCFluxPropertyPage2.h"
#include "BCLeakyPropertyPage2.h"
#include "BCSpecifiedHeadPropertyPage.h"
#include "ICHeadSpreadPropertyPage.h"
#include "ChemICSpreadPropertyPage.h"

void CWPhastDoc::NewZoneListener(vtkObject *caller, unsigned long eid, void *clientdata, void *calldata) 
{
	ASSERT(caller->IsA("CNewZoneWidget"));
	ASSERT(clientdata);

	if (clientdata)
	{
		CWPhastDoc* self = reinterpret_cast<CWPhastDoc*>(clientdata);
		if (eid == vtkCommand::EndInteractionEvent)
		{
			vtkFloatingPointType scaled_meters[6];
			self->NewZoneWidget->GetBounds(scaled_meters);

			// calc zone
			CZone zone;
			vtkFloatingPointType* scale = self->GetScale();
			const CUnits& units = self->GetUnits();
			zone.x1 = scaled_meters[0] / scale[0] / units.horizontal.input_to_si;
			zone.x2 = scaled_meters[1] / scale[0] / units.horizontal.input_to_si;
			zone.y1 = scaled_meters[2] / scale[1] / units.horizontal.input_to_si;
			zone.y2 = scaled_meters[3] / scale[1] / units.horizontal.input_to_si;
			zone.z1 = scaled_meters[4] / scale[2] / units.vertical.input_to_si;
			zone.z2 = scaled_meters[5] / scale[2] / units.vertical.input_to_si;

			TRACE("x(%g-%g) y(%g-%g) z(%g-%g)\n", zone.x1, zone.x2, zone.y1, zone.y2, zone.z1, zone.z2);

			// get type of zone
			//
			ETSLayoutPropertySheet        sheet("Zone Wizard", NULL, 0, NULL, false);

			CNewZonePropertyPage          newZone;
			CMediaSpreadPropertyPage      mediaProps;
			CBCFluxPropertyPage2          fluxProps;
			CBCLeakyPropertyPage2         leakyProps;
			CBCSpecifiedHeadPropertyPage  specifiedProps;
			CICHeadSpreadPropertyPage     icHeadProps;
			CChemICSpreadPropertyPage     chemICProps;

			// CChemICSpreadPropertyPage only needs the flowonly flag when the zone is a
			// default zone
			//
			bool bFlowOnly = self->GetFlowOnly();

			fluxProps.SetFlowOnly(bFlowOnly);
			leakyProps.SetFlowOnly(bFlowOnly);
			specifiedProps.SetFlowOnly(bFlowOnly);

			sheet.AddPage(&newZone);
			sheet.AddPage(&mediaProps);
			sheet.AddPage(&fluxProps);
			sheet.AddPage(&leakyProps);
			sheet.AddPage(&specifiedProps);
			sheet.AddPage(&icHeadProps);
			sheet.AddPage(&chemICProps);

			sheet.SetWizardMode();

			if (sheet.DoModal() == ID_WIZFINISH)
			{
				if (newZone.GetType() == ID_ZONE_TYPE_MEDIA)
				{
					CGridElt elt;
					mediaProps.GetProperties(elt);
					CMediaZoneActor::Create(self, zone, elt, mediaProps.GetDesc());
				}
				else if (newZone.GetType() == ID_ZONE_TYPE_BC_FLUX)
				{
					CBC bc;
					fluxProps.GetProperties(bc);
					CBCZoneActor::Create(self, zone, bc, fluxProps.GetDesc());
				}
				else if (newZone.GetType() == ID_ZONE_TYPE_BC_LEAKY)
				{
					CBC bc;
					leakyProps.GetProperties(bc);
					CBCZoneActor::Create(self, zone, bc, leakyProps.GetDesc());
				}
				else if (newZone.GetType() == ID_ZONE_TYPE_BC_SPECIFIED)
				{
					CBC bc;
					specifiedProps.GetProperties(bc);
					CBCZoneActor::Create(self, zone, bc, specifiedProps.GetDesc());
				}
				else if (newZone.GetType() == ID_ZONE_TYPE_IC_HEAD)
				{
					CHeadIC headic;
					icHeadProps.GetProperties(headic);
					CICHeadZoneActor::Create(self, zone, headic, icHeadProps.GetDesc());
				}
				else if (newZone.GetType() == ID_ZONE_TYPE_IC_CHEM)
				{
					CChemIC chemIC;
					chemICProps.GetProperties(chemIC);
					CICChemZoneActor::Create(self, zone, chemIC, chemICProps.GetDesc());
				}
			}

			// Note: cannot call EndNewZone here
			self->NewZoneWidget->SetEnabled(0);
		}
	}

}

BOOL CWPhastDoc::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if (this->NewZoneWidget && this->NewZoneWidget->GetEnabled())
	{
		return this->NewZoneWidget->OnSetCursor(pWnd, nHitTest, message);
	}
	return FALSE;
}
