#ifndef _INC_TIMESERIES_
#define _INC_TIMESERIES_

#include <map>

#if defined(_MT)
#define _HDF5USEDLL_     /* reqd for Multithreaded run-time library (Win32) */
#endif
#include <hdf5.h>        /* HDF routines */

#include "time.h"
#include "property.h"

template<typename T>
class CTimeSeries : public std::map<Ctime, T>
{
public:
	CTimeSeries& operator=(const struct time_series& rhs);
	void AssertValid(void)const;
	void Serialize(bool bStoring, hid_t loc_id);

	static void SerializeCreate(const char *heading, CTimeSeries& series, hid_t loc_id);
	static void SerializeOpen(const char *heading, CTimeSeries& series, hid_t loc_id);

	HTREEITEM InsertItem(CTreeCtrl* pTreeCtrl, LPCTSTR lpszHeading, HTREEITEM hParent = TVI_ROOT, HTREEITEM hInsertAfter = TVI_LAST)const;
};

// COMMENT: {4/19/2005 4:01:02 PM}template<>
// COMMENT: {4/19/2005 4:01:02 PM}inline void CTimeSeries<Cproperty>::SerializeCreate(const char *heading, CTimeSeries<Cproperty>& series, hid_t loc_id)
// COMMENT: {4/19/2005 4:01:02 PM}{
// COMMENT: {4/19/2005 4:01:02 PM}	hid_t group_id = ::H5Gcreate(loc_id, heading, 0);
// COMMENT: {4/19/2005 4:01:02 PM}	ASSERT(group_id > 0);
// COMMENT: {4/19/2005 4:01:02 PM}	if (group_id > 0)
// COMMENT: {4/19/2005 4:01:02 PM}	{
// COMMENT: {4/19/2005 4:01:02 PM}		series.Serialize(true, group_id);
// COMMENT: {4/19/2005 4:01:02 PM}		::H5Gclose(group_id);
// COMMENT: {4/19/2005 4:01:02 PM}	}
// COMMENT: {4/19/2005 4:01:02 PM}}
// COMMENT: {4/19/2005 4:01:02 PM}
// COMMENT: {4/19/2005 4:01:02 PM}template<>
// COMMENT: {4/19/2005 4:01:02 PM}inline void CTimeSeries<Cproperty>::SerializeOpen(const char *heading, CTimeSeries<Cproperty>& series, hid_t loc_id)
// COMMENT: {4/19/2005 4:01:02 PM}{
// COMMENT: {4/19/2005 4:01:02 PM}	hid_t group_id = ::H5Gopen(loc_id, heading);
// COMMENT: {4/19/2005 4:01:02 PM}	if (group_id > 0)
// COMMENT: {4/19/2005 4:01:02 PM}	{
// COMMENT: {4/19/2005 4:01:02 PM}		series.Serialize(false, group_id);
// COMMENT: {4/19/2005 4:01:02 PM}		::H5Gclose(group_id);
// COMMENT: {4/19/2005 4:01:02 PM}	}
// COMMENT: {4/19/2005 4:01:02 PM}}

template<typename T>
inline void CTimeSeries<T>::SerializeCreate(const char *heading, CTimeSeries<T>& series, hid_t loc_id)
{
	hid_t group_id = ::H5Gcreate(loc_id, heading, 0);
	ASSERT(group_id > 0);
	if (group_id > 0)
	{
		series.Serialize(true, group_id);
		::H5Gclose(group_id);
	}
}

template<typename T>
inline void CTimeSeries<T>::SerializeOpen(const char *heading, CTimeSeries<T>& series, hid_t loc_id)
{
	hid_t group_id = ::H5Gopen(loc_id, heading);
	if (group_id > 0)
	{
		series.Serialize(false, group_id);
		::H5Gclose(group_id);
	}
}

#endif