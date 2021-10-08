/*  RetroArch - A frontend for libretro.
 *  Copyright (C) 2010-2014 - Hans-Kristian Arntzen
 *  Copyright (C) 2011-2017 - Daniel De Matteis
 *  Copyright (C) 2017-2017 - Seong-Woo Kim
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

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

//#include <compat/strl.h>
//#include <string/stdstring.h>

//#include <retro_assert.h>

#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif

//#include "input_driver.h"
//#include "input_keymaps.h"
//#include "input_remapping.h"

//#include "../config.def.keybinds.h"

//#include "../msg_hash.h"
//#include "../configuration.h"
//#include "../file_path_special.h"
//#include "../driver.h"
#include "../retroarch.h"
//#include "../list_special.h"
//#include "../verbosity.h"

#ifdef KOREAN

struct HangulBuffer {
    unsigned short choseong;
    unsigned short jungseong;
    unsigned short jongseong;

    unsigned short stack[12];
    int     index;
};

int ic_state = 0;
static struct HangulBuffer hbuffer;
static struct HangulBuffer *buffer = &hbuffer;

unsigned short preedit_string[64];
unsigned short commit_string[64];
unsigned short flushed_string[64];

static const int hangul_choseong[] = {
  0/*NULL*/, 1/*ㄱ*/,   2/*ㄲ*/,   0/*ㄱㅅ*/, 3/*ㄴ*/,
  0/*ㄴㅈ*/, 0/*ㄴㅎ*/, 4/*ㄷ*/,   5/*ㄸ*/,   6/*ㄹ*/,
  0/*ㄹㄱ*/, 0/*ㄹㅁ*/, 0/*ㄹㅂ*/, 0/*ㄹㅅ*/, 0/*ㄹㅌ*/,
  0/*ㄹㅍ*/, 0/*ㄹㅎ*/, 7/*ㅁ*/,   8/*ㅂ*/,   9/*ㅃ*/,
  0/*ㅂㅅ*/, 10/*ㅅ*/,  11/*ㅆ*/,  12/*ㅇ*/,  13/*ㅈ*/,
  14/*ㅉ*/,  15/*ㅊ*/,  16/*ㅋ*/,  17/*ㅌ*/,  18/*ㅍ*/,   19/*ㅎ*/
};
static const int hangul_jongseong[] = {
  0/*NULL*/, 1/*ㄱ*/,   2/*ㄲ*/,   3/*ㄱㅅ*/, 4/*ㄴ*/,
  5/*ㄴㅈ*/, 6/*ㄴㅎ*/, 7/*ㄷ*/,   0/*ㄸ*/,   8/*ㄹ*/,
  9/*ㄹㄱ*/, 10/*ㄹㅁ*/,11/*ㄹㅂ*/,12/*ㄹㅅ*/,13/*ㄹㅌ*/,
  14/*ㄹㅍ*/,15/*ㄹㅎ*/,16/*ㅁ*/,  17/*ㅂ*/,  0/*ㅃ*/,
  18/*ㅂㅅ*/,19/*ㅅ*/,  20/*ㅆ*/,  21/*ㅇ*/,  22/*ㅈ*/,
  0/*ㅉ*/,   23/*ㅊ*/,  24/*ㅋ*/,  25/*ㅌ*/,  26/*ㅍ*/,  27/*ㅎ*/
};

