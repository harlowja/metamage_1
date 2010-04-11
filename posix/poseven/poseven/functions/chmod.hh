// chmod.hh
// --------
//
// Maintained by Joshua Juran

// Part of the Nitrogen project.
//
// Written 2007 by Joshua Juran.
//
// This code was written entirely by the above contributor, who places it
// in the public domain.


#ifndef POSEVEN_FUNCTIONS_CHMOD_HH
#define POSEVEN_FUNCTIONS_CHMOD_HH

// poseven
#include "poseven/types/errno_t.hh"
#include "poseven/types/mode_t.hh"


namespace poseven
{
	
	inline void chmod( const char* pathname, mode_t mode )
	{
		throw_posix_result( ::chmod( pathname, mode ) );
	}
	
}

#endif

