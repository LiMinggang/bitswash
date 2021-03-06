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
// Class: BitTorrentSession
//
// Created by: lim k. c. <admin@bitswash.org>
// Created on: Sun Aug 12 13:49:33 MYT 2007
//

#ifndef _BITTORRENTSESSION_H_
#define _BITTORRENTSESSION_H_

#include <vector>

#include <memory>
#include <boost/cstdint.hpp>

#include <wx/app.h>
#include <wx/thread.h>
#include <wx/msgqueue.h>
#include <wx/hashset.h>

#include <libtorrent/session.hpp>
#include <libtorrent/torrent_info.hpp>
#include <libtorrent/kademlia/traversal_algorithm.hpp>
#include <libtorrent/session_stats.hpp>

#include "infohash.h"

#include "configuration.h"
#include "torrentconfig.h"

class MagnetUri;

enum BTS_EVENT_TYPE
	{
	BTS_EVENT_INVALID = 0,
	BTS_EVENT_TIMER,
	BTS_EVENT_ALERT,
	BTS_EVENT_QUEUEUPDATE,
	};

typedef wxUint32 bts_event;

typedef struct torrent_handle_t {

	torrent_handle_t() :
		isvalid(false)/*, config(0)*/
	{}

	wxString name;
	InfoHash hash;
	lt::torrent_handle handle;
	std::shared_ptr<lt::torrent_info> info;
	std::shared_ptr<TorrentConfig> config;
	std::vector<long> fileindex;
	bool isvalid;
} torrent_t ;

/* watch out the s after torrent */
typedef std::vector<std::shared_ptr<torrent_t> > torrents_t;
typedef std::map<wxString, size_t> torrents_map;
typedef std::set<wxString> torrents_set;
typedef std::vector<wxString> metadata_t;
WX_DECLARE_HASH_SET( wxString, wxStringHash, wxStringEqual, BitwashStrSet );

class BitTorrentSession : public wxThread
{
public:
	enum TORRENT_STATUS
	{
		TORRENT_STATUS_QUEUED,				  //_( "Queued" )
		TORRENT_STATUS_CHECKING,			  //_( "Checking" )
		TORRENT_STATUS_DOWNLOADING_META_DATA, //_( "Downloading meta data" )
		TORRENT_STATUS_DOWNLOADING, 		  //_( "Downloading" )
		TORRENT_STATUS_FINISHED,			  //_( "Finished" )
		TORRENT_STATUS_SEEDING, 			  //_( "Seeding" )
		TORRENT_STATUS_ALLOCATING,			  //_( "Allocating" )
		TORRENT_STATUS_CHECKING_RESUME_DATA,  //_( "Checking resume data" )
	};
	BitTorrentSession(wxApp* pParent, Configuration* config, wxMutex *mutex = 0, wxCondition *condition = 0);
	~BitTorrentSession();
	BitTorrentSession(BitTorrentSession &) = delete;
	BitTorrentSession& operator=(BitTorrentSession&) = delete;

	virtual void *Entry();
	virtual void OnExit();

	void Configure(lt::settings_pack &settingsPack);
	void ConfigureSession();
	void configurePeerClasses();
	void SetConnection();
	void SetEncryption();
	void StartExtensions();
	void StartUpnp();
	void StartNatpmp();
	
	void AddTorrentToSession(std::shared_ptr<torrent_t>& torrent);
	void AddMagnetUriToSession(std::shared_ptr<torrent_t>& torrent);
	bool AddTorrent(std::shared_ptr<torrent_t>& torrent);
	void RemoveTorrent(std::shared_ptr<torrent_t>& torrent, bool deletedata);
	int FindTorrent(const wxString &hash);
	std::shared_ptr<torrent_t> GetTorrent(const wxString &hash);
	std::shared_ptr<torrent_t> GetTorrent(int idx);
	void MergeTorrent(std::shared_ptr<torrent_t>& dst_torrent, std::shared_ptr<torrent_t>& src_torrent);
	void MergeTorrent(std::shared_ptr<torrent_t>& dst_torrent, MagnetUri& src_magneturi);

	std::shared_ptr<torrent_t> ParseTorrent(const wxString& filename);
	std::shared_ptr<torrent_t> LoadMagnetUri( MagnetUri& magneturi );
	void UpdateTorrentFileSize(std::shared_ptr<torrent_t>& torrent);
	void LoadMagnetUri( const wxString& magneturi );
	static bool SaveTorrent(std::shared_ptr<torrent_t>& torrent, const wxString& filename);
	
	size_t GetTorrentQueueSize();

	void StartTorrent(std::shared_ptr<torrent_t>& torrent, bool force);
	void StopTorrent(std::shared_ptr<torrent_t>& torrent);
	void QueueTorrent(std::shared_ptr<torrent_t>& torrent);
	void QueueTorrent(std::shared_ptr<torrent_t>& torrent, int state);
	void PauseTorrent(std::shared_ptr<torrent_t>& torrent);
	void MoveTorrentUp(std::shared_ptr<torrent_t>& torrent);
	void MoveTorrentDown(std::shared_ptr<torrent_t>& torrent);
	void MoveTorrentTop(std::shared_ptr<torrent_t>& torrent);
	void MoveTorrentBottom(std::shared_ptr<torrent_t>& torrent);
	void ReannounceTorrent(std::shared_ptr<torrent_t>& torrent);
	void RecheckTorrent(std::shared_ptr<torrent_t>& torrent);
	void ConfigureTorrent(std::shared_ptr<torrent_t>& torrent);
	void ConfigureTorrentFilesPriority(std::shared_ptr<torrent_t>& torrent);
	void ConfigureTorrentTrackers(std::shared_ptr<torrent_t>& torrent);

