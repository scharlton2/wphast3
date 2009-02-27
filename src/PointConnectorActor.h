#pragma once
#include <list>

#include <vtkAssembly.h>
#include <vtkPropAssembly.h>
#include <vtkIdType.h>

#include "tree.h"           // CTreeCtrlNode
#include "srcinput/PHAST_Transform.h"

class CUnits;
class CPropertyTreeControlBar;
class CTreeMemento;
class CGridKeyword;

#ifndef vtkFloatingPointType
#define vtkFloatingPointType vtkFloatingPointType
typedef float vtkFloatingPointType;
#endif

class CPointConnectorActor : public vtkAssembly
{
public:
	vtkTypeRevisionMacro(CPointConnectorActor,vtkAssembly);
	static CPointConnectorActor *New();
	void PrintSelf(ostream& os, vtkIndent indent);

	// virtual functions
	//
	virtual vtkIdType InsertNextPoint(double x, double y, double z);
	virtual void InsertPoint(vtkIdType id, double x, double y, double z);

	virtual void DeletePoint(vtkIdType id);
	virtual void Update(CTreeCtrlNode node);

	void SetUnits(const CUnits &units, bool bUpdatePoints = true);
	void SetGridKeyword(const CGridKeyword &gridKeyword, bool bUpdatePoints = true);

	void SetScale(float x, float y, float z);
	void SetScale(double x, double y, double z);
	void SetScale(vtkFloatingPointType scale[3]) { this->SetScale(scale[0], scale[1], scale[2]); }
	virtual PHAST_Transform::COORDINATE_SYSTEM GetCoordinateSystem(void)const      { return PHAST_Transform::GRID; };

	void SetRadius(vtkFloatingPointType radius);
	vtkFloatingPointType GetRadius(void)const;

	void ScaleFromBounds(vtkFloatingPointType bounds[6]);

	void SetZ(double z);
	double GetZ(void)const;

	void UpdatePoints(void);
	void ClearPoints(void);
	void SelectPoint(int index);
	vtkActor* GetPoint(int index);
	void ClearSelection(void);

	// Description:
	// Methods for turning the interactor observer on and off, and determining
	// its state. All subclasses must provide the SetEnabled() method.
	// Enabling a vtkInteractorObserver has the side effect of adding
	// observers; disabling it removes the observers. Prior to enabling the
	// vtkInteractorObserver you must set the render window interactor (via
	// SetInteractor()).
	void SetEnabled(int enabling);
	int GetEnabled() {return this->Enabled;}
	void EnabledOn() {this->SetEnabled(1);}
	void EnabledOff() {this->SetEnabled(0);}
	void On() {this->SetEnabled(1);}
	void Off() {this->SetEnabled(0);}

	// CPropertyTreeControlBar operations
	//
	virtual void Add(CPropertyTreeControlBar *pTree, HTREEITEM hInsertAfter = TVI_LAST);
	void UnAdd(CPropertyTreeControlBar *pTree);

	void Remove(CPropertyTreeControlBar *pTree);
	void UnRemove(CPropertyTreeControlBar *pTree);

	// may want to change CZoneActor's methods to be more compatible
	// ie Insert -> Add
	// req'd for CDragDropAction<>
	void Insert(CPropertyTreeControlBar *pTree, HTREEITEM hInsertAfter = TVI_LAST) { this->Add(pTree, hInsertAfter); }

	enum EventIds
	{
		StartMovePointEvent = vtkCommand::UserEvent + 500,
		MovingPointEvent,
		EndMovePointEvent,

		CancelNewEvent,
		EndNewEvent,

		InsertPointEvent,
		DeletePointEvent,
	};

	virtual void MovePoint(vtkIdType id, double x, double y);

	vtkIdType GetCurrentPointId(void)const;
	double*   GetCurrentPointPosition(void);
	void      GetCurrentPointPosition(double x[3])const;

	size_t GetPointCount(void)const;

	void SetName(LPCTSTR name)       {/* no-op */};
	LPCTSTR GetName(void)const;

	void Serialize(bool bStoring, hid_t loc_id, const CUnits &units);

