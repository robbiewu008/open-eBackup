#ifndef __AGENT_UUID_H__
#define __AGENT_UUID_H__
#ifndef AIX53

#include <vector>

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <Objbase.h>
#include <Guiddef.h>
#elif (defined SOLARIS) || (defined LINUX)
#include <uuid/uuid.h>
#elif defined(HP_UX_IA)
#include <dce/uuid.h>
#else
// AIX
#include <uuid.h>
#endif

#include "common/Types.h"
#include "common/Defines.h"
#include "common/Log.h"

#ifdef WIN32
typedef GUID mp_uuid;
#else
typedef uuid_t mp_uuid;
#endif

class AGENT_API CUuidNum
{
public:
    static mp_int32 GetUuidNumber(mp_string& strUuid);
    static mp_int32 GetUuidStr(mp_string &strUuid);
    static mp_int32 GetUuidStandardStr(mp_string &strUuid);
    static mp_int32 GetUuid(mp_uuid& uuid);
    static mp_int32 CovertStrToUuid(mp_string &strUuid, mp_uuid& uuid);
    static mp_int32 ConvertUuidToStr(mp_uuid& uuid, mp_string& strUuid);
    static mp_int32 CovertStandrdStrToUuid(mp_string &strUuid, mp_uuid& uuid);
    static mp_int32 ConvertUuidToStandardStr(mp_uuid& uuid, mp_string& strUuid);
#ifdef WIN32
    static mp_int32 ConvertGUIDToCharArray(const GUID& pGUID, char pCharBuff[]);
#endif
    static mp_int32 ConvertStrUUIToArray(mp_string& strUUID, mp_char pszCharArray[], mp_uint32 uiLen);
private:
    static mp_int32 FormatUuid(mp_uuid uuid, mp_string& strUuid);
};
#endif

#endif  // __AGENT_UUID_H__
