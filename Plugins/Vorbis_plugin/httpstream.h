/*********************************************
*
*  httpstream.h
*
*  HTTP streaming support for OggVorbis by Aaron Porter <aaron@javasource.org>
*  Licensed under terms of the LGPL
*
*********************************************/

#ifndef _OGG_VORBIS_HTTP_STREAMING_
#define _OGG_VORBIS_HTTP_STREAMING_

void    httpInit();
void    httpSetHwnd(HWND hWnd);
void    httpSetProxy(const char *proxy);
void    httpShutdown();

int     isOggUrl(const char *url);
/*
** Copyright(C) 1999 - 2003 Albert L. Faber 
** & Jack Moffitt <jack@icecast.org> and 
** & Michael Smith <msmith@labyrinth.net.au>
** & Aaron Porter <aaron@javasource.org>
** (basic version is derived from very old Xiph in_vorbis.dll)
**  
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
**(at your option) any later version.
** 
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
** 
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software 
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111 - 1307, USA.
*/

char *  httpGetTitle(const char *url);

void *  httpStartBuffering(const char *url, OggVorbis_File * input_file, BOOL showMessages);
void    httpStopBuffering();

void    setHttpVars();

#endif