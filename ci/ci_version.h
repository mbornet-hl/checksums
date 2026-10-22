/*
 *	Versioning macros for source code identification
 *	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 *	@(#)	[Zen] ci_version.h	Version 1.4 du 26/10/02 - 
 *
 *
 *	Usage in source files :
 *
 *	#include "ci_version.h"
 *	CI_VERSION("1.4");
 */

#ifndef CI_VERSION_H
#define CI_VERSION_H

#define CI_CAT_(a, b) a##b
#define CI_CAT(a, b)  CI_CAT_(a, b)

#ifdef __COUNTER__
#  define CI_UNIQ(prefix) CI_CAT(prefix, __COUNTER__)
#else
#  define CI_UNIQ(prefix) CI_CAT(prefix, __LINE__)   /* if compiler doesn't know __COUNTER__ */
#endif

#define CI_VERSION(v) \
    static const char CI_UNIQ(ci_id_)[] __attribute__((used)) = "@(#)" __FILE__ " " v

#endif /* CI_VERSION_H */
