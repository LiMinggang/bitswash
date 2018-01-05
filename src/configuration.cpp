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
// Class: Configuration
//
// Created by: lim k. c. <admin@bitswash.org>
// Created on: Mon Feb  3 22:11:24 2007
//
#include <wx/stdpaths.h>
#include <wx/settings.h>
#include <wx/filename.h>
#include <wx/log.h>
#include <wx/window.h>
#include <wx/wfstream.h>

#include "bitswash.h"
#include "functions.h"
#include "configuration.h"

//namespace lt = libtorrent;

#ifdef __WXMSW__
wxString Configuration::m_startup_regkey = wxT( "HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run" );
wxString Configuration::m_bitswash_regkey_path = wxT( "HKEY_CURRENT_USER\\Software\\Classes\\" );
#endif

Configuration::Configuration( const wxString& AppName )
	: wxFileConfig( wxEmptyString, wxEmptyString, AppName, wxEmptyString, wxCONFIG_USE_LOCAL_FILE | wxCONFIG_USE_RELATIVE_PATH )
{
	//ctor
	m_configfile        = wxGetApp().ConfigPath();
	wxFileInputStream fis( m_configfile );
	m_appname = AppName;
	//wxLogDebug( _T( "Init Configuration %s\n" ), AppName.c_str() );
	//m_cfg = (wxConfig *) new wxFileConfig( (wxInputStream &)fis);
	m_cfg = new wxFileConfig( ( wxInputStream & )fis );
	Load();
}

Configuration::~Configuration()
{
	delete wxConfigBase::Set( ( wxConfigBase* )nullptr );
	//dtor
}

