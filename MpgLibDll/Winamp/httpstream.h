/*
** Copyright (C) 2001 Albert L. Faber
**  
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
** 
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
** 
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software 
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#ifndef HTTP_STREAM_H_INCLUDED
#define HTTP_STREAM_H_INCLUDED

#define CIRCULAR_BUFFER_SIZE ( 65536 )


void    httpInit();
void    httpSetHwnd( HWND hWnd );
void    httpSetProxy( const char *proxy );
void    httpShutdown( );

int     IsUrl( const char* url );

char *  httpGetTitle( const char *url );
void *  httpStartBuffering( const char* url, BOOL showMessages );
void	httpStopBuffering( void * vp );
void    setHttpVars();

size_t httpRead(	void*	ptr,
					size_t	size,
					size_t	nmemb,
					void*	datasource );

char* httpStartOfStream( void*	datasource, INT nBytesRequested );

#endif