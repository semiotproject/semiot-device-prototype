/*
 * Modified header taken from MessagePack implmenetation, original license below:
 * MessagePack system dependencies
 *
 * Copyright (C) 2008-2010 FURUHASHI Sadayuki
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */
#ifndef SYSDEP_H
#define SYSDEP_H
 
#include <stdint.h>

#ifndef _WIN32
#include "lwip/inet.h"  /* __BYTE_ORDER */
#endif

#if !defined(__LITTLE_ENDIAN__) && !defined(__BIG_ENDIAN__)
#if __BYTE_ORDER == __LITTLE_ENDIAN
#define __LITTLE_ENDIAN__
#elif __BYTE_ORDER == __BIG_ENDIAN
#define __BIG_ENDIAN__
#elif _WIN32
#define __LITTLE_ENDIAN__
#endif
#endif

#ifdef __LITTLE_ENDIAN__

#ifdef _WIN32
#  if defined(ntohs)
#    define endian_be16(x) ntohs(x)
#  elif defined(_byteswap_ushort) || (defined(_MSC_VER) && _MSC_VER >= 1400)
#    define endian_be16(x) ((uint16_t)_byteswap_ushort((unsigned short)x))
#  else
#    define endian_be16(x) ( \
        ((((uint16_t)x) <<  8) ) | \
        ((((uint16_t)x) >>  8) ) )
#  endif
#else
#  define endian_be16(x) ntohs(x)
#endif

#ifdef _WIN32
#  if defined(ntohl)
#    define endian_be32(x) ntohl(x)
#  elif defined(_byteswap_ulong) || (defined(_MSC_VER) && _MSC_VER >= 1400)
#    define endian_be32(x) ((uint32_t)_byteswap_ulong((unsigned long)x))
#  else
#    define endian_be32(x) \
        ( ((((uint32_t)x) << 24)               ) | \
          ((((uint32_t)x) <<  8) & 0x00ff0000U ) | \
          ((((uint32_t)x) >>  8) & 0x0000ff00U ) | \
          ((((uint32_t)x) >> 24)               ) )
#  endif
#else
#  define endian_be32(x) ntohl(x)
#endif

#if defined(_byteswap_uint64) || (defined(_MSC_VER) && _MSC_VER >= 1400)
#  define endian_be64(x) (_byteswap_uint64(x))
#elif defined(bswap_64)
#  define endian_be64(x) bswap_64(x)
#elif defined(__DARWIN_OSSwapInt64)
#  define endian_be64(x) __DARWIN_OSSwapInt64(x)
#else
#define endian_be64(x) \
    ( ((((uint64_t)x) << 56)                         ) | \
      ((((uint64_t)x) << 40) & 0x00ff000000000000ULL ) | \
      ((((uint64_t)x) << 24) & 0x0000ff0000000000ULL ) | \
      ((((uint64_t)x) <<  8) & 0x000000ff00000000ULL ) | \
      ((((uint64_t)x) >>  8) & 0x00000000ff000000ULL ) | \
      ((((uint64_t)x) >> 24) & 0x0000000000ff0000ULL ) | \
      ((((uint64_t)x) >> 40) & 0x000000000000ff00ULL ) | \
      ((((uint64_t)x) >> 56)                         ) )
#endif

#define endian_load16(cast, from) ((cast)( \
        (((uint16_t)((uint8_t*)(from))[0]) << 8) | \
        (((uint16_t)((uint8_t*)(from))[1])     ) ))

#define endian_load32(cast, from) ((cast)( \
        (((uint32_t)((uint8_t*)(from))[0]) << 24) | \
        (((uint32_t)((uint8_t*)(from))[1]) << 16) | \
        (((uint32_t)((uint8_t*)(from))[2]) <<  8) | \
        (((uint32_t)((uint8_t*)(from))[3])      ) ))

#define endian_load64(cast, from) ((cast)( \
        (((uint64_t)((uint8_t*)(from))[0]) << 56) | \
        (((uint64_t)((uint8_t*)(from))[1]) << 48) | \
        (((uint64_t)((uint8_t*)(from))[2]) << 40) | \
        (((uint64_t)((uint8_t*)(from))[3]) << 32) | \
        (((uint64_t)((uint8_t*)(from))[4]) << 24) | \
        (((uint64_t)((uint8_t*)(from))[5]) << 16) | \
        (((uint64_t)((uint8_t*)(from))[6]) << 8)  | \
        (((uint64_t)((uint8_t*)(from))[7])     )  ))

#else // __LITTLE_ENDIAN__

#define endian_be16(x) (x)
#define endian_be32(x) (x)
#define endian_be64(x) (x)

#define endian_load16(cast, from) ((cast)( \
        (((uint16_t)((uint8_t*)(from))[0]) << 8) | \
        (((uint16_t)((uint8_t*)(from))[1])     ) ))

#define endian_load32(cast, from) ((cast)( \
        (((uint32_t)((uint8_t*)(from))[0]) << 24) | \
        (((uint32_t)((uint8_t*)(from))[1]) << 16) | \
        (((uint32_t)((uint8_t*)(from))[2]) <<  8) | \
        (((uint32_t)((uint8_t*)(from))[3])      ) ))

#define endian_load64(cast, from) ((cast)( \
        (((uint64_t)((uint8_t*)(from))[0]) << 56) | \
        (((uint64_t)((uint8_t*)(from))[1]) << 48) | \
        (((uint64_t)((uint8_t*)(from))[2]) << 40) | \
        (((uint64_t)((uint8_t*)(from))[3]) << 32) | \
        (((uint64_t)((uint8_t*)(from))[4]) << 24) | \
        (((uint64_t)((uint8_t*)(from))[5]) << 16) | \
        (((uint64_t)((uint8_t*)(from))[6]) << 8)  | \
        (((uint64_t)((uint8_t*)(from))[7])     )  ))
#endif

#define endian_store16(to, num) \
    do { uint16_t val = endian_be16(num); memcpy(to, &val, 2); } while(0)
#define endian_store32(to, num) \
    do { uint32_t val = endian_be32(num); memcpy(to, &val, 4); } while(0)
#define endian_store64(to, num) \
    do { uint64_t val = endian_be64(num); memcpy(to, &val, 8); } while(0)

#endif // SYSDEP_H

