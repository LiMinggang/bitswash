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
// Class: TorrentConfig
//
// Created by: lim k. c. <admin@bitswash.org>
// Created on: Mon May  7 01:10:30 MYT 2007
//
#include <wx/stdpaths.h>
#include <wx/settings.h>
#include <wx/filename.h>
#include <wx/log.h>
#include <wx/window.h>
#include <wx/wfstream.h>
#include <wx/tokenzr.h>

#include "bitswash.h"
#include "torrentconfig.h"
#include "configuration.h"

//namespace lt = libtorrent;
const lt::download_priority_t TorrentConfig::file_none(BITTORRENT_FILE_NONE);
const lt::download_priority_t TorrentConfig::file_lowest(BITTORRENT_FILE_LOWEST);
const lt::download_priority_t TorrentConfig::file_normal(BITTORRENT_FILE_NORMAL);
const lt::download_priority_t TorrentConfig::file_highest(BITTORRENT_FILE_HIGHEST);

TorrentConfig::TorrentConfig( const wxString& torrentName )
	: wxFileConfig( wxEmptyString, wxEmptyString, torrentName, wxEmptyString, wxCONFIG_USE_LOCAL_FILE | wxCONFIG_USE_RELATIVE_PATH ), m_selected_file_size(0), m_total_file_size(0) 
{
	//ctor
	m_maincfg = wxGetApp().GetConfig();
	m_configfile = wxGetApp().SaveTorrentsPath() + torrentName + _T( ".conf" );

	if( !wxFileName::FileExists( m_configfile ) )
	{
		wxFile *f = new wxFile();
		bool b = f->Create( m_configfile.c_str() );

		if( !b ) { wxLogError( _T( "Failed to create file " ) + m_configfile +_T("\n")); wxASSERT(0);}
	}

	wxFileInputStream fis( m_configfile );
	m_torrentname = torrentName;
	//wxLogDebug( _T( "Init TorrentConfig %s\n" ), m_configfile.c_str() );
	m_cfg =  new wxFileConfig( ( wxInputStream & )fis );
	//m_cfg = (wxConfig*)wxConfig::Get(m_appname);
	Load();
}

TorrentConfig::~TorrentConfig()
{
	//delete wxConfigBase::Set((wxConfigBase*)nullptr);
	//dtor
}

void TorrentConfig::Save()
{
	//Config
	m_cfg->Write( _T( "/Torrent/qindex" ), m_qindex );
	m_cfg->Write( _T( "/Torrent/downloadpath" ), m_downloadpath );

	m_cfg->Write( _T( "/Torrent/storagemode" ), ( long )m_storagemode );
	m_cfg->Write( _T( "/Torrent/state" ), m_torrent_state );
	m_cfg->Write( _T( "/Torrent/ratio" ), m_torrent_ratio );
	m_cfg->Write( _T( "/Torrent/upload_limit" ), m_torrent_upload_limit );
	m_cfg->Write( _T( "/Torrent/download_limit" ), m_torrent_download_limit );
	m_cfg->Write( _T( "/Torrent/max_connections" ), m_torrent_max_connections );
	m_cfg->Write( _T( "/Torrent/enable_video_preview" ), ( bool )m_enable_video_preview );

	wxLongLong tsize(m_total_file_size);
	m_cfg->Write( _T( "/Torrent/total_size_h" ), tsize.GetHi() );
	m_cfg->Write( _T( "/Torrent/total_size_l" ), tsize.GetLo() );

	wxLongLong fsize(m_selected_file_size);
	m_cfg->Write( _T( "/Torrent/selected_size_h" ), fsize.GetHi() );
	m_cfg->Write( _T( "/Torrent/selected_size_l" ), fsize.GetLo() );

	if( m_torrent_max_uploads < 2 ) { m_torrent_max_uploads = 2; }

	m_cfg->Write( _T( "/Torrent/max_uploads" ), m_torrent_max_uploads );
	wxString stats_string = wxString::Format( _T( "%.02f,%s,%s" ),
							m_torrent_stats.progress,
							wxULongLong(m_torrent_stats.total_download).ToString(),
							wxULongLong(m_torrent_stats.total_upload).ToString()
											);
	m_cfg->Write( _T( "/Torrent/stats" ), stats_string );
	WriteFilesPriority();
	WriteTrackersUrl();
	wxFileOutputStream fos( m_configfile );
	m_cfg->Save( fos );
}

