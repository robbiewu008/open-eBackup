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
#ifndef GENERIC_ENDIAN_H
#define GENERIC_ENDIAN_H
 
#if !defined(_AIX) && !defined(SOLARIS)
#include <endian.h>
#elif defined(SOLARIS)
#include <sys/byteorder.h>
#include <sys/isa_defs.h>

#define htobe16(x) BE_16(x)
#define htole16(x) LE_16(x)
#define be16toh(x) BE_16(x)
#define le16toh(x) LE_16(x)
#define htobe32(x) BE_32(x)
#define htole32(x) LE_32(x)
#define be32toh(x) BE_32(x)
#define le32toh(x) LE_32(x)
#define htobe64(x) BE_64(x)
#define htole64(x) LE_64(x)
#define be64toh(x) BE_64(x)
#define le64toh(x) LE_64(x)

#else
#include <inttypes.h>

#ifndef bswap_16
# define bswap_16(x) \
        ((uint16_t)((((uint16_t) (x) & 0xff00) >> 8) |                  \
                    (((uint16_t) (x) & 0x00ff) << 8)))
#endif /* !bswap_16 */

#ifndef bswap_32
# define bswap_32(x) \
        ((uint32_t)((((uint32_t) (x) & 0xff000000) >> 24) |             \
                    (((uint32_t) (x) & 0x00ff0000) >> 8)  |             \
                    (((uint32_t) (x) & 0x0000ff00) << 8)  |             \
                    (((uint32_t) (x) & 0x000000ff) << 24)))
#endif /* !bswap_32 */

#ifndef bswap_64
# define bswap_64(x) \
        ((uint64_t)((((uint64_t) (x) & 0xff00000000000000ULL) >> 56) |  \
                    (((uint64_t) (x) & 0x00ff000000000000ULL) >> 40) |  \
                    (((uint64_t) (x) & 0x0000ff0000000000ULL) >> 24) |  \
                    (((uint64_t) (x) & 0x000000ff00000000ULL) >> 8)  |  \
                    (((uint64_t) (x) & 0x00000000ff000000ULL) << 8)  |  \
                    (((uint64_t) (x) & 0x0000000000ff0000ULL) << 24) |  \
                    (((uint64_t) (x) & 0x000000000000ff00ULL) << 40) |  \
                    (((uint64_t) (x) & 0x00000000000000ffULL) << 56)))
#endif /* !bswap_64 */

#ifndef htobe16
# if BYTE_ORDER == BIG_ENDIAN

#  define htobe16(x) (x)
#  define htole16(x) bswap_16(x)
#  define be16toh(x) (x)
#  define le16toh(x) bswap_16(x)

# else

#  define htobe16(x) bswap_16(x)
#  define htole16(x) (x)
#  define be16toh(x) bswap_16(x)
#  define le16toh(x) (x)

# endif /* BYTE_ORDER == BIG_ENDIAN */
#endif /* !htobe16 */

#ifndef htobe32
# if BYTE_ORDER == BIG_ENDIAN

#  define htobe32(x) (x)
#  define htole32(x) bswap_32(x)
#  define be32toh(x) (x)
#  define le32toh(x) bswap_32(x)

# else

#  define htobe32(x) bswap_32(x)
#  define htole32(x) (x)
#  define be32toh(x) bswap_32(x)
#  define le32toh(x) (x)

# endif /* BYTE_ORDER == BIG_ENDIAN */
#endif /* !htobe32 */

#ifndef htobe64
# if BYTE_ORDER == BIG_ENDIAN

#  define htobe64(x) (x)
#  define htole64(x) bswap_64(x)
#  define be64toh(x) (x)
#  define le64toh(x) bswap_64(x)

#else

#  define htobe64(x) bswap_64(x)
#  define htole64(x) (x)
#  define be64toh(x) bswap_64(x)
#  define le64toh(x) (x)

# endif /* BYTE_ORDER == BIG_ENDIAN */
#endif /* !htobe64 */
#endif // _AIX
#endif // GENERIC_ENDIAN_H
