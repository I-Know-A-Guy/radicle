/* LIBRADICLE - The Radicle Library
 * Copyright (C) 2021 Nils Egger <nilsxegger@gmail.com>
 *
 * This library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <https://www.gnu.org/licenses/>.
 */

/**
 * @file
 * @brief Contains macros for debug printing with source file and line.
 * @author Nils Egger
 *
 * @addtogroup Common 
 * @{
 * @addtogroup Print
 * @{
 */

#ifndef RADICLE_COMMON_INCLUDE_RADICLE_PRINT_H
#define RADICLE_COMMON_INCLUDE_RADICLE_PRINT_H

#include <stdio.h>

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @brief If ALLOW_PRINT_DEBUG is set to 0, all DEBUG calls will result in nothing.
 */
#define ALLOW_PRINT_DEBUG 1

/**
 * @brief If ALLOW_PRINT_INFO is set to 0, all INFO calls will result in nothing.
 */
#define ALLOW_PRINT_INFO 1

/**
 * @brief If ALLOW_PRINT_ERROR is set to 0, all ERROR calls will result in nothing.
 */
#define ALLOW_PRINT_ERROR 1

/**
 * @brief Prints a formatted message to any output. Flushes after print for immediate output.
 *
 * @param io Output stream to print to.
 * @param fmt Format to print.
 * @param args Args to insert into format.
 */
#define PRINT_ANY(io, fmt, args...)\
	fprintf(io, "%s:%d:%s(): " fmt, __FILE__, __LINE__, __func__, ##args);\
	fflush(io)

#if defined(ALLOW_PRINT_DEBUG) && ALLOW_PRINT_DEBUG > 0
/**
 * @brief Calls \ref PRINT_ANY with stdout as stream.
 */
#define DEBUG(fmt, args...) PRINT_ANY(stdout, fmt, ##args)
#else
#define DEBUG(fmt, args...)
#endif

#if defined(ALLOW_PRINT_INFO) && ALLOW_PRINT_INFO > 0
/**
 * @brief Calls \ref PRINT_ANY with stdout as stream.
 */
#define INFO(fmt, args...) PRINT_ANY(stdout, fmt, ##args)
#else
#define INFO(fmt, args...)
#endif

#if defined(ALLOW_PRINT_ERROR) && ALLOW_PRINT_ERROR > 0
/**
 * @brief Calls \ref PRINT_ANY with stderr as stream.
 */
#define ERROR(fmt, args...) PRINT_ANY(stderr, fmt, ##args)
#else
#define ERROR(fmt, args...)
#endif

#if defined(__cplusplus)
}
#endif

#endif // RADICLE_COMMON_INCLUDE_RADICLE_PRINT_H

/** @} */
/** @} */
