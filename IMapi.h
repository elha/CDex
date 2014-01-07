/*
** CDex - Open Source Digital Audio CD Extractor
**
** Copyright (C) 2006 - 2007 Georgy Berdyshev
** Copyright (C) 1999 Albert L. Faber
**
** http://cdexos.sourceforge.net/
** http://sourceforge.net/projects/cdexos 
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef IMAPI_INCLUDED
#define IMAPI_INCLUDED

/*
 *	$Header$
 *
 *	$Log$
 *	Revision 1.1  1999/12/05 21:13:50  afaber
 *	Initial revision
 *
 */

#include <mapi.h>

class CIMapi
{
public:
					CIMapi();
					~CIMapi();

	enum errorCodes
	{
		IMAPI_SUCCESS = 0,
		IMAPI_LOADFAILED,
		IMAPI_INVALIDDLL,
		IMAPI_FAILTO,
		IMAPI_FAILCC,
		IMAPI_FAILATTACH
	};

//	Attributes
	void			Subject(LPCTSTR subject)	{ m_message.lpszSubject = (LPTSTR) subject; }
	void			Text(LPCTSTR text)			{ m_text = text; }

	UINT			Error();
	void			From(LPCTSTR from)			{ m_from.lpszName = (LPTSTR) from; }

	static BOOL		HasEmail();

//	Operations
	BOOL			To(LPCTSTR recip);
	BOOL			Cc(LPCTSTR recip);
	BOOL			Attach(LPCTSTR path, LPCTSTR name = NULL);
	
	BOOL			Send(ULONG flags = 0);

private:
	BOOL			AllocNewTo();

	MapiMessage		m_message;
	MapiRecipDesc	m_from;
	UINT			m_error;
	CString			m_text;

	ULONG (PASCAL *m_lpfnSendMail)(ULONG, ULONG, MapiMessage*, FLAGS, ULONG);
	
	static HINSTANCE m_hInstMail;
	static BOOL		m_isMailAvail;
};


#endif