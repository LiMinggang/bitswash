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

static struct AppIcons_t
{
	enum appicon_id id;
	wxImage *image;
	wxString filename;
} AppIcons[] =
{
	{ BITSWASH_ICON_APP, NULL, _T( "bitswash.png" ) },
	{ BITSWASH_ICON_APP256, NULL, _T( "bitswash_256x256.png" ) },
	{ BITSWASH_ICON_TORRENT_OPEN, NULL, _T( "torrent_open.png" ) },
	{ BITSWASH_ICON_TORRENT_CREATE, NULL, _T( "torrent_create.png" ) },
	{ BITSWASH_ICON_TORRENT_OPENURL, NULL, _T( "torrent_open_url.png" ) },
	{ BITSWASH_ICON_TORRENT_OPENDIR, NULL, _T( "torrent_open_dir.png" ) },
	{ BITSWASH_ICON_TORRENT_START, NULL, _T( "torrent_start.png" ) },
	{ BITSWASH_ICON_TORRENT_PAUSE, NULL, _T( "torrent_pause.png" ) },
	{ BITSWASH_ICON_TORRENT_STOP, NULL, _T( "torrent_stop.png" ) },
	{ BITSWASH_ICON_TORRENT_PROPERTIES, NULL, _T( "torrent_properties.png" ) },
	{ BITSWASH_ICON_TORRENT_REMOVE, NULL, _T( "torrent_remove.png" ) },
	{ BITSWASH_ICON_TORRENT_REMOVEDATA, NULL, _T( "torrent_remove_data.png" ) },
	{ BITSWASH_ICON_TORRENT_MOVEUP, NULL, _T( "torrent_move_up.png" ) },
	{ BITSWASH_ICON_TORRENT_MOVEDOWN, NULL, _T( "torrent_move_down.png" ) },
	{ BITSWASH_ICON_OPTION_PREFERENCE, NULL, _T( "option_preference.png" ) },
	{ BITSWASH_ICON_MAX, NULL, _T( "" ) }
};

