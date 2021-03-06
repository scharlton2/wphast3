#include "StdAfx.h"
#include "NewPrismWidget.h"

#include "Global.h"    // CGlobal::ComputeRadius
#include "Utilities.h" // CUtilities::GetWorldPointAtFixedPlane
#include "Resource.h"  // IDC_NULL
#include <sstream>     // std::ostringstream
#include <vtkLine.h>
#include "srcinput/Prism.h"
#include "GridKeyword.h"

#if defined(USE_INTRINSIC)
#pragma intrinsic(fabs) // using this inlines fabs and is ~ 4x faster
#define FABS(x) fabs(x)
#else
#define FABS(x) ((x < 0) ? -x : x) // this is slightly slower than the intrinsic
#endif

vtkCxxRevisionMacro(CNewPrismWidget, "$Revision$");
vtkStandardNewMacro(CNewPrismWidget);

// Note: No header files should follow the following three lines
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CNewPrismWidget::CNewPrismWidget(void)
: State(CNewPrismWidget::Start)
, Cursor3D(0)
, Cursor3DMapper(0)
, Cursor3DActor(0)
, LastPointId(0)
, LastCellId(0)
, Points(0)
, CellArray(0)
, PolyData(0)
, OutlineMapper(0)
, OutlineActor(0)
, NumberOfClicks(0)
, GridAngle(0)
, CoordinateMode(CWPhastDoc::GridMode)
{
	for (int i = 0; i < 3; ++i)
	{
		this->GridOrigin[i]    = 0;
		this->GeometryScale[i] = 0;
	}

	ASSERT(this->EventCallbackCommand);
	this->EventCallbackCommand->SetCallback(CNewPrismWidget::ProcessEvents);

	// 3D Cursor
	//
	this->Cursor3D = vtkCursor3D::New();
	this->Cursor3D->XShadowsOff();
	this->Cursor3D->YShadowsOff();
	this->Cursor3D->ZShadowsOff();
	this->Cursor3D->OutlineOff();

	this->Cursor3DMapper = vtkPolyDataMapper::New();
	this->Cursor3DMapper->SetInput(this->Cursor3D->GetOutput());
	
	this->Cursor3DActor = vtkActor::New();
	this->Cursor3DActor->SetMapper(this->Cursor3DMapper);
	this->Cursor3DActor->SetPosition(0, 0, 0);
	this->Cursor3DActor->GetProperty()->SetColor(1, 1, 1); // white

	// Outline
	//
	this->Points = vtkPoints::New();
	this->CellArray = vtkCellArray::New();
	
	this->PolyData = vtkPolyData::New();
	this->PolyData->SetPoints(this->Points);
	this->PolyData->SetPolys(this->CellArray);

	this->OutlineMapper = vtkPolyDataMapper::New();
	this->OutlineMapper->SetInput( this->PolyData );

	this->OutlineActor = vtkActor::New();
	this->OutlineActor->SetMapper( this->OutlineMapper );
	this->OutlineActor->SetVisibility(0);
    this->OutlineActor->GetProperty()->SetColor(1, 0, 0);

	this->OutlineActor->GetProperty()->SetRepresentationToWireframe();
	this->OutlineActor->GetProperty()->SetAmbient(1.0); // this makes the back edges easier to see

#if defined(_MFC_VER)
	::SetRect(&this->RectClick, 0, 0, 0, 0);
#endif
}

CNewPrismWidget::~CNewPrismWidget(void)
{
	this->EventCallbackCommand->SetCallback(0);

	this->Cursor3DActor->SetVisibility(0);
	this->OutlineActor->SetVisibility(0);
	if (this->CurrentRenderer)
	{
		this->CurrentRenderer->RemoveActor(this->Cursor3DActor);
		this->CurrentRenderer->RemoveActor(this->OutlineActor);
		if (this->Interactor)
		{
			this->Interactor->Render();
		}
	}

	this->Cursor3D->Delete();
	this->Cursor3DMapper->Delete();
	this->Cursor3DActor->Delete();

	this->Points->Delete();
	this->CellArray->Delete();
	this->PolyData->Delete();

	this->OutlineMapper->Delete();
	this->OutlineActor->Delete();
}

