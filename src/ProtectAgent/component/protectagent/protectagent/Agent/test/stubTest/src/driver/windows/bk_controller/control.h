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
#include "const.h"

#include "..\..\..\..\..\src\inc\driver\share\ctl_define.h"



typedef struct tagIoctlMessage
{
	union
	{
		ProtectStrategy protect_strategy;
		ProtectVol protect_vol;
		TakeSnapshot take_snapshot;
		RemoveSnapshot rm_snapshot;
		GetBitmap get_bitmap;
	}u;
}IoctlMessage;