void Configuration::Save()
{
	//GUI
	m_cfg->Write( _T( "/GUI/x" ), ( long ) m_gui.x );
	m_cfg->Write( _T( "/GUI/y" ), ( long ) m_gui.y );
	m_cfg->Write( _T( "/GUI/w" ), ( long ) m_gui.w );
	m_cfg->Write( _T( "/GUI/h" ), ( long ) m_gui.h + 54 );
	m_cfg->Write( _T( "/GUI/guiversion" ), ( long )m_guiversion );
	m_cfg->Write( _T( "/GUI/auiperspective" ), m_auiperspective );
	m_cfg->Write( _T( "/GUI/torrentlistctrlsetting" ), m_torrentlistctrlsetting );
	m_cfg->Write( _T( "/GUI/peerlistctrlsetting" ), m_peerlistctrlsetting );
	m_cfg->Write( _T( "/GUI/filelistctrlsetting" ), m_filelistctrlsetting );
	m_cfg->Write( _T( "/GUI/trackerlistctrlsetting" ), m_trackerlistctrlsetting );
	//Config
	m_cfg->Write( _T( "/Config/downloadpath" ), m_downloadpath );
	m_cfg->Write( _T( "/Config/portmin" ), ( int )m_portmin );
	m_cfg->Write( _T( "/Config/portmax" ), ( int )m_portmax );
	m_cfg->Write( _T( "/Config/refreshtime" ), ( int )m_refreshtime );
	m_cfg->Write( _T( "/Config/fastresume_save_time" ), ( int )m_fastresume_save_time );
	m_cfg->Write( _T( "/Config/max_start" ), ( int )m_max_start );
	m_cfg->Write( _T( "/Config/use_systray" ), ( bool )m_use_systray );
	m_cfg->Write( _T( "/Config/hidetaskbar" ), ( bool )m_hidetaskbar );
	m_cfg->Write( _T( "/Config/exclude_seed" ), ( bool )m_exclude_seed );
#ifdef __WXMSW__
	m_cfg->Write( _T( "/Config/run_at_startup" ), ( bool )m_run_at_startup );
	wxRegKey regKey( m_startup_regkey );
	if( regKey.Exists() )
	{
		wxString exepath;
		if( !m_run_at_startup )
		{
			if( regKey.QueryValue( APPNAME, exepath ) )
				regKey.DeleteValue( APPNAME );
		}
		else
		{
			exepath = GetExecutablePath();
			regKey.SetValue( APPNAME, exepath );
		}
	}
	
	m_cfg->Write( _T( "/Config/associate_magneturi" ), m_associate_magneturi );
	if (m_associate_magneturi)
		AddMagnetLinkType();
	else
		RemoveMagnetLinkType();
	m_cfg->Write( _T( "/Config/associate_torrent" ), m_associate_torrent);
	if(m_associate_torrent)
		AddType( _T(".torrent") );
	else
		RemoveType(_T(".torrent"));
#endif
	m_cfg->Write( _T( "/Config/enable_upnp" ), ( bool )m_enable_upnp );
	m_cfg->Write( _T( "/Config/enable_natpmp" ), ( bool )m_enable_natpmp );
	m_cfg->Write( _T( "/Config/enable_lsd" ), ( bool )m_enable_lsd );
	m_cfg->Write( _T( "/Config/enable_metadata" ), ( bool )m_enable_metadata );
	m_cfg->Write( _T( "/Config/enable_pex" ), ( bool )m_enable_pex );
	m_cfg->Write( _T( "/Config/language" ), m_language );
	//session
	m_cfg->Write( _T( "/Session/tracker_completion_timeout" ), m_tracker_completion_timeout );
	m_cfg->Write( _T( "/Session/tracker_receive_timeout" ), m_tracker_receive_timeout );
	m_cfg->Write( _T( "/Session/stop_tracker_timeout" ), m_stop_tracker_timeout );
	m_cfg->Write( _T( "/Session/tracker_maximum_response_length" ), m_tracker_maximum_response_length );
	m_cfg->Write( _T( "/Session/piece_timeout" ), m_piece_timeout );
	m_cfg->Write( _T( "/Session/request_queue_time" ), m_request_queue_time );
	m_cfg->Write( _T( "/Session/max_allowed_in_request_queue" ), m_max_allowed_in_request_queue );
	m_cfg->Write( _T( "/Session/max_out_request_queue" ), m_max_out_request_queue );
	m_cfg->Write( _T( "/Session/whole_pieces_threshold" ), m_whole_pieces_threshold );
	m_cfg->Write( _T( "/Session/peer_timeout" ), m_peer_timeout );
	m_cfg->Write( _T( "/Session/urlseed_timeout" ), m_urlseed_timeout );
	m_cfg->Write( _T( "/Session/urlseed_pipeline_size" ), m_urlseed_pipeline_size );
	m_cfg->Write( _T( "/Session/file_pool_size" ), m_file_pool_size );
	m_cfg->Write( _T( "/Session/allow_multiple_connections_per_ip" ), m_allow_multiple_connections_per_ip );
	m_cfg->Write( _T( "/Session/max_failcount" ), m_max_failcount ) ;
	m_cfg->Write( _T( "/Session/min_reconnect_time" ), m_min_reconnect_time );
	m_cfg->Write( _T( "/Session/peer_connect_timeout" ), m_peer_connect_timeout );
	m_cfg->Write( _T( "/Session/ignore_limits_on_local_network" ), m_ignore_limits_on_local_network );
	m_cfg->Write( _T( "/Session/connection_speed" ), m_connection_speed );
	m_cfg->Write( _T( "/Session/send_redundant_have" ), m_send_redundant_have );
	m_cfg->Write( _T( "/Session/lazy_bitfields" ), m_lazy_bitfields );
	m_cfg->Write( _T( "/Session/inactivity_timeout" ), m_inactivity_timeout );
	m_cfg->Write( _T( "/Session/unchoke_interval" ), m_unchoke_interval );
	m_cfg->Write( _T( "/Session/optimistic_unchoke_multiplier" ), m_optimistic_unchoke_multiplier );
	m_cfg->Write( _T( "/Session/num_want" ), m_num_want );
	m_cfg->Write( _T( "/Session/initial_picker_threshold" ), m_initial_picker_threshold );
	m_cfg->Write( _T( "/Session/allowed_fast_set_size" ), m_allowed_fast_set_size );
	m_cfg->Write( _T( "/Session/max_outstanding_disk_bytes_per_connection" ), m_max_outstanding_disk_bytes_per_connection );
	m_cfg->Write( _T( "/Session/handshake_timeout" ), m_handshake_timeout );
	m_cfg->Write( _T( "/Session/use_dht_as_fallback" ), m_use_dht_as_fallback );
	m_cfg->Write( _T( "/Session/free_torrent_hashes" ), m_free_torrent_hashes );
	m_cfg->Write( _T( "/Session/use_oscache" ), ( bool )m_useoscache );
	//legacy
	//if( m_storagemode != lt::storage_mode_compact )
	m_cfg->Write( _T( "/Torrent/storagemode" ), ( long )m_storagemode );
	m_cfg->Write( _T( "/Torrent/global_upload_limit" ), m_global_upload_limit );
	m_cfg->Write( _T( "/Torrent/global_download_limit" ), m_global_download_limit );
	m_cfg->Write( _T( "/Torrent/global_max_connections" ), m_global_max_connections );
	if( m_global_max_uploads < 2 ) m_global_max_uploads = 2;
	m_cfg->Write( _T( "/Torrent/global_max_uploads" ), m_global_max_uploads );
	m_cfg->Write( _T( "/Torrent/use_default" ), ( bool )m_usedefault );
	//dht
	//
	m_cfg->Write( _T( "/DHT/dht_enabled" ), ( bool )m_dht_enabled );
	m_cfg->Write( _T( "/DHT/dht_port" ), ( int )m_dht_port );
	m_cfg->Write( _T( "/DHT/dht_max_peers" ), ( int )m_dht_max_peers );
	m_cfg->Write( _T( "/DHT/dht_search_branching" ), ( int )m_dht_search_branching );
	m_cfg->Write( _T( "/DHT/dht_max_fail" ), ( int )m_dht_max_fail );
	//encryption
	m_cfg->Write( _T( "/Encryption/enc_enabled" ), ( bool )m_enc_enabled );
	m_cfg->Write( _T( "/Encryption/enc_policy" ), ( int )m_enc_policy );
	m_cfg->Write( _T( "/Encryption/enc_level" ), ( int )m_enc_level );
	m_cfg->Write( _T( "/Encryption/enc_prefer_rc4" ), ( bool )m_enc_enabled );
	//per torrent
	//  m_cfg->Write(_T("/Torrent/default_state"), m_default_state);
	m_cfg->Write( _T( "/Torrent/default_state" ), ( int )m_default_state );
	m_cfg->Write( _T( "/Torrent/default_ratio" ), m_default_ratio );
	m_cfg->Write( _T( "/Torrent/default_upload_limit" ), m_default_upload_limit );
	m_cfg->Write( _T( "/Torrent/default_download_limit" ), m_default_download_limit );
	m_cfg->Write( _T( "/Torrent/default_max_connections" ), m_default_max_connections );
	if( m_default_max_uploads < 2 ) m_default_max_uploads = 2;
	m_cfg->Write( _T( "/Torrent/default_max_uploads" ), m_default_max_uploads );
	m_cfg->Write( _T( "/Torrent/enable_video_preview" ), ( bool )m_enable_video_preview );
	m_cfg->Write( _T( "/Torrent/sequential_download" ), ( bool )m_sequential_download );
	//logging
	m_cfg->Write( _T( "/Log/log_severity" ), ( int )m_log_severity );
	m_cfg->Write( _T( "/Log/log_linecount" ), ( int )m_log_linecount );
	m_cfg->Write( _T( "/Log/log_file" ), ( bool )m_log_file );
	m_cfg->Write( _T( "/Log/log_max_files" ), ( int )m_max_log_files );
	wxFileOutputStream fos( m_configfile );
	m_cfg->Save( fos );
	WriteSavePath();
}

