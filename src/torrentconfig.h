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


#ifndef _TORRENTCONFIG_H_
#define _TORRENTCONFIG_H_

#include <wx/config.h>
#include <wx/string.h>
#include <wx/arrstr.h>
#include <map>
#include <vector>

#include "configuration.h"

typedef struct {
	double progress;
	long total_download;
	long total_upload;
	
} stats_t;

/* 
 * STOP - torrent not added into libtorrent
 * START - torrent is downloading/seeding
 * QUEUE - torrent is in libtorrent, but paused, started if criteria met
 * PAUSE - torrent is forced into pause (added into libtorrent)
 * FORCE_START - torrent is forced into start mode
 */

enum torrent_state {
	TORRENT_STATE_STOP = 0,
	TORRENT_STATE_START = 1,
	TORRENT_STATE_QUEUE = 2, // when torrent is in queue, it's in the libtorrent
	TORRENT_STATE_PAUSE = 3, // do not use this state
	TORRENT_STATE_FORCE_START = 4,
};

class TorrentConfig : public wxFileConfig
{
    public:
        TorrentConfig(const wxString& TorrentName);
        ~TorrentConfig();

	void Save();
	void Load();

	wxFileConfig* GetConfig();

	int GetQIndex() { return m_qindex; }
	void SetQIndex( int qindex ) { m_qindex = qindex; }
	const wxString GetDownloadPath() { return m_downloadpath; }
	void SetDownloadPath(wxString path) { m_downloadpath = path; }

	bool GetTorrentCompactAlloc() { return m_compactalloc; }
	void SetTorrentCompactAlloc(bool compactalloc) { m_compactalloc = compactalloc; } 
	
	libtorrent::storage_mode_t GetTorrentStorageMode() { return m_storagemode; }
	void SetTorrentStorageMode(libtorrent::storage_mode_t storagemode) { m_storagemode = storagemode; }

	int GetTorrentState() { return m_torrent_state; }
	void SetTorrentState(int state) { m_torrent_state = state;}

	int GetTorrentRatio() { return m_torrent_ratio; }
	void SetTorrentRatio(int ratio) { m_torrent_ratio = ratio;}

	int GetTorrentUploadLimit() { return m_torrent_upload_limit; }
	void SetTorrentUploadLimit(int upload_limit) { m_torrent_upload_limit = upload_limit;}
		
	int GetTorrentDownloadLimit() { return m_torrent_download_limit; }
	void SetTorrentDownloadLimit(int download_limit) { m_torrent_download_limit = download_limit;}

	int GetTorrentMaxUploads() { return m_torrent_max_uploads; }
	void SetTorrentMaxUploads(int uploads) { m_torrent_max_uploads = uploads;}

	int GetTorrentMaxConnections() { return m_torrent_max_connections; }
	void SetTorrentMaxConnections(int connections) { m_torrent_max_connections = connections;}

	stats_t& GetTorrentStats() { return m_torrent_stats; }
	void SetTorrentStats(stats_t& stats) { m_torrent_stats = stats;}

	std::vector<int>& GetFilesPriorities() { return m_files_priority ;}
	void SetFilesPriority(std::vector<int>& files);
	std::vector<libtorrent::announce_entry>& GetTrackersURL() { return m_trackers_url;}
	void  SetTrackersURL(std::vector<libtorrent::announce_entry>& trackers) ;
	void WriteFilesPriority();
	void ReadFilesPriority();
	void WriteTrackersUrl();
	void ReadTrackersUrl();

protected:
private:
	wxString m_torrentname;
	wxString m_configfile;
	wxFileConfig *m_cfg;
	Configuration *m_maincfg;

	int m_qindex;
        wxString m_downloadpath;

	//legacy
        bool 	m_compactalloc;
        libtorrent::storage_mode_t m_storagemode;

	//per torrent settings
	int m_torrent_state;
	int m_torrent_ratio; //torrent 150/100 (float in libtorrent)
	int m_torrent_upload_limit; 
	int m_torrent_download_limit;
	int m_torrent_max_uploads;
	int m_torrent_max_connections;
	std::vector<int> m_files_priority;
	std::vector<libtorrent::announce_entry> m_trackers_url;
	stats_t m_torrent_stats;
};



#endif // _TORRENTCONFIG_H_
