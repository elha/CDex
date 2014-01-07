/*
** CDex - Open Source Digital Audio CD Extractor
**
** Copyright (C) 2006 - 2007 Georgy Berdyshev
** Copyright (C) 1999 - 2007 Albert L. Faber
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


#include "stdafx.h"
#include <string.h>
#include <math.h>
#include "Registry.h"


/*******************************************************************/
/* Construction/Destruction */

/*
CRegistry(const char* ApplicationName, int Mode, int Access);

ARGUMENTS
	ApplicationName	- pass in the path uniqely identifying your application.
		This will be typically be "CompanyName\\ApplicationName\\ApplicationVersion".
		An appropriate entry in the registry is created if it does not
		already exist.

	Mode - one of the predefined modes:
		modeCurrentUserPrefs - indicates that you will be accessing
		preferences for the currently logged on user

		modeDefUserPrefs - indicates that you will be accessing
		preferences for the default users. This information is usually
		supplemented with the overriden preferences for the current user.

		modeLocalMachineProps - indicates that you will be accessing
		the information about the local machine.

	Access - can be a combination of accessRead and accessWrite.
		It must contain at least one of the two.

DESCRIPTION
	You would typically want to use this constructor to do most of
	your work. However should you need more control over where your
	data gets put in the registry use the second form of the constructor.

NOTE
	The following sections are created in the registry depending on
	the value of 'Mode':

	modeCurrentUserPrefs - "HKEY_CURRENT_USER\\ApplicationName"
	modeDefUserPrefs - "HKEY_USERS\\.DEFAULT\\Software\\ApplicationName"
	modeLocalMachineProps - "HKEY_LOCAL_MACHINE\\SOFTWARE\\ApplicationName"
*/
CRegistry::CRegistry( const CUString& strApplicationName, int Mode, int Access) :
	mRootArea(NULL),
	mRootSection(_W( "" )),
	mActiveSectionKey(NULL),
	mActiveSection( _W( "" )),
	mAccess(0)
{
	ASSERT( strApplicationName.GetLength() > 0);
	ASSERT((Mode == modeCurrentUserPrefs) ||
		   (Mode == modeDefUserPrefs) ||
		   (Mode == modeLocalMachineProps));

	HKEY key;
	CUString section;

	// Choose an appropriate place in the registry to store
	// application preferences based on the 'Mode' argument.

	if (Mode == modeCurrentUserPrefs)
	{
		key = HKEY_CURRENT_USER;
		section = _T( "Software\\" );
		section += strApplicationName;
	}
	else if (Mode == modeDefUserPrefs)
	{
		key = HKEY_USERS;
		section = _T( ".DEFAULT\\Software\\" );
		section += strApplicationName;
	}
	else if (Mode == modeLocalMachineProps)
	{
		key = HKEY_LOCAL_MACHINE;
		section = _T( "SOFTWARE\\" );
		section += strApplicationName;
	}

	OpenRoot(key, section, Access);
}

/*
CRegistry(HKEY Area, const char* RootSection, int Access)

ARGUMENTS
	Area - pass in an already open registry key or one of the
		predefined ones (HKEY_CURRENT_USER, HKEY_LOCAL_MACHINE,
		HKEY_CLASSES_ROOT, HKEY_USERS, HKEY_CURRENT_CONFIG,
		HKEY_DYN_DATA).

	RootSection - pass in an exact path to the section you will
		be working with. The section is created if it does not
		already exist.

	Access - can be a combination of accessRead and accessWrite.
		It must contain at least one of the two.	
*/
CRegistry::CRegistry(HKEY Area, const CUString& strRootSection, int Access)
{
	OpenRoot(Area, strRootSection, Access);
}

/*
~CRegistry()

DESCRIPTION
	Automatically closes any open links to the registry.
*/
CRegistry::~CRegistry()
{
	Close();
}


/*******************************************************************/
/* Attributes */

