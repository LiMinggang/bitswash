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


#ifndef _CONFIGURATION_H_
#define _CONFIGURATION_H_

#include <wx/config.h>
#include <wx/fileconf.h>
#include <wx/string.h>
#include <wx/arrstr.h>
#include <wx/url.h>

#include <map>
#include <vector>

class Configuration ;

#include "torrentconfig.h"

struct gui_metric {
	int x;
	int y;
	int h;
	int w;
};


class Configuration : public wxFileConfig
{
public:
	enum BT_PROTOCOLS
	{
		PROTO_BOTH = 0,
		PROTO_TCP  = 1,
		PROTO_UTP  = 2,
	};

	enum choking_algorithm
	{
		FIXED_SLOTS = 0,
		RATE_BASED  = 1,
		BITTYRANT   = 2,
	};

	enum seed_choking_algorithm
	{
		ROUND_ROBIN = 0,
		FASTEST_UPLOAD,
		ANTI_LEECH,
	};
    explicit Configuration(const wxString& AppName);
    ~Configuration();

	wxIPV4address m_local_ip;

	void Save();
	void Load();

	wxConfig* GetConfig();
	unsigned int GetPortMin() { return m_portmin; }
	void SetPortMin(unsigned int portmin) { m_portmin = portmin; }

	unsigned int GetPortMax() { return m_portmax; }
	void SetPortMax(unsigned int portmax) { m_portmax = portmax; }

	const wxString& GetAppName() { return m_appname; }

	//GUI
	struct gui_metric* GetGui() { return &m_gui; }
	void SetGui(int x, int y, int w, int h)
	{
	    m_gui.x = x;
	    m_gui.y = y;
	    m_gui.w = w ;
	    m_gui.h = h;
    	}

	unsigned int GetGuiVersion() { return m_guiversion; }

	const wxString& GetAuiPerspective() { return m_auiperspective; }
	void SetAuiPerspective(const wxString& auiperspective) { m_auiperspective = auiperspective; }

	const wxString& GetTorrentListCtrlSetting() { return m_torrentlistctrlsetting; }
	void SetTorrentListCtrlSetting(const wxString& setting) { m_torrentlistctrlsetting = setting; }
	const wxString& GetPeerListCtrlSetting() { return m_peerlistctrlsetting; }
	void SetPeerListCtrlSetting(const wxString& setting) { m_peerlistctrlsetting = setting; }
	const wxString& GetFileListCtrlSetting() { return m_filelistctrlsetting; }
	void SetFileListCtrlSetting(const wxString& setting) { m_filelistctrlsetting = setting; }
	
	const wxString& GetTrackerListCtrlSetting() { return m_trackerlistctrlsetting; }
	void SetTrackerListCtrlSetting(const wxString& setting) { m_trackerlistctrlsetting = setting; }

	bool GetUseSystray() { return m_use_systray; }
	void SetUseSystray(bool use_systray ) { m_use_systray = use_systray; }

	// Config
	const wxString GetDownloadPath() { return m_downloadpath; }
	void SetDownloadPath(const wxString& path) { m_downloadpath = path; AddSavePath(path); }
	const wxString GetOpenTorrentPath() { return m_opentorrentpath; }
	void SetOpenTorrentPath(const wxString& path) { m_opentorrentpath = path; AddOpenPath(path); }

	unsigned int GetRefreshTime() { return m_refreshtime; }
	void SetRefreshTime(unsigned int refreshtime) { m_refreshtime = refreshtime; }

	unsigned int GetFastResumeSaveTime() { return m_fastresume_save_time; }
	void SetFastResumeSaveTime(unsigned int fastresume_save_time) { m_fastresume_save_time = fastresume_save_time; }

	bool GetHideTaskbar() { return m_hidetaskbar; }
	void SetHideTaskbar( bool hidetaskbar ) { m_hidetaskbar = hidetaskbar ; }

	unsigned int GetLogSeverity() { return m_log_severity; }
	void SetLogSeverity(unsigned int log_severity) { m_log_severity = log_severity; }

