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