/*
void SetRootSection(const char* ApplicationName, int Mode)

ARGUMENTS
	ApplicationName	- pass in the path uniqely identifying your application.
		This will be typically be "CompanyName\\ApplicationName\\ApplicationVersion".
		An appropriate entry in the registry is created if it does not
		already exist.

	Mode - one of the predefined modes:
		modeCurrentUserPrefs - indicates that you will be accessing
		preferences for the currently logged on user

		modeDefUserPrefs - indicates that you will be accessing
		preferences for the default users. This information is usually
		supplemented with the overriden preferences for the current user.

		modeLocalMachineProps - indicates that you will be accessing
		the information about the local machine.

DESCRIPTION
	Works the same as the first form of the constructor.

	Access priviliges are not changed. Use SetAccess() to specify
	a different access privilege level.
*/
void CRegistry::SetRootSection(const CUString& strApplicationName, int Mode)
{
	ASSERT( strApplicationName.GetLength() > 0);
	ASSERT((Mode == modeCurrentUserPrefs) ||
		   (Mode == modeDefUserPrefs) ||
		   (Mode == modeLocalMachineProps));

	HKEY key;
	CUString section;

	// Choose an appropriate place in the registry to store
	// application preferences based on the 'Mode' argument.

	if (Mode == modeCurrentUserPrefs)
	{
		key = HKEY_CURRENT_USER;
		section = _T( "Software\\" );
		section += strApplicationName;
	}
	else if (Mode == modeDefUserPrefs)
	{
		key = HKEY_USERS;
		section = _T( ".DEFAULT\\Software\\" );
		section += strApplicationName;
	}
	else if (Mode == modeLocalMachineProps)
	{
		key = HKEY_LOCAL_MACHINE;
		section = _T( "SOFTWARE\\" );
		section += strApplicationName;
	}

	Close();
	OpenRoot(key, section, mAccess);
}

/*
void SetRootSection(HKEY Area, const char* RootSection)

ARGUMENTS
	Area - pass in an already open registry key or one of the
		predefined ones (HKEY_CURRENT_USER, HKEY_LOCAL_MACHINE,
		HKEY_CLASSES_ROOT, HKEY_USERS, HKEY_CURRENT_CONFIG,
		HKEY_DYN_DATA).

	RootSection - pass in an exact path to the section you will
		be working with. The section is created if it does not
		already exist.

DESCRIPTION
	Works the same as the second form of the constructor.

	Access priviliges are not changed. Use SetAccess() to specify
	a different access privilege level.
*/
void CRegistry::SetRootSection(HKEY Area, const CUString& strRootSection)
{
	Close();
	OpenRoot(Area, strRootSection, mAccess);
}

/*
void SetAccess(int Access)

ARGUMENTS
	Access - can be a combination of accessRead and accessWrite.
		It must contain at least one of the two.	

DESCRIPTION
	Alters the access priviliges of an already open registry section.
*/
void CRegistry::SetAccess(int Access)
{
	Close();
	OpenRoot(mRootArea, mRootSection, Access);
}


/*******************************************************************/
/* Iteration */

/*
POSITION GetFirstKeyPos()

DESCRIPTION
	Returns a handle to the first key in the current section;
	NULL if no keys exist in the section.

	Use this function in conjunction with GetNextKey() to
	enumerate all the key in a section.
*/
POSITION CRegistry::GetFirstKeyPos()
{
	// Determine how much memory to reserve for the value name.
	DWORD maxValueNameLen = 0;
	RegQueryInfoKey(mActiveSectionKey, NULL, NULL, NULL, NULL, NULL, 
		NULL, NULL, &maxValueNameLen, NULL, NULL, NULL);

	// Retrieve the name of the first available key.
	DWORD keyNameLen = maxValueNameLen + 1;
	TCHAR* keyName = new TCHAR[keyNameLen];

	LONG result = RegEnumValue(mActiveSectionKey, 0, keyName, &keyNameLen, NULL, NULL, NULL, NULL);

	delete[] keyName;

	// If we could not successfully retrieve the name then
	// this key does not exist.
	if (result == ERROR_NO_MORE_ITEMS)
		return NULL;

	return (POSITION) 1;
}

