#pragma once

#include <vtk3DWidget.h>
#include <map>
#include "WPhastDoc.h"
#include "Units.h"
#include "Zone.h"

class CGridActor;
class vtkCursor3D;
class vtkPolyDataMapper;
class vtkActor;
class CGridKeyword;
class Cube;

class CNewZoneWidget : public vtk3DWidget
{
public:
	// Description:
	// Instantiate the object.
	static CNewZoneWidget *New();

	vtkTypeRevisionMacro(CNewZoneWidget, vtk3DWidget);

	// Description:
	// Methods that satisfy the superclass' API.
	virtual void SetEnabled(int);
	virtual void PlaceWidget(double bounds[6]);

	double* GetBounds(void) { return this->OutlineActor->GetBounds(); }
	void GetBounds(double bounds[6]) { this->OutlineActor->GetBounds(bounds); }

	Cube* GetCube();	

	CWPhastDoc::CoordinateState GetCoordinateMode(void)const;

	void SetGridKeyword(const CGridKeyword& gridKeyword, const CUnits& units);
	void SetCoordinateMode(CWPhastDoc::CoordinateState mode);
	void SetScale(double x, double y, double z);

	void SetCursorColor(double r, double g, double b);
	void SetCursorColor(double a[3]) { this->SetCursorColor(a[0], a[1], a[2]); };

#ifdef WIN32
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
#endif

protected:
	CNewZoneWidget(void);
	//virtual ~CGridElementsSelector(void);
	~CNewZoneWidget(void);

	// Handles the events
	static void ProcessEvents(vtkObject* object, unsigned long event, void* clientdata, void* calldata);

	// ProcessEvents() dispatches to these methods.
	void OnMouseMove();
	void OnLeftButtonDown();
	void OnLeftButtonUp();

	int State;
	enum WidgetState
	{
		Start=0,
		Selecting,
	};

	// 3D Cursor
	vtkCursor3D           *Cursor3D;
	vtkPolyDataMapper     *Cursor3DMapper;
	vtkActor              *Cursor3DActor;

	// wireframe outline
	vtkActor             *OutlineActor;
	vtkPolyDataMapper    *OutlineMapper;
	vtkOutlineSource     *OutlineSource;
	double                StartPoint[3];

	// Mouse state
	double                FixedPlanePoint[3];
	int                   FixedPlane;  // [0-5]
	int                   FixedCoord;  // [0-2]

	// coordinate system vars
	double                       GridOrigin[3];
	double                       GridAngle;
	double                       GeometryScale[3];
	CUnits                       Units;
	CWPhastDoc::CoordinateState  CoordinateMode;
	CZone                        MapZone;

private:
	CNewZoneWidget(const CNewZoneWidget&);  // Not implemented.
	void operator=(const CNewZoneWidget&);   // Not implemented.

};