void CNewPrismWidget::SetEnabled(int enabling)
{
	if ( ! this->Interactor )
	{
		vtkErrorMacro(<<"The interactor must be set prior to enabling/disabling widget");
		return;
	}

	if ( enabling ) //------------------------------------------------------------
	{
		vtkDebugMacro(<<"Enabling widget");

		if ( this->Enabled ) //already enabled, just return
		{
			return;
		}

		if ( ! this->CurrentRenderer )
		{
			this->SetCurrentRenderer(this->Interactor->FindPokedRenderer(
				this->Interactor->GetLastEventPosition()[0],
				this->Interactor->GetLastEventPosition()[1]));
			if (this->CurrentRenderer == NULL)
			{
				return;
			}
		}

		this->Enabled = 1;

		// listen to the following events
		vtkRenderWindowInteractor *i = this->Interactor;
		i->AddObserver(vtkCommand::MouseMoveEvent,
            this->EventCallbackCommand, this->Priority);
		i->AddObserver(vtkCommand::LeftButtonPressEvent, 
			this->EventCallbackCommand, this->Priority);
		i->AddObserver(vtkCommand::LeftButtonReleaseEvent, 
			this->EventCallbackCommand, this->Priority);
		i->AddObserver(vtkCommand::KeyPressEvent, 
			this->EventCallbackCommand, this->Priority);
		i->AddObserver(vtkCommand::KeyReleaseEvent, 
			this->EventCallbackCommand, this->Priority);

		this->CurrentRenderer->AddActor(this->Cursor3DActor);
		this->CurrentRenderer->AddActor(this->OutlineActor);

		this->InvokeEvent(vtkCommand::EnableEvent, NULL);

	}

	else //disabling-------------------------------------------------------------
	{
		vtkDebugMacro(<<"Disabling widget");

		if ( ! this->Enabled ) //already disabled, just return
		{
			return;
		}

		this->Enabled = 0;

		if (this->CurrentRenderer)
		{
			this->CurrentRenderer->RemoveActor(this->Cursor3DActor);
			this->CurrentRenderer->RemoveActor(this->OutlineActor);
		}

		// don't listen for events any more
		this->Interactor->RemoveObserver(this->EventCallbackCommand);
		this->InvokeEvent(vtkCommand::DisableEvent, NULL);
		this->SetCurrentRenderer(NULL);
	}

	this->Interactor->Render();
}

void CNewPrismWidget::PlaceWidget(double bounds[6])
{
	this->OutlineActor->SetVisibility(1);
}

void CNewPrismWidget::ProcessEvents(vtkObject* object, unsigned long event, void* clientdata, void* calldata)
{
	CNewPrismWidget* self = reinterpret_cast<CNewPrismWidget *>(clientdata);

	if (!self || !self->Prop3D || !self->Interactor) return;

	int X = self->Interactor->GetEventPosition()[0];
	int Y = self->Interactor->GetEventPosition()[1];

	vtkRenderer *ren = self->Interactor->FindPokedRenderer(X, Y);
	if (ren != self->CurrentRenderer)
	{
		return;
	}

	switch(event)
	{
	case vtkCommand::LeftButtonPressEvent:
		self->OnLeftButtonDown();
		break;
	case vtkCommand::LeftButtonReleaseEvent:
		self->OnLeftButtonUp();
		break;
	case vtkCommand::MouseMoveEvent:
		self->OnMouseMove();
		break;
	case vtkCommand::KeyPressEvent:
		self->OnKeyPress();
		break;
	}
}

void CNewPrismWidget::OnMouseMove()
{
	if (!this->CurrentRenderer) return;
	if (!this->Prop3D) return;

	this->FixedCoord = 2; // always z-axis
	this->FixedPlane = 5; // always zmax

	ASSERT( 0 <= this->FixedPlane && this->FixedPlane < 6 );
	ASSERT( 0 <= this->FixedCoord && this->FixedCoord < 3 );

	// get point of intersection of axis=this->FixedCoord with a value of bounds[this->this->FixedPlane]
	// this->FixedPlane       this->FixedCoord
	//    0 => xmin        0 => x
	//    1 => xmax        1 => y
	//    2 => ymin        2 => z
	//    3 => ymax
	//    4 => zmin
	//    5 => zmax
	//

	double* bounds = this->Prop3D->GetBounds();
	CUtilities::GetWorldPointAtFixedPlane(this->Interactor, this->CurrentRenderer, this->FixedCoord, bounds[this->FixedPlane], this->FixedPlanePoint);

	if (this->LastPointId > 0)
	{
		// update LastPointId-1 and LastPointId
		this->Points->SetPoint(this->LastPointId-1, this->FixedPlanePoint[0], this->FixedPlanePoint[1], bounds[4]);
		this->Points->SetPoint(this->LastPointId, this->FixedPlanePoint[0], this->FixedPlanePoint[1], bounds[5]);
		this->Points->Modified();
	}

	double dim = 0.016 * CGlobal::ComputeRadius(this->CurrentRenderer);
	this->Cursor3D->SetModelBounds(-dim, dim, -dim, dim, -dim, dim);

	this->Cursor3DActor->SetPosition(this->FixedPlanePoint);
	this->CurrentRenderer->ResetCameraClippingRange();

	this->Interactor->Render();
}

