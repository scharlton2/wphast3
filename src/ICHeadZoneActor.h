#pragma once
#include "ICZoneActor.h"

class CICHeadZoneActor : public CICZoneActor
{
public:
	vtkTypeRevisionMacro(CICHeadZoneActor,CICZoneActor);
	static CICHeadZoneActor *New();
	static void Create(CWPhastDoc* pWPhastDoc, const CZone& zone, const CHeadIC& headIC, LPCTSTR desc);

	CHeadIC GetData(void)const;
	void SetData(const CHeadIC& rHeadIC);

	virtual void Insert(CPropertyTreeControlBar* pTreeControlBar, HTREEITEM hInsertAfter = TVI_LAST);
	virtual void Update(CTreeCtrl* pTreeCtrl, HTREEITEM htiParent);
	virtual void Edit(CTreeCtrl* pTreeCtrl);
	virtual void Add(CWPhastDoc *pWPhastDoc);
	virtual void Remove(CWPhastDoc *pWPhastDoc);

	static const char szHeading[];

	void Serialize(bool bStoring, hid_t loc_id, const CUnits& units);

protected:
	CICHeadZoneActor(void);
	virtual ~CICHeadZoneActor(void);
	CHeadIC m_headIC;

private:
	CICHeadZoneActor(const CICHeadZoneActor&);  // Not implemented.
	void operator=(const CICHeadZoneActor&);  // Not implemented.
};