/**
 * This headers contains all the windows-specific includes and some defines
 * and redefines to replace posix-calls with their windows equivalents.
 * They are in this header because no standard unistd.h is available on windows,
 * so this custom version is picked up by the msvc compiler; this way we don't
 * have to make any windows-specific changes to the rest of the sources.
 */
#ifndef UNISTD_H
#define UNISTD_H

#include <io.h>

const W_OK = 2;
const R_OK = 4;
const S_IFLNK = 40960;

#define access _access
#define ftruncate _chsize

#undef TAGLIB_EXPORT
#ifdef TAGLIB_EXPORTS
#define TAGLIB_EXPORT __declspec(dllexport)
#else
#define TAGLIB_EXPORT __declspec(dllimport)
#endif

#endif //UNISTD_H
