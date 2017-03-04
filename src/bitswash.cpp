/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */


//
// Class: BitSwash
//
// Created by: lim k. c. <admin@bitswash.org>
// Created on: Mon Feb  1 01:23:21 2007
//
#ifdef HAVE_CONFIG_H
	#  include <config.h>
#endif

#ifdef __WXMSW__
	#include <WinSock2.h>
	#define PREFIX "."
#endif

#include <wx/stdpaths.h>
#include <wx/filename.h>
#include <wx/url.h>
#include <wx/cmdline.h>
#include <wx/mstream.h>

#include "../icons/flags/un.h"
#include "icon.h"
#include "bitswash.h"
#include "mainframe.h"
#include "functions.h"

const wxString g_BitSwashServerStr = wxT( "BitSwashMainApp" );
const wxString g_BitSwashTopicStr = wxT( "single-instance" );

wxConnectionBase *BitSwashAppSrv::OnAcceptConnection( const wxString& topic )
{
	if( topic.Lower() == g_BitSwashTopicStr )
	{
		// Check that there are no modal dialogs active
		wxWindowList::Node* node = wxTopLevelWindows.GetFirst();

		while( node )
		{
			wxDialog* dialog = wxDynamicCast( node->GetData(), wxDialog );

			if( dialog && dialog->IsModal() )
			{
				return NULL;
			}

			node = node->GetNext();
		}

		return new BitSwashAppConn();
	}
	else
		return NULL;
}

// Opens a file passed from another instance
bool BitSwashAppConn::OnExecute( const wxString& topic,
#if wxMAJOR_VERSION < 2 || (wxMAJOR_VERSION == 2 && wxMINOR_VERSION < 9)
	wxChar* data,
	int WXUNUSED( size ),
#else
	const void * data,
	size_t WXUNUSED( size ),
#endif
	wxIPCFormat WXUNUSED( format ) )
{
	MainFrame* frame = wxDynamicCast( wxGetApp().GetTopWindow(), MainFrame );
	wxString filenames( ( wxChar* )data );

	if( filenames.IsEmpty() )
	{
		// Just raise the main window
		if( frame )
		{
			frame->Restore();    // for minimized frame
			frame->Raise();
		}
	}
	else
	{
		// Check if the filename is already open,
		// and raise that instead.
		MainFrame::ReceiveTorrent(filenames);
	}

	return true;
}

IMPLEMENT_APP( BitSwash )

extern MainFrame * g_BitSwashMainFrame;

std::map<wxString, int> CountryCodeIndexMap;
wxString g_BitSwashAppDir;
wxString g_BitSwashHomeDir;

static const wxCmdLineEntryDesc g_cmdLineDesc [] =
{
	{
		wxCMD_LINE_SWITCH, "h", "help", "Displays help on the command line parameters",
		wxCMD_LINE_VAL_NONE, wxCMD_LINE_OPTION_HELP
	},
	{ wxCMD_LINE_PARAM, NULL, NULL, "File(s) to be opened", wxCMD_LINE_VAL_STRING,  wxCMD_LINE_PARAM_OPTIONAL | wxCMD_LINE_PARAM_MULTIPLE },

	{ wxCMD_LINE_NONE }
};