static const int hangul_table[][3] = {
  {  0,  0,  0}, /*0: NULL*/
  {  1,  0,  1}, /*1: ㄱ=ㄱ*/
  {  1,  1,  2}, /*2: ㄱ+ㄱ=ㄲ*/
  {  1, 21,  2}, /*3: ㄱ+ㅅ=ㄱㅅ*/
  {  4,  0,  1}, /*4: ㄴ=ㄴ*/
  {  4, 24,  2}, /*5: ㄴ+ㅈ=ㄴㅈ*/
  {  4, 30,  2}, /*6: ㄴ+ㅎ=ㄴㅎ*/
  {  7,  0,  1}, /*7: ㄷ=ㄷ*/
  {  7,  7,  1}, /*8: ㄷ+ㄷ=ㄸ*/
  {  9,  0,  1}, /*9: ㄹ=ㄹ*/
  {  9,  1,  2}, /*10: ㄹ+ㄱ=ㄹㄱ*/
  {  9, 17,  2}, /*11: ㄹ+ㅁ=ㄹㅁ*/
  {  9, 18,  2}, /*12: ㄹ+ㅂ=ㄹㅂ*/
  {  9, 21,  2}, /*13: ㄹ+ㅅ=ㄹㅅ*/
  {  9, 28,  2}, /*14: ㄹ+ㅌ=ㄹㅌ*/
  {  9, 29,  2}, /*15: ㄹ+ㅍ=ㄹㅍ*/
  {  9, 30,  2}, /*16: ㄹ+ㅎ=ㄹㅎ*/
  { 17,  0,  1}, /*17: ㅁ=ㅁ*/
  { 18,  0,  1}, /*18: ㅂ=ㅂ*/
  { 18, 18,  1}, /*19: ㅂ+ㅂ=ㅃ*/
  { 18, 21,  2}, /*20: ㅂ+ㅅ=ㅂㅅ*/
  { 21, -1,  1}, /*21: ㅅ=ㅅ*/
  { 21, 21,  2}, /*22: ㅅ+ㅅ=ㅆ*/
  { 23,  0,  1}, /*23: ㅇ=ㅇ*/
  { 24,  0,  1}, /*24: ㅈ=ㅈ*/
  { 24, 24,  1}, /*25: ㅈ+ㅈ=ㅉ*/
  { 26,  0,  1}, /*26: ㅊ=ㅊ*/
  { 27,  0,  1}, /*27: ㅋ=ㅋ*/
  { 28,  0,  1}, /*28: ㅌ=ㅌ*/
  { 29,  0,  1}, /*29: ㅍ=ㅍ*/
  { 30,  0,  1}, /*30: ㅎ=ㅎ*/
  { 31,  0,  1}, /*31: ㅏ=ㅏ*/
  { 32,  0,  1}, /*32: ㅐ=ㅐ*/
  { 33,  0,  1}, /*33: ㅑ=ㅑ*/
  { 34,  0,  1}, /*34: ㅒ=ㅒ*/
  { 35,  0,  1}, /*35: ㅓ=ㅓ*/
  { 36,  0,  1}, /*36: ㅔ=ㅔ*/
  { 37,  0,  1}, /*37: ㅕ=ㅕ*/
  { 38,  0,  1}, /*38: ㅖ=ㅖ*/
  { 39,  0,  1}, /*39: ㅗ=ㅗ*/
  { 39, 31,  2}, /*40: ㅗ+ㅏ=ㅘ*/
  { 39, 32,  2}, /*41: ㅗ+ㅐ=ㅙ*/
  { 39, 51,  2}, /*42: ㅗ+ㅣ=ㅚ*/
  { 43,  0,  1}, /*43: ㅛ=ㅛ*/
  { 44,  0,  1}, /*44: ㅜ=ㅜ*/
  { 44, 35,  2}, /*45: ㅜ+ㅓ=ㅝ*/
  { 44, 36,  2}, /*46: ㅜ+ㅔ=ㅞ*/
  { 44, 51,  2}, /*47: ㅜ+ㅣ=ㅟ*/
  { 48,  0,  1}, /*48: ㅠ=ㅠ*/
  { 49,  0,  1}, /*49: ㅡ=ㅡ*/
  { 49, 51,  2}, /*50: ㅡ+ㅣ=ㅢ*/
  { 51,  0,  1}, /*51: ㅣ=ㅣ*/
};

static bool    hangul_buffer_push(unsigned short ch);
static unsigned short hangul_buffer_pop (void);
static unsigned short hangul_buffer_peek(void);

static void    hangul_buffer_clear(void);
static int     hangul_buffer_get_jamo_string(unsigned short *buf, int buflen);

static void    hangul_flush_internal(void);

/**
 * hangul_get_jamo:
 * @idx                : index of driver to get handle to.
 *
 * Returns: handle to input driver at index. Can be NULL
 * if nothing found.
 **/
unsigned short hangul_get_jamo(unsigned short ch)
{
    if (ch>=0x3131&&ch<=0x3163) // 0x3131='ㄱ', 0x3163='ㅣ'
        return ch - 0x3130;
    else
        return 0;
}

bool
hangul_is_jamo(unsigned short ch)
{
    return (ch > 0 && ch <= 94);
}

bool
hangul_is_choseong(unsigned short ch)
{
    return (ch > 0 && ch <= 30 && hangul_choseong[ch]);
}

bool
hangul_is_jongseong(unsigned short ch)
{
    return ((ch > 0 && ch <= 30 && !hangul_jongseong[ch]) ||
            (ch > 64 && ch <= 94));
}

