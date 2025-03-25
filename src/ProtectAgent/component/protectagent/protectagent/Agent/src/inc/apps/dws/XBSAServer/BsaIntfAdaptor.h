#ifndef _BSA_INTF_ADAPTOR_H_
#define _BSA_INTF_ADAPTOR_H_

#include "common/Types.h"
#include "xbsa/xbsa.h"
#include "apps/dws/XBSAServer/xbsa_types.h"
#include "apps/dws/XBSAServer/BsaObjManager.h"

class BsaIntfAdaptor {
public:
    static mp_uint64 BsaU64ToU64(const BsaUInt64 &b64);
    static void U64ToBsaU64(mp_uint64 u64, BsaUInt64 &b64);

    static mp_bool HandleValid(const int64_t handle);
    static mp_bool StringValid(const std::string &str, mp_uint64 maxLen, mp_bool canBeEmpty = MP_TRUE);
    static mp_bool BsaObjectOwnerValid(const std::string &bsaObjectOwner, mp_bool canBeEmpty = MP_TRUE);
    static mp_bool AppObjectOwnerValid(const std::string &appObjectOwner, mp_bool canBeEmpty = MP_TRUE);
    static mp_bool ObjectSpaceNameValid(const std::string &objectSpaceName, mp_bool canBeEmpty = MP_TRUE);
    static mp_bool PathNameValid(const std::string &pathName, mp_bool canBeEmpty = MP_TRUE);
    static mp_bool ResourceTypeValid(const std::string &resourceType, mp_bool canBeEmpty = MP_TRUE);
    static mp_bool ObjectDescriptionValid(const std::string &objectDescription, mp_bool canBeEmpty = MP_TRUE);
    static mp_bool ObjectInfoValid(const std::string &objectInfo, mp_bool canBeEmpty = MP_TRUE);

    static mp_bool CopyTypeValid(const int32_t copyType, mp_bool canBeAny = MP_TRUE);
    static mp_bool ObjectTypeValid(const int32_t objectType, mp_bool canBeAny = MP_TRUE);
    static mp_bool ObjectStatusValid(const int32_t objectStatus, mp_bool canBeAny = MP_TRUE);
    static mp_bool VoteValid(const int32_t vote);

    static mp_void ConvertCreateReqObj(const BsaObjectDescriptor &src, BsaObjInfo &dst,
        const BSA_ObjectOwner &sessionOwner);
    static mp_void ConvertCreateRspObj(const BsaObjInfo &src, BsaObjectDescriptor &dst,
        const BSA_ObjectOwner &sessionOwner);

    static mp_void ConvertQueryReqObj(mp_long sessionId, const BsaQueryDescriptor &src, BsaObjInfo &dst,
        const BSA_ObjectOwner &sessionOwner);
    static mp_void ConvertQueryRspObj(const BsaObjInfo &src, BsaObjectDescriptor &dst);
};


#endif // _BSA_INTF_ADAPTOR_H_