void Configuration::Load()
{
#ifdef __WXMSW__
	wxString def_download_path = wxStandardPaths::Get().GetDataDir();
#else
	wxString def_download_path = wxStandardPaths::Get().GetUserConfigDir();
#endif
	wxFileName dn;
	dn.AssignDir(def_download_path);
	dn.AssignDir(dn.GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR) + _T("Downloads"));
	def_download_path = dn.GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR);

	//GUI
	int default_width = wxSystemSettings::GetMetric( wxSYS_SCREEN_X );
	int default_height = wxSystemSettings::GetMetric( wxSYS_SCREEN_Y ) ;
	default_width = ( default_width > 800 ) ? 800 : default_width;
	default_height = ( default_height > 600 ) ? 600 : default_height;
	m_gui.x = m_cfg->Read( _T( "/GUI/x" ), ( long )0 );
	m_gui.y = m_cfg->Read( _T( "/GUI/y" ), ( long )0 );
	m_gui.w = m_cfg->Read( _T( "/GUI/w" ), default_width );
	m_gui.h = m_cfg->Read( _T( "/GUI/h" ), default_height );
	m_guiversion = m_cfg->Read( _T( "/GUI/guiversion" ), ( long )0 );
	if( m_guiversion == BITSWASH_GUIVERSION )
		m_auiperspective = m_cfg->Read( _T( "/GUI/auiperspective" ), wxEmptyString );
	else
		m_auiperspective = wxEmptyString;
	if( m_guiversion == 0 )
	{
		m_guiversion = BITSWASH_GUIVERSION;
	}
	m_torrentlistctrlsetting = m_cfg->Read( _T( "/GUI/torrentlistctrlsetting" ), wxEmptyString );
	m_peerlistctrlsetting = m_cfg->Read( _T( "/GUI/peerlistctrlsetting" ), wxEmptyString );
	m_filelistctrlsetting = m_cfg->Read( _T( "/GUI/filelistctrlsetting" ), wxEmptyString );
	m_trackerlistctrlsetting = m_cfg->Read( _T( "/GUI/trackerlistctrlsetting" ), wxEmptyString );
	//config
	wxFileName dpath;
	dpath.AssignDir(m_cfg->Read( _T( "/Config/downloadpath" ), def_download_path ));
	m_downloadpath = dpath.GetPathWithSep();
	m_portmin = m_cfg->Read( _T( "/Config/portmin" ), 16881 );
	m_portmax = m_cfg->Read( _T( "/Config/portmax" ), 16889 );
	m_refreshtime = m_cfg->Read( _T( "/Config/refreshtime" ), 5 );
	//legacy
	if( m_refreshtime >= 1000 )
		m_refreshtime /= 1000;
	//save fast resume every 10 minutes
	m_fastresume_save_time = m_cfg->Read( _T( "/Config/fastresume_save_time" ), 600 );
	m_max_start = m_cfg->Read( _T( "/Config/max_start" ), 5 );
	if( ( m_max_start > 100 ) || ( m_max_start <= 0 ) )  m_max_start = 2;
	m_log_severity = m_cfg->Read( _T( "/Log/log_severity" ), 2 );
	if( ( m_log_severity > 7 ) || ( m_log_severity <= 0 ) )  m_log_severity = 2;
	m_log_linecount = m_cfg->Read( _T( "/Log/log_linecount" ), 100 );
	if( ( m_log_linecount > 1000 ) || ( m_log_linecount < 100 ) )  m_log_linecount = 100;
	m_cfg->Read( _T( "/Log/log_file" ), &m_log_file, false );
	m_max_log_files = m_cfg->Read( _T( "/Log/log_max_files" ), 5 );
	if(m_max_log_files <= 0) m_max_log_files = 5;
	if(m_max_log_files > 20) m_max_log_files = 20;
	m_cfg->Read( _T( "/Config/use_systray" ), &m_use_systray, true );
	//if (! (m_cfg->Read(_T("/Config/use_systray"), &m_use_systray))) m_use_systray = true;
	m_cfg->Read( _T( "/Config/hidetaskbar" ), &m_hidetaskbar, false ) ;
	m_cfg->Read( _T( "/Config/exclude_seed" ), &m_exclude_seed, true );
