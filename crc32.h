/*
 * crc32.h
 * 
 * This file is a part of NSIS.
 * 
 * Copyright (C) 1999-2023 Nullsoft and Contributors
 * 
 * Licensed under the zlib/libpng license (the "License");
 * you may not use this file except in compliance with the License.
 * 
 * Licence details can be found in the file COPYING.
 * 
 * This software is provided 'as-is', without any express or implied
 * warranty.
 *
 * Reviewed for Unicode support by Jim Park -- 08/24/2007
 */

#ifndef ___CRC32__H___
#define ___CRC32__H___

#include "Platform.h"
#include <stddef.h> // size_t

typedef UINT32 crc32_t;

#define NSIS_CONFIG_CRC_SUPPORT

#ifdef __cplusplus
extern "C"
#endif
crc32_t CRC32(crc32_t crc, const unsigned char *buf, size_t len);

#endif//!___CRC32__H___