/*
CUString	GetNextKey(POSITION& Pos)

ARGUMENTS
	Pos - handle to the key whose name is to be retrieved

DESCRIPTION
	You must pass a valid handle to a key; NULL values are not
	accepted.

	A string containing the name of the key referred to by 'Pos'
	is returned and 'Pos' set to the next key in the section.
	If there are no more keys available, 'Pos' is set to NULL.
*/
CUString	CRegistry::GetNextKey(POSITION& Pos)
{
	ASSERT(Pos != NULL);

	// Determine how much memory to reserve for the value name.
	DWORD maxValueNameLen = 0;
	RegQueryInfoKey(mActiveSectionKey, NULL, NULL, NULL, NULL, NULL, 
		NULL, NULL, &maxValueNameLen, NULL, NULL, NULL);

	// Retrieve the name of the key whose index is given in 'Pos'.
	DWORD keyNameLen = maxValueNameLen + 1;
	TCHAR* keyName = new TCHAR[keyNameLen];
	DWORD index = ((DWORD)Pos) - 1;
	LONG result = RegEnumValue(mActiveSectionKey, index, keyName, &keyNameLen, NULL, NULL, NULL, NULL);
	ASSERT(result != ERROR_NO_MORE_ITEMS);
	CUString name = keyName;

	// Determine whether there are more keys available.
	result = RegEnumValue(mActiveSectionKey, index + 1, keyName, &keyNameLen, NULL, NULL, NULL, NULL);
	if (result == ERROR_NO_MORE_ITEMS)
		Pos = NULL;
	else
		Pos = (POSITION) (index + 2);
	delete[] keyName;

	return name;
}

/*
POSITION GetFirstSectionPos()

DESCRIPTION
	Returns a handle to the first subsection in the current section;
	NULL if no subsections exist in the section.

	Use this function in conjunction with GetNextSection() to
	enumerate all the subsections in a section.
*/
POSITION CRegistry::GetFirstSectionPos()
{
	// Determine how much memory to reserve for the section name.
	DWORD maxSectionNameLen = 0;
	RegQueryInfoKey(mActiveSectionKey, NULL, NULL, NULL, NULL, 
		&maxSectionNameLen, NULL, NULL, NULL, NULL, NULL, NULL);

	// Retrieve the name of the first available section.
	DWORD sectionNameLen = maxSectionNameLen + 1;
	TCHAR* sectionName = new TCHAR[sectionNameLen];
	FILETIME lastWriteToTime;
	LONG result = RegEnumKeyEx(mActiveSectionKey, 0, sectionName, &sectionNameLen, NULL, NULL, NULL, &lastWriteToTime);
	delete[] sectionName;

	// If we could not successfully retrieve the name then
	// this section does not exist.
	if (result == ERROR_NO_MORE_ITEMS)
		return NULL;

	return (POSITION) 1;
}

/*
CUString	GetNextSection(POSITION& Pos)

ARGUMENTS
	Pos - handle to the section whose name is to be retrieved

DESCRIPTION
	You must pass a valid handle to a section; NULL values are not
	accepted.

	A string containing the name of the section referred to by 'Pos'
	is returned and 'Pos' is set to the next available section.
	If there are no more sections, 'Pos' is set to NULL.
*/
CUString	CRegistry::GetNextSection(POSITION& Pos)
{
	ASSERT(Pos != NULL);

	// Determine how much memory to reserve for the section name.
	DWORD maxSectionNameLen = 0;
	RegQueryInfoKey(mActiveSectionKey, NULL, NULL, NULL, NULL, 
		&maxSectionNameLen, NULL, NULL, NULL, NULL, NULL, NULL);

	// Retrieve the name of the section whose index is given in 'Pos'.
	DWORD sectionNameLen = maxSectionNameLen + 1;
	TCHAR* sectionName = new TCHAR[sectionNameLen];
	FILETIME lastWriteToTime;
	DWORD index = ((DWORD)Pos) - 1;
	LONG result = RegEnumKeyEx(mActiveSectionKey, index, sectionName, &sectionNameLen, NULL, NULL, NULL, &lastWriteToTime);
	ASSERT(result != ERROR_NO_MORE_ITEMS);
	CUString name = sectionName;

	// Determine whether there are more sections available.
	result = RegEnumKeyEx(mActiveSectionKey, index + 1, sectionName, &sectionNameLen, NULL, NULL, NULL, &lastWriteToTime);
	if (result == ERROR_NO_MORE_ITEMS)
		Pos = NULL;
	else
		Pos = (POSITION) (index + 2);
	delete[] sectionName;

	return name;
}


