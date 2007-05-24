/*	===========
 *	sendmail.cc
 *	===========
 */

// Standard C++
#include <algorithm>
#include <string>
#include <vector>

// Standard C/C++
#include <cctype>

// POSIX
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

// Io
#include "io/slurp.hh"

// Nitrogen
#include "Nitrogen/Folders.h"
#include "Nitrogen/OpenTransportProviders.h"

// POSeven
#include "POSeven/FileDescriptor.hh"

// Nitrogen Extras / Iteration
#include "Iteration/FSContents.h"

// Arcana / SMTP
#include "SMTP.hh"

// Kerosene
#if !TARGET_RT_MAC_MACHO
#include "SystemCalls.hh"
#endif

// Orion
#include "Orion/GetOptions.hh"
#include "Orion/Main.hh"
#include "Orion/StandardIO.hh"


namespace N = Nitrogen;
namespace NN = Nucleus;
namespace P7 = POSeven;
namespace O = Orion;

using namespace io::path_descent_operators;


inline bool operator<( const InetMailExchange& a, const InetMailExchange& b )
{
	return a.preference < b.preference;
}


static std::string gRelay;

struct BadEmailAddress : N::ParamErr {};


static N::FSDirSpec QueueDirectory()
{
	return N::FSDirSpec( io::system_root< N::FSDirSpec >() / "j" / "var" / "spool" / "jmail" / "queue" );
}


static std::string DomainFromEmailAddress( const std::string& emailAddr )
{
	std::string::size_type at = emailAddr.find( '@' );
	
	if ( at >= emailAddr.size() - 1 )
	{
		throw BadEmailAddress();
	}
	
	return emailAddr.substr( at + 1, emailAddr.find( '>' ) - (at + 1) );
}

/*
static std::string ResolverLookup( const std::string& domain )
{
	try
	{
		std::vector< MX > results = Resolver::LookupMX( domain );
		//ASSERT(!results.empty());
		
		std::sort( results.begin(),
		           results.end() );
		
		return results.front().mxHost;
	}
	catch ( Resolver::ResolverError error )
	{
		switch ( error.Errno() )
		{
			case NO_DATA:
				// No MX data.  No problem, we'll try A instead.
				break;
			
			default:
				throw;
		}
	}
	
	return "";
}
*/

static std::string OTLookup( const std::string& domain )
{
#if TARGET_RT_MAC_MACHO
	
	return "";
	
#else
	
	if ( TARGET_CPU_68K )
	{
		return "";
	}
	
	std::vector< InetMailExchange > results;	
	
	results.resize( 10 );  // Should be more than enough
	
	N::OTInetMailExchange( InternetServices(),
	                       (char*) domain.c_str(),
	                       results );
	
	/*
	UInt16 num = 10;
	N::ThrowOSStatus( O::OTInetMailExchange( (char*)domain.c_str(), &num, &results.front() ) );
	results.resize( num );
	*/
	
	std::sort( results.begin(),
	           results.end() );
	
	return results.front().exchange;
	
#endif
}

static struct in_addr ResolveHostname( const char* hostname )
{
	hostent* hosts = gethostbyname( hostname );
	
	if ( !hosts || h_errno )
	{
		Io::Err << "Domain name lookup failed: " << h_errno << "\n";
		O::ThrowExitStatus( 1 );
	}
	
	in_addr addr = *(in_addr*) hosts->h_addr;
	
	return addr;
}

