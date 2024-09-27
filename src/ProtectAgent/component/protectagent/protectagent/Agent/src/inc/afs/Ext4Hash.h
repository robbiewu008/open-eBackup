#ifndef EXT4HASH_H_INCLUDED
#define EXT4HASH_H_INCLUDED

#include "afs/FSCommon.h"

#define DX_HASH_LEGACY 0
#define DX_HASH_HALF_MD4 1
#define DX_HASH_TEA 2
#define DX_HASH_LEGACY_UNSIGNED 3
#define DX_HASH_HALF_MD4_UNSIGNED 4
#define DX_HASH_TEA_UNSIGNED 5

#ifdef CPPUNIT_MAIN
u32 dx_hack_hash_signed(const char *name, int len);
void TEA_transform(u32 buf[4], u32 const in[]);
u32 dx_hack_hash_unsigned(const char *name, int len);
void str2hashbuf_signed(const char *msg, int len, u32 *buf, int num);
void str2hashbuf_unsigned(const char *msg, int len, u32 *buf, int num);
#endif

/* hash info structure used by the directory hash
 */
struct dx_hash_info {
    uint32_t hash;
    uint32_t minor_hash;
    int32_t hash_version;
    uint32_t *seed;
};

/* * \brief ext4 计算Hash值
 *
 * \param name const char*
 * \param len int  长度
 * \param hinfo struct dx_hash_info* Hash信息结构体指针
 * \return int 0 成功 -1 失败
 *
 */
int ext_calcDirHashValue(const char *name, int len, struct dx_hash_info *hinfo);

#endif // EXT4HASH_H_INCLUDED
