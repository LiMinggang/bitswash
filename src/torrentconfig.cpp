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


TorrentConfig::TorrentConfig(const wxString& TorrentName)
: wxFileConfig (wxEmptyString, wxEmptyString, TorrentName, wxEmptyString, wxCONFIG_USE_LOCAL_FILE | wxCONFIG_USE_RELATIVE_PATH)
{
    	//ctor
	m_maincfg = wxGetApp().GetConfig();

	m_configfile = wxGetApp().SaveTorrentsPath() + wxFileName::GetPathSeparator(wxPATH_NATIVE) + TorrentName + _T(".conf");

	if( !wxFileName::FileExists( m_configfile )) {
		wxFile *f = new wxFile();
		bool b = f->Create( m_configfile.c_str());
		
		if(!b) wxLogError(_T("Failed to create file ") + m_configfile );
	}


	wxFileInputStream fis( m_configfile );

   	m_torrentname= TorrentName;
	wxLogDebug(_T("Init TorrentConfig %s"), m_configfile.c_str());

	m_cfg =  new wxFileConfig( (wxInputStream &)fis);
	//m_cfg = (wxConfig*)wxConfig::Get(m_appname);
	
	Load();

}

TorrentConfig::~TorrentConfig()
{
    //delete wxConfigBase::Set((wxConfigBase*)NULL);
    //dtor
}

void TorrentConfig::Save()
{

	//Config
    	m_cfg->Write(_T("/Torrent/qindex"), m_qindex);
    	m_cfg->Write(_T("/Torrent/downloadpath"), m_downloadpath);

	/* legacy */
	if (m_storagemode != libtorrent::storage_mode_compact)
		m_compactalloc = false;

	m_cfg->Write(_T("/Torrent/compact_alloc"), (bool)m_compactalloc);
	m_cfg->Write(_T("/Torrent/storagemode"), (long)m_storagemode);

	m_cfg->Write(_T("/Torrent/state"), m_torrent_state);
	m_cfg->Write(_T("/Torrent/ratio"), m_torrent_ratio);
	m_cfg->Write(_T("/Torrent/upload_limit"), m_torrent_upload_limit);
	m_cfg->Write(_T("/Torrent/download_limit"), m_torrent_download_limit);
	m_cfg->Write(_T("/Torrent/max_connections"), m_torrent_max_connections);
	
	if (m_torrent_max_uploads <2) m_torrent_max_uploads = 2;
	m_cfg->Write(_T("/Torrent/max_uploads"), m_torrent_max_uploads);

	wxString stats_string = wxString::Format(_T("%.02f,%ld,%ld"),
			m_torrent_stats.progress,
			m_torrent_stats.total_download,
			m_torrent_stats.total_upload
			);

	m_cfg->Write(_T("/Torrent/stats"), stats_string);

	WriteFilesPriority();
	WriteTrackersUrl();

	wxFileOutputStream fos( m_configfile );
	m_cfg->Save(fos);

}

