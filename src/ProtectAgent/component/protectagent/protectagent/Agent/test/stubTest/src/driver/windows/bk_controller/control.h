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