static struct CountryFlags_t
{
	const wxChar code[4];
	wxString name;
	int imgidx;             /* index in the imagelist */
} CountryFlags[] =
{
	{ _T( "--" ), _( "Unknown" ) },
	{ _T( "AF" ), _( "Afghanistan" ) },
	{ _T( "AL" ), _( "Albania" ) },
	{ _T( "DZ" ), _( "Algeria" ) },
	{ _T( "AS" ), _( "American Samoa" ) },
	{ _T( "AD" ), _( "Andorra" ) },
	{ _T( "AO" ), _( "Angola" ) },
	{ _T( "AI" ), _( "Anguilla" ) },
	{ _T( "AG" ), _( "Antigua and Barbuda" ) },
	{ _T( "AR" ), _( "Argentina" ) },
	{ _T( "AM" ), _( "Armenia" ) },
	{ _T( "AW" ), _( "Aruba" ) },
	{ _T( "AU" ), _( "Australia" ) },
	{ _T( "AT" ), _( "Austria" ) },
	{ _T( "AZ" ), _( "Azerbaijan" ) },
	{ _T( "BS" ), _( "Bahamas" ) },
	{ _T( "BH" ), _( "Bahrain" ) },
	{ _T( "BD" ), _( "Bangladesh" ) },
	{ _T( "BB" ), _( "Barbados" ) },
	{ _T( "BY" ), _( "Belarus" ) },
	{ _T( "BE" ), _( "Belgium" ) },
	{ _T( "BZ" ), _( "Belize" ) },
	{ _T( "BJ" ), _( "Benin" ) },
	{ _T( "BM" ), _( "Bermuda" ) },
	{ _T( "BT" ), _( "Bhutan" ) },
	{ _T( "BO" ), _( "Bolivia" ) },
	{ _T( "BA" ), _( "Bosnia and Herzegovina" ) },
	{ _T( "BW" ), _( "Botswana" ) },
	{ _T( "BV" ), _( "Bouvet Island Bouvetoya" ) },
	{ _T( "BR" ), _( "Brazil" ) },
	{ _T( "IO" ), _( "British Indian Ocean Territory Chagos Archipelago" ) },
	{ _T( "VG" ), _( "British Virgin Islands" ) },
	{ _T( "BN" ), _( "Brunei Darussalam" ) },
	{ _T( "BG" ), _( "Bulgaria" ) },
	{ _T( "BF" ), _( "Burkina Faso" ) },
	{ _T( "BI" ), _( "Burundi" ) },
	{ _T( "KH" ), _( "Cambodia" ) },
	{ _T( "CM" ), _( "Cameroon" ) },
	{ _T( "CA" ), _( "Canada" ) },
	{ _T( "CV" ), _( "Cape Verde" ) },
	{ _T( "KY" ), _( "Cayman Islands" ) },
	{ _T( "CF" ), _( "Central African Republic" ) },
	{ _T( "TD" ), _( "Chad" ) },
	{ _T( "CL" ), _( "Chile" ) },
	{ _T( "CN" ), _( "China" ) },
	{ _T( "CX" ), _( "Christmas Island" ) },
	{ _T( "CC" ), _( "Cocos Keeling Islands" ) },
	{ _T( "CO" ), _( "Colombia" ) },
	{ _T( "KM" ), _( "Comoros" ) },
	{ _T( "CD" ), _( "Congo" ) },
	{ _T( "CG" ), _( "Congo" ) },
	{ _T( "CK" ), _( "Cook Islands" ) },
	{ _T( "CR" ), _( "Costa Rica" ) },
	{ _T( "CI" ), _( "Cote D'Ivoire" ) },
	{ _T( "CU" ), _( "Cuba" ) },
	{ _T( "CY" ), _( "Cyprus" ) },
	{ _T( "CZ" ), _( "Czech Republic" ) },
	{ _T( "DK" ), _( "Denmark" ) },
	{ _T( "DJ" ), _( "Djibouti" ) },
	{ _T( "DM" ), _( "Dominica" ) },
	{ _T( "DO" ), _( "Dominican Republic" ) },
	{ _T( "EC" ), _( "Ecuador" ) },
	{ _T( "EG" ), _( "Egypt" ) },
	{ _T( "SV" ), _( "El Salvador" ) },
	{ _T( "GQ" ), _( "Equatorial Guinea" ) },
	{ _T( "ER" ), _( "Eritrea" ) },
	{ _T( "EE" ), _( "Estonia" ) },
	{ _T( "ET" ), _( "Ethiopia" ) },
	{ _T( "EU" ), _( "Europe" ) },
	{ _T( "FO" ), _( "Faeroe Islands" ) },
	{ _T( "FK" ), _( "Falkland Islands Malvinas" ) },
	{ _T( "FJ" ), _( "Fiji" ) },
	{ _T( "FI" ), _( "Finland" ) },
	{ _T( "FR" ), _( "France" ) },
	{ _T( "GF" ), _( "French Guiana" ) },
	{ _T( "PF" ), _( "French Polynesia" ) },
	{ _T( "TF" ), _( "French Southern Territories" ) },
	{ _T( "GA" ), _( "Gabon" ) },
	{ _T( "GM" ), _( "Gambia" ) },
	{ _T( "GE" ), _( "Georgia" ) },
	{ _T( "DE" ), _( "Germany" ) },
	{ _T( "GH" ), _( "Ghana" ) },
	{ _T( "GI" ), _( "Gibraltar" ) },
	{ _T( "GR" ), _( "Greece" ) },
	{ _T( "GL" ), _( "Greenland" ) },
	{ _T( "GD" ), _( "Grenada" ) },
	{ _T( "GP" ), _( "Guadaloupe" ) },
	{ _T( "GU" ), _( "Guam" ) },
	{ _T( "GT" ), _( "Guatemala" ) },
	{ _T( "GN" ), _( "Guinea" ) },
	{ _T( "GW" ), _( "Guinea-Bissau" ) },
	{ _T( "GY" ), _( "Guyana" ) },
	{ _T( "HT" ), _( "Haiti" ) },
	{ _T( "HM" ), _( "Heard and McDonald Islands" ) },
	{ _T( "VA" ), _( "Holy See Vatican City State" ) },
	{ _T( "HN" ), _( "Honduras" ) },
	{ _T( "HK" ), _( "Hong Kong" ) },
	{ _T( "HR" ), _( "Hrvatska Croatia" ) },
	{ _T( "HU" ), _( "Hungary" ) },
	{ _T( "IS" ), _( "Iceland" ) },
	{ _T( "IN" ), _( "India" ) },
	{ _T( "ID" ), _( "Indonesia" ) },
	{ _T( "IR" ), _( "Iran" ) },
	{ _T( "IQ" ), _( "Iraq" ) },
	{ _T( "IE" ), _( "Ireland" ) },
	{ _T( "IL" ), _( "Israel" ) },
	{ _T( "IT" ), _( "Italy" ) },
	{ _T( "JM" ), _( "Jamaica" ) },
	{ _T( "JP" ), _( "Japan" ) },
	{ _T( "JO" ), _( "Jordan" ) },
	{ _T( "KZ" ), _( "Kazakhstan" ) },
	{ _T( "KE" ), _( "Kenya" ) },
	{ _T( "KI" ), _( "Kiribati" ) },
	{ _T( "KP" ), _( "Korea" ) },
	{ _T( "KR" ), _( "Korea" ) },
	{ _T( "KW" ), _( "Kuwait" ) },
	{ _T( "KG" ), _( "Kyrgyz Republic" ) },
	{ _T( "LA" ), _( "Lao People's Democratic Republic" ) },
	{ _T( "LV" ), _( "Latvia" ) },
	{ _T( "LB" ), _( "Lebanon" ) },
	{ _T( "LS" ), _( "Lesotho" ) },
	{ _T( "LR" ), _( "Liberia" ) },
	{ _T( "LY" ), _( "Libyan Arab Jamahiriya" ) },
	{ _T( "LI" ), _( "Liechtenstein" ) },
	{ _T( "LT" ), _( "Lithuania" ) },
	{ _T( "LU" ), _( "Luxembourg" ) },
	{ _T( "MO" ), _( "Macao" ) },
	{ _T( "MK" ), _( "Macedonia" ) },
	{ _T( "MG" ), _( "Madagascar" ) },
	{ _T( "MW" ), _( "Malawi" ) },
	{ _T( "MY" ), _( "Malaysia" ) },
	{ _T( "MV" ), _( "Maldives" ) },
	{ _T( "ML" ), _( "Mali" ) },
	{ _T( "MT" ), _( "Malta" ) },
	{ _T( "MH" ), _( "Marshall Islands" ) },
	{ _T( "MQ" ), _( "Martinique" ) },
	{ _T( "MR" ), _( "Mauritania" ) },
	{ _T( "MU" ), _( "Mauritius" ) },
	{ _T( "YT" ), _( "Mayotte" ) },
	{ _T( "MX" ), _( "Mexico" ) },
	{ _T( "FM" ), _( "Micronesia" ) },
	{ _T( "MD" ), _( "Moldova" ) },
	{ _T( "MC" ), _( "Monaco" ) },
	{ _T( "MN" ), _( "Mongolia" ) },
	{ _T( "MS" ), _( "Montserrat" ) },
	{ _T( "MA" ), _( "Morocco" ) },
	{ _T( "MZ" ), _( "Mozambique" ) },
	{ _T( "MM" ), _( "Myanmar" ) },
	{ _T( "NA" ), _( "Namibia" ) },
	{ _T( "NR" ), _( "Nauru" ) },
	{ _T( "NP" ), _( "Nepal" ) },
	{ _T( "AN" ), _( "Netherlands Antilles" ) },
	{ _T( "NL" ), _( "Netherlands" ) },
	{ _T( "NC" ), _( "New Caledonia" ) },
	{ _T( "NZ" ), _( "New Zealand" ) },
	{ _T( "NI" ), _( "Nicaragua" ) },
	{ _T( "NE" ), _( "Niger" ) },
	{ _T( "NG" ), _( "Nigeria" ) },
	{ _T( "NU" ), _( "Niue" ) },
	{ _T( "NF" ), _( "Norfolk Island" ) },
	{ _T( "MP" ), _( "Northern Mariana Islands" ) },
	{ _T( "NO" ), _( "Norway" ) },
	{ _T( "OM" ), _( "Oman" ) },
	{ _T( "PK" ), _( "Pakistan" ) },
	{ _T( "PW" ), _( "Palau" ) },
	{ _T( "PS" ), _( "Palestinian Territory" ) },
	{ _T( "PA" ), _( "Panama" ) },
	{ _T( "PG" ), _( "Papua New Guinea" ) },
	{ _T( "PY" ), _( "Paraguay" ) },
	{ _T( "PE" ), _( "Peru" ) },
	{ _T( "PH" ), _( "Philippines" ) },
	{ _T( "PN" ), _( "Pitcairn Island" ) },
	{ _T( "PL" ), _( "Poland" ) },
	{ _T( "PT" ), _( "Portugal" ) },
	{ _T( "PR" ), _( "Puerto Rico" ) },
	{ _T( "QA" ), _( "Qatar" ) },
	{ _T( "RE" ), _( "Reunion" ) },
	{ _T( "RO" ), _( "Romania" ) },
	{ _T( "RU" ), _( "Russian Federation" ) },
	{ _T( "RW" ), _( "Rwanda" ) },
	{ _T( "SH" ), _( "St. Helena" ) },
	{ _T( "KN" ), _( "St. Kitts and Nevis" ) },
	{ _T( "LC" ), _( "St. Lucia" ) },
	{ _T( "PM" ), _( "St. Pierre and Miquelon" ) },
	{ _T( "VC" ), _( "St. Vincent and the Grenadines" ) },
	{ _T( "WS" ), _( "Samoa" ) },
	{ _T( "SM" ), _( "San Marino" ) },
	{ _T( "ST" ), _( "Sao Tome and Principe" ) },
	{ _T( "SA" ), _( "Saudi Arabia" ) },
	{ _T( "SN" ), _( "Senegal" ) },
	{ _T( "CS" ), _( "Serbia and Montenegro" ) },
	{ _T( "SC" ), _( "Seychelles" ) },
	{ _T( "SL" ), _( "Sierra Leone" ) },
	{ _T( "SG" ), _( "Singapore" ) },
	{ _T( "SK" ), _( "Slovakia Slovak Republic" ) },
	{ _T( "SI" ), _( "Slovenia" ) },
	{ _T( "SB" ), _( "Solomon Islands" ) },
	{ _T( "SO" ), _( "Somalia" ) },
	{ _T( "ZA" ), _( "South Africa" ) },
	{ _T( "GS" ), _( "South Georgia and the South Sandwich Islands" ) },
	{ _T( "ES" ), _( "Spain" ) },
	{ _T( "LK" ), _( "Sri Lanka" ) },
	{ _T( "SD" ), _( "Sudan" ) },
	{ _T( "SR" ), _( "Suriname" ) },
	{ _T( "SJ" ), _( "Svalbard & Jan Mayen Islands" ) },
	{ _T( "SZ" ), _( "Swaziland" ) },
	{ _T( "SE" ), _( "Sweden" ) },
	{ _T( "CH" ), _( "Switzerland" ) },
	{ _T( "SY" ), _( "Syrian Arab Republic" ) },
	{ _T( "TW" ), _( "Taiwan" ) },
	{ _T( "TJ" ), _( "Tajikistan" ) },
	{ _T( "TZ" ), _( "Tanzania" ) },
	{ _T( "TH" ), _( "Thailand" ) },
	{ _T( "TL" ), _( "Timor-Leste" ) },
	{ _T( "TG" ), _( "Togo" ) },
	{ _T( "TK" ), _( "Tokelau Tokelau Islands" ) },
	{ _T( "TO" ), _( "Tonga" ) },
	{ _T( "TT" ), _( "Trinidad and Tobago" ) },
	{ _T( "TN" ), _( "Tunisia" ) },
	{ _T( "TR" ), _( "Turkey" ) },
	{ _T( "TM" ), _( "Turkmenistan" ) },
	{ _T( "TC" ), _( "Turks and Caicos Islands" ) },
	{ _T( "TV" ), _( "Tuvalu" ) },
	{ _T( "VI" ), _( "US Virgin Islands" ) },
	{ _T( "UG" ), _( "Uganda" ) },
	{ _T( "UA" ), _( "Ukraine" ) },
	{ _T( "AE" ), _( "United Arab Emirates" ) },
	{ _T( "GB" ), _( "United Kingdom of Great Britain & N. Ireland" ) },
	{ _T( "UM" ), _( "United States Minor Outlying Islands" ) },
	{ _T( "US" ), _( "United States of America" ) },
	{ _T( "UY" ), _( "Uruguay" ) },
	{ _T( "UZ" ), _( "Uzbekistan" ) },
	{ _T( "VU" ), _( "Vanuatu" ) },
	{ _T( "VE" ), _( "Venezuela" ) },
	{ _T( "VN" ), _( "Viet Nam" ) },
	{ _T( "WF" ), _( "Wallis and Futuna Islands" ) },
	{ _T( "EH" ), _( "Western Sahara" ) },
	{ _T( "YE" ), _( "Yemen" ) },
	{ _T( "ZM" ), _( "Zambia" ) },
	{ _T( "ZW" ), _( "Zimbabwe" ) },
	{ _T( "" ), _T( "" )}
};

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
	m_SigleAppChecker = 0;
	m_AppServer = 0;

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
			wxLogDebug( wxGetTranslation(wxT( "Failed to create an IPC service." ) ));
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
			wxMessageBox( wxGetTranslation(wxT( "Sorry, the existing instance may be too busy too respond.\nPlease close any open dialogs and retry." )),
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
#ifndef __WXMSW__
	m_iconspath =  wxString::FromAscii( PREFIX ) + _T( "/share/bitswash/" ) + RESOURCE_DIR;
#else
	m_iconspath = wxFileName::GetCwd() + dirsep +  _T( "icons" );
#endif
	wxInitAllImageHandlers();
	if( !wxFileName::DirExists( m_iconspath ) )
	{
		m_iconspath = wxFileName::GetCwd() + _T( "/../icons" );
		if( ! wxFileName::DirExists( m_iconspath ) )
		{
			wxLogError( _T( "Failed to locate icons %s directory " ), m_iconspath.c_str() );
		}
	}
#ifndef __WXMSW__
	m_flagspath =  wxString::FromAscii( PREFIX ) + _T( "/share/bitswash/" ) + RESOURCE_DIR + dirsep + _T( "flags" );
#else
	m_flagspath = wxFileName::GetCwd() + dirsep + _T( "icons" ) + dirsep + _T( "flags" );
#endif
	if( !wxFileName::DirExists( m_flagspath ) )
	{
		m_flagspath = wxFileName::GetCwd() + _T( "/../icons/flags" );
		if( ! wxFileName::DirExists( m_flagspath ) )
		{
			wxLogError( _T( "Failed to locate flags %s directory " ), m_flagspath.c_str() );
		}
	}
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

	return 0;
}

