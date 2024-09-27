#pragma once

#include "header.h"

#include <initguid.h>
#include <vds.h>



class CVdsVolume;
class CVdsWrapper
{
public:
	CVdsWrapper();
	~CVdsWrapper();

public:
	DWORD Initialize();
	CVdsVolume* GetVdsVolume(const CString& strVolume);

private:
	map<CString, CVdsVolume> m_vecVolume;	
};

class CVdsVolume
{
public:
	CVdsVolume( CComPtr<IVdsVolume> pVolume, const CString& strVolumeName, VDS_VOLUME_TYPE volType);
	~CVdsVolume();

public:
	VDS_VOLUME_TYPE GetVolumeType();
	ULONG GetStripeSize();

private:
	CComPtr<IVdsVolume> m_pVolume;
	CString m_strVolumeName;
	VDS_VOLUME_TYPE m_volType;
};
