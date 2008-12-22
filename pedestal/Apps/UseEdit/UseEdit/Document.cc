/*	===========
 *	Document.cc
 *	===========
 */

#include "UseEdit/Document.hh"

// Nucleus
#include "Nucleus/NAssert.h"

// Io
#include "io/slurp.hh"

// Pedestal
#include "Pedestal/TEView.hh"


namespace UseEdit
{
	
	namespace N = Nitrogen;
	namespace NN = Nucleus;
	namespace Ped = Pedestal;
	
	template < class FileSpec >
	static std::string ReadFileData( const FileSpec& file )
	{
		std::string data = io::slurp_file< NN::StringFlattener< std::string > >( file );
		
		// Allow LF newlines
		std::replace( data.begin(),
		              data.end(),
		              '\n',
		              '\r' );
		
		return data;
	}
	
	inline ConstStr255Param GetFilenameAsPascalString( const FSSpec& file )
	{
		return file.name;
	}
	
	static N::Str255 GetFilenameAsPascalString( const FSRef& file )
	{
		N::FSGetCatalogInfo_Result info = N::FSGetCatalogInfo( file, kFSCatInfoNone );
		
		N::UniString unistring = NN::Convert< N::UniString >( info.outName );
		
		std::string string( unistring.begin(), unistring.end() );
		
		return N::Str255( string );
	}
	
	
	Document::Document()
	: 
		itsWindow( new Window ),
		itHasFile( false      ),
		itIsDirty( false      )   // A new document is never dirty, even if not saved
	{
	}
	
	static void LoadText( View& scroller, const std::string& text )
	{
		Ped::TEView& editor( scroller.GetSubView< Ped::TEView >() );
		
		editor.AppendChars( text.data(), text.size(), false );
		
		editor.SetSelection( 0, 0 );
		
		scroller.Calibrate();
	}
	
	Document::Document( const FSSpec& file )
	: 
		itsWindow( new Window( GetFilenameAsPascalString( file ) ) ),
		itHasFile( true  ),
		itIsDirty( false )
	{
		LoadText( itsWindow->SubView< View >(), ReadFileData( file ) );
	}
	
	Document::Document( const FSRef& file )
	: 
		itsWindow( new Window( GetFilenameAsPascalString( file ) ) ),
		itHasFile( true  ),
		itIsDirty( false )
	{
		LoadText( itsWindow->SubView< View >(), ReadFileData( file ) );
	}
	
}

