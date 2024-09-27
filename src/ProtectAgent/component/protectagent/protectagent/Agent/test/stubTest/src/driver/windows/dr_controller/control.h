#pragma once
#include "const.h"

#include "..\..\..\..\..\src\inc\driver\share\ctl_define.h"



typedef struct tagIoctlMessage
{
	union
	{
		ProtectStrategy protect_strategy;
		ProtectVol protect_vol;
		NotifyChange notify_change;
		QueryStart query_start;
	}u;
}IoctlMessage;