static const int VTK_NO_INTERSECTION=0;
static const int VTK_YES_INTERSECTION=2;
static const int VTK_ON_LINE=3;

void CNewPrismWidget::OnLeftButtonDown()
{
	TRACE("CNewPrismWidget::OnLeftButtonDown\n");

#if defined(_MFC_VER)
	_AFX_THREAD_STATE *pState = AfxGetThreadState();
	POINT point = { MAKEPOINTS(pState->m_msgCur.lParam).x, MAKEPOINTS(pState->m_msgCur.lParam).y };
	DWORD tmClick = GetMessageTime();
	if (!::PtInRect(&this->RectClick, point) || tmClick - this->TimeLastClick > ::GetDoubleClickTime())
	{
		this->NumberOfClicks = 0;
	}
	this->NumberOfClicks++;
	this->TimeLastClick = tmClick;
	::SetRect(&this->RectClick, MAKEPOINTS(pState->m_msgCur.lParam).x, MAKEPOINTS(pState->m_msgCur.lParam).y,
		MAKEPOINTS(pState->m_msgCur.lParam).x, MAKEPOINTS(pState->m_msgCur.lParam).y);
	::InflateRect(&this->RectClick,
		::GetSystemMetrics(SM_CXDOUBLECLK) / 2,
		::GetSystemMetrics(SM_CYDOUBLECLK) / 2);
#endif
	if (this->NumberOfClicks > 1)
	{
		TRACE("double-click detected\n");
		ASSERT(this->State == CNewPrismWidget::CreatingPrism);
		this->EndNew();
		return;
	}

	// set state
	//
	this->State = CNewPrismWidget::CreatingPrism;


	// get point of intersection of axis=FixedCoord with a value of bounds[this->FixedPlane]
	// FixedPlane       FixedCoord
	//    0 => xmin        0 => x
	//    1 => xmax        1 => y
	//    2 => ymin        2 => z
	//    3 => ymax
	//    4 => zmin
	//    5 => zmax
	//
	double* bounds = this->Prop3D->GetBounds();
	double pt[3];
	CUtilities::GetWorldPointAtFixedPlane(this->Interactor, this->CurrentRenderer, this->FixedCoord, bounds[this->FixedPlane], pt);

	if (this->LastPointId == 0) // first click
	{
		// create first point (0,1)
		this->LastPointId = this->Points->InsertNextPoint(pt[0], pt[1], bounds[4]);
		this->LastPointId = this->Points->InsertNextPoint(pt[0], pt[1], bounds[5]);

		// duplicate first point (2,3)
		this->LastPointId = this->Points->InsertNextPoint(pt[0], pt[1], bounds[4]);
		this->LastPointId = this->Points->InsertNextPoint(pt[0], pt[1], bounds[5]);

		// create first cell (0,1,3,2)
		this->LastCellId = this->CellArray->InsertNextCell(4);
		this->CellArray->InsertCellPoint(this->LastPointId-3);
		this->CellArray->InsertCellPoint(this->LastPointId-2);
		this->CellArray->InsertCellPoint(this->LastPointId);
		this->CellArray->InsertCellPoint(this->LastPointId-1);

		// create connection cell (2,3,1,0)
		this->LastCellId = this->CellArray->InsertNextCell(4);
		this->CellArray->InsertCellPoint(this->LastPointId-1);
		this->CellArray->InsertCellPoint(this->LastPointId);
		this->CellArray->InsertCellPoint(1);
		this->CellArray->InsertCellPoint(0);

		this->StartInteraction();
		this->InvokeEvent(vtkCommand::StartInteractionEvent, NULL);
	}
	else
	{
		bool AllowIntersection = false;
		if (!AllowIntersection)
		{
			double p1[3];
			double p2[3];
			double x1[3];
			double x2[3];
			double u;
			double v;
			p1[0] = pt[0];
			p1[1] = pt[1];
			p1[2] = pt[2];
			this->Points->GetPoint(this->LastPointId-2, p2);

			for (vtkIdType i = 3; i < this->LastPointId-2; i+=2)
			{
				this->Points->GetPoint(i-2, x1);
				this->Points->GetPoint(i, x2);
				if (vtkLine::Intersection(p1, p2, x1, x2, u, v) == VTK_YES_INTERSECTION)
				{
					TRACE("last point crosses\n");
#ifdef _DEBUG
					std::ostringstream oss;
					this->PrintSelf(oss, vtkIndent(0));
					//TRACE(oss.str().c_str());
					::OutputDebugStringA(oss.str().c_str());
#endif

#ifdef WIN32
					::MessageBeep(MB_ICONEXCLAMATION);
#endif
					
					// stop forwarding event
					this->EventCallbackCommand->SetAbortFlag(1);
					this->Interactor->Render();
					return;
				}
			}
		}

		// avoid adding duplicate points (possible double-click)
		//
		double lastpt[3];
		double sNext[3];
		this->Points->GetPoint(this->LastPointId-2, lastpt);
		for (int i = 0; i < 3; ++i) 
		{
			sNext[i] = pt[i] - lastpt[i];
		}

		if (vtkMath::Normalize(sNext) == 0.0)
		{
			TRACE("duplicate point found\n");
#ifdef _DEBUG
			std::ostringstream oss;
			this->PrintSelf(oss, vtkIndent(0));
			::OutputDebugStringA(oss.str().c_str());
#endif
			// stop forwarding event
			this->EventCallbackCommand->SetAbortFlag(1);
			this->Interactor->Render();
			return;
		}

		// update LastPtId-1 and LastPtId
		this->Points->SetPoint(this->LastPointId-1, pt[0], pt[1], bounds[4]);
		this->Points->SetPoint(this->LastPointId, pt[0], pt[1], bounds[5]);

		// add duplicate pts LastPtId-1 and LastPtId
		this->LastPointId = this->Points->InsertNextPoint(pt[0], pt[1], bounds[4]);
		this->LastPointId = this->Points->InsertNextPoint(pt[0], pt[1], bounds[5]);

		// update connection cell
		vtkIdType ids[4];
		ids[0] = this->LastPointId-3;
		ids[1] = this->LastPointId-2;
		ids[2] = this->LastPointId;
		ids[3] = this->LastPointId-1;
		this->CellArray->ReplaceCell(this->LastCellId*5, 4, ids);

		// create NEW connection cell
		this->LastCellId = this->CellArray->InsertNextCell(4);
		this->CellArray->InsertCellPoint(this->LastPointId-1);
		this->CellArray->InsertCellPoint(this->LastPointId);
		this->CellArray->InsertCellPoint(1);
		this->CellArray->InsertCellPoint(0);
	}

#ifdef _DEBUG
	std::ostringstream oss;
	this->PrintSelf(oss, vtkIndent(0));
	TRACE(oss.str().c_str());
#endif

	this->OutlineActor->SetVisibility(1);


	// stop forwarding event
	//
	this->EventCallbackCommand->SetAbortFlag(1);
	this->Interactor->Render();
}