	unsigned int GetLogLineCount() { return m_log_linecount; }
	void SetLogLineCount(unsigned int log_linecount) { m_log_linecount = log_linecount; }

	bool GetLogFile() { return m_log_file; }
	void SetLogFile( bool log_file ) { m_log_file = log_file ; }

	unsigned int GetLogMaxLogFiles() { return m_max_log_files; }
	void SetLogMaxLogFiles(unsigned int max_file_num) { m_max_log_files = max_file_num; }

	const wxString& GetDhtBootstrapNodes() { return m_dhtbootstrapnodes; }
	void SetDhtBootstrapNodes(const wxString& dhtnodes) { m_dhtbootstrapnodes = dhtnodes; }

	unsigned int GetMaxStart() { return m_max_start; }
	void SetMaxStart(unsigned int max_start) { m_max_start = max_start; }
	
	unsigned int GetExitWaitingTime() { return m_exit_waiting_time; }
	void SetExitWaitingTime(unsigned int waiting_time) { m_exit_waiting_time = waiting_time; }

	bool GetEnableUpnp() { return m_enable_upnp; }
	void SetEnableUpnp( bool enable_upnp ) { m_enable_upnp = enable_upnp ; }

	bool GetEnableNatpmp() { return m_enable_natpmp; }
	void SetEnableNatpmp( bool enable_natpmp ) { m_enable_natpmp = enable_natpmp ; }

	bool GetEnableLsd() { return m_enable_lsd; }
	void SetEnableLsd( bool enable_lsd ) { m_enable_lsd = enable_lsd ; }

	bool GetEnableMetadata() { return m_enable_metadata; }
	void SetEnableMetadata( bool enable_metadata ) { m_enable_metadata = enable_metadata ; }

	bool GetEnablePex() { return m_enable_pex; }
	void SetEnablePex( bool enable_pex ) { m_enable_pex = enable_pex ; }

	const wxString GetLanguage() { return m_language; }
	void SetLanguage(const wxString& language) { m_language = language; }

	bool GetExcludeSeed() { return m_exclude_seed; }
	void SetExcludeSeed(bool exclude_seed ) { m_exclude_seed = exclude_seed; }
#ifdef __WXMSW__
    bool GetRunAtStartup() { return m_run_at_startup; }
    void SetRunAtStartup(bool run_at_startup ) { m_run_at_startup = run_at_startup; }

    bool GetAssociateTorrent() { return m_associate_torrent; }
    void SetAssociateTorrent(bool associate_torrent) { m_associate_torrent = associate_torrent; }

    bool GetAssociateMagnetURI() { return m_associate_magneturi; }
    void SetAssociateMagnetURI(bool associate_magneturi) { m_associate_magneturi = associate_magneturi; }
#endif
    bool GetEnableVideoPreview() { return m_enable_video_preview; }
    void SetEnableVideoPreview(bool enable_preview) { m_enable_video_preview = enable_preview; }

    // libtorrent
	bool GetSequentialDownload() { return m_sequential_download; }
	void SetSequentialDownload(bool isSequential) { m_sequential_download = isSequential; }

	libtorrent::storage_mode_t GetDefaultStorageMode() { return m_storagemode; }
	void SetDefaultStorageMode(const libtorrent::storage_mode_t& storagemode) { m_storagemode = storagemode; } 

	int GetGlobalUploadLimit() { return m_global_upload_limit; }
	void SetGlobalUploadLimit(int uploads) { m_global_upload_limit = uploads;}

	int GetGlobalDownloadLimit() { return m_global_download_limit; }
	void SetGlobalDownloadLimit(int downloads) { m_global_download_limit = downloads;}

	int GetGlobalMaxUploads() { return m_global_max_uploads; }
	void SetGlobalMaxUploads(int uploads) { m_global_max_uploads = uploads;}

	int GetGlobalMaxConnections() { return m_global_max_connections; }
	void SetGlobalMaxConnections(int connections) { m_global_max_connections = connections;}

	enum torrent_state GetDefaultState() { return m_default_state; }
	void SetDefaultState(enum torrent_state state) { m_default_state = state;}