#define N_COUNTRY sizeof(CountryFlags) / sizeof(struct CountryFlags_t)
void BitSwash::LoadFlags()
{
	int i;
	wxChar dirsep = wxFileName::GetPathSeparator( wxPATH_NATIVE );
	wxString un = wxString::Format( _T( "%s%c%s%s" ), m_flagspath.c_str(), dirsep, "un", _T( ".png" ) );
	wxImage empty(un);
	int emptyidx = 0;
	m_imglist_ctryflags = new wxImageList( 16, 11, false );
	emptyidx = m_imglist_ctryflags->Add( wxBitmap( empty ) );
	for( i = 0; i < N_COUNTRY ; ++i )
	{
		CountryCodeIndexMap[wxString( CountryFlags[i].code )] = i;
		if( ( wxIsEmpty( CountryFlags[i].code ) ) || ( ! wxStricmp( CountryFlags[i].code, _T( "--" ) ) ) )
		{
			CountryFlags[i].imgidx = emptyidx;
			continue;
		}
		wxString filename = wxString::Format( _T( "%s%c%s%s" ), m_flagspath.c_str(), dirsep, wxString( CountryFlags[i].code ).Lower().c_str(), _T( ".png" ) );
		wxImage flags;
		if( ( flags.LoadFile( filename ) ) )
		{
			CountryFlags[i].imgidx = m_imglist_ctryflags->Add( wxBitmap( flags ) );
		}
		else
		{
			wxLogWarning( _T( "%s Flags %s not found \n" ), CountryFlags[i].name.c_str(), CountryFlags[i].code );
			CountryFlags[i].imgidx = -1;
		}
	}
}