	void RemoveTorrent( int idx, bool deletedata );
	void StartTorrent( int idx, bool force );
	void StopTorrent( int idx );
	void QueueTorrent( int idx );
	void PauseTorrent( int idx );
	void MoveTorrentUp( int idx );
	void MoveTorrentDown( int idx );
	void MoveTorrentTop( int idx );
	void MoveTorrentBottom( int idx );
	void ReannounceTorrent( int idx );
	void RecheckTorrent(int idx);
	void ConfigureTorrentFilesPriority( int idx );
	void ConfigureTorrentTrackers( int idx );
	void ConfigureTorrent( int idx );
	bool GetTorrentMagnetUri( int idx, wxString& magneturi );
	void GetPendingMetadata(metadata_t & mdq);
	//lt::session* GetLibTorrent() { return m_libbtsession;}

	void HandleTorrentAlerts();
	bool HandleTorrentAddAlert(lt::add_torrent_alert *p);
	bool HandleTorrentMetaDataAlert(lt::metadata_received_alert *p);
	void PostEvent(bts_event & evt) { m_evt_queue.Post(evt);}
	void PostLibTorrentAlertEvent()
		{
			static bts_event lbtAlert(BTS_EVENT_ALERT);
			PostEvent(lbtAlert);
		}
	void PostQueueUpdateEvent()
		{
			static bts_event queueChange(BTS_EVENT_QUEUEUPDATE);
			PostEvent(queueChange);
		}
	
	wxDouble GetDownloadRate() { return m_download_rate; }
	wxDouble GetUploadRate() { return m_upload_rate; }
	boost::uint64_t GetNumPeers() { return m_cnt[0][m_num_peers_idx]; }
	boost::uint64_t GetDhtNodes() { return m_cnt[0][m_dht_nodes_idx]; }
	bool HasInComingConns() { return (m_cnt[0][m_has_incoming_connections_idx] > 0 ? true : false); }
	void PostStatusUpdate();
	void UpdateCounters(lt::span<std::int64_t const>& stats_counters, boost::uint64_t t);
	bool DHTStatusToString(wxString & status);

private:
	void ScanTorrentsDirectory(const wxString& dirname);
	int find_torrent_by_hash(const wxString& hash);

	void SaveTorrentResumeData( lt::save_resume_data_alert * p );
	void SaveAllTorrent();

	void DumpTorrents();
	void CheckQueueItem();

	bool m_upnp_started;
	bool m_natpmp_started;
	bool m_lsd_started;

	wxMutex m_torrent_queue_lock;
	torrents_t m_torrent_queue;
	torrents_map m_running_torrent_map;
	torrents_set m_queue_torrent_set;
	wxMutex m_metadata_queue_lock;
	metadata_t m_metadata_queue;

	wxMutex m_dht_status_lock;
	bool m_dht_changed;
	std::vector<lt::dht_lookup> m_dht_active_requests;
	std::vector<lt::dht_routing_bucket> m_dht_routing_table;

	wxApp* m_pParent;
	Configuration* m_config;
	lt::session* m_libbtsession;
    wxCondition *m_condition;
    wxMutex *m_mutex;
	wxMessageQueue< bts_event > m_evt_queue;
	
	// there are two sets of counters. the current one and the last one. This
	// is used to calculate rates
	std::vector<std::int64_t> m_cnt[2];

	// the timestamps of the counters in m_cnt[0] and m_cnt[1]
	// respectively. The timestamps are microseconds since session start
	std::uint64_t m_timestamp[2];

	int m_queued_bytes_idx;
	int m_wasted_bytes_idx;
	int m_failed_bytes_idx;
	int m_has_incoming_connections_idx;
	int m_num_peers_idx;
	int m_recv_payload_idx;
	int m_sent_payload_idx;
	int m_unchoked_idx;
	int m_unchoke_slots_idx;
	int m_limiter_up_queue_idx;
	int m_limiter_down_queue_idx;
	int m_queued_writes_idx;
	int m_queued_reads_idx;
	int m_writes_cache_idx;
	int m_reads_cache_idx;
	int m_pinned_idx;
	int m_num_blocks_read_idx;
	int m_cache_hit_idx;
	int m_blocks_in_use_idx;
	int m_blocks_written_idx;
	int m_write_ops_idx;
	int m_dht_nodes_idx;

	int m_mfu_size_idx;
	int m_mfu_ghost_idx;
	int m_mru_size_idx;
	int m_mru_ghost_idx;

	int m_utp_idle;
	int m_utp_syn_sent;
	int m_utp_connected;
	int m_utp_fin_sent;
	int m_utp_close_wait;

	wxDouble m_download_rate;
	wxDouble m_upload_rate;

	static BitwashStrSet m_preview_ext_set;
};

#endif // _BITTORRENTSESSION_H_
