// Copyright 2025 HarvestX Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef H6X_DYNAMIC_PACKET_HANDLER__H6X_DYNAMIC_PACKET_BIG_ENDIAN_HPP
#define H6X_DYNAMIC_PACKET_HANDLER__H6X_DYNAMIC_PACKET_BIG_ENDIAN_HPP

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

static inline int is_little_endian(void)
{
  uint16_t test = 0x0001;
  return *(uint8_t *)&test == 1;
}

static inline uint16_t swap_uint16(uint16_t value)
{
  return __builtin_bswap16(value);
}

static inline uint32_t swap_uint32(uint32_t value)
{
  return __builtin_bswap32(value);
}

static inline uint64_t swap_uint64(uint64_t value)
{
  return __builtin_bswap64(value);
}

static inline uint8_t big_endian_uint8(uint8_t value)
{
  return value;
}

static inline int8_t big_endian_int8(int8_t value)
{
  return value;
}

static inline uint16_t big_endian_uint16(uint16_t value)
{
  return is_little_endian() ? swap_uint16(value) : value;
}

static inline int16_t big_endian_int16(int16_t value)
{
  return (int16_t)big_endian_uint16((uint16_t)value);
}

static inline uint32_t big_endian_uint32(uint32_t value)
{
  return is_little_endian() ? swap_uint32(value) : value;
}

static inline int32_t big_endian_int32(int32_t value)
{
  return (int32_t)big_endian_uint32((uint32_t)value);
}

static inline uint64_t big_endian_uint64(uint64_t value)
{
  return is_little_endian() ? swap_uint64(value) : value;
}

static inline int64_t big_endian_int64(int64_t value)
{
  return (int64_t)big_endian_uint64((uint64_t)value);
}

static inline float big_endian_float(float value)
{
  union { float f; uint32_t i; } converter;
  converter.f = value;
  converter.i = big_endian_uint32(converter.i);
  return converter.f;
}

static inline double big_endian_double(double value)
{
  union { double d; uint64_t i; } converter;
  converter.d = value;
  converter.i = big_endian_uint64(converter.i);
  return converter.d;
}

static inline intptr_t big_endian_intptr(intptr_t value)
{
  return (intptr_t)big_endian_uint64((uint64_t)value);
}

static inline uintptr_t big_endian_uintptr(uintptr_t value)
{
  return (uintptr_t)big_endian_uint64((uint64_t)value);
}

#if __STDC_VERSION__ >= 201112L
#define big_endian(x) _Generic( \
    (x), \
    int8_t:    big_endian_int8, \
    uint8_t:   big_endian_uint8, \
    int16_t:   big_endian_int16, \
    uint16_t:  big_endian_uint16, \
    int32_t:   big_endian_int32, \
    uint32_t:  big_endian_uint32, \
    int64_t:   big_endian_int64, \
    uint64_t:  big_endian_uint64, \
    float :     big_endian_float, \
    double :    big_endian_double, \
    intptr_t:  big_endian_intptr, \
    uintptr_t: big_endian_uintptr \
)(x)
#endif

#ifdef __cplusplus
}
#endif

#endif // H6X_DYNAMIC_PACKET_HANDLER__H6X_DYNAMIC_PACKET_BIG_ENDIAN_HPP
