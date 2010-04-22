/*
	var_string.hh
	-------------
	
	Copyright 2010, Joshua Juran
*/

#ifndef PLUS_VARSTRING_HH
#define PLUS_VARSTRING_HH

// plus
#include "plus/string.hh"


namespace plus
{
	
	class var_string : public string
	{
		private:
			char* mutable_data() const  { return const_cast< char* >( data() ); }
		
		public:
			var_string()
			{
			}
			
			var_string( const char*    p,
			            size_type      length,
			            delete_policy  policy ) : string( p, length, policy )
			{
			}
			
			var_string( const char* p, size_type length ) : string( p, length )
			{
			}
			
			var_string( const char* p, const char* q ) : string( p, q )
			{
			}
			
			var_string( const char* s ) : string( s )
			{
			}
			
			var_string( const string& other ) : string( other )  {}
			
			var_string& operator=( const string& other )
			{
				return assign( other );
			}
			
			var_string& operator=( const char* s )  { return assign( s ); }
			
			char* begin() const  { return mutable_data(); }
			
			char* end() const  { return begin() + size(); }
			
			var_string& assign( const char*    p,
			                    size_type      length,
			                    delete_policy  policy,
			                    size_type      capacity = 0 )
			{
				string::assign( p, length, policy, capacity );
				
				return *this;
			}
			
			var_string& assign( const char* p, size_type length )
			{
				string::assign( p, length );
				
				return *this;
			}
			
			var_string& assign( const char* p, const char* q )
			{
				string::assign( p, q );
				
				return *this;
			}
			
			var_string& assign( const char* s )
			{
				string::assign( s );
				
				return *this;
			}
			
			var_string& assign( const string& other )
			{
				string::assign( other );
				
				return *this;
			}
			
			char& operator[]( size_type i )  { return mutable_data()[ i ]; }
			
			void reserve( size_type size );
			void resize ( size_type size, char c = '\0' );
			
			void append( const char* p, size_type length );
			
			void append( const char* p, const char* q )  { append( p, q - p ); }
			
			void append( const char* s );
			
			void append( char c )  { append( &c, sizeof c ); }
			
			void append( const string& other )  { append( other.data(), other.size() ); }
			
			string& operator+=( const string& s )  { append( s );  return *this; }
			string& operator+=( const char*   s )  { append( s );  return *this; }
			string& operator+=( char          c )  { append( c );  return *this; }
	};
	
}

#endif