bool BitSwash::OnInit()
{
	m_btsession = 0;
	m_logold = 0;
	m_config = 0;
	m_imglist_ctryflags = 0;
	m_imglist_settingicons = 0;
	m_locale = 0;
	m_SigleAppChecker = 0;
	m_AppServer = 0;
#ifdef USE_LIBGEOIP
	m_geoip = 0;
#endif

	//==========================================================================
	if( !wxApp::OnInit() )
		return false;

	wxString name = wxString::Format( wxT( "BitSwash-%s" ), wxGetUserId().GetData() );
	m_SigleAppChecker = new wxSingleInstanceChecker( name );

	// If using a single instance, use IPC to
	// communicate with the other instance
	if( !m_SigleAppChecker->IsAnotherRunning() )
	{
		// Create a new server
		m_AppServer = new BitSwashAppSrv;

		if( !m_AppServer->Create( g_BitSwashServerStr ) )
		{
			wxLogDebug( wxGetTranslation(_( "Failed to create an IPC service." ) ));
		}
	}
	else
	{
		wxLogNull logNull;
		// OK, there IS another one running, so try to connect to it
		// and send it any filename before exiting.
		BitSwashAppClnt* client = new BitSwashAppClnt;
		// ignored under DDE, host name in TCP/IP based classes
		wxString hostName = wxT( "localhost" );
		// Create the connection
		wxConnectionBase* connection = client->MakeConnection( hostName, g_BitSwashServerStr, g_BitSwashTopicStr );

		if( connection )
		{
			// Only file names would be send to the instance, ignore other switches, options
			// Ask the other instance to open a file or raise itself
			wxString fnames;

			for( size_t i = 0; i < m_FileNames.GetCount(); ++i )
			{
				//The name is what follows the last \ or /
				fnames +=  m_FileNames[i] + wxT( ' ' );
			}

			connection->Execute( fnames );
			connection->Disconnect();
			delete connection;
		}
		else
		{
			wxMessageBox( wxGetTranslation(_( "Sorry, the existing instance may be too busy too respond.\nPlease close any open dialogs and retry." )),
						   APPNAME, wxICON_INFORMATION | wxOK );
		}

		delete client;
		return false;
	}

	wxFileName filename( GetExecutablePath() );
	filename.MakeAbsolute();
	g_BitSwashAppDir = filename.GetPath( wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR );
#ifdef __WXMSW__
	g_BitSwashHomeDir = g_BitSwashAppDir;
#else //linux: ~/.madedit
	g_BitSwashHomeDir = wxStandardPaths::Get().GetUserDataDir() + wxFILE_SEP_PATH;

	if( !wxDirExists( g_BitSwashHomeDir ) )
	{
		wxLogNull nolog; // no error message
		wxMkdir( g_BitSwashHomeDir );
	}
#endif

	wxHandleFatalExceptions();
	//==========================================================================

	int initcountdown = 1;
	SetVendorName( APPNAME );
	SetAppName( APPBINNAME );
	m_locale = NULL;
	m_btinitdone = false;
	//log to stderr
	m_logold = wxLog::SetActiveTarget( new wxLogStderr() );
	wxChar dirsep = wxFileName::GetPathSeparator( wxPATH_NATIVE );
#ifdef USERDATADIR
	wxString confDirName = _T( USERDATADIR );
#elif defined(__WXMSW__)
	wxString confDirName = wxStandardPaths::Get().GetDataDir();
#else
	wxString confDirName = wxStandardPaths::Get().GetUserDataDir();
#endif
	if( !wxFileName::DirExists( confDirName ) )
	{
		bool b = wxFileName::Mkdir( confDirName );
		if( !b ) wxLogError( _T( "Failed to create directory " ) + confDirName );
	}
	// change working directory to ~/.bitswash
	wxSetWorkingDirectory( confDirName );
	m_datapath = confDirName;
	m_configpath = confDirName + dirsep + APPBINNAME + _T( ".conf" );
	if( !wxFileName::FileExists( m_configpath ) )
	{
		wxFile *f = new wxFile();
		bool b = f->Create( m_configpath.c_str() );
		if( !b ) wxLogError( _T( "Failed to create file " ) + m_configpath );
	}
	m_config = new Configuration( APPNAME );
	/* workaround for libtorrent unable to resolve our ip address */
	wxIPV4address remote;
	remote.Hostname( _T( "www.google.com" ) );
	remote.Service( 80 );
	wxIPV4address local;
	wxSocketClient client;
	if( client.Connect( remote, false ) ) client.GetLocal( local );
	wxString ipAddr = local.IPAddress();
	wxLogDebug( _T( "Local ip %s\n" ), ipAddr.c_str() );
	m_config->m_local_ip = local;
	/* end workaround */
	SetLogLevel();
	SetLocale( m_config->GetLanguage() );
	m_dhtstatepath = confDirName + dirsep + APPBINNAME + _T( ".dhtstate" );
	m_savetorrentspath = confDirName + dirsep + _T( "torrents" ) ;
	if( !wxFileName::DirExists( m_savetorrentspath ) )
	{
		bool b = wxFileName::Mkdir( m_savetorrentspath );
		if( !b ) wxLogError( _T( "Failed to create directory " ) + m_savetorrentspath );
	}
	m_logpath = confDirName + dirsep + _T( "logs" ) ;
	if( !wxFileName::DirExists( m_logpath ) )
	{
		bool b = wxFileName::Mkdir( m_logpath );
		if( !b ) wxLogError( _T( "Failed to create directory " ) + m_logpath );
	}

	wxInitAllImageHandlers();
#ifdef USE_LIBGEOIP
	m_geoip = GeoIP_open(g_BitSwashHomeDir + "GeoIP.dat", GEOIP_STANDARD | GEOIP_CHECK_CACHE);
#endif
	LoadIcons();
	LoadFlags();
	LoadSettingIcons();
	m_btsession = new BitTorrentSession( this, m_config );
	if( m_btsession->Create() != wxTHREAD_NO_ERROR )
	{
		wxLogError( _T( "Error creating bit torrent session thread\n" ) );
		exit( -1 );
	}
	m_btsession->Run();
	//XXX shows some splash
	while( !m_btinitdone || ( initcountdown-- > 0 ) )
		wxSleep( 1 );
	g_BitSwashMainFrame = new MainFrame( 0L, APPNAME );
	if(!( m_config->GetUseSystray() && m_config->GetHideTaskbar() ))
	{
		g_BitSwashMainFrame->Show( TRUE );
		SetTopWindow( g_BitSwashMainFrame );
	}
	for( size_t i = 0; i < m_FileNames.GetCount(); ++i )
	{
		MainFrame::ReceiveTorrent(m_FileNames[i]);
	}
	SetExitOnFrameDelete( true );
	return TRUE;
}