	int GetDefaultRatio() { return m_default_ratio; }
	void SetDefaultRatio(int ratio) { m_default_ratio = ratio;}

	int GetDefaultUploadLimit() { return m_default_upload_limit; }
	void SetDefaultUploadLimit(int upload_limit) { m_default_upload_limit = upload_limit;}
	
	int GetDefaultDownloadLimit() { return m_default_download_limit; }
	void SetDefaultDownloadLimit(int download_limit) { m_default_download_limit = download_limit;}

	int GetMaxUploads() { return m_default_max_uploads; }
	void SetMaxUploads(int uploads) { m_default_max_uploads = uploads;}

	int GetMaxConnections() { return m_default_max_connections; }
	void SetMaxConnections(int connections) { m_default_max_connections = connections;}
	bool GetUseDefault() { return m_usedefault; }
	void SetUseDefault(bool usedefault) { m_usedefault = usedefault; } 

	//dht
	bool GetDHTEnabled() { return m_dht_enabled; }
	void SetDHTEnabled(bool dht_enabled ) { m_dht_enabled = dht_enabled; }

	unsigned int GetDHTPort() { return m_dht_port; }
	void SetDHTPort(unsigned int dht_port) { m_dht_port = dht_port ; }

	unsigned int GetDHTMaxPeers() { return m_dht_max_peers; }
	void SetDHTMaxPeers(unsigned int dht_max_peers) { m_dht_max_peers = dht_max_peers ; }

	unsigned int GetDHTSearchBranching() { return m_dht_search_branching; }
	void SetDHTSearchBranching(unsigned int dht_search_branching) { m_dht_search_branching = dht_search_branching ; }

	unsigned int GetDHTMaxFail() { return m_dht_max_fail; }
	void SetDHTMaxFail(unsigned int dht_max_fail) { m_dht_max_fail = dht_max_fail ; }

	 int GetTrackerCompletionTimeout() { return m_tracker_completion_timeout ; }
	void SetTrackerCompletionTimeout( int tracker_completion_timeout) { m_tracker_completion_timeout = tracker_completion_timeout; }

	 int GetTrackerReceiveTimeout() { return m_tracker_receive_timeout ; }
	void SetTrackerReceiveTimeout( int tracker_receive_timeout) { m_tracker_receive_timeout = tracker_receive_timeout; }

	 int GetStopTrackerTimeout() { return m_stop_tracker_timeout ; }
	void SetStopTrackerTimeout( int stop_tracker_timeout) { m_stop_tracker_timeout = stop_tracker_timeout; }

	 int GetTrackerMaximumResponseLength() { return m_tracker_maximum_response_length ; }
	void SetTrackerMaximumResponseLength( int tracker_maximum_response_length) { m_tracker_maximum_response_length = tracker_maximum_response_length; }

	 int GetPieceTimeout() { return m_piece_timeout ; }
	void SetPieceTimeout( int piece_timeout) { m_piece_timeout = piece_timeout; }
	
	double GetRequestQueueTime() { return m_request_queue_time ; }
	void SetRequestQueueTime(double request_queue_time) { m_request_queue_time = request_queue_time; }

	 int GetMaxAllowedInRequestQueue() { return m_max_allowed_in_request_queue ; }
	void SetMaxAllowedInRequestQueue( int max_allowed_in_request_queue) { m_max_allowed_in_request_queue = max_allowed_in_request_queue; }

	 int GetMaxOutRequestQueue() { return m_max_out_request_queue ; }
	void SetMaxOutRequestQueue( int max_out_request_queue) { m_max_out_request_queue = max_out_request_queue; }

	 int GetWholePiecesThreshold() { return m_whole_pieces_threshold ; }
	void SetWholePiecesThreshold( int whole_pieces_threshold) { m_whole_pieces_threshold = whole_pieces_threshold; }

	 int GetPeerTimeout() { return m_peer_timeout ; }
	void SetPeerTimeout( int peer_timeout) { m_peer_timeout = peer_timeout; }
	