/*******************************************************************/
/* Operations */

/*
void Descend(const char* Section)

ARGUMENTS
	Section - name of the subsection to be assumed as the current section

DESCRIPTION
	You would typically use this function in conjunction with
	Ascend(). When an object that keeps some of its data in the
	registry initializes itself, Descend() is called to obtain
	access to its own private area within the parent object's
	registry space. Call Ascend() to return to the parent object's
	registry space upon termination.

	You can think of Descend() and Ascend() as 'PUSH' and 'POP'
	operations; before a subroutine is executed registers are
	pushed onto the stack thus giving the subroutine the freedom
	to do almost anything without worrying about corrupting the
	state of data at the point of call. When the subroutine is
	done it pops the old data. If you use Descend() or Ascend(),
	you should always pair the calls.
*/
void CRegistry::Descend(const char* Section)
{
	ASSERT(Section != NULL);
	ASSERT(strlen(Section) > 0);

	mActiveSection += _W( "\\" );
	mActiveSection += CUString( Section, CP_UTF8 );

	CUString rootSection = mRootSection;

	Close();
	OpenRoot(mRootArea, mActiveSection, mAccess);
	
	mRootSection = rootSection;
}

/*
void Ascend()

DESCRIPTION
	Backs up to the parent section of the current section.
	The parent section is set to be the current section.

	Note that Ascend() will only back up to the root section.
	It will not go any further no matter how many times it
	gets called.
*/
void CRegistry::Ascend()
{
	// Ascending above the root section not allowed.
	if (mActiveSection == mRootSection)
		return;

	int pos = mActiveSection.ReverseFind('\\');
	ASSERT(pos != -1);
	mActiveSection = mActiveSection.Left(pos);

	CUString rootSection = mRootSection;

	Close();
	OpenRoot(mRootArea, mActiveSection, mAccess);
	
	mRootSection = rootSection;
}

/*
bool KeyExists(const CUString& strKey)

ARGUMENTS
	Key - name of a key

DESCRIPTION
	Checks for existence of the key named 'Key' in the
	current section. Returns 'true' if the key exists 
	and 'false' otherwise.
*/
bool CRegistry::KeyExists(const CUString& strKey)
{
    CUStringConvert strCnv; 

	LONG result = ::RegQueryValueEx(mActiveSectionKey, strCnv.ToT( strKey ), NULL, NULL, NULL, NULL);
	if (result != ERROR_SUCCESS)
		return false;

	return true;
}

/*
bool SectionExists(const char* Section)

ARGUMENTS
	Section - name of a section

DESCRIPTION
	Checks for existence of the subsection named 'Section' in
	the current section. Returns 'true' if the subsection exists
	and 'false' otherwise.
*/
bool CRegistry::SectionExists(const CUString& strSection)
{
	REGSAM accessMask = 0;
	if (mAccess & accessRead)
    		accessMask |= KEY_ENUMERATE_SUB_KEYS | KEY_EXECUTE | KEY_QUERY_VALUE | KEY_READ;
	if (mAccess & accessWrite)
		accessMask |= KEY_CREATE_LINK | KEY_CREATE_SUB_KEY | KEY_SET_VALUE | KEY_WRITE;

	HKEY tempKey;
    CUStringConvert strCnv;

    LONG result = ::RegOpenKeyEx(mActiveSectionKey, strCnv.ToT(  strSection ), 0, accessMask, &tempKey);
	if (result != ERROR_SUCCESS)
		return false;

	::RegCloseKey(tempKey);

	return true;
}