int BitSwash::OnExit()
{
#if 0
	int i;
	wxLogDebug( _T( "BitSwash OnExit\n" ) );
	// kill bittorrent session
	//XXX lock
#endif
	wxLogDebug( _T( "Awaiting BitTorrent session exit ...\n" ) );
	if( m_btsession->IsAlive() )
	{
		m_btsession->Delete();
		wxLogDebug( _T( "BitTorrent session exited with code %ld\n" ),
					( long )m_btsession->Wait() );
	}
#if 0
	for( i = 0; i < BITSWASH_ICON_MAX; i++ )
	{
		delete AppIcons[i].image;
	}
	delete m_imglist_ctryflags;
	//restore log handle
#endif
	wxLog::SetActiveTarget( m_logold );
	if( m_SigleAppChecker )
		delete m_SigleAppChecker;

	if( m_AppServer )
		delete m_AppServer;

#ifdef USE_LIBGEOIP
	if(m_geoip) GeoIP_delete(m_geoip);
#endif
	return 0;
}

void BitSwash::LoadFlags()
{
	wxChar dirsep = wxFileName::GetPathSeparator( wxPATH_NATIVE );
    wxMemoryInputStream fstream(&un_bin[0], sizeof(un_bin)/sizeof(un_bin[0]));
	wxImage empty(fstream);
	int emptyidx = 0;
	m_imglist_ctryflags = new wxImageList( 16, 11, false );
	emptyidx = m_imglist_ctryflags->Add( wxBitmap( empty ) );
	for(unsigned int i = 0; i < N_COUNTRY ; ++i )
	{
		CountryCodeIndexMap[wxString( CountryFlags[i].code )] = i;
		if( ( wxIsEmpty( CountryFlags[i].code ) ) || ( ! wxStricmp( CountryFlags[i].code, _T( "--" ) ) ) )
		{
			CountryFlags[i].imgidx = emptyidx;
			continue;
		}

        wxMemoryInputStream stream(CountryFlags[i].filebuf, CountryFlags[i].filesize);
        wxImage flag( stream );
        CountryFlags[i].imgidx = m_imglist_ctryflags->Add( wxBitmap( flag ) );
	}
}

