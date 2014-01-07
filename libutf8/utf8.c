/* utf8.c
 **
 ** This program is distributed under the GNU Library General Public License, version 2.
 ** A copy of this license is included with this source.
 **
 ** Copyright 2001, Warren Spits <spits@cyberdude.com>
 **
 ** UTF-8 conversion functions. For details see RFC2279.
 **
 **/

#include "utf8.h"
#include <windows.h>

/* Converts a UTF-8 character sequence to a UCS-4 character */
int _utf8_to_ucs4(unsigned int *target, const char *utf8, int n)
{
	unsigned int result = 0;
	int count;
	int i;

	/* Determine the number of characters in sequence */
	if ((*utf8 & 0x80) == 0)
		count = 1;
	else if ((*utf8 & 0xE0) == 0xC0)
		count = 2;
	else if ((*utf8 & 0xF0) == 0xE0)
		count = 3;
	else if ((*utf8 & 0xF8) == 0xF0)
		count = 4;
	else if ((*utf8 & 0xFC) == 0xF8)
		count = 5;
	else if ((*utf8 & 0xFE) == 0xFC)
		count = 6;
	else
		return UTF8_ILSEQ; /* Invalid start byte */

	if (n < count)
		return UTF8_ILSEQ; /* Not enough characters */

	if (count == 2 && (*utf8 & 0x1E) == 0)
		return UTF8_ILSEQ; /* Overlong sequence */

	/* Convert the first character */
	if (count == 1)
		result = *utf8;
	else
		result = (0xFF >> (count +1)) & *utf8;

	/* Convert the continuation bytes */
	for (i = 1; i < count; i++)
	{
		if ((utf8[i] & 0xC0) != 0x80)
			return UTF8_ILSEQ; /* Not a continuation byte */
		if (result == 0 &&
			i == 2 &&
			((utf8[i] & 0x7F) >> (7 - count)) == 0)
			return UTF8_ILSEQ; /* Overlong sequence */
		result = (result << 6) | (utf8[i] & 0x3F);
	}

	if (target != 0)
		*target = result;

	return count;
}

/* Converts a UCS-4 character to a UTF-8 character sequence */
int _ucs4_to_utf8(char *target, unsigned int ucs4)
{
	int count;

	/* Determine the bytes required */
	if (ucs4 < 0x80)
		count = 1;
	else if (ucs4 < 0x800)
		count = 2;
	else if (ucs4 < 0x10000)
		count = 3;
	else if (ucs4 < 0x200000)
		count = 4;
	else if (ucs4 < 0x4000000)
		count = 5;
	else if (ucs4 <= 0x7FFFFFFF)
		count = 6;
	else
		return UTF8_ILSEQ;

	if (target == 0)
		return count;

	/* Encode the UCS-4 value - fall through cases */
	switch (count)
	{
    case 6:
		target[5] = 0x80 | (ucs4 & 0x3F);
		ucs4 = ucs4 >> 6;
		ucs4 |= 0x4000000;
    case 5:
		target[4] = 0x80 | (ucs4 & 0x3F);
		ucs4 = ucs4 >> 6;
		ucs4 |= 0x200000;
    case 4:
		target[3] = 0x80 | (ucs4 & 0x3F);
		ucs4 = ucs4 >> 6;
		ucs4 |= 0x10000;
    case 3:
		target[2] = 0x80 | (ucs4 & 0x3F);
		ucs4 = ucs4 >> 6;
		ucs4 |= 0x800;
    case 2:
		target[1] = 0x80 | (ucs4 & 0x3F);
		ucs4 = ucs4 >> 6;
		ucs4 |= 0xC0;
	case 1:
		target[0] = ucs4;
	}

	return count;
}

/* Converts a WCHAR string to a UTF-8 string */
int WideCharToUTF8(LPSTR target, LPCWSTR str)
{
	WCHAR current;
	int count = 0;
	int conv;

	do
	{
		current = *(str++);
		if (current >= 0xd800 && current < 0xe000)
			return UTF8_ILSEQ; /* Can't handle surrogate pairs */
		conv = _ucs4_to_utf8(target, current);
		if (conv == UTF8_ILSEQ) return UTF8_ILSEQ;
		if (target != 0) target += conv;
		count += conv;
	} while (current != 0x0000);

	return count;
}

/* Converts a UTF-8 string to a WCHAR string */
int UTF8ToWideChar(LPWSTR target, LPCSTR utf8)
{
	int wcount = 0;
	int conv;
	unsigned int ucs4;
	int count = lstrlenA(utf8) +1;

	while (count != 0)
	{
		conv = _utf8_to_ucs4(&ucs4, utf8, count);
		if (conv == UTF8_ILSEQ) return UTF8_ILSEQ;
		if (ucs4 > 0xFFFF)
			return UTF8_ILSEQ; /* Can only handle the BMP */
		if (target != 0)
		{
			*target = (WCHAR) ucs4;
			target++;
		}
		wcount++;
		count -= conv;
		utf8 += conv;
	}

	return wcount;
}