/*******************************************************************/
/*
	The following functions store key-value pairs into the current section.

	If the specified key does not exist, it is created; otherwise
	its value is overwritten with the new one.
*/

void CRegistry::Store(const CUString& strKey, signed char Value)
{
	StoreDWORD(strKey, (DWORD)Value);
}

void CRegistry::Store(const CUString& strKey, unsigned char Value)
{
	StoreDWORD(strKey, (DWORD)Value);
}

void CRegistry::Store(const CUString& strKey, signed short Value)
{
	StoreDWORD(strKey, (DWORD)Value);
}

void CRegistry::Store(const CUString& strKey, unsigned short Value)
{
	StoreDWORD(strKey, (DWORD)Value);
}

void CRegistry::Store(const CUString& strKey, signed int Value)
{
	StoreDWORD(strKey, (DWORD)Value);
}

void CRegistry::Store(const CUString& strKey, unsigned int Value)
{
	StoreDWORD(strKey, (DWORD)Value);
}

void CRegistry::Store(const CUString& strKey, signed long Value)
{
	StoreDWORD(strKey, (DWORD)Value);
}

void CRegistry::Store(const CUString& strKey, unsigned long Value)
{
	StoreDWORD(strKey, (DWORD)Value);
}

void CRegistry::Store(const CUString& strKey, float Value)
{
	CUString value;
	value.Format( _W( "%f" ) , Value);

	StoreString(strKey, value);
}

void CRegistry::Store(const CUString& strKey, double Value)
{
	// Resolution of the 'double' type is 16 decimal digits.
	// We use 17 just in case.
	CUString value;
	value.Format( _W( "%.17e" ), Value);

	StoreString(strKey, value);
}

void CRegistry::Store(const CUString& strKey, const CUString& Value)
{
	StoreString(strKey, Value);
}

/*******************************************************************/
/*
	The following functions restore key-value pairs from the current section.

	If the specified key does not exist, the value of 'Value' is unchanged.
*/

void CRegistry::Restore(const CUString& strKey, signed char& Value)
{
	DWORD value = Value;
	RestoreDWORD(strKey, value);

	Value = (signed char)value;
}

void CRegistry::Restore(const CUString& strKey, unsigned char& Value)
{
	DWORD value = Value;
	RestoreDWORD(strKey, value);

	Value = (unsigned char)value;
}

void CRegistry::Restore(const CUString& strKey, signed short& Value)
{
	DWORD value = Value;
	RestoreDWORD(strKey, value);

	Value = (signed short)value;
}

void CRegistry::Restore(const CUString& strKey, unsigned short& Value)
{
	DWORD value = Value;
	RestoreDWORD(strKey, value);

	Value = (unsigned short)value;
}

void CRegistry::Restore(const CUString& strKey, signed int& Value)
{
	DWORD value = Value;
	RestoreDWORD(strKey, value);

	Value = (signed int)value;
}

void CRegistry::Restore(const CUString& strKey, unsigned int& Value)
{
	DWORD value = Value;
	RestoreDWORD(strKey, value);

	Value = (unsigned int)value;
}

void CRegistry::Restore(const CUString& strKey, signed long& Value)
{
	DWORD value = Value;
	RestoreDWORD(strKey, value);

	Value = (signed long)value;
}

void CRegistry::Restore(const CUString& strKey, unsigned long& Value)
{
	DWORD value = Value;
	RestoreDWORD(strKey, value);

	Value = (unsigned long)value;
}

void CRegistry::Restore(const CUString& strKey, float& Value)
{
	CUString value;
	value.Format( _W( "%f" ), Value);

	RestoreString( strKey, value );

    CUStringConvert strCnv;
     
	_stscanf( strCnv.ToT( value ), _T( "%f" ), &Value );
}

