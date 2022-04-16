/* Copyright (C) 2018-2019 Bho Matthiesen, Christoph Hellings
 * 
 * This program is used in the article:
 *
 * Bho Matthiesen, Christoph Hellings, Eduard A. Jorswieck, and Wolfgang
 * Utschick, "Mixed Monotonic Programming for Fast Global Optimization,"
 * submitted to IEEE  Transactions on Signal Processing.
 * 
 * 
 * License:
 * This program is licensed under the GPLv2 license. If you in any way use this
 * code for research that results in publications, please cite our original
 * article listed above.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details. */


#ifndef _DEBUG_H
#define _DEBUG_H

#include <cmath>
#include <vector>
#include <memory>


/*
    MOVED FROM util.cpp to here!! (because of template-class-instantiation errors)
    ---------------------------------------------------------------
*/


/*
 * Author:  David Robert Nadeau
 * Site:    http://NadeauSoftware.com/
 * License: Creative Commons Attribution 3.0 Unported License
 *          http://creativecommons.org/licenses/by/3.0/deed.en_US
 */

#if defined(_WIN32)
#include <windows.h>
#include <psapi.h>

#elif defined(__unix__) || defined(__unix) || defined(unix) || (defined(__APPLE__) && defined(__MACH__))
#include <unistd.h>
#include <sys/resource.h>

#if defined(__APPLE__) && defined(__MACH__)
#include <mach/mach.h>

#elif (defined(_AIX) || defined(__TOS__AIX__)) || (defined(__sun__) || defined(__sun) || defined(sun) && (defined(__SVR4) || defined(__svr4__)))
#include <fcntl.h>
#include <procfs.h>

#elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__gnu_linux__)
#include <stdio.h>

#endif

#else
#error "Cannot define getPeakRSS( ) or getCurrentRSS( ) for an unknown OS."
#endif

/**
 * Returns the peak (maximum so far) resident set size (physical
 * memory use) measured in bytes, or zero if the value cannot be
 * determined on this OS.
 */
size_t getPeakRSS( )
{
#if defined(_WIN32)
    /* Windows -------------------------------------------------- */
    PROCESS_MEMORY_COUNTERS info;
    GetProcessMemoryInfo( GetCurrentProcess( ), &info, sizeof(info) );
    return (size_t)info.PeakWorkingSetSize;

#elif (defined(_AIX) || defined(__TOS__AIX__)) || (defined(__sun__) || defined(__sun) || defined(sun) && (defined(__SVR4) || defined(__svr4__)))
    /* AIX and Solaris ------------------------------------------ */
    struct psinfo psinfo;
    int fd = -1;
    if ( (fd = open( "/proc/self/psinfo", O_RDONLY )) == -1 )
        return (size_t)0L;      /* Can't open? */
    if ( read( fd, &psinfo, sizeof(psinfo) ) != sizeof(psinfo) )
    {
        close( fd );
        return (size_t)0L;      /* Can't read? */
    }
    close( fd );
    return (size_t)(psinfo.pr_rssize * 1024L);

#elif defined(__unix__) || defined(__unix) || defined(unix) || (defined(__APPLE__) && defined(__MACH__))
    /* BSD, Linux, and OSX -------------------------------------- */
    struct rusage rusage;
    getrusage( RUSAGE_SELF, &rusage );
#if defined(__APPLE__) && defined(__MACH__)
    return (size_t)rusage.ru_maxrss;
#else
    return (size_t)(rusage.ru_maxrss * 1024L);
#endif

#else
    /* Unknown OS ----------------------------------------------- */
    return (size_t)0L;          /* Unsupported. */
#endif
}

//extern "C" {
//	#include <stddef.h>
//	#include <sys/time.h>
//	#include <sys/resource.h>
//}
//
//size_t
//getPeakRSS()
//{
//	struct rusage rusage;
//	getrusage( RUSAGE_SELF, &rusage );
//	return (size_t)rusage.ru_maxrss;
//}

/*
    ---------------------------------------------------------------
*/

//size_t getPeakRSS();

template <typename T>
bool
close(const T a, const T b, const double rtol = 1e-4, const double atol = 1e-7)
{
	return std::fabs(a - b) > atol + rtol * std::fabs(b);
}

template <class InputIt1, class InputIt2>
bool
allclose(InputIt1 first1, InputIt1 last1, InputIt2 first2, const double rtol = 1e-4, const double atol = 1e-7)
{
	while (first1 != last1)
	{
		if (close(*first1, *first2, rtol, atol))
			return false;

		++first1;
		++first2;
	}
	return true;
}

template <class T>
class MiniPool
{
public:
	std::unique_ptr<T> get()
	{
		if (cache_.size() == 0)
		{
			return std::make_unique<T>();
		}
		auto r = std::move(cache_.back());
		cache_.pop_back();
		return r;
	}

	void put(std::unique_ptr<T>&& ptr)
	{
		cache_.emplace_back(std::move(ptr));
	}

	auto size()
	{
		return cache_.size();
	}
private:
	std::vector<std::unique_ptr<T>> cache_;
};
#endif