	virtual void SetVisibility(int visibility);
	int GetVisibility(void);

	// Description:
	// This method is used to associate the widget with the render window
	// interactor.  Observers of the appropriate events invoked in the render
	// window interactor are set up as a result of this method invocation.
	// The SetInteractor() method must be invoked prior to enabling the
	// vtkInteractorObserver.
	virtual void SetInteractor(vtkRenderWindowInteractor* iren);
	vtkGetObjectMacro(Interactor, vtkRenderWindowInteractor);

	void CancelNew(void);
	void EndNew(void);

	CTreeCtrlNode GetTreeCtrlNode(void)      { return Node; }

// COMMENT: {10/8/2008 1:57:03 PM}	static const char szHeading[];

	static void SetStaticColor(COLORREF cr);

#ifdef WIN32
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
#endif

protected:
	CPointConnectorActor(void);
	~CPointConnectorActor(void);

	void AddGraphicPoint(void);
	void DeleteGraphicPoint(void);

	enum PointConnectorState
	{
		None,
		MovingPoint,
		CreatingRiver,
	} State;

	// ghost point
	vtkSphereSource*   GhostSphereSource;
	vtkPolyDataMapper* GhostPolyDataMapper;
	vtkActor*          GhostActor;

	// Description:
	// Main process event method
	static void ProcessEvents(vtkObject* object, unsigned long event, void* clientdata, void* calldata);

	// Properties used to control the appearance of selected objects and
	// the manipulator in general.
	vtkProperty *SelectedHandleProperty;
	vtkProperty *EnabledHandleProperty;
	void CreateDefaultProperties(void);

	virtual vtkProperty *GetHandleProperty(void)      { return 0; };
	virtual vtkProperty *GetConnectorProperty(void)   { return 0; };

	int HighlightHandle(vtkProp *prop);

	// ProcessEvents() dispatches to these methods.
	void OnMouseMove(void);
	void OnLeftButtonDown(void);
	void OnLeftButtonUp(void);
	void OnKeyPress(void);

	//
	void Update(void);
	double WorldPointXYPlane[4];
	double WorldSIPoint[4];
	double WorldScaledUnitPoint[4];

	// Used to process events
	vtkCallbackCommand* EventCallbackCommand;

	// Used to associate observers with the interactor
	vtkRenderWindowInteractor *Interactor;

	// Internal ivars for processing events
	vtkRenderer *CurrentRenderer;

	std::string          Name;
	vtkFloatingPointType Radius;
	double               Z;
	int                  Enabled;

	vtkLineSource     *ConnectingLineSource;
	vtkPolyDataMapper *ConnectingMapper;
	vtkActor          *ConnectingActor;

private:
	CPointConnectorActor(const CPointConnectorActor&);  // Not implemented.
	void operator=(const CPointConnectorActor&);  // Not implemented.

protected:
	std::list<vtkSphereSource*>   SphereSources;
	std::list<vtkPolyDataMapper*> PolyDataMappers;
	std::list<vtkActor*>          Actors;

	std::list<vtkLineSource*>     LineSources;
	std::list<vtkPolyDataMapper*> LinePolyDataMappers;
	std::list<vtkTubeFilter*>     TubeFilters;
	std::list<vtkActor*>          LineActors;

	vtkPoints     *Points;
	vtkTransform  *TransformUnits;
	vtkTransform  *TransformScale;
	vtkTransform  *TransformGrid;

	// Do the picking
	vtkCellPicker   *CellPicker;
	vtkCellPicker   *LineCellPicker;
	vtkActor        *CurrentHandle;
	vtkSphereSource *CurrentSource;
	vtkIdType        CurrentId;

	CTreeCtrlNode       Node;
	CTreeMemento       *TreeMemento;

	// 3D Cursor
	vtkCursor3D                    *Cursor3D;
	vtkPolyDataMapper              *Cursor3DMapper;
	vtkActor                       *Cursor3DActor;

	// for updating status bar
	CString HorizonalUnits;
	CString VerticalUnits;

	double  GridOrigin[3];
	double  GridAngle;
};