	 int GetUrlseedTimeout() { return m_urlseed_timeout ; }
	void SetUrlseedTimeout( int urlseed_timeout) { m_urlseed_timeout = urlseed_timeout; }

	 int GetUrlseedPipelineSize() { return m_urlseed_pipeline_size ; }
	void SetUrlseedPipelineSize( int urlseed_pipeline_size) { m_urlseed_pipeline_size = urlseed_pipeline_size; }
	
	 int GetFilePoolSize() { return m_file_pool_size ; }
	void SetFilePoolSize( int file_pool_size) { m_file_pool_size = file_pool_size; }

	bool GetAllowMultipleConnectionsPerIP() { return m_allow_multiple_connections_per_ip; }
	void SetAllowMultipleConnectionsPerIP(bool allow_multiple_connections_per_ip) { m_allow_multiple_connections_per_ip = allow_multiple_connections_per_ip ; }
	
	bool GetRateLimitPeersOnLan() { return m_rate_limit_peers_on_lan; }
	void SetRateLimitPeersOnLan(bool rate_limit_peers_on_lan) { m_rate_limit_peers_on_lan = rate_limit_peers_on_lan ; }

	bool GetRateLimitUtp() { return m_rate_limit_utp; }
	void SetRateLimitUtp(bool rate_limit_utp) { m_rate_limit_utp = rate_limit_utp ; }

	 int GetMaxFailCount() { return m_max_failcount ; }
	void SetMaxFailCount( int max_failcount) { m_max_failcount = max_failcount; }
	
	 int GetMinReconnectTime() { return m_min_reconnect_time ; }
	void SetMinReconnectTime( int min_reconnect_time) { m_min_reconnect_time = min_reconnect_time; }
	
	 int GetPeerConnectTimeout() { return m_peer_connect_timeout ; }
	void SetPeerConnectTimeout( int peer_connect_timeout) { m_peer_connect_timeout = peer_connect_timeout; }

	bool GetIgnoreLimitsOnLocalNetwork() { return m_ignore_limits_on_local_network; }
	void SetIgnoreLimitsOnLocalNetwork(bool ignore_limits_on_local_network) { m_ignore_limits_on_local_network = ignore_limits_on_local_network ; }

	 int GetConnectionSpeed() { return m_connection_speed ; }
	void SetConnectionSpeed( int connection_speed) { m_connection_speed = connection_speed; }

	 bool GetSendRedundantHave() { return m_send_redundant_have ; }
	void SetSendRedundantHave( bool send_redundant_have) { m_send_redundant_have = send_redundant_have; }

	bool GetLazyBitfields() { return m_lazy_bitfields; }
	void SetLazyBitfields(bool lazy_bitfields) { m_lazy_bitfields = lazy_bitfields ; }

	 int GetInactivityTimeout() { return m_inactivity_timeout ; }
	void SetInactivityTimeout( int inactivity_timeout) { m_inactivity_timeout = inactivity_timeout; }

	 int GetUnchokeInterval() { return m_unchoke_interval ; }
	void SetUnchokeInterval( int unchoke_interval) { m_unchoke_interval = unchoke_interval; }

	 int GetOptimisticUnchokeMultiplier() { return m_optimistic_unchoke_multiplier ; }
	void SetOptimisticUnchokeMultiplier( int optimistic_unchoke_multiplier) { m_optimistic_unchoke_multiplier = optimistic_unchoke_multiplier; }

	 int GetNumWant() { return m_num_want ; }
	void SetNumWant(int num_want) { m_num_want = num_want; }

	 int GetInitialPickerThreshold() { return m_initial_picker_threshold ; }
	void SetInitialPickerThreshold( int initial_picker_threshold) { m_initial_picker_threshold = initial_picker_threshold; }

	 int GetAllowedFastSetSize() { return m_allowed_fast_set_size ; }
	void SetAllowedFastSetSize( int allowed_fast_set_size) { m_allowed_fast_set_size = allowed_fast_set_size; }

