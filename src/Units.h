#pragma once

#if defined(_MT)
#define _HDF5USEDLL_     /* reqd for Multithreaded run-time library (Win32) */
#endif
#include <hdf5.h>        /* HDF routines */

extern "C" {
#define EXTERNAL extern
#include "srcinput/hstinpt.h"
#undef EXTERNAL
}

#include <iosfwd> // std::ostream

class CUnits :
	public units
{
public:
	// ctor
	CUnits(void);

	// copy ctor
	CUnits(const struct units& src);

	// copy assignment
	CUnits& operator=(const CUnits& rhs);
	CUnits& CUnits::operator=(const struct units& rhs);

	void Insert(CTreeCtrl* pTreeCtrl, HTREEITEM htiUnits);
	void Serialize(bool bStoring, hid_t loc_id);
	void Edit(CTreeCtrl* pTreeCtrl);
	HTREEITEM GetTreeItem(void)const { return m_htiUnits; }
protected:
	friend std::ostream& operator<< (std::ostream &os, const CUnits &a);
	HTREEITEM m_htiUnits;
};