#ifdef __WXMSW__
	m_cfg->Read( _T( "/Config/run_at_startup" ), &m_run_at_startup, false );
	wxRegKey regKey( m_startup_regkey );
	if( regKey.Exists() )
	{
		if( !m_run_at_startup )
		{
			wxString value;
			if( regKey.QueryValue( APPNAME, value ) )
				regKey.DeleteValue( APPNAME );
		}
		else
		{
			wxString exepath( _T( "\"" ) );
			exepath += GetExecutablePath() + _T( "\"" );
			regKey.SetValue( APPNAME, exepath );
		}
	}

	m_cfg->Read( _T( "/Config/associate_torrent" ), &m_associate_torrent, false );
	if(m_associate_torrent)
		AddType( _T(".torrent") );
	else
		RemoveType(_T(".torrent"));
	m_cfg->Read( _T( "/Config/associate_magneturi" ), &m_associate_magneturi, false );
	if (m_associate_magneturi)
		AddMagnetLinkType();
	else
		RemoveMagnetLinkType();
#endif
	//
	m_cfg->Read( _T( "/Config/enable_upnp" ), &m_enable_upnp, true );
	m_cfg->Read( _T( "/Config/enable_natpmp" ), &m_enable_natpmp, true );
	m_cfg->Read( _T( "/Config/enable_lsd" ), &m_enable_lsd, true );
	m_cfg->Read( _T( "/Config/enable_metadata" ), &m_enable_metadata, true );
	m_cfg->Read( _T( "/Config/enable_pex" ), &m_enable_pex, true );
	m_language = m_cfg->Read( _T( "/Config/language" ), _T( "" ) );
	//session
	//
	m_tracker_completion_timeout = m_cfg->Read( _T( "/Session/tracker_completion_timeout" ), 60 );
	m_tracker_receive_timeout = m_cfg->Read( _T( "/Session/tracker_receive_timeout" ), 20 );
	m_stop_tracker_timeout = m_cfg->Read( _T( "/Session/stop_tracker_timeout" ), 5 );
	m_tracker_maximum_response_length = m_cfg->Read( _T( "/Session/tracker_maximum_response_length" ), 1024 * 1024 );
	m_piece_timeout = m_cfg->Read( _T( "/Session/piece_timeout" ), 10 );
	m_cfg->Read( _T( "/Session/request_queue_time" ), &m_request_queue_time, ( double )3.f );
	m_max_allowed_in_request_queue = m_cfg->Read( _T( "/Session/max_allowed_in_request_queue" ), 250 );
	m_max_out_request_queue = m_cfg->Read( _T( "/Session/max_out_request_queue" ), 200 );
	m_whole_pieces_threshold = m_cfg->Read( _T( "/Session/whole_pieces_threshold" ), 20 );
	m_peer_timeout = m_cfg->Read( _T( "/Session/peer_timeout" ), 120 );
	m_urlseed_timeout = m_cfg->Read( _T( "/Session/urlseed_timeout" ), 20 );
	m_urlseed_pipeline_size = m_cfg->Read( _T( "/Session/urlseed_pipeline_size" ), 5 );
	m_file_pool_size = m_cfg->Read( _T( "/Session/file_pool_size" ), 40 );
	m_cfg->Read( _T( "/Session/allow_multiple_connections_per_ip" ), &m_allow_multiple_connections_per_ip, false );
	m_max_failcount = m_cfg->Read( _T( "/Session/max_failcount" ), 3 ) ;
	m_min_reconnect_time = m_cfg->Read( _T( "/Session/min_reconnect_time" ), 60 );
	m_peer_connect_timeout = m_cfg->Read( _T( "/Session/peer_connect_timeout" ), 7 );
	m_cfg->Read( _T( "/Session/ignore_limits_on_local_network" ), &m_ignore_limits_on_local_network, true );
	m_connection_speed = m_cfg->Read( _T( "/Session/connection_speed" ), 20 );
	m_cfg->Read( _T( "/Session/send_redundant_have" ), &m_send_redundant_have, false );
	m_cfg->Read( _T( "/Session/lazy_bitfields" ), &m_lazy_bitfields, true );
	m_inactivity_timeout = m_cfg->Read( _T( "/Session/inactivity_timeout" ), 600 );
	m_unchoke_interval = m_cfg->Read( _T( "/Session/unchoke_interval" ), 15 );
	m_optimistic_unchoke_multiplier = m_cfg->Read( _T( "/Session/optimistic_unchoke_multiplier" ), 4 );
	m_num_want = m_cfg->Read( _T( "/Session/num_want" ), 200 );
	m_initial_picker_threshold = m_cfg->Read( _T( "/Session/initial_picker_threshold" ), 4 );
	m_allowed_fast_set_size = m_cfg->Read( _T( "/Session/allowed_fast_set_size" ), 10 );
	m_max_outstanding_disk_bytes_per_connection = m_cfg->Read( _T( "/Session/max_outstanding_disk_bytes_per_connection" ), 64 * 1024 );
	m_handshake_timeout = m_cfg->Read( _T( "/Session/handshake_timeout" ), 10 );
	m_cfg->Read( _T( "/Session/use_dht_as_fallback" ), &m_use_dht_as_fallback, true );
	m_cfg->Read( _T( "/Session/free_torrent_hashes" ), &m_free_torrent_hashes, true );
	m_cfg->Read( _T( "/Session/anonymous_mode" ), &m_anonymous_mode, false );
	m_cfg->Read( _T( "/Session/use_oscache" ), &m_useoscache, true );
	lt::storage_mode_t defstoragemode = lt::storage_mode_sparse;
	m_storagemode = ( lt::storage_mode_t ) m_cfg->Read( _T( "/Torrent/storagemode" ), ( long ) defstoragemode );
	m_global_upload_limit = m_cfg->Read( _T( "/Torrent/global_upload_limit" ), -1 );
	m_global_download_limit = m_cfg->Read( _T( "/Torrent/global_download_limit" ), -1 );
	m_global_max_connections = m_cfg->Read( _T( "/Torrent/global_max_connections" ), 250 );
	m_global_max_uploads = m_cfg->Read( _T( "/Torrent/global_max_uploads" ), 20 );
	if( m_global_max_uploads < 2 ) m_global_max_uploads = 2;
	m_cfg->Read( _T( "/Torrent/use_default" ), &m_usedefault, false );
	//dht
	//
	m_cfg->Read( _T( "/DHT/dht_enabled" ), &m_dht_enabled, true );
	m_dht_port = m_cfg->Read( _T( "/DHT/dht_port" ), 26881 );
	m_dht_max_peers = m_cfg->Read( _T( "/DHT/dht_max_peers" ), 50 );
	m_dht_search_branching = m_cfg->Read( _T( "/DHT/dht_search_branching" ), 5 );
	m_dht_max_fail = m_cfg->Read( _T( "/DHT/dht_max_fail" ), 20 );
	//enc
	//
	m_cfg->Read( _T( "/Encryption/enc_enabled" ), &m_enc_enabled,  true );
	m_enc_policy = m_cfg->Read( _T( "/Encryption/enc_policy" ), 1 );
	m_enc_level = m_cfg->Read( _T( "/Encryption/enc_level" ), 2 );
	m_cfg->Read( _T( "/Encryption/enc_enabled" ), &m_enc_prefer_rc4, true );
	//
	//per torrent
	//
	m_default_state = ( enum torrent_state ) m_cfg->Read( _T( "/Torrent/default_state" ), TORRENT_STATE_QUEUE );
	m_default_ratio = m_cfg->Read( _T( "/Torrent/default_ratio" ), 150 );
	m_default_upload_limit = m_cfg->Read( _T( "/Torrent/default_upload_limit" ), -1 );
	m_default_download_limit = m_cfg->Read( _T( "/Torrent/default_download_limit" ), -1 );
	m_default_max_connections = m_cfg->Read( _T( "/Torrent/default_max_connections" ), 50 );
	m_default_max_uploads = m_cfg->Read( _T( "/Torrent/default_max_uploads" ), 4 );
	if( m_default_max_uploads < 2 ) m_default_max_uploads = 2;
	//read save path
	ReadSavePath();
	m_cfg->Read( _T( "/Torrent/enable_video_preview" ), &m_enable_video_preview, true );
	m_cfg->Read( _T( "/Torrent/sequential_download" ), &m_sequential_download, false );
}