static void Relay( const std::string&  returnPath,
                   const std::string&  forwardPath,
                   const FSSpec&       messageFile )
{
	if ( forwardPath == "" )
	{
		// This will be the case if the dest file's data fork was empty,
		// which will be true of an Icon file.
		// The file will be deleted after we return.
		return;
	}
	
	Io::Out << "Relaying from "
	        << returnPath
	        << " to "
	        << forwardPath
	        << "\n";
	
	std::string smtpServer = gRelay;
	
	if ( !gRelay.empty() )
	{
		Io::Out << "Using relay " << gRelay << ".\n";
	}
	else
	{
		std::string rcptDomain = DomainFromEmailAddress( forwardPath );
		
		//smtpServer = ResolverLookup( rcptDomain );
		smtpServer = OTLookup( rcptDomain );
		
		if ( !smtpServer.empty() )
		{
			Io::Out << "MX for " << rcptDomain << " is " << smtpServer << ".\n";
		}
		else
		{
			smtpServer = rcptDomain;
			Io::Out << "Using domain " << rcptDomain << " as server.\n";
		}
		
	}
	
	int smtpPort = 25;
	
	struct in_addr ip = ResolveHostname( smtpServer.c_str() );
	
	Io::Out << "Address of " << smtpServer << " is " << inet_ntoa( ip ) << ".\n";
	
	// Make a new socket
	
	int sock = socket( PF_INET, SOCK_STREAM, INET_TCP );
	
	P7::FileDescriptor serverStream = P7::FileDescriptor( sock );
	
	// and connect to the server.  This could fail, thanks to a bunch of Cox.
	
	struct sockaddr_in inetAddress;
	
	inetAddress.sin_family = AF_INET;
	inetAddress.sin_port   = smtpPort;
	inetAddress.sin_addr   = ip;
	
	int result = connect( sock, (const sockaddr*) &inetAddress, sizeof (sockaddr_in) );
	
	SMTP::Client::Session< P7::FileDescriptor > smtpSession( serverStream );
	
	NN::Owned< N::FSFileRefNum > messageStream = io::open_for_reading( messageFile );
	
	//smtpSession.Hello      ( HW::GetHostname() );
	smtpSession.Hello      ( "hostname"  );
	smtpSession.MailFrom   ( returnPath  );
	smtpSession.RecipientTo( forwardPath );
	
	smtpSession.BeginData();
	
	while ( true )
	{
		const std::size_t kDataSize = 4096;
		
		char data[ kDataSize ];
		
		try
		{
			int bytes = io::read( messageStream, data, kDataSize );
			
			io::write( serverStream, data, bytes );
		}
		catch ( const io::end_of_input& )
		{
			break;
		}
	}
	
	smtpSession.EndData();
	smtpSession.Quit();
}

static bool IsControlChar( char c )
{
	return std::iscntrl( c );
}

static std::string ReadOneLiner( N::FSFileRefNum fileH )
{
	std::string contents;
	
	contents.resize( N::GetEOF( fileH ) );
	
	io::read( fileH, &contents[0], contents.size() );
	
	const std::string::const_iterator end_of_first_line = std::find_if( contents.begin(),
	                                                                    contents.end(),
	                                                                    std::ptr_fun( IsControlChar ) );
	
	const std::size_t length_of_first_line = end_of_first_line - contents.begin();
	
	contents.resize( length_of_first_line );
	
	return contents;
}

static std::string ReadOneLiner( const FSSpec& file )
{
	return ReadOneLiner( io::open_for_reading( file ) );
}


class Transmitter
{
	private:
		std::string  itsReturnPath;
		FSSpec       itsMessageFile;
	
	public:
		Transmitter( const std::string&  returnPath,
		             const FSSpec&       message ) : itsReturnPath ( returnPath ),
		                                             itsMessageFile( message    )
		{}
		
		void operator()( const FSSpec& destFile );
};

void Transmitter::operator()( const FSSpec& destFile )
{
	try
	{
		using N::fsRdWrPerm;
		
		// destFile serves as a lock on this destination
		Relay( itsReturnPath,
		       ReadOneLiner( N::FSpOpenDF( destFile, fsRdWrPerm ) ),
		       itsMessageFile );
		
		N::FSpDelete( destFile );
	}
	catch ( ... )
	{
		
	}
}


static void ProcessMessage( const FSSpec& msgFolderItem )
{
	if ( !io::directory_exists( msgFolderItem ) )  return;  // Icon files, et al
	
	N::FSDirSpec msgFolder = NN::Convert< N::FSDirSpec >( msgFolderItem );
	
	FSSpec       message    = msgFolder / "Message";
	FSSpec       returnPath = msgFolder / "Return-Path";
	
	N::FSDirSpec destFolder( msgFolder / "Destinations" );
	
	std::for_each( N::FSContents( destFolder ).begin(),
	               N::FSContents( destFolder ).end(),
	               Transmitter( ReadOneLiner( returnPath ),
	                            message ) );
	
	io::delete_empty_directory( destFolder );  // this fails if destinations remain
	io::delete_file           ( returnPath );
	io::delete_file           ( message    );
	io::delete_empty_directory( msgFolder  );
}


int O::Main(int argc, const char *const argv[])
{
	if ( argc >= 3 )
	{
		if ( argv[ 1 ] == std::string( "--relay" ) )
		{
			gRelay = argv[ 2 ];
		}
	}
	
	N::FSDirSpec queue = QueueDirectory();
	
	std::for_each( N::FSContents( queue ).begin(),
	               N::FSContents( queue ).end(),
	               std::ptr_fun( ProcessMessage ) );
	
	return 0;
}

