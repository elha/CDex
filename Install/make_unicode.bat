cd ..
set PATH=%PATH%;"C:\Program Files\Microsoft Visual Studio\VC98\bin"
set PATH=%PATH%;"c:\Program Files\Microsoft Visual Studio\Common\MSDev98\Bin

msdev cdex.dsw /MAKE "logging - Win32 Release UNICODE" /NORECURSE /CLEAN
msdev cdex.dsw /MAKE "zlib - Win32 Release" /NORECURSE /CLEAN
msdev cdex.dsw /MAKE "libutf8 - Win32 Release" /NORECURSE /CLEAN
msdev cdex.dsw /MAKE "libsndfile - Win32 Release UNICODE" /NORECURSE /CLEAN
msdev cdex.dsw /MAKE "MP2EncDll - Win32 Release NASM" /NORECURSE /CLEAN
msdev cdex.dsw /MAKE "id3lib - Win32 Release UNICODE" /NORECURSE /CLEAN
msdev cdex.dsw /MAKE "CDRip - Win32 Release" /NORECURSE /CLEAN
msdev cdex.dsw /MAKE "CDex - Win32 Release UNICODE" /NORECURSE /CLEAN

msdev cdex.dsw /MAKE "logging - Win32 Release UNICODE" /NORECURSE /REBUILD
msdev cdex.dsw /MAKE "zlib - Win32 Release" /NORECURSE /REBUILD
msdev cdex.dsw /MAKE "libutf8 - Win32 Release" /NORECURSE /REBUILD
msdev cdex.dsw /MAKE "libsndfile - Win32 Release UNICODE" /NORECURSE /REBUILD
msdev cdex.dsw /MAKE "MP2EncDll - Win32 Release NASM" /NORECURSE /REBUILD
msdev cdex.dsw /MAKE "id3lib - Win32 Release UNICODE" /NORECURSE /REBUILD
msdev cdex.dsw /MAKE "CDRip - Win32 Release" /NORECURSE /REBUILD
msdev cdex.dsw /MAKE "CDex - Win32 Release UNICODE" /NORECURSE /REBUILD

cd install

call update cdex_170b2 lang_enu Release_UNICODE