bool
hangul_is_jungseong(unsigned short ch)
{
    return (ch > 30 && ch <= 51);
}

unsigned short hangul_combine(int first, int second) {
    int i;
    unsigned short combine = 0;
    int firstc = first & ~0x40;
    int secondc = second & ~0x40;
    int jong = first & 0x40;

    for (i = firstc; i <= 51 && hangul_table[i][0] == firstc; i++) {
        if (hangul_table[i][1] == secondc) {
            combine = i;
            break;
        }
    }
    if (jong)
        combine = combine | 0x40;
    return combine;
}

unsigned char hangul_choseong_to_jongseong(unsigned char c)
{
    if (c > 0 && c <= 30 && hangul_choseong[c])
        return c | 0x40;
    else
        return 0;
}

unsigned char hangul_jongseong_to_choseong(unsigned char c)
{
    return c & ~0x40;
}

unsigned short hangul_get_diff(int prevjong, int jong) {
    unsigned short cho = 0;

    if (prevjong == 0) {
        cho = hangul_jongseong_to_choseong(jong);
    } else {
        int diff;
        diff = hangul_table[jong & ~0x40][2] - hangul_table[prevjong & ~0x40][2];

        if (diff >= 0 && diff < 2) {
            cho = hangul_table[jong & ~0x40][diff];
        } else if (diff == 2) {
            cho = hangul_jongseong_to_choseong(jong);
        }
    }

    return cho;
}
#if 0
static bool
hangul_buffer_is_empty(void)
{
    return buffer->choseong == 0 && buffer->jungseong == 0 &&
	   buffer->jongseong == 0;
}

static bool
hangul_buffer_has_choseong(void)
{
    return buffer->choseong != 0;
}

static bool
hangul_buffer_has_jungseong(void)
{
    return buffer->jungseong != 0;
}

static bool
hangul_buffer_has_jongseong(void)
{
    return buffer->jongseong != 0;
}
#endif
static bool
hangul_buffer_push(unsigned short ch)
{
    if (!hangul_is_jamo(ch)) {
        hangul_flush_internal();
        return false;
    }
    if (hangul_is_choseong(ch)) {
	buffer->choseong = ch;
    } else if (hangul_is_jungseong(ch)) {
	buffer->jungseong = ch;
    } else if (hangul_is_jongseong(ch)) {
	buffer->jongseong = ch;
    } else {
    }

    buffer->stack[++buffer->index] = ch;
    return true;
}

static unsigned short
hangul_buffer_pop(void)
{
    return buffer->stack[buffer->index--];
}

static unsigned short
hangul_buffer_peek(void)
{
    if (buffer->index < 0)
	return 0;

    return buffer->stack[buffer->index];
}

static void
hangul_buffer_clear(void)
{
    buffer->choseong = 0;
    buffer->jungseong = 0;
    buffer->jongseong = 0;

    buffer->index = -1;
    buffer->stack[0]  = 0;
    buffer->stack[1]  = 0;
    buffer->stack[2]  = 0;
    buffer->stack[3]  = 0;
    buffer->stack[4]  = 0;
    buffer->stack[5]  = 0;
    buffer->stack[6]  = 0;
    buffer->stack[7]  = 0;
    buffer->stack[8]  = 0;
    buffer->stack[9]  = 0;
    buffer->stack[10] = 0;
    buffer->stack[11] = 0;
}

static int
hangul_buffer_get_jamo_string(unsigned short *buf, int buflen)
{
    int n = 0;

    if (buffer->choseong || buffer->jungseong || buffer->jongseong) {
	if (buffer->choseong) {
	    buf[n++] = buffer->choseong;
	} else {
	    buf[n++] = 0x80;
	}
	if (buffer->jungseong) {
	    buf[n++] = buffer->jungseong;
	} else {
	    buf[n++] = 0x80;
	}
	if (buffer->jongseong) {
	    buf[n++] = buffer->jongseong;
	} else {
	    buf[n++] = 0x80;
	}
    }
    buf[n] = 0;

    return n;
}

int unicode_to_utf8(unsigned short uc, char* UTF8)
{
    if (uc <= 0x7f) {
        UTF8[0] = (char) uc;
        return 1;
    }
    else if (uc <= 0x7ff) {
        UTF8[0] = (char) 0xc0 | ((uc >> 6)&0x1f);
        UTF8[1] = (char) 0x80 | (uc & 0x3f);
        UTF8[2] = (char) 0;
        return 3;
    }
    else if (uc <= 0xffff) {
        UTF8[0] = (char) 0xe0 | ((uc >> 12) & 0x0f);
        UTF8[1] = (char) 0x80 | ((uc >> 6) & 0x3f);
        UTF8[2] = (char) 0x80 | (uc & 0x3f);
        return 3;
    }
    return 0;
}