	 int GetMaxOutstandingDiskBytesPerConnection() { return m_max_outstanding_disk_bytes_per_connection ; }
	void SetMaxOutstandingDiskBytesPerConnection( int max_outstanding_disk_bytes_per_connection) { m_max_outstanding_disk_bytes_per_connection = max_outstanding_disk_bytes_per_connection; }

	 int GetHandshakeTimeout() { return m_handshake_timeout ; }
	void SetHandshakeTimeout(int handshake_timeout) { m_handshake_timeout = handshake_timeout; }

	bool GetUseDhtAsFallback() { return m_use_dht_as_fallback; }
	void SetUseDhtAsFallback(bool use_dht_as_fallback) { m_use_dht_as_fallback = use_dht_as_fallback ; }

	bool GetFreeTorrentHashes() { return m_free_torrent_hashes; }
	void SetFreeTorrentHashes(bool free_torrent_hashes) { m_free_torrent_hashes = free_torrent_hashes ; }

	//encryption
	bool GetEncEnabled() { return m_enc_enabled; }
	void SetEncEnabled(bool enc_enabled) { m_enc_enabled = enc_enabled ; }

	unsigned int GetEncPolicy() { return m_enc_policy; }
	void SetEncPolicy(unsigned int enc_policy) { m_enc_policy = enc_policy ; }

	unsigned int GetEncLevel() { return m_enc_level; }
	void SetEncLevel(unsigned int enc_level) { m_enc_level = enc_level ; }

	bool GetEncPreferRC4() { return m_enc_prefer_rc4; }
	void SetEncPreferRC4(bool enc_prefer_rc4) { m_enc_prefer_rc4 = enc_prefer_rc4 ; }

	bool GetAnonymousMode() { return m_anonymous_mode; }
	void SetAnonymousMode( bool anonymous_mode ) { m_anonymous_mode = anonymous_mode ; }

	bool GetUseOSCache() { return m_useoscache; }
	void SetUseOSCache( bool useoscache ) { m_useoscache = useoscache ; }

	int GetMaxActiveSeeds() { return m_max_active_seeds; }
	void SetMaxActiveSeeds(int active_seeds) { m_max_active_seeds = active_seeds; }

	bool GetSupperSeeding() { return m_super_seeding; }
	void SetSupperSeeding( bool supperseeding ) { m_super_seeding = supperseeding; }

	bool GetIgnoreSlowTorrents() { return m_ignore_slow_torrents; }
	void SetIgnoreSlowTorrents( bool ignore ) { m_ignore_slow_torrents = ignore; }

	bool GetSingleInstance() { return m_single_instance; }
	void SetSingleInstance( bool single ) { m_single_instance = single ; }

	int GetEnabledProtocols() { return m_enabled_protocols; }
	void SetEnabledProtocols(int protocol) { m_enabled_protocols = protocol; }

	bool GetRateLimitIpOverhead() { return m_rate_limit_ip_overhead; }
	void SetRateLimitIpOverhead( bool enable ) { m_rate_limit_ip_overhead = enable; }

	bool GetPreferUdpTrackers() { return m_prefer_udp_trackers; }
	void SetPreferUdpTrackers( bool enable ) { m_prefer_udp_trackers = enable; }

	 int GetChokingAlgorithm() { return m_choking_algorithm ; }
	void SetChokingAlgorithm( int alg) { m_choking_algorithm = alg; }

	 int GetSeedChokingAlgorithm() { return m_seed_choking_algorithm ; }
	void SetSeedChokingAlgorithm( int alg) { m_seed_choking_algorithm = alg; }

	//save path history
#define BITSWASH_MAX_SAVE_PATH 10
#define BITSWASH_MAX_OPEN_PATH 10
	std::vector<wxString>& GetSavePathHistory() { return m_savepathhistory; }
	void AddSavePath(wxString path)
	{
		auto p = m_savepathhistory.begin();
		wxString existpath;
		for(p=m_savepathhistory.begin(); p != m_savepathhistory.end(); ++p)
		{
			existpath = *p;
			if ((existpath.CmpNoCase(path)) == 0)
			{
				m_savepathhistory.erase(p);
				break;
			}
		}
		if (m_savepathhistory.size() >= BITSWASH_MAX_SAVE_PATH)
		{
			p = m_savepathhistory.begin();
			m_savepathhistory.erase(p);
		}
		m_savepathhistory.push_back(path);
	}

