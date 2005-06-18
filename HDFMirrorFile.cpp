#include "StdAfx.h"
#include "HDFMirrorFile.h"
// #include <afximpl.h>
#include <Shlwapi.h> // PathStripToRoot

void SrcGetRoot(LPCTSTR lpszPath, CString& strRoot)
{
	ASSERT(lpszPath != NULL);

	LPTSTR lpszRoot = strRoot.GetBuffer(_MAX_PATH);
	memset(lpszRoot, 0, _MAX_PATH);
	lstrcpyn(lpszRoot, lpszPath, _MAX_PATH);
	::PathStripToRoot(lpszRoot);
	strRoot.ReleaseBuffer();
}

void SrcTimeToFileTime(const CTime& time, LPFILETIME pFileTime)
{
	SYSTEMTIME sysTime;
	sysTime.wYear = (WORD)time.GetYear();
	sysTime.wMonth = (WORD)time.GetMonth();
	sysTime.wDay = (WORD)time.GetDay();
	sysTime.wHour = (WORD)time.GetHour();
	sysTime.wMinute = (WORD)time.GetMinute();
	sysTime.wSecond = (WORD)time.GetSecond();
	sysTime.wMilliseconds = 0;

	// convert system time to local file time
	FILETIME localTime;
	if (!SystemTimeToFileTime((LPSYSTEMTIME)&sysTime, &localTime))
		CFileException::ThrowOsError((LONG)::GetLastError());

	// convert local file time to UTC file time
	if (!LocalFileTimeToFileTime(&localTime, pFileTime))
		CFileException::ThrowOsError((LONG)::GetLastError());
}

