/* utf8.h
 **
 ** This program is distributed under the GNU Library General Public License, version 2.
 ** A copy of this license is included with this source.
 **
 ** Copyright 2001, Warren Spits <spits@cyberdude.com>
 **
 ** UTF-8 conversion functions. For details see RFC2279.
 **
 **/

#ifndef _UTF8_H_INCLUDED
#define _UTF8_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include <windows.h>

#define UTF8_ILSEQ	-1

int WideCharToUTF8(LPSTR target, LPCWSTR str);
int UTF8ToWideChar(LPWSTR target, LPCSTR utf8);

#ifdef __cplusplus
}
#endif

#endif /* _UTF8_H_INCLUDED */
