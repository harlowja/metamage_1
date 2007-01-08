/*	===================
 *	GenieExecHandler.cc
 *	===================
 */

#include "Genie/ExecHandler.hh"

// Nitrogen
#include "Nitrogen/AEDataModel.h"
#include "Nitrogen/OSStatus.h"

// Pedestal
#include "Pedestal/ApplicationContext.hh"
#include "Pedestal/Application.hh"

// Genie
#include "Genie/FileSystem/ResolvePathname.hh"
#include "Genie/PipeOrganSuite.h"
#include "Genie/Process.hh"


// FIXME:  Replace this with a userspace interface, in the manner of Berkeley sockets.

namespace Genie
{
	
	namespace N = Nitrogen;
	namespace NN = Nucleus;
	
	using N::keyDirectObject;
	
	
	void GenieExecHandler::AppleEventHandler( const AppleEvent& appleEvent, AppleEvent& reply, GenieExecHandler* handler )
	{
		handler->HandleAppleEvent( appleEvent, reply );
	}
	
	GenieExecHandler::GenieExecHandler()
	:
		NN::Owned< N::AEEventHandler >
		(
			N::AEInstallEventHandler< GenieExecHandler*, AppleEventHandler >( kAEPipeOrganSuite,
			                                                                  kAEExec,
			                                                                  this )
		)
	{
		//MakeNoteToTouchParam(keyDirectObject);
		//MakeNoteToTouchParam(keyCurrentWorkingDir);
		//MakeNoteToTouchParam(keyRedirectedInput);
		//MakeNoteToTouchParam(keyInlineInput);
		//MakeNoteToTouchParam(keyRedirectedOutput);
		//MakeNoteToTouchParam(keyAppendedOutput);
		//MakeNoteToTouchParam(keyAppend);
	}
	
	static N::FSDirSpec GetCWD(const AEDesc& appleEvent)
	{
		try
		{
			return NN::Convert< N::FSDirSpec >( N::AEGetParamPtr< typeFSS >( appleEvent,
			                                                                 keyCurrentWorkingDir ) );
		}
		catch ( N::ErrAEDescNotFound )
		{
			return N::FSDirSpec();
		}
	}
	
	void GenieExecHandler::HandleAppleEvent( const AEDesc& appleEvent, AEDesc& outReply )
	{
		int returnValue = -1;
		
		N::FSDirSpec cwd = GetCWD( appleEvent );
		
		// Currently, we're expecting a string or a list of strings.
		
		if ( N::AESizeOfParam( appleEvent, keyDirectObject ).typeCode != typeAEList )
		{
			std::string str = N::AEGetParamPtr< typeChar >( appleEvent, keyDirectObject );
			
			returnValue = ExecString(str, cwd);
		} 
		else
		{
			NN::Owned< N::AEDescList > list = N::AEGetParamDesc( appleEvent, keyDirectObject, typeWildCard );
			
			std::vector< std::string > argVec;
			
			long argCount = N::AECountItems( list );
			
			// Load the arg vector
			for ( int index = 1;  index <= argCount;  index++ )
			{
				argVec.push_back( N::AEGetNthPtr< typeChar >( list, index ) );
			}
			
			// Attach our flow director to the output console to redirect output.
			
			returnValue = ExecArgList(argVec);
		}
		// Put the return value into the reply.
		//N::AEPutParamPtr<typeSInt32>( outReply, keyDirectObject, returnValue );
	}
	
	int GenieExecHandler::ExecArgList( const std::vector< std::string >& args )
	{
		int result = 0;
		
		std::vector< const char* > argv( args.size() + 1 );
		
		for ( int i = 0; i < args.size();  ++i )
		{
			argv[i] = args[i].c_str();
		}
		
		argv.back() = NULL;
		
	#if 0
		// Echo the input line to the console.  Use stderr, not stdout.
		if ( args.size() > 0 )
		{
			std::fprintf( stderr, "$ " );
			std::fprintf( stderr, "%s", argv[ 0 ] );
			
			for ( int i = 1;  i < args.size();  ++i )
			{
				fprintf( stderr, " %s", argv[ i ] );
			}
			
			fprintf( stderr, "\n" );
		}
	#endif
		
		FSSpec program = ResolvePathname( argv[0], FSTreePtr() )->GetFSSpec();
		
		const int ppid = 1;
		
		GenieProcess* process = new GenieProcess( ppid );
		
		try
		{
			process->Exec( program, &argv[0], NULL );
		}
		catch ( ... )
		{
			process->Terminate();
		}
		
		//int pid = gProcessTable[GenieProcess::kRootProcessPID].ForkExec(inArgVec);
		
		// FIXME:  We're assuming an immediate result
		//result = gProcessTable[pid].Result();
		
		return result;
	}
	
	int GenieExecHandler::ExecString( const std::string& cmd, const N::FSDirSpec& cwd )
	{
		/*
		int pid = ( new GenieShell( GenieProcess::kRootProcessPID, cmd ) )->ProcessID();
		gProcessTable[ pid ].ChangeDirectory( cwd );
		
		// FIXME:  We're assuming an immediate result
		int result = gProcessTable[ pid ].Result();
		
		return result;
		*/
		
		std::vector< std::string > argv;
		
		argv.push_back( "/bin/sh" );
		argv.push_back( "-c" );
		argv.push_back( cmd );
		
		return ExecArgList( argv );
	}
	
}