unsigned short utf8_to_unicode(unsigned char* UTF8)
{
    unsigned short uc = 0;

    if (UTF8[0] <= 0x7f) {
        uc = (unsigned short) UTF8[0];
    }
    else if ((UTF8[0] & 0xc0) == 0xc0 && (UTF8[0] & 0xe0) != 0xe0) {
        uc = (unsigned short)(((unsigned short)(UTF8[0] & 0x1f) << 6) | (UTF8[1] & 0x3f));
    }
    else if ((UTF8[0] & 0xf0) == 0xe0) {
        uc = (unsigned short)(((unsigned short)(UTF8[0] & 0x0f) << 12) | ((unsigned short)(UTF8[1] & 0x3f) << 6) | (UTF8[2] & 0x3f));
    }
    return uc;
}

static unsigned short
hangul_get_unicode_from_jamo(unsigned short cho, unsigned short jung, unsigned short jong)
{
    if (cho <= 0 || jung < 0 || jong < 0)
        return 0;
    else if ((jung&~0x80) == 0) {
        //return (0x3131 + ((cho > 0) ? (cho-1) : (jung-1)));
        return (0x3131 + (cho&~0x80) - 1);
    }
    return (0xac00 + 28*21*(hangul_choseong[cho&~0x80]-1) + 28*(jung-31) + (hangul_jongseong[jong&~0xc0]));
}

static int
hangul_get_string_from_jamo(unsigned short *dst, unsigned short *buf, int buflen)
{
    int i, cnt;
    unsigned short uchr;

    for (i=0, cnt=0; i < buflen; i+=3, cnt++) {
        uchr = hangul_get_unicode_from_jamo(buf[i], buf[i+1], buf[i+2]);
        //printf("%04x\n", uchr);
        if (!uchr) break;
        dst[cnt] = uchr;
    }
    dst[cnt] = 0;
    return cnt;
}

static int
hangul_get_utf8_from_jamo(char *dst, unsigned short *buf, int buflen)
{
    int i, cnt, len=0;
    unsigned short uchr;

    for (i=0, cnt=0; i < buflen; i+=3, cnt+=len) {
        uchr = hangul_get_unicode_from_jamo(buf[i], buf[i+1], buf[i+2]);
        //printf("%04x\n", uchr);
        if (!uchr) break;
        len = unicode_to_utf8(uchr, &dst[cnt]);
    }
    dst[cnt] = 0;
    return cnt;
}

static bool
hangul_buffer_backspace()
{
    if (buffer->index >= 0) {
	unsigned short ch = hangul_buffer_pop();
	if (ch == 0)
	    return false;

	if (buffer->index >= 0) {
	    if (hangul_is_choseong(ch)) {
		ch = hangul_buffer_peek();
		buffer->choseong = hangul_is_choseong(ch) ? ch : 0;
		return true;
	    } else if (hangul_is_jungseong(ch)) {
		ch = hangul_buffer_peek();
		buffer->jungseong = hangul_is_jungseong(ch) ? ch : 0;
		return true;
	    } else if (hangul_is_jongseong(ch)) {
		ch = hangul_buffer_peek();
		buffer->jongseong = hangul_is_jongseong(ch) ? ch : 0;
		return true;
	    }
	} else {
	    buffer->choseong = 0;
	    buffer->jungseong = 0;
	    buffer->jongseong = 0;
	    return true;
	}
    }
    return false;
}

static inline void
hangul_save_preedit_string()
{
    hangul_buffer_get_jamo_string(preedit_string, 64);
}

static inline void
hangul_append_commit_string(unsigned short ch)
{
    int i;

    for (i = 0; i < 64; i++) {
	if (commit_string[i] == 0)
	    break;
    }

    if (i + 1 < 64) {
	commit_string[i++] = ch;
	commit_string[i] = 0;
    }
}

static inline void
hangul_save_commit_string()
{
    unsigned short *string = commit_string;
    int len = 64;

    while (len > 0) {
	if (*string == 0)
	    break;
	len--;
	string++;
    }

    hangul_buffer_get_jamo_string(string, len);

    hangul_buffer_clear();
}