void Configuration::ReadSavePath()
{
	wxString historypathfile = wxGetApp().DataPath() + wxGetApp().PathSeparator() + _T( "save_history" );
	wxTextFile *history_f = new wxTextFile( historypathfile );
	if( history_f->Open() )
	{
		wxString path;
		for( path = history_f->GetFirstLine(); !history_f->Eof(); path = history_f->GetNextLine() )
		{
			m_savepathhistory.push_back( path );
			if( m_savepathhistory.size() >= BITSWASH_MAX_SAVE_PATH )
			{
				break;
			}
		}
	}
	else
	{
		wxLogError( _T( "Failed opening download path history file %s\n" ), historypathfile.c_str() );
	}
}

void Configuration::WriteSavePath()
{
	wxString historypathfile = wxGetApp().DataPath() + wxGetApp().PathSeparator() + _T( "save_history" );
	wxTextFile *history_f = new wxTextFile( historypathfile );
	if( history_f->Exists() )
	{
		if( !history_f->Open() )
		{
			wxLogError( _T( "Failed opening %n for writing\n" ), historypathfile.c_str() );
		}
	}
	else
	{
		if( !history_f->Create() )
		{
			wxLogError( _T( "Failed creating %n for writing\n" ), historypathfile.c_str() );
		}
	}
	if( history_f->IsOpened() )
	{
		history_f->Clear();
		std::vector<wxString>::const_iterator path;
		for( path = m_savepathhistory.begin(); path != m_savepathhistory.end(); ++path )
		{
			history_f->AddLine( *path );
		}
		if( !history_f->Write() )
		{
			wxLogError( _T( "Error writing into file %s\n" ), historypathfile.c_str() );
		}
	}
}