// Modified from CMirrorFile::Open 
// BUGBUG: This obliterates hard links no NTFS
BOOL CHDFMirrorFile::Open(LPCTSTR lpszFileName, UINT nOpenFlags,
	CFileException* pError)
{
	ASSERT(lpszFileName != NULL);
	m_strMirrorName.Empty();

	CFileStatus status;
	if (nOpenFlags & CFile::modeCreate) //opened for writing
	{
		if (CFile::GetStatus(lpszFileName, status))
		{
			CString strRoot;
			SrcGetRoot(lpszFileName, strRoot);

			DWORD dwSecPerClus, dwBytesPerSec, dwFreeClus, dwTotalClus;
			int nBytes = 0;
			if (GetDiskFreeSpace(strRoot, &dwSecPerClus, &dwBytesPerSec, &dwFreeClus,
				&dwTotalClus))
			{
				nBytes = dwFreeClus*dwSecPerClus*dwBytesPerSec;
			}
			if (nBytes > 2*status.m_size) // at least 2x free space avail
			{
				// get the directory for the file
				TCHAR szPath[_MAX_PATH];
				LPTSTR lpszName;
				GetFullPathName(lpszFileName, _MAX_PATH, szPath, &lpszName);
				*lpszName = NULL;

				//let's create a temporary file name
				GetTempFileName(szPath, _T("WPH"), 0,
					m_strMirrorName.GetBuffer(_MAX_PATH+1));
				m_strMirrorName.ReleaseBuffer();
			}
		}
	}

	if (!m_strMirrorName.IsEmpty() &&
		(m_hidFile = ::H5Fcreate(m_strMirrorName, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT)))
//		CFile::Open(m_strMirrorName, nOpenFlags, pError))
	{
		m_strFileName = lpszFileName;
		FILETIME ftCreate, ftAccess, ftModify;
		if (::GetFileTime((HANDLE)m_hFile, &ftCreate, &ftAccess, &ftModify))
		{
			SrcTimeToFileTime(status.m_ctime, &ftCreate);
			SetFileTime((HANDLE)m_hFile, &ftCreate, &ftAccess, &ftModify);
		}

		DWORD dwLength = 0;
		PSECURITY_DESCRIPTOR pSecurityDescriptor = NULL;
		if (GetFileSecurity(lpszFileName, DACL_SECURITY_INFORMATION,
			NULL, dwLength, &dwLength))
		{
			pSecurityDescriptor = (PSECURITY_DESCRIPTOR) new BYTE[dwLength];
			if (::GetFileSecurity(lpszFileName, DACL_SECURITY_INFORMATION,
				pSecurityDescriptor, dwLength, &dwLength))
			{
				SetFileSecurity(m_strMirrorName, DACL_SECURITY_INFORMATION, pSecurityDescriptor);
			}
			delete[] (BYTE*)pSecurityDescriptor;
		}
		return TRUE;
	}
	m_strMirrorName.Empty();
	if (nOpenFlags & CFile::modeCreate) //opened for writing
	{
		m_hidFile = ::H5Fcreate(lpszFileName, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
		if (m_hidFile > 0) return TRUE;
		return FALSE;
	}
	if (nOpenFlags == (CFile::modeRead|CFile::shareDenyWrite))
	{
		//{{NEW 08/06/2003
		BOOL b = CFile::Open(lpszFileName, nOpenFlags, pError);
		if (!b) return b;
		CFile::Close();
		//}}NEW 08/06/2003
		m_hidFile = ::H5Fopen(lpszFileName, H5F_ACC_RDWR, H5P_DEFAULT);
		if (m_hidFile > 0) return TRUE;
		pError->m_cause = CFileException::generic;
		return FALSE;
	}
	ASSERT(FALSE); // TODO
	return FALSE;
	//return CFile::Open(lpszFileName, nOpenFlags, pError);
}

void CHDFMirrorFile::Abort()
{
	herr_t status;
	//CFile::Abort();
	if (this->m_hidFile > 0) {
		status = ::H5Fclose(this->m_hidFile);
		ASSERT(status >= 0);
		this->m_hidFile = -1;
	}
	if (!m_strMirrorName.IsEmpty())
		CFile::Remove(m_strMirrorName);
}

void CHDFMirrorFile::Close()
{
	herr_t status;
	CString strName = m_strFileName; //file close empties string
	// CFile::Close();
	ASSERT(this->m_hidFile > 0);
	if (this->m_hidFile > 0) {
		status = ::H5Fclose(this->m_hidFile);
		ASSERT(status >= 0);
		this->m_hidFile = -1;
	}
	if (!m_strMirrorName.IsEmpty())
	{
		BOOL (__stdcall *pfnReplaceFile)(LPCTSTR, LPCTSTR, LPCTSTR, DWORD, LPVOID, LPVOID);

		HMODULE hModule = GetModuleHandle(_T("KERNEL32"));
		ASSERT(hModule != NULL);

		pfnReplaceFile = (BOOL (__stdcall *)(LPCTSTR, LPCTSTR, LPCTSTR, DWORD, LPVOID, LPVOID))
#ifndef _UNICODE
			GetProcAddress(hModule, "ReplaceFileA");
#else
			GetProcAddress(hModule, "ReplaceFileW");
#endif

		if(!pfnReplaceFile || !pfnReplaceFile(strName, m_strMirrorName, NULL, 0, NULL, NULL))
		{
			CFile::Remove(strName);
			CFile::Rename(m_strMirrorName, strName);
		}
	}
}

////

// COMMENT: {6/17/2005 9:11:13 PM}/*-------------------------------------------------------------------------
// COMMENT: {6/17/2005 9:11:13 PM} * Function:         write_axis
// COMMENT: {6/17/2005 9:11:13 PM} *
// COMMENT: {6/17/2005 9:11:13 PM} * Purpose:          Writes the given vector <name> to the loc_id group.
// COMMENT: {6/17/2005 9:11:13 PM} *
// COMMENT: {6/17/2005 9:11:13 PM} * Preconditions:    loc_id                     open
// COMMENT: {6/17/2005 9:11:13 PM} *
// COMMENT: {6/17/2005 9:11:13 PM} * Postconditions:   "loc_id/<name>" is written to HDF
// COMMENT: {6/17/2005 9:11:13 PM} *-------------------------------------------------------------------------
// COMMENT: {6/17/2005 9:11:13 PM} */
// COMMENT: {6/17/2005 9:11:13 PM}void CHDFMirrorFile::write_axis(hid_t loc_id, double* a, int na, const char* name)
// COMMENT: {6/17/2005 9:11:13 PM}{
// COMMENT: {6/17/2005 9:11:13 PM}    hsize_t dims[1], maxdims[1];
// COMMENT: {6/17/2005 9:11:13 PM}    hid_t dspace_id;
// COMMENT: {6/17/2005 9:11:13 PM}    hid_t dset_id;
// COMMENT: {6/17/2005 9:11:13 PM}    herr_t status;
// COMMENT: {6/17/2005 9:11:13 PM}  
// COMMENT: {6/17/2005 9:11:13 PM}    if (!(na > 0)) return;
// COMMENT: {6/17/2005 9:11:13 PM}  
// COMMENT: {6/17/2005 9:11:13 PM}    /* Create the "/Grid/name" dataspace. */
// COMMENT: {6/17/2005 9:11:13 PM}    dims[0] = maxdims[0] = na;
// COMMENT: {6/17/2005 9:11:13 PM}    dspace_id = H5Screate_simple(1, dims, maxdims);
// COMMENT: {6/17/2005 9:11:13 PM}    
// COMMENT: {6/17/2005 9:11:13 PM}    /* Create the "/Grid/name" dataset */
// COMMENT: {6/17/2005 9:11:13 PM}    dset_id = H5Dcreate(loc_id, name, H5T_NATIVE_FLOAT, dspace_id, H5P_DEFAULT);
// COMMENT: {6/17/2005 9:11:13 PM}    
// COMMENT: {6/17/2005 9:11:13 PM}    /* Write the "/Grid/name" dataset */
// COMMENT: {6/17/2005 9:11:13 PM}    if (H5Dwrite(dset_id, H5T_NATIVE_DOUBLE, dspace_id, H5S_ALL, H5P_DEFAULT, a) < 0) {
// COMMENT: {6/17/2005 9:11:13 PM}        ///sprintf(error_string, "HDF Error: Unable to write \"/%s/%s\" dataset\n", szGrid, name);
// COMMENT: {6/17/2005 9:11:13 PM}        ///error_msg(error_string, STOP);
// COMMENT: {6/17/2005 9:11:13 PM}		return;
// COMMENT: {6/17/2005 9:11:13 PM}    }
// COMMENT: {6/17/2005 9:11:13 PM}    
// COMMENT: {6/17/2005 9:11:13 PM}    /* Close the "/Grid/name" dataset */
// COMMENT: {6/17/2005 9:11:13 PM}    status = H5Dclose(dset_id);
// COMMENT: {6/17/2005 9:11:13 PM}    ASSERT(status >= 0);
// COMMENT: {6/17/2005 9:11:13 PM}  
// COMMENT: {6/17/2005 9:11:13 PM}    /* Close the "/Grid/name" dataspace */
// COMMENT: {6/17/2005 9:11:13 PM}    status = H5Sclose(dspace_id);
// COMMENT: {6/17/2005 9:11:13 PM}    ASSERT(status >= 0);
// COMMENT: {6/17/2005 9:11:13 PM}}