void CNewPrismWidget::OnLeftButtonUp()
{
	TRACE("CNewPrismWidget::OnLeftButtonUp\n");
	if ( this->State == CNewPrismWidget::CreatingPrism )
	{
		this->EventCallbackCommand->SetAbortFlag(1);
		if (this->Interactor)
		{
			this->Interactor->Render();
		}
	}
}

#if defined(_MFC_VER)
BOOL CNewPrismWidget::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if (this->Enabled)
	{
		::SetCursor(::AfxGetApp()->LoadCursor(IDC_NULL));
		return TRUE;
	}
	return FALSE;
}
#endif

void CNewPrismWidget::PrintSelf(ostream& os, vtkIndent indent)
{
	this->Superclass::PrintSelf(os, indent);

	os << indent << "LastPointId = " << this->LastPointId << "\n";
	os << indent << "LastCellId = " << this->LastCellId << "\n";
	os << indent << "CellArray\n";
	os << indent << "NumberOfCells: " << this->CellArray->GetNumberOfCells() << "\n";
	this->CellArray->InitTraversal();
	for (vtkIdType i = 0; i < this->CellArray->GetNumberOfCells(); ++i)
	{
		vtkIdType *pts;
		vtkIdType npts;
		this->CellArray->GetNextCell(npts, pts);
		ASSERT(npts==4);
		os << indent << "npts = " << npts << "\n";
		os << indent << "CellId(" << i << ")=" << pts[0] << "," << pts[1] << "," << pts[2] << "," << pts[3] << "\n";
	}
	os << "Points\n";	
	this->Points->PrintSelf(os, vtkIndent(0));
	for (vtkIdType i = 0; i < this->Points->GetNumberOfPoints(); ++i)
	{
		double *pt = this->Points->GetPoint(i);
		os << indent << "PointId(" << i << ")=(" << pt[0] << "," << pt[1] << "," << pt[2] << ")\n";
	}
}