#ifdef __WXMSW__
bool Configuration::DetectType(const wxString& type)
{
	wxLogNull nolog; // disable error log
	wxString value;
	wxRegKey *pRegKey = new wxRegKey( m_bitswash_regkey_path + type );

	if( pRegKey->Exists() ) { pRegKey->QueryValue( wxEmptyString, value ); }

	delete pRegKey;

	if( !value.IsEmpty() )
	{
		pRegKey = new wxRegKey( m_bitswash_regkey_path
								+ value
								+ wxString( wxT( "\\shell\\open\\command" ) ) );
		value.Empty();

		if( pRegKey->Exists() ) { pRegKey->QueryValue( wxEmptyString, value ); }

		delete pRegKey;
		wxString exepath = GetExecutablePath();

		if( value.Upper().Find( exepath.Upper() ) >= 0 )
		{
			return true;
		}
	}

	return false;
}

void Configuration::AddType( const wxString& type )
{
	wxString value;
	wxString bitswash_type = wxString( APPNAME ) + type;
	wxRegKey *pRegKey = new wxRegKey( m_bitswash_regkey_path + type );

	if( !pRegKey->Exists() ) { pRegKey->Create(); }
	else { pRegKey->QueryValue( wxEmptyString, value ); }

	if( value != bitswash_type )
	{
		if( !value.IsEmpty() ) //save old default value
		{
			pRegKey->SetValue( wxT( "Old_Default" ), value );
			//if(type == wxT(".txt"))
			//{
			//    wxRegKey *pRegKey1 = new wxRegKey(wxString(wxT("HKEY_CLASSES_ROOT\\")) + type);
			//    pRegKey->QueryValue(wxEmptyString, txt_name);
			//    delete pRegKey1;
			//}
		}

		value = bitswash_type;
		pRegKey->SetValue( wxEmptyString, value );
	}

	delete pRegKey;
	wxString name( m_bitswash_regkey_path );
	name += value;
	name += wxT( "\\shell\\open\\command" );
	pRegKey = new wxRegKey( name );
	pRegKey->Create();
	wxString exepath = GetExecutablePath();
	pRegKey->SetValue( wxEmptyString, wxString( wxT( '"' ) ) + exepath + wxString( wxT( "\" \"%1\"" ) ) );
	delete pRegKey;
	name = m_bitswash_regkey_path;
	name += value;
	name += wxT( "\\DefaultIcon" );
	pRegKey = new wxRegKey( name );
	pRegKey->Create();
	pRegKey->SetValue( wxEmptyString, exepath + wxString( wxT( ",1" ) ) );
	delete pRegKey;
}

