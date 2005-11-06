/*	=======
 *	aevt.cc
 *	=======
 */

// Mac OS
#include <TargetConditionals.h>
#if TARGET_RT_MAC_MACHO
	#include <Carbon/Carbon.h>
#else
	#include <AEHelpers.h>
	#include <Processes.h>
	#include <Script.h>
#endif

// Standard C++
#include <vector>

// Nitrogen
#include "Nitrogen/AEInteraction.h"
#include "Nitrogen/MacErrors.h"
#include "Nitrogen/Str.h"

#if CALL_NOT_IN_CARBON

// ClassicToolbox
#include "ClassicToolbox/AEDataModel.h"
#include "ClassicToolbox/AppleTalk.h"
#include "ClassicToolbox/EPPC.h"
#include "ClassicToolbox/PPCToolbox.h"

#endif

// Orion
#include "Orion/GetOptions.hh"
#include "Orion/Main.hh"
#include "Orion/StandardIO.hh"


namespace O = Orion;

namespace jTools
{
	
	namespace N = Nitrogen;
	
	using std::string;
	using std::vector;
	
	
	static N::Owned< AEDesc > BuildAppleEvent
	(
		AEEventClass eventClass, AEEventID eventID, 
		const AEAddressDesc& address, const char* buildString, va_list args, 
		AEReturnID returnID = kAutoGenerateReturnID, 
		AETransactionID transactionID = kAnyTransactionID )
	{
		std::size_t addrSize = N::AEGetDescDataSize( address );
		vector<char> addrData( addrSize );
		N::AEGetDescData( address, &addrData.front(), addrSize );
		
		N::AEDesc appleEvent;
		AEBuildError aeErr;
		
		N::ThrowOSStatus
		(
			::vAEBuildAppleEvent
			(
				eventClass, eventID,
				address.descriptorType, &addrData.front(), addrSize,
				returnID, transactionID, &appleEvent, 
				&aeErr,
				buildString, args
			)
		);
		
		return N::Owned< AEDesc >::Seize( appleEvent );
	}
	
#if CALL_NOT_IN_CARBON
	
	static TargetID LocateTarget( const string& appName, const string& machine, const string& host )
	{
		PPCPortRec name = N::Make< PPCPortRec >( N::Str32( appName ), "\p=" );
		
		LocationNameRec location = machine.empty() 
			? host.empty()
				? N::Make< LocationNameRec >()
				: N::Make< LocationNameRec >( N::Make< PPCAddrRec >( N::Make< PPCXTIAddress >( host ) ) )
			: N::Make< LocationNameRec >( N::Make< EntityName >( N::Str32( machine ), "\pPPCToolbox" ) );
		
		return N::Make< TargetID >( N::IPCListPortsSync( name, location ).name, location );
	}
	
#endif
	
	static N::Owned< AEDesc > SelectAddress( OSType sig, const string& app, const string& machine, const string& host, const string& url )
	{
		if ( sig != kUnknownType )
		{
			return N::AECreateDesc< typeApplSignature >( sig );
		}
		else
		{
		#if CALL_NOT_IN_CARBON
			
			return N::AECreateDesc< typeTargetID >( LocateTarget( app, machine, host ) );
			
		#else
			
			return N::AECreateDesc< typeApplicationURL >( url );
			
		#endif
		}
		
		// Not reached
		return N::Owned< AEDesc >();
	}
	
	// shell$ aevt -m Otter -a Genie |gan Exec '----':[�shutdown�,�-h�]
	// shell$ aevt -s hhgg aevt quit
	
	enum
	{
		optURL, 
		optHost, 
		optMachine, 
		optApplicationName, 
		optApplicationSignature
	};
	
	static O::Options DefineOptions()
	{
		O::Options options;
		
		options.DefineSetString( "-u", optURL );
		options.DefineSetString( "-h", optHost );
		options.DefineSetString( "-m", optMachine );
		options.DefineSetString( "-a", optApplicationName );
		options.DefineSetString( "-s", optApplicationSignature );
		
		options.DefineSetString( "--url", optURL );
		options.DefineSetString( "--host", optHost );
		options.DefineSetString( "--machine", optMachine );
		options.DefineSetString( "--appname", optApplicationName );
		options.DefineSetString( "--signature", optApplicationSignature );
		
		options.DefineSetString( "--app", optApplicationName );
		options.DefineSetString( "--sig", optApplicationSignature );
		
		return options;
	}
	
	static int Main( int argc, const char *const argv[] )
	{
		O::Options options = DefineOptions();
		options.GetOptions( argc, argv );
		
		const vector< const char* >& params = options.GetFreeParams();
		
		string argBuild, argEventClass, argEventID;
		long i = 0;
		
		if ( params.size() == 0 )
		{
			Io::Err << 
				"Usage:  aevt [-m machine] {-a app | -s sign} class id [params]\n"
				"Examples: aevt -s hhgg aevt quit\n"
				"          aevt -m 'Headless Mac' -a Genie |gan Exec \"'----':[�shutdown -h�]\"\n"
				"          aevt -s R*ch misc slct \"'----':obj{want:type(clin), form:indx, seld:42,\n"
				"               from:obj{want:type(cwin), form:indx, seld:1, from:null()}}\"\n";
			return 0;
		}
		else if ( params.size() < 2 )
		{
			Io::Err << "aevt: missing arguments" "\n";
			return 1;
		}
		else if ( params.size() < 3 )
		{
			argBuild = "";
		}
		else
		{
			argBuild = params[2];
		}
		argEventClass = params[0];
		argEventID = params[1];
		
		if ( argEventClass.size() != 4  ||  argEventID.size() != 4 )
		{
			Io::Err << "aevt: invalid parameter" "\n";
			return 1;
		}
		
		string url     = options.GetString( optURL );
		string host    = options.GetString( optHost );
		string machine = options.GetString( optMachine );
		string app     = options.GetString( optApplicationName );
		string sig     = options.GetString( optApplicationSignature );
		
		OSType sigCode = (sig.size() == 4) ? N::Convert< N::OSType >( sig ).Get() : kUnknownType;
		
		AEEventClass eventClass = N::Convert< N::AEEventClass >( argEventClass );
		AEEventID    eventID    = N::Convert< N::AEEventID    >( argEventID    );
		
		N::AESend
		(
			BuildAppleEvent
			(
				eventClass, eventID, 
				SelectAddress( sigCode, app, machine, host, url ), 
				argBuild.c_str(), NULL
			), 
			kAENoReply | kAECanInteract
		);
		
		return 0;
	}
	
}

int O::Main( int argc, const char *const argv[] )
{
	return jTools::Main( argc, argv );
}