bool input_hangul_process(unsigned short c)
{
    unsigned short ch = hangul_get_jamo(c);
    unsigned short jong, combined;

    if (!hangul_is_jamo(ch)) {
        hangul_save_commit_string();
        //hangul_append_commit_string(c);
        return true;
    }
    if (buffer->jongseong) {
        if (hangul_is_choseong(ch)) {
            jong = hangul_choseong_to_jongseong(ch);
            combined = hangul_combine(buffer->jongseong, jong);
            if (hangul_is_jongseong(combined)) {
                if (!hangul_buffer_push(combined)) {
                    if (!hangul_buffer_push(ch)) {
                        return false;
                    }
                }
            } else {
                hangul_save_commit_string();
                if (!hangul_buffer_push(ch)) {
                    return false;
                }
            }
        } else if (hangul_is_jungseong(ch)) {
            unsigned short pop, peek;
            pop = hangul_buffer_pop();
            peek = hangul_buffer_peek();

            if (hangul_is_jongseong(peek)) {
                unsigned short choseong = hangul_get_diff(peek, buffer->jongseong);
                if (choseong == 0) {
                    hangul_save_commit_string();
                    if (!hangul_buffer_push(ch)) {
                        return false;
                    }
                } else {
                    buffer->jongseong = peek;
                    hangul_save_commit_string();
                    hangul_buffer_push(choseong);
                    if (!hangul_buffer_push(ch)) {
                        return false;
                    }
                }
            } else {
                buffer->jongseong = 0;
                hangul_save_commit_string();
                hangul_buffer_push(hangul_jongseong_to_choseong(pop));
                //printf("pop=%02x\n",buffer->choseong);
                if (!hangul_buffer_push(ch)) {
                    return false;
                }
            }
        } else {
            goto flush;
        }
    } else if (buffer->jungseong) {
        if (hangul_is_choseong(ch)) {
            if (buffer->choseong) {
                jong = hangul_choseong_to_jongseong(ch);
                if (hangul_is_jongseong(jong)) {
                    if (!hangul_buffer_push(jong)) {
                        if (!hangul_buffer_push(ch)) {
                            return false;
                        }
                    }
                } else {
                    hangul_save_commit_string();
                    if (!hangul_buffer_push(ch)) {
                        return false;
                    }
                }
            } else {
                if (!hangul_buffer_push(ch)) {
                    if (!hangul_buffer_push(ch)) {
                        return false;
                    }
                }
            }
        } else if (hangul_is_jungseong(ch)) {
            combined = hangul_combine(buffer->jungseong, ch);
            if (hangul_is_jungseong(combined)) {
                if (!hangul_buffer_push(combined)) {
                    return false;
                }
            } else {
                hangul_save_commit_string();
                if (!hangul_buffer_push(ch)) {
                    return false;
                }
            }
        } else {
            goto flush;
        }
    } else if (buffer->choseong) {
        if (hangul_is_choseong(ch)) {
            combined = hangul_combine(buffer->choseong, ch);
            if (!hangul_buffer_push(combined)) {
                if (!hangul_buffer_push(ch)) {
                    return false;
                }
            }
        } else {
            if (!hangul_buffer_push(ch)) {
                if (!hangul_buffer_push(ch)) {
                    return false;
                }
            }
        }
    } else {
        if (!hangul_buffer_push(ch)) {
            return false;
        }
    }

    hangul_save_preedit_string();
    return true;

flush:
    hangul_flush_internal();
    return false;
}

void
hangul_reset()
{
    int i;
    for (i=0; i<64; i++) preedit_string[i] = 0;
    for (i=0; i<64; i++) commit_string[i] = 0;
    for (i=0; i<64; i++) flushed_string[i] = 0;

    hangul_buffer_clear();
}

static void
hangul_flush_internal()
{
    int i;
    for (i=0; i<64; i++) preedit_string[i] = 0;

    hangul_save_commit_string();
    hangul_buffer_clear();
}

const unsigned short *
input_hangul_flush()
{
    // get the remaining string and clear the buffer
    int i;
    for (i=0; i<64; i++) preedit_string[i] = 0;
    for (i=0; i<64; i++) commit_string[i] = 0;
    for (i=0; i<64; i++) flushed_string[i] = 0;

    hangul_buffer_get_jamo_string(flushed_string, 64);
    hangul_buffer_clear();

    return flushed_string;
}