void CRegistry::Restore(const CUString& strKey, double& Value)
{
	// Resolution of the 'double' type is 16 decimal digits.
	// We use 17 just in case.
	CUString value;
	value.Format( _W( "%.17e" ), Value);

	RestoreString(strKey, value);

    CUStringConvert strCnv;
	_stscanf( strCnv.ToT( value ), _T( "%.17e" ), &Value );
}

void CRegistry::Restore(const CUString& strKey, CUString& Value)
{
	RestoreString(strKey, Value);
}


/*******************************************************************/
/* Implementation */

void CRegistry::OpenRoot(HKEY Area, const CUString& RootSection, int Access)
{
	ASSERT(Area != NULL);
	ASSERT(Access != 0);
	ASSERT((Access & accessRead) || (Access & accessWrite));

	mRootArea = Area;
	mRootSection = RootSection;
	mActiveSection = mRootSection;
	mAccess = Access;
	
	REGSAM accessMask = 0;
	if (mAccess & accessRead)
		accessMask |= KEY_ENUMERATE_SUB_KEYS | KEY_EXECUTE | KEY_QUERY_VALUE | KEY_READ;
	if (mAccess & accessWrite)
		accessMask |= KEY_CREATE_LINK | KEY_CREATE_SUB_KEY | KEY_SET_VALUE | KEY_WRITE;

	DWORD disposition;
    CUStringConvert strCnv; 
	
    LONG result = ::RegCreateKeyEx(mRootArea, strCnv.ToT( mRootSection ), 0, NULL, 0, accessMask, NULL, &mActiveSectionKey, &disposition);
	if (result != ERROR_SUCCESS)
	{
		mRootArea = NULL;
		mRootSection = _T( "" );
		mActiveSectionKey = NULL;
		mActiveSection = _T( "" );
		mAccess = 0;
	}
}

void CRegistry::Close()
{
	if (mActiveSectionKey)
		::RegCloseKey(mActiveSectionKey);
}

void CRegistry::StoreDWORD(const CUString& strKey, DWORD Value)
{
    CUStringConvert strCnv;
	::RegSetValueEx(mActiveSectionKey, strCnv.ToT( strKey ), 0, REG_DWORD, (unsigned char*)&Value, sizeof(Value));
}

void CRegistry::RestoreDWORD(const CUString& strKey, DWORD& Value)
{
	DWORD type = 0;
    CUStringConvert strCnv; 
	::RegQueryValueEx(mActiveSectionKey, strCnv.ToT( strKey ), NULL, &type, NULL, NULL);
	
	if (type != REG_DWORD)
		return;

	DWORD valueLen = sizeof(Value);
	::RegQueryValueEx(mActiveSectionKey, strCnv.ToT( strKey ), NULL, NULL, (unsigned char*)&Value, &valueLen);
}

void CRegistry::StoreString(const CUString& strKey, const CUString& Value)
{
    CUStringConvert strCnv; 
    CUStringConvert strCnv1; 
    ::RegSetValueEx(mActiveSectionKey, strCnv.ToT( strKey ), 0, REG_SZ, (const BYTE*)strCnv1.ToT( Value ), Value.GetLength() + 1);
}

void CRegistry::RestoreString(const CUString& strKey, CUString& Value)
{
	DWORD type = 0;
	DWORD len = 0;
    CUStringConvert strCnv; 
	::RegQueryValueEx(mActiveSectionKey, strCnv.ToT( strKey ), NULL, &type, NULL, &len);
	
	if (type != REG_SZ)
		return;

	TCHAR* data = new TCHAR[ len ];

	data[0] = '\0';

	LONG result = ::RegQueryValueEx(mActiveSectionKey, strCnv.ToT( strKey ), NULL, NULL, (BYTE*)data, &len);

	if (result == ERROR_SUCCESS)
		Value = CUString( data );

	delete[] data;
}
