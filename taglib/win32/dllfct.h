#ifndef DLLFCT_H
#define DLLFCT_H

#ifdef WIN32
	#ifdef TAGLIB_EXPORTS
		#define TAGLIB_EXPORT __declspec(dllexport)
	#else
		#define TAGLIB_EXPORT __declspec(dllimport)
	#endif
#else
	#ifdef __GNUC__
		#define TAGLIB_EXPORT __attribute__((visibility("default")))
	#endif
#endif

#endif