int BitSwash::GetCountryFlag( const wxString& code )
{
	if( code.IsEmpty() )
		return 0;
#ifdef USE_LIBGEOIP
	if(!m_geoip)
		return -1;
	// code is IP actually
	wxString wxcode(_T("--"));
	const char * ccode = GeoIP_country_code_by_addr(m_geoip, code.ToStdString().c_str());
	if(ccode)
		wxcode = wxString::FromAscii(ccode);
	std::map<wxString, int>::iterator it = CountryCodeIndexMap.find( wxcode );
#else
	std::map<wxString, int>::iterator it = CountryCodeIndexMap.find( code );
#endif
	if( it != CountryCodeIndexMap.end() )
	{
		wxASSERT( ((unsigned int)it->second) < N_COUNTRY );
		return CountryFlags[it->second].imgidx;
	}
	/*for (i=0; i< N_COUNTRY ; i++)
	{
	    strcode2.sprintf(_T("%s"), CountryFlags[i].code);

	    if ( ! (strcode2.CmpNoCase(strcode.Mid(0,2)) ) )
	        return CountryFlags[i].imgidx;
	}*/
	return -1;
}
#undef N_COUNTRY

void BitSwash::LoadSettingIcons()
{
	wxChar dirsep = wxFileName::GetPathSeparator( wxPATH_NATIVE );
	wxImage empty( 16, 16, true );
	m_imglist_settingicons = new wxImageList( 16, 16, false );

	for( unsigned int i = 0; i < N_SETTINGS; i++ )
	{
        wxMemoryInputStream stream(SettingIcons[i].filebuf, SettingIcons[i].filesize);
        wxImage flags(stream);
        m_imglist_settingicons->Add( wxBitmap( flags ) );
	}
}

bool BitSwash::SetLocale( wxString lang )
{
	const wxLanguageInfo * const langinfo = wxLocale::FindLanguageInfo( lang );
	int langid = wxLANGUAGE_DEFAULT;
	if( lang != _T( "" ) )
	{
		langid = langinfo->Language;
	}
	if( m_locale != NULL )
	{
		delete m_locale;
	}
	m_locale = new wxLocale( langid/*, wxLOCALE_CONV_ENCODING*/ );
	if( !m_locale->IsOk() )
	{
		wxLogError( _T( "Language %s is not supported by your system." ), langinfo->CanonicalName.c_str() );
	}
	else
	{
#ifdef  __WXMSW__
		wxString localedir =  wxStandardPaths::Get().GetDataDir() + wxFileName::GetPathSeparator( wxPATH_NATIVE ) + _T( "locale" );
#else
#ifndef PACKAGE_LOCALE_DIR
		wxString localedir =  wxString::FromAscii( PREFIX ) + _T( "/share/locale" );
#else
		wxString localedir = wxString::FromAscii( PACKAGE_LOCALE_DIR );
#endif
#endif
		m_locale->AddCatalogLookupPathPrefix( _T( "." ) );
		wxLocale::AddCatalogLookupPathPrefix( localedir );
		m_locale->AddCatalog( APPBINNAME );
		m_locale->AddCatalog( _T( "wxstd" ) );
	}
	return true;
}

void BitSwash::SetLogLevel()
{
	int wx_loglevel = wxLOG_Debug - m_config->GetLogSeverity();
	if( wx_loglevel >= wxLOG_Info )
		wxLog::SetVerbose( true );
	else
		wxLog::SetVerbose( false );
	wxLog::SetLogLevel( wx_loglevel );
}

void BitSwash::LoadIcons()
{
	int i;
	wxChar dirsep = wxFileName::GetPathSeparator( wxPATH_NATIVE );

	for( i = 0; i < BITSWASH_ICON_MAX; ++i )
	{
        wxMemoryInputStream stream(AppIcons[i].filebuf, AppIcons[i].filesize);
        AppIcons[i].image = new wxImage( stream );
	}
}