	std::vector<wxString>& GetOpenPathHistory() { return m_openpathhistory; }
	void AddOpenPath(wxString path)
	{
		auto p = m_openpathhistory.begin();
		wxString existpath;
		for(p=m_openpathhistory.begin(); p != m_openpathhistory.end(); ++p)
		{
			existpath = *p;
			if ((existpath.CmpNoCase(path)) == 0)
			{
				m_openpathhistory.erase(p);
				break;
			}
		}
		if (m_openpathhistory.size() >= BITSWASH_MAX_OPEN_PATH)
		{
			p = m_openpathhistory.begin();
			m_openpathhistory.erase(p);
		}
		m_openpathhistory.push_back(path);
	}
#ifdef __WXMSW__
	bool DetectType(const wxString& type);
	void AddType(const wxString& type);
	void RemoveType(const wxString& type);
	void AddMagnetLinkType();
	void RemoveMagnetLinkType();
#endif
protected:
	void ReadSavePath();
	void WriteSavePath();

private:
	wxString m_appname;
	wxString m_configfile;
	wxFileConfig *m_cfg;


	//GUI
	struct gui_metric m_gui;
	unsigned int m_guiversion;
	wxString m_auiperspective;
	wxString m_dhtbootstrapnodes;

	wxString m_torrentlistctrlsetting;
	wxString m_peerlistctrlsetting;
	wxString m_filelistctrlsetting;
	wxString m_trackerlistctrlsetting;

	// Config
	wxString m_downloadpath;
	wxString m_opentorrentpath;
	unsigned int	m_portmin;
	unsigned int	m_portmax;
	unsigned int	m_refreshtime;
	unsigned int	m_fastresume_save_time;
	unsigned int	m_max_start;
	unsigned int	m_exit_waiting_time;
	int				m_max_active_seeds;
	int				m_enabled_protocols;
	bool		m_use_systray;
	bool 		m_hidetaskbar;
	bool		m_enable_upnp;
	bool		m_enable_natpmp;
	bool		m_enable_lsd;
	bool		m_enable_metadata;
	bool		m_enable_pex;
		
	bool 		m_exclude_seed;
	bool 		m_enable_video_preview;
	bool		m_sequential_download;
#ifdef __WXMSW__
    bool        m_run_at_startup;
    bool        m_associate_torrent;
	bool        m_associate_magneturi;
    static wxString m_startup_regkey;
	static wxString m_bitswash_regkey_path;
#endif

	wxString m_language;

    //Session
	libtorrent::storage_mode_t m_storagemode;
	int 	m_global_upload_limit; 
	int 	m_global_download_limit;
	int 	m_global_max_uploads;
	int 	m_global_max_connections;
	bool 	m_usedefault;
	bool    m_rate_limit_peers_on_lan;
	bool    m_rate_limit_utp;

	/***********************************************************
	 * from libtorrent/include/libtorrent/settings_pack.hpp *
	 ***********************************************************/
	
	// the number of seconds to wait until giving up on a
	// tracker request if it hasn't finished
	int m_tracker_completion_timeout;
	
	// the number of seconds where no data is received
	// from the tracker until it should be considered
	// as timed out
	int m_tracker_receive_timeout;

	// the time to wait when sending a stopped message
	// before considering a tracker to have timed out.
	// this is usually shorter, to make the client quit
	// faster
	int m_stop_tracker_timeout;

	// if the content-length is greater than this value
	// the tracker connection will be aborted
	int m_tracker_maximum_response_length;

	// the number of seconds from a request is sent until
	// it times out if no piece response is returned.
	int m_piece_timeout;

	// the length of the request queue given in the number
	// of seconds it should take for the other end to send
	// all the pieces. i.e. the actual number of requests
	// depends on the download rate and this number.
	double m_request_queue_time;
	
