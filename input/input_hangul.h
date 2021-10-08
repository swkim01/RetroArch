/*  RetroArch - A frontend for libretro.
 *  Copyright (C) 2010-2014 - Hans-Kristian Arntzen
 *  Copyright (C) 2011-2017 - Daniel De Matteis
 * 
 *  RetroArch is free software: you can redistribute it and/or modify it under the terms
 *  of the GNU General Public License as published by the Free Software Found-
 *  ation, either version 3 of the License, or (at your option) any later version.
 *
 *  RetroArch is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with RetroArch.
 *  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __INPUT_HANGUL__H
#define __INPUT_HANGUL__H

#include <stdint.h>
#include <stdlib.h>
//#include <stddef.h>
//#include <sys/types.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <boolean.h>
#include <retro_common_api.h>
#include <retro_inline.h>
#include <libretro.h>

#include "input_defines.h"

RETRO_BEGIN_DECLS

/**
 * input_driver_find_handle:
 * @index              : index of driver to get handle to.
 *
 * Returns: handle to input driver at index. Can be NULL
 * if nothing found.
 **/
void hangul_reset();
bool input_hangul_process(unsigned short c);
const unsigned short * input_hangul_flush();
bool input_hangul_backspace();
char * hangul_get_preedit_string();
char * hangul_get_commit_string();
unsigned short hangul_get_jamo(unsigned short ch);
char * hangul_get_preedit_string_utf8();
char * hangul_get_commit_string_utf8();
unsigned short utf8_to_unicode(unsigned char* UTF8);

RETRO_END_DECLS

#endif
