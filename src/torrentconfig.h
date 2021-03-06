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
#include <libtorrent/announce_entry.hpp>
#include <libtorrent/download_priority.hpp>
#include <libtorrent/disk_interface.hpp>
//#include "storage.hpp"

enum
{
	BITTORRENT_FILE_NONE     = 0,
	BITTORRENT_FILE_LOWEST   = 1,
	BITTORRENT_FILE_RESERVE2 = 2,
	BITTORRENT_FILE_RESERVE3 = 3,
	BITTORRENT_FILE_NORMAL   = 4,
	BITTORRENT_FILE_RESERVE5 = 5,
	BITTORRENT_FILE_RESERVE6 = 6,
	BITTORRENT_FILE_HIGHEST  = 7,
};

typedef struct
{
	double progress;
	wxULongLong_t total_download;
	wxULongLong_t total_upload;
} stats_t;

/*
 * STOP - torrent not added into libtorrent
 * START - torrent is downloading/seeding
 * QUEUE - torrent is in libtorrent, but paused, started if criteria met
 * PAUSE - torrent is forced into pause (added into libtorrent)
 * FORCE_START - torrent is forced into start mode
 */

enum torrent_state
{
	TORRENT_STATE_STOP = 0,
	TORRENT_STATE_START = 1,
	TORRENT_STATE_QUEUE = 2, // when torrent is in queue, it's in the libtorrent
	TORRENT_STATE_PAUSE = 3, // do not use this state
	TORRENT_STATE_FORCE_START = 4,
};

class TorrentConfig : public wxFileConfig
{
public:
	explicit TorrentConfig( const wxString& TorrentName );
	~TorrentConfig();

	void Save();
	void Load();

	wxFileConfig* GetConfig();

	int GetQIndex() { return m_qindex; }
	void SetQIndex( int qindex ) { m_qindex = qindex; }
	const wxString& GetDownloadPath() { return m_downloadpath; }
	void SetDownloadPath( wxString& path ) { m_downloadpath = path; }

	bool GetEnableVideoPreview() { return m_enable_video_preview; }
	void SetEnableVideoPreview( bool enable ) { m_enable_video_preview = enable; }

	std::uint8_t GetConnectBoostCounter() { return m_connect_boost; }
	void SetConnectBoostCounter(const std::uint8_t connectBoost) { m_connect_boost = connectBoost; } 

	libtorrent::storage_mode_t GetTorrentStorageMode() { return m_storagemode; }
	void SetTorrentStorageMode( libtorrent::storage_mode_t storagemode ) { m_storagemode = storagemode; }

	bool GetSequentialDownload() { return m_sequential_download; }
	void SetSequentialDownload( bool isSequential ) { m_sequential_download = isSequential;}

	int GetTorrentState() { return m_torrent_state; }
	void SetTorrentState( int state ) { m_torrent_state = state;}

	int GetTorrentRatio() { return m_torrent_ratio; }
	void SetTorrentRatio( int ratio ) { m_torrent_ratio = ratio;}

	int GetTorrentUploadLimit() { return m_torrent_upload_limit; }
	void SetTorrentUploadLimit( int upload_limit ) { m_torrent_upload_limit = upload_limit;}

	int GetTorrentDownloadLimit() { return m_torrent_download_limit; }
	void SetTorrentDownloadLimit( int download_limit ) { m_torrent_download_limit = download_limit;}

	int GetTorrentMaxUploads() { return m_torrent_max_uploads; }
	void SetTorrentMaxUploads( int uploads ) { m_torrent_max_uploads = uploads;}

	int GetTorrentMaxConnections() { return m_torrent_max_connections; }
	void SetTorrentMaxConnections( int connections ) { m_torrent_max_connections = connections;}

	stats_t& GetTorrentStats() { return m_torrent_stats; }
	void SetTorrentStats( stats_t& stats ) { m_torrent_stats = stats;}

	std::vector<lt::download_priority_t>& GetFilesPriorities() { return m_files_priority ;}
	void SetFilesPriority( std::vector<lt::download_priority_t>& files );
	std::vector<libtorrent::announce_entry>& GetTrackersURL() { return m_trackers_url;}
	void  SetTrackersURL( std::vector<libtorrent::announce_entry>& trackers ) ;
	void WriteFilesPriority();
	void ReadFilesPriority();
	void WriteTrackersUrl();
	void ReadTrackersUrl();
	void SetTotalSize(const wxULongLong_t& fsize) {m_total_file_size = fsize;}
	const wxULongLong_t& GetTotalSize() {return m_total_file_size;}
	void SetSelectedSize(const wxULongLong_t& fsize) {m_selected_file_size = fsize;}
	const wxULongLong_t& GetSelectedSize() {return m_selected_file_size;}

	const wxString & GetTorrentMagnetUri() { return m_magneturi; }
	void SetTorrentMagnetUri( const wxString& magneturi ) { m_magneturi = magneturi;}

protected:
public:
	static const lt::download_priority_t file_none;
	static const lt::download_priority_t file_lowest;
	static const lt::download_priority_t file_normal;
	static const lt::download_priority_t file_highest;
private:
	wxString m_torrentname;
	wxString m_configfile;
	wxFileConfig *m_cfg;
	Configuration *m_maincfg;

	int m_qindex;
	wxString m_downloadpath;
	wxString m_magneturi;

	std::uint8_t m_connect_boost;
	bool m_sequential_download;
	bool m_enable_video_preview;
	libtorrent::storage_mode_t m_storagemode;

	//per torrent settings
	int m_torrent_state;
	int m_torrent_ratio; //torrent 150/100 (float in libtorrent)
	int m_torrent_upload_limit;
	int m_torrent_download_limit;
	int m_torrent_max_uploads;
	int m_torrent_max_connections;
	wxULongLong_t m_selected_file_size;
	wxULongLong_t m_total_file_size;
	std::vector<lt::download_priority_t> m_files_priority;
	std::vector<libtorrent::announce_entry> m_trackers_url;
	stats_t m_torrent_stats;
};



#endif // _TORRENTCONFIG_H_