	// the number of outstanding block requests a peer is
	// allowed to queue up in the client. If a peer sends
	// more requests than this (before the first one has
	// been sent) the last request will be dropped.
	// the higher this is, the faster upload speeds the
	// client can get to a single peer.
	int m_max_allowed_in_request_queue;
	
	// the maximum number of outstanding requests to
	// send to a peer. This limit takes precedence over
	// request_queue_time.
	int m_max_out_request_queue;

	// if a whole piece can be downloaded in this number
	// of seconds, or less, the peer_connection will prefer
	// to request whole pieces at a time from this peer.
	// The benefit of this is to better utilize disk caches by
	// doing localized accesses and also to make it easier
	// to identify bad peers if a piece fails the hash check.
	int m_whole_pieces_threshold;
	
	// the number of seconds to wait for any activity on
	// the peer wire before closing the connectiong due
	// to time out.
	int m_peer_timeout;
	
	// same as peer_timeout, but only applies to url-seeds.
	// this is usually set lower, because web servers are
	// expected to be more reliable.
	int m_urlseed_timeout;
	
	// controls the pipelining size of url-seeds
	int m_urlseed_pipeline_size;
	
	// sets the upper limit on the total number of files this
	// session will keep open. The reason why files are
	// left open at all is that some anti virus software
	// hooks on every file close, and scans the file for
	// viruses. deferring the closing of the files will
	// be the difference between a usable system and
	// a completely hogged down system. Most operating
	// systems also has a limit on the total number of
	// file descriptors a process may have open. It is
	// usually a good idea to find this limit and set the
	// number of connections and the number of files
	// limits so their sum is slightly below it.
	int m_file_pool_size;
	
	// false to not allow multiple connections from the same
	// IP address. true will allow it.
	bool m_allow_multiple_connections_per_ip;

	// the number of times we can fail to connect to a peer
	// before we stop retrying it.
	int m_max_failcount;
	
	// the number of seconds to wait to reconnect to a peer.
	// this time is multiplied with the failcount.
	int m_min_reconnect_time;

	// this is the timeout for a connection attempt. If
	// the connect does not succeed within this time, the
	// connection is dropped. The time is specified in seconds.
	int m_peer_connect_timeout;

	// if set to true, upload, download and unchoke limits
	// are ignored for peers on the local network.
	bool m_ignore_limits_on_local_network;

	// the number of connection attempts that
	// are made per second.
	int m_connection_speed;

	// if this is set to true, have messages will be sent
	// to peers that already have the piece. This is
	// typically not necessary, but it might be necessary
	// for collecting statistics in some cases. Default is false.
	bool m_send_redundant_have;

	// if this is true, outgoing bitfields will never be fuil. If the
	// client is seed, a few bits will be set to 0, and later filled
	// in with have messages. This is to prevent certain ISPs
	// from stopping people from seeding.
	bool m_lazy_bitfields;

	// if a peer is uninteresting and uninterested for longer
	// than this number of seconds, it will be disconnected.
	// default is 10 minutes
	int m_inactivity_timeout;

	// the number of seconds between chokes/unchokes
	int m_unchoke_interval;

	// the number of unchoke intervals between
	// optimistic unchokes
	int m_optimistic_unchoke_multiplier;

	// the num want sent to trackers
	int m_num_want;

	// while we have fewer pieces than this, pick
	// random pieces instead of rarest first.
	int m_initial_picker_threshold;

	// the number of allowed pieces to send to peers
	// that supports the fast extensions
	int m_allowed_fast_set_size;

	// the maximum number of bytes a connection may have
	// pending in the disk write queue before its download
	// rate is being throttled. This prevents fast downloads
	// to slow medias to allocate more and more memory
	// indefinitely. This should be set to at least 32 kB
	// to not completely disrupt normal downloads.
	int m_max_outstanding_disk_bytes_per_connection;

	// the number of seconds to wait for a handshake
	// response from a peer. If no response is received
	// within this time, the peer is disconnected.
	int m_handshake_timeout;

