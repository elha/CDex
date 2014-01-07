/*
** CDex - Open Source Digital Audio CD Extractor
**
** Copyright (C) 2006 - 2007 Georgy Berdyshev
** Copyright (C) 1998 Sascha Djolich
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


#pragma once

class CRegistry
{
public:
	enum
	{
		modeCurrentUserPrefs = 0,
		modeDefUserPrefs,
		modeLocalMachineProps,
		
		accessRead  = 0x1,
		accessWrite = 0x2,
	};

// Construction/Destruction
public:
	CRegistry(const CUString& strApplicationName, int Mode = modeCurrentUserPrefs, int Access = accessRead | accessWrite);
	CRegistry(HKEY Area, const CUString& strRootSection, int Access = accessRead | accessWrite);
	~CRegistry();
	
// Attributes
public:
	void SetRootSection(const CUString& strApplicationName, int Mode = modeCurrentUserPrefs);
	void SetRootSection(HKEY Area, const CUString& strRootSection );

	void SetAccess(int Access = accessRead | accessWrite);

// Iteration
public:
	POSITION	GetFirstKeyPos();
	CUString		GetNextKey(POSITION& Pos);

	POSITION	GetFirstSectionPos();
	CUString		GetNextSection(POSITION& Pos);

// Operations
public:
	void Descend(const char* Section);
	void Ascend();

	bool KeyExists(const CUString& strKey);
	bool SectionExists(const CUString& strSection);

	void Store(const CUString& strKey, signed char Value);
	void Store(const CUString& strKey, unsigned char Value);
	void Store(const CUString& strKey, signed short Value);
	void Store(const CUString& strKey, unsigned short Value);
	void Store(const CUString& strKey, signed int Value);
	void Store(const CUString& strKey, unsigned int Value);
	void Store(const CUString& strKey, signed long Value);
	void Store(const CUString& strKey, unsigned long Value);
	void Store(const CUString& strKey, float Value);
	void Store(const CUString& strKey, double Value);
	void Store(const CUString& strKey, const CUString& Value);

	void Restore(const CUString& strKey, signed char& Value);
	void Restore(const CUString& strKey, unsigned char& Value);
	void Restore(const CUString& strKey, signed short& Value);
	void Restore(const CUString& strKey, unsigned short& Value);
	void Restore(const CUString& strKey, signed int& Value);
	void Restore(const CUString& strKey, unsigned int& Value);
	void Restore(const CUString& strKey, signed long& Value);
	void Restore(const CUString& strKey, unsigned long& Value);
	void Restore(const CUString& strKey, float& Value);
	void Restore(const CUString& strKey, double& Value);
	void Restore(const CUString& strKey, CUString& Value);

private:
	void OpenRoot(HKEY Area, const CUString& RootSection, int Access);
	void Close();

	void StoreDWORD(const CUString& strKey, DWORD Value);
	void StoreString(const CUString& strKey, const CUString& Value);

	void RestoreDWORD(const CUString& strKey, DWORD& Value);
	void RestoreString(const CUString& strKey, CUString& Value);

	HKEY	mRootArea;
	CUString mRootSection;
	
	HKEY	mActiveSectionKey;
	CUString mActiveSection;

	int		mAccess;
};