bool
input_hangul_backspace()
{
    int ret;
    int i;
    for (i=0; i<64; i++) preedit_string[i] = 0;
    for (i=0; i<64; i++) commit_string[i] = 0;

    ret = hangul_buffer_backspace();
    if (ret)
	hangul_save_preedit_string();
    return ret;
}

char *
hangul_get_preedit_string()
{
    char *str;

    if (!preedit_string[0]) return NULL;
    str = malloc (strlen((char *)preedit_string) / 3 * 2);
    hangul_get_string_from_jamo((unsigned short *)str, preedit_string, 64);

    return str;
}

char *
hangul_get_commit_string()
{
    char *str;

    if (!commit_string[0]) return NULL;
    str = malloc (strlen((char *)commit_string) / 3 * 2);
    hangul_get_string_from_jamo((unsigned short *)str, commit_string, 64);

    return str;
}

char *
hangul_get_preedit_string_utf8()
{
    char *str;

    if (!preedit_string[0]) return NULL;
    str = malloc (strlen((char *)preedit_string) / 3 * 3 + 1);
    hangul_get_utf8_from_jamo(str, preedit_string, 64);

    return str;
}

char *
hangul_get_commit_string_utf8()
{
    char *str;

    if (!commit_string[0]) return NULL;
    str = malloc (strlen((char *)commit_string) / 3 * 3 + 1);
    hangul_get_utf8_from_jamo(str, commit_string, 64);

    return str;
}

#endif

#ifdef TEST_KOREAN
int main()
{
    //char *test_str="ㄱㅏㄴㅓ";
    //unsigned short *test_str="ㄱㅏ";
    //unsigned char *test_str="ㄱ\0";
    //unsigned char *test_str="가\0";
    //unsigned char *test_str="ㄱㅏ\0";
    //unsigned char *test_str="ㄱㅏㄴ\0";
    //unsigned char *test_str="ㄱㅏㄴㅏ\0";
    //unsigned char *test_str="ㄱㅏㄴㅏㄴ\0";
    //unsigned char *test_str="ㄱㅏㄴㄷㅏ\0";
    //unsigned char *test_str="ㄱㅏㄴㅏㄷㅏ\0";
    //unsigned char *test_str="ㄱㅏㄴㅈㅏㄷㅏ\0";
    unsigned char *test_str="ㄱㅏㄴㅈㅈㅏㄷㅏ\0";
    //unsigned short test_str[]={0x3131, 0};
    //unsigned char test_str[]="ㅏ\0";
    unsigned short *commit;
    unsigned short *preedit;
    unsigned short p;
    int i;
 
    hangul_reset();
    printf("p=%02x%02x%02x\n", test_str[0], test_str[1], test_str[2]);

    //printf("p=%04x%04x\n", p[0],p[1]);
    //while (*p != '\0') {
    //     input_hangul_process(*p);
    //     p++;
    //}
    for (i=0; test_str[i] != 0; i+=3) {
        p = utf8_to_unicode(&test_str[i]);
        printf("p=%04x\n", p);
        input_hangul_process(p);
    }
/*
    p = utf8_to_unicode(test_str);
    printf("p=%04x\n", p);
    input_hangul_process(p);

    p = utf8_to_unicode(&test_str[3]);
    printf("p=%04x\n", p);
    input_hangul_process(p);

    p = utf8_to_unicode(&test_str[6]);
    printf("p=%04x\n", p);
    input_hangul_process(p);

    p = utf8_to_unicode(&test_str[9]);
    printf("p=%04x\n", p);
    input_hangul_process(p);

    p = utf8_to_unicode(&test_str[12]);
    printf("p=%04x\n", p);
    input_hangul_process(p);

    p = utf8_to_unicode(&test_str[15]);
    printf("p=%04x\n", p);
    input_hangul_process(p);

    //p = utf8_to_unicode(&test_str[18]);
    //printf("p=%04x\n", p);
    //input_hangul_process(p);
*/
    preedit = hangul_get_preedit_string();
    commit = hangul_get_commit_string();
    //commit = input_hangul_flush();

    printf("preedit=%x, %x\n", preedit_string[0], preedit_string[1]);
    printf("preedit=%s\n", preedit);
    printf("preedit=%x, %x, %x\n", preedit[0], preedit[1]);
    printf("commit=%x, %x\n", commit_string[0], commit_string[1]);
    printf("commit=%s\n", commit);
    printf("commit=%x, %x, %x\n", commit[0], commit[1], commit[2]);
}
#endif