void Configuration::RemoveType( const wxString& type )
{
	if( type.IsEmpty() ) { return; }

	wxString value, old_default;
	wxString bitswash_type = wxString( APPNAME ) + type;
	wxRegKey *pRegKey = new wxRegKey( m_bitswash_regkey_path + type );

	if( pRegKey->Exists() )
	{
		pRegKey->QueryValue( wxT( "Old_Default" ), old_default );
		pRegKey->QueryValue( wxEmptyString, value );
	}

	if( !old_default.IsEmpty() )
	{
		pRegKey->DeleteValue( wxT( "Old_Default" ) );
		pRegKey->SetValue( wxEmptyString, old_default );
	}
	else
		if( !value.IsEmpty() )
		{
			if( value == bitswash_type )
			{
				pRegKey->DeleteSelf();
			}
		}

	delete pRegKey;

	if( value == bitswash_type )
	{
		pRegKey = new wxRegKey( m_bitswash_regkey_path + value );

		if( pRegKey->Exists() ) { pRegKey->DeleteSelf(); }

		delete pRegKey;
	}
}

void Configuration::AddMagnetLinkType( )
{
	wxString type(wxT("magnet"));
	wxString bitswash_type = wxString(APPNAME) + wxT(".") + type;
	wxRegKey *pRegKey = new wxRegKey( m_bitswash_regkey_path + type );

	wxString name(m_bitswash_regkey_path);
	name +=  wxT("\\magnet\\shell\\open\\command");
	pRegKey = new wxRegKey( name );
	pRegKey->Create();
	wxString exepath = GetExecutablePath();
	pRegKey->SetValue( wxEmptyString, wxString( wxT( '"' ) ) + exepath + wxString( wxT( "\" \"%1\"" ) ) );
	delete pRegKey;
	name = m_bitswash_regkey_path;
	name += wxT( "\\magnet\\DefaultIcon\\Default" );
	pRegKey = new wxRegKey( name );
	pRegKey->Create();
	pRegKey->SetValue( wxEmptyString, exepath + wxString( wxT( ",1" ) ) );
	delete pRegKey;
	name = m_bitswash_regkey_path;
	name += wxT( "\\magnet\\Default" );
	pRegKey = new wxRegKey( name );
	pRegKey->Create();
	pRegKey->SetValue( wxEmptyString, wxT("URL:Magnet link") );
	delete pRegKey;
	name = m_bitswash_regkey_path;
	name += wxT( "\\magnet\\Content Type" );
	pRegKey = new wxRegKey( name );
	pRegKey->Create();
	pRegKey->SetValue( wxEmptyString, wxT("application/x-magnet") );
	delete pRegKey;
	name = m_bitswash_regkey_path;
	name += wxT( "\\magnet\\URL Protocol" );
	pRegKey = new wxRegKey( name );
	pRegKey->Create();
	pRegKey->SetValue( wxEmptyString, wxT("") );
	delete pRegKey;
	name = m_bitswash_regkey_path;
	name += wxT( "\\magnet\\shell\\Default" );
	pRegKey = new wxRegKey( name );
	pRegKey->Create();
	pRegKey->SetValue( wxEmptyString, wxT("open") );
	delete pRegKey;
}

void Configuration::RemoveMagnetLinkType( )
{
	wxRegKey regKey(m_bitswash_regkey_path + wxT("magnet"));

	if (regKey.Exists()) { regKey.DeleteSelf(); }
}

#endif