wxImage & BitSwash::GetAppIcon( enum appicon_id id )
{
	wxASSERT( id < BITSWASH_ICON_MAX );
	
	static wxImage empty( 16, 16, true );
	//wxLogMessage(_T("size %dx%d\n"), AppIcons[id].image->GetHeight(), AppIcons[id].image->GetWidth());
	if(AppIcons[id].image)
		return *AppIcons[id].image;
	else
		return empty;
}

void BitSwash::OnInitCmdLine( wxCmdLineParser& cmdParser )
{
	cmdParser.SetDesc( g_cmdLineDesc );
	// must refuse '/' as parameter starter or cannot use "/path" style paths
	cmdParser.SetSwitchChars( wxT( "-" ) );
}

bool BitSwash::OnCmdLineParsed( wxCmdLineParser& cmdParser )
{
	wxFileName filename;

	// parse commandline to filenames, every file is with a trailing char '|', ex: filename1|filename2|
	m_FileNames.Empty();
	// to get at your unnamed parameters use GetParam
	int flags = wxDIR_FILES | wxDIR_HIDDEN;
	wxString fname;

	for( size_t i = 0; i < cmdParser.GetParamCount(); i++ )
	{
		fname = cmdParser.GetParam( i );
		fname.Replace(wxT("\\\\"), wxT("\\"));
		filename = fname;

		filename.MakeAbsolute();
		fname = filename.GetFullName();

		//WildCard
		wxArrayString files;
		size_t nums = wxDir::GetAllFiles(filename.GetPath(), &files, fname, flags);

		for (size_t i = 0; i < nums; ++i)
		{
			m_FileNames.Add(files[i]);
		}
	}

	// and other command line parameters
	// then do what you need with them.
	return true;
}


#if (wxUSE_ON_FATAL_EXCEPTION == 1) && (wxUSE_STACKWALKER == 1)
#include <wx/longlong.h>
void BitSwashStackWalker::OnStackFrame( const wxStackFrame & frame )
{
	if( m_DumpFile )
	{
		wxULongLong address( ( size_t )frame.GetAddress() );
#if defined(__x86_64__) || defined(__LP64__) || defined(_WIN64)
		wxString fmt( wxT( "[%02u]:[%08X%08X] %s(%i)\t%s%s\n" ) );
#else
		wxString fmt( wxT( "[%02u]:[%08X] %s(%i)\t%s%s\n" ) );
#endif
		wxString paramInfo( wxT( "(" ) );
#if defined(_WIN32)
		wxString type, name, value;
		size_t count = frame.GetParamCount(), i = 0;

		while( i < count )
		{
			frame.GetParam( i, &type, &name, &value );
			paramInfo += type + wxT( " " ) + name + wxT( " = " ) + value;

			if( ++i < count ) paramInfo += wxT( ", " );
		}

#endif
		paramInfo += wxT( ")" );
		m_DumpFile->Write( wxString::Format( fmt,
											 ( unsigned )frame.GetLevel(),
#if defined(__x86_64__) || defined(__LP64__) || defined(_WIN64)
											 address.GetHi(),
#endif
											 address.GetLo(),
											 frame.GetFileName().c_str(),
											 ( unsigned )frame.GetLine(),
											 frame.GetName().c_str(),
											 paramInfo.c_str() )
						 );
	}
}

void BitSwash::OnFatalException()
{
	wxString name = g_BitSwashHomeDir + wxString::Format(
						wxT( "%s_%s_%lu.dmp" ),
						wxTheApp ? ( const wxChar* )wxTheApp->GetAppDisplayName().c_str()
						: wxT( "wxwindows" ),
						wxDateTime::Now().Format( wxT( "%Y%m%dT%H%M%S" ) ).c_str(),
#if defined(__WXMSW__)
						::GetCurrentProcessId()
#else
						( unsigned )getpid()
#endif
					);
	wxFile dmpFile( name.c_str(), wxFile::write );

	if( dmpFile.IsOpened() )
	{
		m_StackWalker.SetDumpFile( &dmpFile );
		m_StackWalker.WalkFromException();
		dmpFile.Close();
	}
}
#endif