	// while this is true, the dht will note be used unless the
	// tracker is online
	bool m_use_dht_as_fallback;

	// if this is true, the piece hashes will be freed, in order
	// to save memory, once the torrent is seeding. This will
	// make the get_torrent_info() function to return an incomplete
	// torrent object that cannot be passed back to add_torrent()
	bool m_free_torrent_hashes;

	// if set to true, the estimated TCP/IP overhead is drained from the
	// rate limiters, to avoid exceeding the limits with the total traffic
	bool m_rate_limit_ip_overhead;

	// ``prefer_udp_trackers`` is true by default. It means that trackers
	// may be rearranged in a way that udp trackers are always tried
	// before http trackers for the same hostname. Setting this to false
	// means that the trackers' tier is respected and there's no
	// preference of one protocol over another.
	bool m_prefer_udp_trackers;

	// ``choking_algorithm`` specifies which algorithm to use to determine
	// which peers to unchoke.
	//
	// The options for choking algorithms are:
	//
	// * ``fixed_slots_choker`` is the traditional choker with a fixed
	//	 number of unchoke slots (as specified by
	//	 ``settings_pack::unchoke_slots_limit``).
	//
	// * ``rate_based_choker`` opens up unchoke slots based on the upload
	//	 rate achieved to peers. The more slots that are opened, the
	//	 marginal upload rate required to open up another slot increases.
	//
	// * ``bittyrant_choker`` attempts to optimize download rate by
	//	 finding the reciprocation rate of each peer individually and
	//	 prefers peers that gives the highest *return on investment*. It
	//	 still allocates all upload capacity, but shuffles it around to
	//	 the best peers first. For this choker to be efficient, you need
	//	 to set a global upload rate limit
	//	 (``settings_pack::upload_rate_limit``). For more information
	//	 about this choker, see the paper_. This choker is not fully
	//	 implemented nor tested.
	//
	// .. _paper: http://bittyrant.cs.washington.edu/#papers
	//
	// ``seed_choking_algorithm`` controls the seeding unchoke behavior.
	// The available options are:
	//
	// * ``round_robin`` which round-robins the peers that are unchoked
	//	 when seeding. This distributes the upload bandwidht uniformly and
	//	 fairly. It minimizes the ability for a peer to download everything
	//	 without redistributing it.
	//
	// * ``fastest_upload`` unchokes the peers we can send to the fastest.
	//	 This might be a bit more reliable in utilizing all available
	//	 capacity.
	//
	// * ``anti_leech`` prioritizes peers who have just started or are
	//	 just about to finish the download. The intention is to force
	//	 peers in the middle of the download to trade with each other.
	int m_choking_algorithm;
	int m_seed_choking_algorithm;

	/***************************************************************
	 * end from libtorrent/include/libtorrent/settings_pack.hpp *
	 ***************************************************************/

	/* dht */
	bool		m_dht_enabled;
	unsigned int 	m_dht_port;
	int 	m_dht_max_peers;
	int 	m_dht_search_branching;
	int 	m_dht_max_fail;

	/* encryption */
	bool	m_enc_enabled; /* will enable both in & out */
	int		m_enc_policy; /* allow / not allow unencrypted will set enc_level to both if allowed */
	int 	m_enc_level;
	bool 	m_enc_prefer_rc4;

	//per torrent settings
	enum torrent_state m_default_state; //RUN, PAUSE, STOP
	int m_default_ratio; //default 150/100 (float in libtorrent)
	int m_default_upload_limit; 
	int m_default_download_limit;
	int m_default_max_uploads;
	int m_default_max_connections;

	//logging
	unsigned int m_log_severity;
	unsigned int m_log_linecount;
	bool m_log_file;
	bool m_anonymous_mode;
	bool m_useoscache;
	bool m_super_seeding;
	bool m_ignore_slow_torrents;
	bool m_single_instance;
	unsigned int m_max_log_files;

	//torrent history
	std::vector<wxString> m_savepathhistory;
	std::vector<wxString> m_openpathhistory;
};

#endif // _CONFIGURATION_H_