int BitSwash::GetCountryFlag( const wxString& code )
{
	if( code.IsEmpty() )
		return 0;
	std::map<wxString, int>::iterator it = CountryCodeIndexMap.find( code );
	if( it != CountryCodeIndexMap.end() )
	{
		wxASSERT( it->second < N_COUNTRY );
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
	int i;
	wxChar dirsep = wxFileName::GetPathSeparator( wxPATH_NATIVE );
	wxImage empty( 16, 16, true );
	wxImage flags;
	m_imglist_settingicons = new wxImageList( 16, 16, false );
	static const wxChar* settingfiles[] =
	{
		_T( "settinggeneral.png" ),
		_T( "settingconnection.png" ),
		_T( "settingtorrent.png" ),
		_T( "settingextension.png" )
	};
	for( i = 0; i < 4; i++ )
	{
		wxString filename = wxString::Format( _T( "%s%c%s" ), m_iconspath.c_str(), dirsep,  settingfiles[i] );
		if( ( flags.LoadFile( filename ) ) )
		{
			m_imglist_settingicons->Add( wxBitmap( flags ) );
		}
		else
		{
			m_imglist_settingicons->Add( wxBitmap( empty ) );
			wxLogWarning( _T( "Configuration icon %s not found \n" ), filename.c_str() );
		}
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
		wxString filename = m_iconspath + dirsep + AppIcons[i].filename ;
		wxImage icon;
		if( wxFileExists( filename ) )
		{
			AppIcons[i].image = new wxImage( filename );
		}
		else
		{
			wxLogWarning( _T( "Icon file %s not found \n" ), filename.c_str() );
		}
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


