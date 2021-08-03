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
 * @brief File used for reading config files.
 * @author Nils Egger
 *
 * @addtogroup Config 
 * @{
 */

#ifndef RADICLE_LIBCONFIG_INCLUDE_RADICLE_CONFIG_H
#define RADICLE_LIBCONFIG_INCLUDE_RADICLE_CONFIG_H

#include "jansson.h"

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @brief Loads data from config file and stores it in json format.
 *
 * @param file Path to config file.
 * @param data Double pointer to json pointer.
 *
 * @returns Returns 0 on success.
 */
int radicle_load_config(const char* file, json_t** data);

#if defined(__cplusplus)
}
#endif

#endif // RADICLE_LIBCONFIG_INCLUDE_RADICLE_CONFIG_H

/** @} */