void TorrentConfig::Load()
{
	wxString tmp_stats;
	m_qindex = m_cfg->Read( _T( "/Torrent/qindex" ), -1 );
	wxFileName fn;
	fn.AssignDir(m_cfg->Read( _T( "/Torrent/downloadpath" ), m_maincfg->GetDownloadPath() ));
	m_downloadpath = fn.GetPathWithSep();
	m_storagemode = ( lt::storage_mode_t ) m_cfg->Read( _T( "/Torrent/storagemode" ), ( long ) m_maincfg->GetDefaultStorageMode() );
	m_torrent_state = m_cfg->Read( _T( "/Torrent/state" ), m_maincfg->GetDefaultState() );
	m_torrent_ratio = m_cfg->Read( _T( "/Torrent/ratio" ), m_maincfg->GetDefaultRatio() );
	m_torrent_upload_limit = m_cfg->Read( _T( "/Torrent/upload_limit" ), m_maincfg->GetDefaultUploadLimit() );
	m_torrent_download_limit = m_cfg->Read( _T( "/Torrent/download_limit" ), m_maincfg->GetDefaultDownloadLimit() );
	m_torrent_max_connections = m_cfg->Read( _T( "/Torrent/max_connections" ), m_maincfg->GetMaxConnections() );
	m_cfg->Read( _T( "/Torrent/enable_video_preview" ), &m_enable_video_preview, true );

	long s_hi = m_cfg->Read( _T( "/Torrent/total_size_h" ), 0L );
	unsigned long s_lo = m_cfg->Read( _T( "/Torrent/total_size_l" ), 0L );
	if(s_hi || s_lo) m_total_file_size = wxLongLong(s_hi, s_lo).GetValue();
	s_hi = m_cfg->Read( _T( "/Torrent/selected_size_h" ), 0L );
	s_lo = m_cfg->Read( _T( "/Torrent/selected_size_l" ), 0L );
	if(s_hi || s_lo) m_selected_file_size = wxLongLong(s_hi, s_lo).GetValue();

	m_torrent_max_uploads = m_cfg->Read( _T( "/Torrent/max_uploads" ), m_maincfg->GetMaxUploads() );

	if( m_torrent_max_uploads < 2 ) { m_torrent_max_uploads = 2; }

	tmp_stats = m_cfg->Read( _T( "/Torrent/stats" ), _T( "0,0,0" ) );
	wxString tmpstr;
	wxString tmpstr2;
	wxChar token = ',';
	tmpstr2 = tmp_stats;
	tmpstr = tmpstr2.BeforeFirst( token );
	tmpstr2 = tmpstr2.AfterFirst( token );
	tmpstr.ToDouble( &m_torrent_stats.progress );
	tmpstr = tmpstr2.BeforeFirst( token );
	tmpstr2 = tmpstr2.AfterFirst( token );
	tmpstr.ToULongLong( &m_torrent_stats.total_download );
	tmpstr = tmpstr2.BeforeFirst( token );
	tmpstr2 = tmpstr2.AfterFirst( token );
	tmpstr.ToULongLong( &m_torrent_stats.total_upload );
	ReadFilesPriority();
	ReadTrackersUrl();
}

void TorrentConfig::WriteFilesPriority()
{
	wxString filespriority_data = wxEmptyString;
	std::vector<lt::download_priority_t>::const_iterator i = m_files_priority.begin();

	for( i = m_files_priority.begin(); i != m_files_priority.end() ; ++i )
	{
		filespriority_data << int(std::uint8_t(*i)) << _T( "," );
	}

	filespriority_data = filespriority_data.BeforeLast( ',' );
	m_cfg->Write( _T( "/Torrent/files_priority" ), filespriority_data );
}

void TorrentConfig::WriteTrackersUrl()
{
	wxString trackers = wxEmptyString;
	std::vector<lt::announce_entry>::const_iterator t = m_trackers_url.begin();

	if( m_trackers_url.size() == 0 )
	{ return ; }

	std::string t_url;

	for( t = m_trackers_url.begin(); t != m_trackers_url.end() ; ++t )
	{
		t_url = t->url;
		int t_tier = t->tier;
		trackers += wxString::FromUTF8( t_url.c_str() ) + _T( "|" ) + wxString::Format( _T( "%d" ), t_tier ) + _T( ";" );
	}

	//wxLogDebug( _T( "Writing trackers %s\n" ), trackers.c_str() );
	trackers = trackers.BeforeLast( ';' );
	m_cfg->Write( _T( "/Torrent/trackers" ), trackers );
}

void TorrentConfig::ReadFilesPriority()
{
	wxString files_priority = wxEmptyString;
	m_cfg->Read( _T( "/Torrent/files_priority" ), &files_priority );
	m_files_priority.clear();
	wxStringTokenizer tokens( files_priority, _T( "," ) );

	while( tokens.HasMoreTokens() )
	{
		wxString token = tokens.GetNextToken();
		std::uint8_t pr = wxAtoi(token);
		if (pr > 7) pr = 4;
		m_files_priority.push_back(lt::download_priority_t(pr) );
	}
}

void TorrentConfig::ReadTrackersUrl()
{
	wxString trackers = wxEmptyString;
	m_cfg->Read( _T( "/Torrent/trackers" ), &trackers );
	m_trackers_url.clear();

	if( trackers.Len() <= 0 )
	{ return; }

	wxStringTokenizer tokens( trackers, _T( ";" ) );

	while( tokens.HasMoreTokens() )
	{
		wxString token = tokens.GetNextToken();
		std::string t_str( token.BeforeFirst( '|' ).mb_str( wxConvUTF8 ) );
		int t_tier = wxAtoi( token.AfterFirst( '|' ) );
		lt::announce_entry e( t_str );
		e.tier = t_tier;
		m_trackers_url.push_back( e );
	}
}
void TorrentConfig::SetFilesPriority( std::vector<lt::download_priority_t>& files )
{
	m_files_priority.clear();
	m_files_priority.swap( files ) ;
}

void TorrentConfig::SetTrackersURL( std::vector<lt::announce_entry>& trackers )
{
	m_trackers_url.clear();
	m_trackers_url.swap( trackers ) ;
}