void CNewPrismWidget::OnKeyPress()
{
	if (!this->Interactor) return;

#ifdef _DEBUG
	{
		std::ostringstream oss;
		this->PrintSelf(oss, vtkIndent(1));
		::OutputDebugStringA("CNewPrismWidget::OnKeyPress IN\n");
		::OutputDebugStringA(oss.str().c_str());
	}
#endif

	if (this->State == CNewPrismWidget::CreatingPrism)
	{
		char* keysym = this->Interactor->GetKeySym();		
		if (::strcmp(keysym, "Escape") == 0)
		{
			TRACE("CancelNewPrism\n");
			this->CancelNew();
		}
		else
		{
			TRACE("EndNewPrism\n");
			this->EndNew();
		}
	}

#ifdef _DEBUG
	{
		std::ostringstream oss;
		this->PrintSelf(oss, vtkIndent(1));
		::OutputDebugStringA("CNewPrismWidget::OnKeyPress OUT\n");
		::OutputDebugStringA(oss.str().c_str());
	}
#endif
}

void CNewPrismWidget::CancelNew(void)
{
	// stop forwarding event
	//
	if (this->EventCallbackCommand)
	{
		this->EventCallbackCommand->SetAbortFlag(1);
	}
	if (this->Interactor)
	{
		this->Interactor->Render();
	}
}

void CNewPrismWidget::EndNew(void)
{
	// remove last cell
	//
	vtkCellArray *ca = vtkCellArray::New();
	this->CellArray->GetData()->SetNumberOfValues(this->LastCellId*5);
	ca->SetCells(this->LastCellId, this->CellArray->GetData());

	vtkCellArray *tmp = this->CellArray;
	this->CellArray = ca;
	this->PolyData->SetPolys(this->CellArray);
	tmp->Delete();

	// update connection cell
	//
	vtkIdType ids[4];
	ids[0] = this->LastPointId-3;
	ids[1] = this->LastPointId-2;
	ids[2] = 1;
	ids[3] = 0;
	this->CellArray->ReplaceCell((this->LastCellId-1)*5, 4, ids);

	// update points
	//
	this->Points->SetNumberOfPoints(this->Points->GetNumberOfPoints()-2);

	// stop forwarding event
	//
	if (this->EventCallbackCommand)
	{
		this->EventCallbackCommand->SetAbortFlag(1);
	}
	if (this->Interactor)
	{
		this->Interactor->Render();
	}

	// cancel creation
	//
	this->State = CNewPrismWidget::Start;
	this->LastPointId = 0;

	// notify
	//
	this->EndInteraction();
	this->InvokeEvent(vtkCommand::EndInteractionEvent, NULL);
}

void CNewPrismWidget::SetGridKeyword(const CGridKeyword& gridKeyword, const CUnits& units)
{
	// set up for transform
	this->GridAngle     = gridKeyword.m_grid_angle;
	this->GridOrigin[0] = gridKeyword.m_grid_origin[0];
	this->GridOrigin[1] = gridKeyword.m_grid_origin[1];
	this->GridOrigin[2] = gridKeyword.m_grid_origin[2];
	this->Units         = units;
}

void CNewPrismWidget::SetScale(double x, double y, double z)
{
	this->GeometryScale[0] = x;
	this->GeometryScale[1] = y;
	this->GeometryScale[2] = z;
}

void CNewPrismWidget::SetCoordinateMode(CWPhastDoc::CoordinateState mode)
{
	this->CoordinateMode = mode;

	if (mode == CWPhastDoc::GridMode)
	{
		this->Cursor3DActor->SetOrientation(0, 0, 0);
	}
	else if (mode == CWPhastDoc::MapMode)
	{
		this->Cursor3DActor->SetOrientation(0, 0, -this->GridAngle);
	}
	else
	{
		ASSERT(FALSE);
	}
}

