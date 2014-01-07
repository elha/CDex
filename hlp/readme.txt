CDEX - README.TXT FILE

===============================================================
 Copyright (C) 2006-2007 by Georgy Berdyshev
 Copyright (C) 1998-2006 by Albert L Faber
 COPYRIGHT NOTICE:
 All content in this file may not be copied (partly or as a 
 whole), or re-used in other applications (e.g. CDex spin-offs), 
 without the  express written permission of the copyright 
 holder(s)
===============================================================

INTRODUCTION:
===============================================================

Cdex is a is utility which can extract Digital Audio (DA) 
tracks  from an Audio CD into files on the hard disk. The 
extracted audio tracks can be stored as WAV or MP3 (MPEG-1 
Audio Layer-3) files. CDex is using the Adaptec's ASPI library 
Manager to communicate with the CD-ROM device, So make sure 
that you have installed the proper ASPI drivers (see also the 
HELP file and the FAQ page!)

The latest version of CDex can be downloaded from 

http://cdexos.sourceforge.net


SYSTEM REQUIREMENTS:
===============================================================

1)	Microsoft Windows 95/98/ME & Windows-NT4 SP3, Windows 2000,
        WindowsXP or Windows Vista
2)	Adaptec ASPI for Win32 Manager (for Windows 95/95/ME )
3)	Digital Audio Extracting capable CD drive
4)	Intel x386 or compatible processor


HOW TO INSTALL CDEX (ZIP FILE):
===============================================================
There are no special installation requirements, extract all the
files that are in the zip file to a directory where you want 
to install CDex.  Once installed, you can start the CDex 
application by selecting the CDex.exe, or you can create a
dekstop short-cut. However, if you do create a desktop short-cut
make sure that you set the "Start In" field to the directory
where you extracted the CDex files. For more detailed 
information, please consult the help file !


KNOWN PROBLEMS:
===============================================================
None


Bugs, comments & suggestions:
===============================================================
Bugs, undefined behavior etc. and suggestions can be forwarded 
to the author (Albert Faber). If you have a question, make sure
that you have READ the HELP file first, If you can can't find 
the answer to your question, please consult the forum page at 
the  CDex home page ( http://cdexos.sourceforge.net/boards/ )



Warranty & licence:
===============================================================

see Help file for GNU GPL license


This distribution of CDex (ZIP file) should contain the 
following files:
===============================================================

----- Main application -----
CDex.exe (The CDex executable)
CDex.hlp (CDex Help File)
CDex.cnt (CDex Help Contents File)
CDRip.dll (library file with CD-ripper routines)
CDex.exe.manifest (Windows XP manifest file)

----- Encoder libraries -----
lame_enc.dll			( MPEG-I Layer 3 encoder )
Mp2Enc.dll				( MPEG-I Layer 2 encoder )
libsndfile.dll			( WAV sound library )
WMA8Connect.dll			( WMA8 encoder DLL )
vorbis.dll				( library file for Ogg Vorbis encoder )
vorbisenc.dll			( library file for Ogg Vorbis encoder )
ogg.dll					( library file for Ogg Vorbis encoder )
MACDll.dll				( Monkey's Audio Encoder library )

----- Decoder libraries -----
Plugins\in_mp3.dll
Plugins\in_vorbis.dll
Plugins\in_vorbis.txt	

----- Additonal Text files -----
Changes.txt (Change log with release information)
Readme.txt (this file)

----- Language Text files -----
lang\deutsch.cdex.lang               ( German resource file           )
lang\english.cdex.lang               ( English resource file          )
lang\spanish.cdex.lang               ( Spanish resource file          )
lang\italiano.cdex.lang              ( Italian resource file          )


Many more language are supported, however, to keep the distribution
to an acceptable level, only a few language files are standard shipped
with CDex. Please check out the 
Options->Select Langauge->Check for additional/updated language files
menu option, or visit the http://cdexos.sourceforge.net/lang/cdex_v1.70
WEB page to get an overview of the available language files
 
                    ======= ### ======
