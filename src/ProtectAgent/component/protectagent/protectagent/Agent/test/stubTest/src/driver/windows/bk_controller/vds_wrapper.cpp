/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
#include "stdafx.h"
#include "vds_wrapper.h"





vector< CComPtr<IUnknown> > EnumerateVdsObjects(IEnumVdsObject * pEnumeration);


CVdsWrapper::CVdsWrapper()
{
	CoInitialize(NULL);
}

CVdsWrapper::~CVdsWrapper()
{
	//CoUninitialize();
}

DWORD CVdsWrapper::Initialize()
{
	DWORD dwRet = 0;
	do
	{
		CComPtr<IVdsServiceLoader> pServiceLoader = NULL;
		HRESULT hr = CoCreateInstance(CLSID_VdsLoader, NULL, CLSCTX_LOCAL_SERVER, IID_IVdsServiceLoader, (LPVOID*)&pServiceLoader);
		if (FAILED(hr))
		{
			dwRet = hr;
			break;
		}

		CComPtr<IVdsService> pService = NULL;
		hr = pServiceLoader->LoadService(NULL, &pService);
		if (FAILED(hr))
		{
			dwRet = hr;
			break;
		}

		hr = pService->WaitForServiceReady();
		if (FAILED(hr))
		{
			dwRet = hr;
			break;
		}

		CComPtr<IEnumVdsObject> pEnumProvider = NULL;
		pService->QueryProviders(VDS_QUERY_SOFTWARE_PROVIDERS, &pEnumProvider);
		if (FAILED(hr))
		{
			dwRet = hr;
			break;
		}

		vector< CComPtr<IUnknown> > vecProvider = EnumerateVdsObjects(pEnumProvider);

		for (size_t iProvider = 0; iProvider < vecProvider.size(); iProvider++)
		{
			CComQIPtr<IVdsSwProvider> pSwProvider = vecProvider[iProvider];

			CComPtr<IEnumVdsObject> pEnumPack = NULL;
			hr = pSwProvider->QueryPacks(&pEnumPack);
			if (FAILED(hr))
			{
				dwRet = hr;
				break;
			}

			vector< CComPtr<IUnknown> > vecPacks = EnumerateVdsObjects(pEnumPack);
			for (size_t iPack = 0; iPack < vecPacks.size(); iPack++)
			{
				CComQIPtr<IVdsPack> pPack = vecPacks[iPack];

				CComPtr<IEnumVdsObject> pEnumVolumes = NULL;
				hr = pPack->QueryVolumes(&pEnumVolumes);
				if (FAILED(hr))
				{
					dwRet = hr;
					break;
				}

				vector< CComPtr<IUnknown> > vecVolume = EnumerateVdsObjects(pEnumVolumes);
				for (size_t iVolume = 0; iVolume < vecVolume.size(); iVolume++)
				{
					CComQIPtr<IVdsVolume> pVolume = vecVolume[iVolume];

					VDS_VOLUME_PROP volProp = { 0 };
					hr = pVolume->GetProperties(&volProp);
					if (FAILED(hr))
					{
						dwRet = hr;
						break;
					}

					if ((volProp.status == VDS_VS_FAILED) && (volProp.health == VDS_H_FAILED) || !volProp.pwszName)
					{
						continue;
					}

					m_vecVolume.insert(make_pair(volProp.pwszName, CVdsVolume(pVolume, volProp.pwszName, volProp.type)));
				}
			}

			if (FAILED(hr))
			{
				dwRet = hr;
				break;
			}
		}
		
		if (FAILED(hr))
		{
			dwRet = hr;
			break;
		}
	} while (0);

	return dwRet;
}

CVdsVolume* CVdsWrapper::GetVdsVolume(const CString& strVolume)
{
	CString strTemp = L"\\\\?\\GLOBALROOT" + strVolume;
	map<CString, CVdsVolume>::iterator it = m_vecVolume.find(strTemp);
	if (it == m_vecVolume.end())
	{
		return NULL;
	}

	return (&it->second);
}



CVdsVolume::CVdsVolume(CComPtr<IVdsVolume> pVolume, const CString& strVolumeName, VDS_VOLUME_TYPE volType)
{
	m_pVolume = pVolume;
	m_strVolumeName = strVolumeName;
	m_volType = volType;
}

CVdsVolume::~CVdsVolume()
{
}

VDS_VOLUME_TYPE CVdsVolume::GetVolumeType()
{
	return m_volType;
}

ULONG CVdsVolume::GetStripeSize()
{
	ULONG ret = -1;
	do
	{
		if (m_volType != VDS_VT_STRIPE)
		{
			break;
		}

		CComPtr<IEnumVdsObject> pEnumPlexes = NULL;
		HRESULT hr = m_pVolume->QueryPlexes(&pEnumPlexes);
		if (FAILED(hr))
		{
			break;
		}

		vector< CComPtr<IUnknown> > vecPlex = EnumerateVdsObjects(pEnumPlexes);
		if (vecPlex.size() != 1)
		{
			break;
		}

		CComQIPtr<IVdsVolumePlex> pPlex = vecPlex[0];

		VDS_VOLUME_PLEX_PROP plexProp = { 0 };
		hr = pPlex->GetProperties(&plexProp);
		if (FAILED(hr))
		{
			break;
		}

		ret = plexProp.ulStripeSize;
	} while (0);

	return ret;
}





vector< CComPtr<IUnknown> > EnumerateVdsObjects(IEnumVdsObject * pEnumeration)
{
	vector< CComPtr<IUnknown> > vecObject;

	while (1)
	{
		CComPtr<IUnknown> pUnknown;
		ULONG ulFetched = 0;
		HRESULT hr = pEnumeration->Next(1, &pUnknown, &ulFetched);
		if (FAILED(hr))
		{
			break;
		}

		if (ulFetched == 0)
		{
			break;
		}

		vecObject.push_back(pUnknown);
	}

	return vecObject;
}