Prism* CNewPrismWidget::GetPrism(void)
{
	if (this->CoordinateMode == CWPhastDoc::GridMode)
	{
		double pt[3];
		std::vector< Point > pts;
		vtkPoints *points = this->GetPoints();
		for (vtkIdType i = 0; i < points->GetNumberOfPoints(); i+=2)
		{
			//double *pt = points->GetPoint(i);
			points->GetPoint(i, pt);
			pt[0] = pt[0] / this->GeometryScale[0] / this->Units.horizontal.input_to_si;
			pt[1] = pt[1] / this->GeometryScale[1] / this->Units.horizontal.input_to_si;
			pt[2] = 0.;
			pts.push_back(Point(pt[0], pt[1], pt[1]));
		}
		Prism *p = new Prism(pts, PHAST_Transform::GRID);
		ASSERT(p->bottom.Get_user_source_type() == Data_source::NONE);
		ASSERT(p->top.Get_user_source_type() == Data_source::NONE);
		ASSERT(p->bottom.Get_user_coordinate_system() == PHAST_Transform::GRID);
		ASSERT(p->top.Get_user_coordinate_system() == PHAST_Transform::GRID);
		p->Tidy();
#ifdef _DEBUG
		{
			std::ostringstream oss;
			oss << (*p) << std::endl;
			TRACE(oss.str().c_str());
		}
#endif
		return p;
	}
	else if (this->CoordinateMode == CWPhastDoc::MapMode)
	{
		double pt[3];
		std::vector< Point > pts;
		vtkPoints *points = this->GetPoints();
		for (vtkIdType i = 0; i < points->GetNumberOfPoints(); i+=2)
		{
			//double *pt = points->GetPoint(i);
			points->GetPoint(i, pt);
			pt[0] = pt[0] / this->GeometryScale[0] / this->Units.horizontal.input_to_si;
			pt[1] = pt[1] / this->GeometryScale[1] / this->Units.horizontal.input_to_si;
			pt[2] = 0.;
			pts.push_back(Point(pt[0], pt[1], pt[1]));
		}
		Prism *p = new Prism(pts, PHAST_Transform::GRID);

		// Set up transform
		double scale_h = this->Units.map_horizontal.input_to_si / this->Units.horizontal.input_to_si;
		double scale_v = this->Units.map_vertical.input_to_si / this->Units.vertical.input_to_si;
		PHAST_Transform map2grid(
			this->GridOrigin[0],
			this->GridOrigin[1],
			this->GridOrigin[2],
			this->GridAngle,
			scale_h,
			scale_h,
			scale_v);
#if !defined(PERIMETER_ONLY)
		p->perimeter.Convert_coordinates(PHAST_Transform::MAP, &map2grid);
		p->perimeter.Set_user_coordinate_system(PHAST_Transform::MAP);
		//{{
		p->perimeter.Set_user_points(p->perimeter.Get_points());
		//}}
		p->Tidy();
		ASSERT(p->bottom.Get_user_source_type() == Data_source::NONE);
		ASSERT(p->top.Get_user_source_type() == Data_source::NONE);
		ASSERT(p->bottom.Get_user_coordinate_system() == PHAST_Transform::GRID);
		ASSERT(p->top.Get_user_coordinate_system() == PHAST_Transform::GRID);
#else
		// THIS DOESN'T WORK YET (THE TOPS AND BOTTOMS AREN'T RIGHT WHEN
		// MAP UNITS AREN'T THE SAME AS GRID UNITS
		//
		p->Tidy();
		p->Convert_coordinates(PHAST_Transform::MAP, &map2grid);
		p->perimeter.Set_user_coordinate_system(PHAST_Transform::MAP);
		p->top.Set_user_coordinate_system(PHAST_Transform::MAP);
		p->bottom.Set_user_coordinate_system(PHAST_Transform::MAP);
#endif
#ifdef _DEBUG
		{
			std::ostringstream oss;
			oss << (*p) << std::endl;
			TRACE(oss.str().c_str());
		}
#endif
		return p;
	}
	else
	{
		ASSERT(FALSE);
		Prism *p = new Prism;
		p->Tidy();
#ifdef _DEBUG
		{
			std::ostringstream oss;
			oss << (*p) << std::endl;
			TRACE(oss.str().c_str());
		}
#endif
		return p;
	}
}

void CNewPrismWidget::SetCursorColor(double r, double g, double b)
{
	this->Cursor3DActor->GetProperty()->SetColor(r, g, b);
}