void TorrentConfig::Load()
{
	wxString tmp_stats;

	m_qindex = m_cfg->Read(_T("/Torrent/qindex"), -1);
	m_downloadpath = m_cfg->Read(_T("/Torrent/downloadpath"), m_maincfg->GetDownloadPath());

	m_cfg->Read(_T("/Torrent/compact_alloc"), &m_compactalloc, m_maincfg->GetCompactAlloc());

	m_storagemode =(libtorrent::storage_mode_t) m_cfg->Read(_T("/Torrent/storagemode"),(long) m_maincfg->GetDefaultStorageMode());

	m_torrent_state = m_cfg->Read(_T("/Torrent/state"), m_maincfg->GetDefaultState());
	m_torrent_ratio = m_cfg->Read(_T("/Torrent/ratio"), m_maincfg->GetDefaultRatio());
	m_torrent_upload_limit = m_cfg->Read(_T("/Torrent/upload_limit"), m_maincfg->GetDefaultUploadLimit());
	m_torrent_download_limit = m_cfg->Read(_T("/Torrent/download_limit"), m_maincfg->GetDefaultDownloadLimit());
	m_torrent_max_connections = m_cfg->Read(_T("/Torrent/max_connections"), m_maincfg->GetMaxConnections());
	m_torrent_max_uploads = m_cfg->Read(_T("/Torrent/max_uploads"), m_maincfg->GetMaxUploads());
	if (m_torrent_max_uploads <2) m_torrent_max_uploads = 2;

	tmp_stats = m_cfg->Read(_T("/Torrent/stats"), _T("0,0,0"));

	wxString tmpstr;
	wxString tmpstr2;
	wxChar token = ',';
	tmpstr2 =tmp_stats;

	tmpstr = tmpstr2.BeforeFirst(token);
	tmpstr2 = tmpstr2.AfterFirst(token);
	
	tmpstr.ToDouble(&m_torrent_stats.progress);

	tmpstr = tmpstr2.BeforeFirst(token);
	tmpstr2 = tmpstr2.AfterFirst(token);

	tmpstr.ToLong(&m_torrent_stats.total_download);

	tmpstr = tmpstr2.BeforeFirst(token);
	tmpstr2 = tmpstr2.AfterFirst(token);
	
	tmpstr.ToLong(&m_torrent_stats.total_upload);

	ReadFilesPriority();
	ReadTrackersUrl();

}

void TorrentConfig::WriteFilesPriority()
{
	wxString filespriority_data = wxEmptyString;

	std::vector<int>::const_iterator i = m_files_priority.begin();

	for (i=m_files_priority.begin(); i != m_files_priority.end() ; i++)
	{
		filespriority_data << *i << _T(",");
	}


	filespriority_data = filespriority_data.BeforeLast(',');
	
	m_cfg->Write(_T("/Torrent/files_priority"), filespriority_data);
}

void TorrentConfig::WriteTrackersUrl()
{
	wxString trackers = wxEmptyString;

	std::vector<libtorrent::announce_entry>::const_iterator t = m_trackers_url.begin();

	if (m_trackers_url.size() == 0)
		return ;

	
	std::string t_url;
	int t_tier;

	for (t=m_trackers_url.begin(); t != m_trackers_url.end() ; t++)
	{
		t_url = t->url;
		t_tier = t->tier;
		trackers += wxString::FromAscii(t_url.c_str()) + _T("|") + wxString::Format(_T("%d"), t_tier) + _T(";");
	}

	wxLogDebug(_T("Writing trackers %s\n"), trackers.c_str());

	trackers = trackers.BeforeLast(';');
	
	m_cfg->Write(_T("/Torrent/trackers"), trackers);
}

void TorrentConfig::ReadFilesPriority()
{

	wxString files_priority = wxEmptyString;
	m_cfg->Read(_T("/Torrent/files_priority"), &files_priority);

	m_files_priority.clear();

	wxStringTokenizer tokens(files_priority, _T(","));

	while(tokens.HasMoreTokens())
	{
			wxString token = tokens.GetNextToken();
			m_files_priority.push_back(wxAtoi(token));
	}

}

void TorrentConfig::ReadTrackersUrl()
{

	wxString trackers = wxEmptyString;
	m_cfg->Read(_T("/Torrent/trackers"), &trackers);

	m_trackers_url.clear();

	if (trackers.Len() <= 0)
		return;
	wxStringTokenizer tokens(trackers, _T(";"));

	while(tokens.HasMoreTokens())
	{
		wxString token = tokens.GetNextToken();


		std::string t_str(token.BeforeFirst('|').mb_str(wxConvUTF8));

		int t_tier = wxAtoi(token.AfterFirst('|'));

		libtorrent::announce_entry e(t_str);
		e.tier = t_tier;
		m_trackers_url.push_back(e);
	}

}
void TorrentConfig::SetFilesPriority(std::vector<int>& files)
{
	  m_files_priority.clear(); 
	  m_files_priority.swap(files) ;

}

void TorrentConfig::SetTrackersURL(std::vector<libtorrent::announce_entry>& trackers)
{
	  m_trackers_url.clear(); 
	  m_trackers_url.swap(trackers) ;
	  

}
