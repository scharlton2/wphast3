#pragma once
#include <vtkimplicitplanewidget.h>

class CGridLineWidget : public vtkImplicitPlaneWidget
{
public:
  // Description:
  // Instantiate the object.
  static CGridLineWidget *New();

  vtkTypeRevisionMacro(CGridLineWidget,vtkImplicitPlaneWidget);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual void SetEnabled(int);

protected:
	CGridLineWidget(void);
	~CGridLineWidget(void);

	//handles the events
	static void ProcessEvents(vtkObject* object, unsigned long event,
							void* clientdata, void* calldata);

	// ProcessEvents() dispatches to these methods.
	void OnLeftButtonDown();
	//void OnLeftButtonUp();
	//void OnMiddleButtonDown();
	//void OnMiddleButtonUp();
	//void OnRightButtonDown();
	//void OnRightButtonUp();
	//void OnMouseMove();


private:
  CGridLineWidget(const CGridLineWidget&);  //Not implemented
  void operator=(const CGridLineWidget&);  //Not implemented
};
