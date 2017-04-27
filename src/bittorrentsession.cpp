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

#include <vector>
#include <algorithm> // for std::max
#include <wx/wx.h>
#include <wx/app.h>
#include <wx/dir.h>
#include <wx/log.h>
#include <wx/filesys.h>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <libtorrent/session.hpp>
#include <libtorrent/entry.hpp>
#include <libtorrent/bencode.hpp>
#include <libtorrent/session.hpp>
#include <libtorrent/fingerprint.hpp>
#include <libtorrent/session_settings.hpp>
#include <libtorrent/identify_client.hpp>
#include <libtorrent/alert_types.hpp>
#include <libtorrent/create_torrent.hpp>
#include <libtorrent/session_status.hpp>

//plugins
#include <libtorrent/extensions/ut_metadata.hpp>
#include <libtorrent/extensions/ut_pex.hpp>
#include <libtorrent/extensions/smart_ban.hpp>
#include <libtorrent/extensions/ut_pex.hpp>

#include "functions.h"
#include "magneturi.h"
#include "bitswash.h"
#include "bittorrentsession.h"

namespace lt = libtorrent;

#ifdef TORRENT_DISABLE_DHT
	#error you must not disable DHT
#endif

struct BTQueueSortAsc
{
	bool operator()( const shared_ptr<torrent_t>& torrent_start, const shared_ptr<torrent_t>& torrent_end ) {
		return torrent_start->config->GetQIndex() < torrent_end->config->GetQIndex();
	}
};

struct BTQueueSortDsc
{
	bool operator()( const shared_ptr<torrent_t>& torrent_start, const shared_ptr<torrent_t>& torrent_end ) {
		return torrent_start->config->GetQIndex() > torrent_end->config->GetQIndex();
	}
};


BitTorrentSession::BitTorrentSession( wxApp* pParent, Configuration* config, wxMutex *mutex/* = 0*/, wxCondition *condition/* = 0*/ )
	: wxThread( wxTHREAD_JOINABLE ), m_upnp_started( false ), m_natpmp_started( false ), m_lsd_started( false ), m_libbtsession( 0 )
{
	m_pParent = pParent;
	m_config = config;
	m_condition = condition;
    m_mutex = mutex;
	//boost::filesystem::path::default_name_check(boost::filesystem::no_check);
	using lt::find_metric_idx;

	std::vector<lt::stats_metric> metrics = lt::session_stats_metrics();
	m_cnt[0].resize(metrics.size(), 0);
	m_cnt[1].resize(metrics.size(), 0);

	m_queued_bytes_idx = find_metric_idx("disk.queued_write_bytes");
	m_wasted_bytes_idx = find_metric_idx("net.recv_redundant_bytes");
	m_failed_bytes_idx = find_metric_idx("net.recv_failed_bytes");
	m_has_incoming_connections_idx = find_metric_idx("net.has_incoming_connections");
	m_num_peers_idx = find_metric_idx("peer.num_peers_connected");
	m_recv_payload_idx = find_metric_idx("net.recv_payload_bytes");
	m_sent_payload_idx = find_metric_idx("net.sent_payload_bytes");
	m_unchoked_idx = find_metric_idx("peer.num_peers_up_unchoked");
	m_unchoke_slots_idx = find_metric_idx("ses.num_unchoke_slots");
	m_limiter_up_queue_idx = find_metric_idx("net.limiter_up_queue");
	m_limiter_down_queue_idx = find_metric_idx("net.limiter_down_queue");
	m_queued_writes_idx = find_metric_idx("disk.num_write_jobs");
	m_queued_reads_idx = find_metric_idx("disk.num_read_jobs");

	m_writes_cache_idx = find_metric_idx("disk.write_cache_blocks");
	m_reads_cache_idx = find_metric_idx("disk.read_cache_blocks");
	m_pinned_idx = find_metric_idx("disk.pinned_blocks");
	m_num_blocks_read_idx = find_metric_idx("disk.num_blocks_read");
	m_cache_hit_idx = find_metric_idx("disk.num_blocks_cache_hits");
	m_blocks_in_use_idx = find_metric_idx("disk.disk_blocks_in_use");
	m_blocks_written_idx = find_metric_idx("disk.num_blocks_written");
	m_write_ops_idx = find_metric_idx("disk.num_write_ops");

	m_mfu_size_idx = find_metric_idx("disk.arc_mfu_size");
	m_mfu_ghost_idx = find_metric_idx("disk.arc_mfu_ghost_size");
	m_mru_size_idx = find_metric_idx("disk.arc_mru_size");
	m_mru_ghost_idx = find_metric_idx("disk.arc_mru_ghost_size");

	m_utp_idle = find_metric_idx("utp.num_utp_idle");
	m_utp_syn_sent = find_metric_idx("utp.num_utp_syn_sent");
	m_utp_connected = find_metric_idx("utp.num_utp_connected");
	m_utp_fin_sent = find_metric_idx("utp.num_utp_fin_sent");
	m_utp_close_wait = find_metric_idx("utp.num_utp_close_wait");
	
	m_dht_nodes_idx = find_metric_idx("dht.dht_nodes");
	m_download_rate = 0.0;
	m_upload_rate = 0.0;
}

BitTorrentSession::~BitTorrentSession()
{
}

static const char PEER_ID[] = "SW";

void *BitTorrentSession::Entry()
{
	WXLOGDEBUG(( _T( "BitTorrentSession Thread lt::entry pid 0x%lx priority %u.\n" ), GetId(), GetPriority() ));
	ConfigureSession();

	// Resume main thread
	if(m_mutex != 0 && m_condition != 0)
	{
		wxMutexLocker lock(*m_mutex); 
		m_condition->Broadcast(); // same as Signal() here -- one waiter only
	}

	ScanTorrentsDirectory( wxGetApp().SaveTorrentsPath() );

	//( ( BitSwash* )m_pParent )->BTInitDone();
	bts_event evt = BTS_EVENT_INVALID;
	wxMessageQueueError result = wxMSGQUEUE_NO_ERROR;
	time_t starttime = wxDateTime::GetTimeNow(), now;
	bool check_now = true;
	do
	{
		evt = BTS_EVENT_INVALID;
		result = m_evt_queue.ReceiveTimeout( 1000, evt );
		if(result == wxMSGQUEUE_NO_ERROR)
		{
			switch(evt)
			{
				case BTS_EVENT_ALERT:
				{
					HandleTorrentAlert();
					break;
				}
				case BTS_EVENT_QUEUEUPDATE:
				{
					check_now = true;
					break;
				}
				default:
					break;
			}
		}

		if(!check_now)
		{
			now = wxDateTime::GetTimeNow();
			int timediff = now - starttime;
			check_now = (timediff >= 5);
		}

		if(check_now)
		{
			//DumpTorrents();
			CheckQueueItem();
			starttime = wxDateTime::GetTimeNow();
			check_now = false;
		}

		if( TestDestroy() )
		{ break; }
	}
	while(result != wxMSGQUEUE_MISC_ERROR);

	Exit( 0 );
	return NULL;
}

typedef boost::function<void()> null_notify_func_t;
void BitTorrentSession::OnExit()
{
	//XXX have 1 soon
	//
	//wxCriticalSectionLocker locker(wxGetApp().m_btcritsect);
	//save DHT lt::entry
	WXLOGDEBUG(( _T( "BitTorrent Session exiting\n" ) ));

	if( m_config->GetDHTEnabled() )
	{
		wxString dhtstatefile = wxGetApp().DHTStatePath();

		try
		{
			lt::entry dht_state;
			m_libbtsession->save_state(dht_state, lt::session::save_dht_state);
			std::ofstream out( ( const char* )dhtstatefile.mb_str( wxConvFile ), std::ios_base::binary );
			out.unsetf( std::ios_base::skipws );
			bencode( std::ostream_iterator<char>( out ), dht_state );
		}
		catch( std::exception& e )
		{
			wxLogError( _T( "%s\n" ), e.what() );
		}
	}

	SaveAllTorrent();
	m_evt_queue.Clear();
	std::vector<lt::alert*> alerts;
	m_libbtsession->pop_alerts( &alerts );
	m_libbtsession->set_alert_notify(null_notify_func_t());
	delete m_libbtsession;
	{
		wxMutexLocker ml( m_torrent_queue_lock );
		m_torrent_queue.empty();
		m_torrent_queue.swap( m_torrent_queue );
	}
}

void BitTorrentSession::Configure(lt::settings_pack &settingsPack)
{
	std::string peerId = lt::generate_fingerprint(PEER_ID, BITSWASH_FINGERPRINT_VERSION);
	settingsPack.set_str( lt::settings_pack::user_agent, wxGetApp().UserAgent().ToStdString( ));

	// Speed limit
	//const bool altSpeedLimitEnabled = isAltGlobalSpeedLimitEnabled();
	//settingsPack.set_int(lt::settings_pack::download_rate_limit, altSpeedLimitEnabled ? altGlobalDownloadSpeedLimit() : globalDownloadSpeedLimit());
	//settingsPack.set_int(lt::settings_pack::upload_rate_limit, altSpeedLimitEnabled ? altGlobalUploadSpeedLimit() : globalUploadSpeedLimit());
	settingsPack.set_int( lt::settings_pack::upload_rate_limit, m_config->GetGlobalUploadLimit() );
	settingsPack.set_int( lt::settings_pack::download_rate_limit, m_config->GetGlobalDownloadLimit() );
	// * Max Half-open connections
	//settingsPack.set_int(lt::settings_pack::half_open_limit, m_config->GetGlobalMaxHalfConnect());
	// * Max connections limit
	settingsPack.set_int(lt::settings_pack::connections_limit, m_config->GetGlobalMaxConnections());
	// * Global max upload slots
	settingsPack.set_int(lt::settings_pack::unchoke_slots_limit, m_config->GetGlobalMaxUploads());
	// uTP

	settingsPack.set_int( lt::settings_pack::tracker_completion_timeout, m_config->GetTrackerCompletionTimeout());
	settingsPack.set_int( lt::settings_pack::tracker_receive_timeout, m_config->GetTrackerReceiveTimeout());
	settingsPack.set_int( lt::settings_pack::stop_tracker_timeout, m_config->GetStopTrackerTimeout());
	settingsPack.set_int( lt::settings_pack::tracker_maximum_response_length, m_config->GetTrackerMaximumResponseLength());
	settingsPack.set_int( lt::settings_pack::piece_timeout, m_config->GetPieceTimeout());
	settingsPack.set_int( lt::settings_pack::request_queue_time, m_config->GetRequestQueueTime());
	settingsPack.set_int( lt::settings_pack::max_allowed_in_request_queue, m_config->GetMaxAllowedInRequestQueue());
	settingsPack.set_int( lt::settings_pack::max_out_request_queue, m_config->GetMaxOutRequestQueue());
	settingsPack.set_int( lt::settings_pack::whole_pieces_threshold, m_config->GetWholePiecesThreshold());
	settingsPack.set_int( lt::settings_pack::peer_timeout, m_config->GetPeerTimeout());
	settingsPack.set_int( lt::settings_pack::urlseed_timeout, m_config->GetUrlseedTimeout());
	settingsPack.set_int( lt::settings_pack::urlseed_pipeline_size, m_config->GetUrlseedPipelineSize());
	settingsPack.set_int( lt::settings_pack::file_pool_size, m_config->GetFilePoolSize());
	settingsPack.set_bool( lt::settings_pack::allow_multiple_connections_per_ip, m_config->GetAllowMultipleConnectionsPerIP());
	settingsPack.set_int( lt::settings_pack::max_failcount, m_config->GetMaxFailCount());
	settingsPack.set_int( lt::settings_pack::min_reconnect_time, m_config->GetMinReconnectTime());
	settingsPack.set_int( lt::settings_pack::peer_connect_timeout, m_config->GetPeerConnectTimeout());
	// Ignore limits on LAN
	//settingsPack.set_bool( lt::settings_pack::ignore_limits_on_local_network, m_config->GetIgnoreLimitsOnLocalNetwork());
	settingsPack.set_int( lt::settings_pack::connection_speed, m_config->GetConnectionSpeed());
	settingsPack.set_bool( lt::settings_pack::send_redundant_have, m_config->GetSendRedundantHave());
	settingsPack.set_bool( lt::settings_pack::lazy_bitfields, m_config->GetLazyBitfields());
	settingsPack.set_int( lt::settings_pack::inactivity_timeout, m_config->GetInactivityTimeout());
	settingsPack.set_int( lt::settings_pack::unchoke_interval, m_config->GetUnchokeInterval());
	//	settingsPack.set_int( lt::settings_pack::optimistic_unchoke_multiplier, m_config->GetOptimisticUnchokeMultiplier());
	settingsPack.set_int( lt::settings_pack::num_want, m_config->GetNumWant());
	settingsPack.set_int( lt::settings_pack::initial_picker_threshold, m_config->GetInitialPickerThreshold());
	settingsPack.set_int( lt::settings_pack::allowed_fast_set_size, m_config->GetAllowedFastSetSize());
	//	settingsPack.set_int( lt::settings_pack::max_outstanding_disk_bytes_per_connection, m_config->GetMaxOutstandingDiskBytesPerConnection());
	settingsPack.set_int( lt::settings_pack::handshake_timeout, m_config->GetHandshakeTimeout());
	settingsPack.set_bool( lt::settings_pack::use_dht_as_fallback, m_config->GetUseDhtAsFallback());
	//settingsPack.set_bool( lt::settings_pack::free_torrent_hashes, m_config->GetFreeTorrentHashes());

	//Todo: new options=======================================
	settingsPack.set_bool( lt::settings_pack::announce_to_all_tiers, true);
	settingsPack.set_bool( lt::settings_pack::announce_to_all_trackers, true);

	lt::settings_pack::io_buffer_mode_t mode = /*useOSCache() ? */lt::settings_pack::enable_os_cache;
                                                              //: lt::settings_pack::disable_os_cache;
	settingsPack.set_int(lt::settings_pack::disk_io_read_mode, mode);
	settingsPack.set_int(lt::settings_pack::disk_io_write_mode, mode);
	settingsPack.set_bool(lt::settings_pack::anonymous_mode, false);

	
    // The most secure, rc4 only so that all streams are encrypted
    //settingsPack.set_int(lt::settings_pack::allowed_enc_level, ( lt::pe_settings::enc_level )( m_config->GetEncLevel()));
    settingsPack.set_bool(lt::settings_pack::prefer_rc4, m_config->GetEncEnabled());
	
	if( m_config->GetEncEnabled() )
	{
		settingsPack.set_int(lt::settings_pack::out_enc_policy, lt::settings_pack::pe_enabled);
		settingsPack.set_int(lt::settings_pack::in_enc_policy, lt::settings_pack::pe_enabled);
		wxLogMessage( _T( "Encryption enabled, policy %d\n" ), m_config->GetEncPolicy() );
	}
	else
	{
		settingsPack.set_int(lt::settings_pack::out_enc_policy, lt::settings_pack::pe_disabled);
		settingsPack.set_int(lt::settings_pack::in_enc_policy, lt::settings_pack::pe_disabled);
		wxLogMessage( _T( "Encryption is disabled" ) );
	}
    //case 1: // Forced
    //    settingsPack.set_int(lt::settings_pack::out_enc_policy, lt::settings_pack::pe_forced);
    //    settingsPack.set_int(lt::settings_pack::in_enc_policy, lt::settings_pack::pe_forced);
	/*
	if (isQueueingSystemEnabled()) {
        adjustLimits(settingsPack);

        settingsPack.set_int(lt::settings_pack::active_seeds, maxActiveUploads());
        settingsPack.set_bool(lt::settings_pack::dont_count_slow_torrents, ignoreSlowTorrentsForQueueing());
    }
    else {
        settingsPack.set_int(lt::settings_pack::active_downloads, -1);
        settingsPack.set_int(lt::settings_pack::active_seeds, -1);
        settingsPack.set_int(lt::settings_pack::active_limit, -1);
    }
	*/
	// 1 active torrent force 2 connections. If you have more active torrents * 2 than connection limit,
	// connection limit will get extended. Multiply max connections or active torrents by 10 for queue.
	// Ignore -1 values because we don't want to set a max int message queue
	//settingsPack.set_int(lt::settings_pack::alert_queue_size, std::max(1000,
	//	10 * std::max(maxActiveTorrents() * 2, maxConnections())));
	
	// Outgoing ports
	//settingsPack.set_int(lt::settings_pack::outgoing_port, m_config->GetPortMin());
	//settingsPack.set_int(lt::settings_pack::num_outgoing_ports, m_config->GetPortMax() - m_config->GetPortMin() + 1);
	
	// Include overhead in transfer limits
	settingsPack.set_bool(lt::settings_pack::rate_limit_ip_overhead, true);
	// IP address to announce to trackers
	//settingsPack.set_str(lt::settings_pack::announce_ip, Utils::String::toStdString(announceIP()));
	// Super seeding
	settingsPack.set_bool(lt::settings_pack::strict_super_seeding, false);
	settingsPack.set_bool(lt::settings_pack::enable_incoming_utp, true);
	settingsPack.set_bool(lt::settings_pack::enable_outgoing_utp, true);
	// uTP rate limiting
	//settingsPack.set_bool(lt::settings_pack::rate_limit_utp, isUTPRateLimited());
	//settingsPack.set_int(lt::settings_pack::mixed_mode_algorithm, isUTPRateLimited()
	//					 ? lt::settings_pack::prefer_tcp
	//					 : lt::settings_pack::peer_proportional);
	
	//settingsPack.set_bool(lt::settings_pack::apply_ip_filter_to_trackers, isTrackerFilteringEnabled());

	//Todo: new options=======================================

    settingsPack.set_int(lt::settings_pack::active_tracker_limit, -1);
    settingsPack.set_int(lt::settings_pack::active_dht_limit, -1);
    settingsPack.set_int(lt::settings_pack::active_lsd_limit, -1);

	// Set severity level of libtorrent session
    int alertMask = lt::alert::error_notification
                    | lt::alert::peer_notification
                    | lt::alert::port_mapping_notification
                    | lt::alert::storage_notification
                    | lt::alert::tracker_notification
                    | lt::alert::status_notification
                    | lt::alert::ip_block_notification
                    | lt::alert::progress_notification
                    | lt::alert::stats_notification
                    ;
#if 0
	int alertMask = lt::alert::all_categories
				& ~(lt::alert::dht_notification
				+ lt::alert::progress_notification
				+ lt::alert::stats_notification
				+ lt::alert::session_log_notification
				+ lt::alert::torrent_log_notification
				+ lt::alert::peer_log_notification
				+ lt::alert::dht_log_notification
				+ lt::alert::picker_log_notification
				);
#endif
	settingsPack.set_int(lt::settings_pack::alert_mask, alertMask);
	settingsPack.set_str(lt::settings_pack::peer_fingerprint, peerId);
	settingsPack.set_bool(lt::settings_pack::listen_system_port_fallback, false);
	settingsPack.set_bool(lt::settings_pack::upnp_ignore_nonrouters, true);
	settingsPack.set_bool(lt::settings_pack::use_dht_as_fallback, false);
	// Disable support for SSL torrents for now
	settingsPack.set_int(lt::settings_pack::ssl_listen, 0);
	// To prevent ISPs from blocking seeding
	// Speed up exit
	settingsPack.set_int(lt::settings_pack::auto_scrape_interval, 1200); // 20 minutes
	settingsPack.set_int(lt::settings_pack::auto_scrape_min_interval, 900); // 15 minutes
	settingsPack.set_bool(lt::settings_pack::no_connect_privileged_ports, false);
	settingsPack.set_int(lt::settings_pack::seed_choking_algorithm, lt::settings_pack::fastest_upload);

	settingsPack.set_bool(lt::settings_pack::enable_dht, m_config->GetDHTEnabled());

	if( m_config->GetDHTEnabled() )
	{
		settingsPack.set_str(lt::settings_pack::dht_bootstrap_nodes, "dht.libtorrent.org:25401,router.bittorrent.com:6881,router.utorrent.com:6881,dht.transmissionbt.com:6881,dht.aelitis.com:6881");
	}
	settingsPack.set_bool(lt::settings_pack::enable_lsd, m_config->GetEnableLsd());
}

void BitTorrentSession::ConfigureSession()
{
	lt::settings_pack pack;

    Configure(pack);

    m_libbtsession = new lt::session(pack, 0);

	m_libbtsession->set_alert_notify(boost::bind(&BitTorrentSession::PostLibTorrentAlertEvent, this));

	if( m_config->GetDHTEnabled() )
	{
		lt::entry dht_state;
		long dhtport = m_config->GetDHTPort();
		wxString dhtstatefile = wxGetApp().DHTStatePath();
		struct lt::dht_settings DHTSettings;

		DHTSettings.privacy_lookups = true;
		DHTSettings.max_peers_reply = m_config->GetDHTMaxPeers();
		DHTSettings.search_branching = m_config->GetDHTSearchBranching();
		DHTSettings.max_fail_count = m_config->GetDHTMaxFail();
		m_libbtsession->set_dht_settings( DHTSettings );

		if( wxFileExists( dhtstatefile ) )
		{
			wxLogInfo( _T( "Restoring previous DHT state " ) + dhtstatefile +_T("\n") );
			std::ifstream in( ( const char* )dhtstatefile.mb_str( wxConvFile ), std::ios_base::binary );
			in.unsetf( std::ios_base::skipws );

			try
			{
				dht_state = lt::bdecode( std::istream_iterator<char>( in ), std::istream_iterator<char>() );
			}
			catch( std::exception& e )
			{
				wxLogWarning( _T( "Unable to restore dht state file %s - %s\n" ), dhtstatefile.c_str(), wxString::FromUTF8( e.what() ).c_str() );
			}
		}
		else
		{
			wxLogWarning( _T( "Previous DHT state not found " ) + dhtstatefile  +_T("\n"));
		}
	}

	SetLogSeverity();
	//Network settings
	SetConnection();
	StartExtensions();
	StartUpnp();
	StartNatpmp();
}

void BitTorrentSession::SetLogSeverity()
{
	wxASSERT( m_libbtsession != NULL );
	//m_libbtsession->set_severity_level( ( lt::alert::severity_t )m_config->GetLogSeverity() );
}

void BitTorrentSession::SetConnection()
{
	wxASSERT( m_libbtsession != NULL );
	long portmin, portmax;
	portmin = m_config->GetPortMin();
	portmax = m_config->GetPortMax();

	if( ( portmin < 0 ) || ( portmin > 65535 ) )
	{
		portmin = 6881 ;
		m_config->SetPortMin( portmin );
	}

	if( ( portmax < 0 ) || ( portmax > 65535 ) || ( portmax < portmin ) )
	{
		portmax = portmin + 8;
		m_config->SetPortMax( portmax );
	}

	/*error_code ec;
	m_libbtsession->listen_on( std::make_pair( portmin, portmax ), ec,
							   m_config->m_local_ip.IPAddress().ToAscii() );*/
}

void BitTorrentSession::StartExtensions()
{
	wxASSERT( m_libbtsession != NULL );

	/* XXX no way to remove extension currently
	 * restart is needed
	 */
	if( m_config->GetEnableMetadata() )
	{
		m_libbtsession->add_extension( &lt::create_ut_metadata_plugin );
	}

	if( m_config->GetEnablePex() )
	{
		m_libbtsession->add_extension( &lt::create_ut_pex_plugin );
	}

	m_libbtsession->add_extension( &lt::create_smart_ban_plugin );
}

void BitTorrentSession::StartUpnp()
{
	wxASSERT( m_libbtsession != NULL );

	if( m_config->GetEnableUpnp() )
	{
		// XXX workaround upnp crashed when started twice
		// libtorrent bugs
		if( !m_upnp_started )
		{
			lt::settings_pack settingsPack = m_libbtsession->get_settings();
			settingsPack.set_bool(lt::settings_pack::enable_upnp, true );
			m_libbtsession->apply_settings( settingsPack );
			m_upnp_started = true;
		}
	}
	else
	{
		lt::settings_pack settingsPack = m_libbtsession->get_settings();
		settingsPack.set_bool(lt::settings_pack::enable_upnp, false );
		m_libbtsession->apply_settings( settingsPack );
		m_upnp_started = false;
	}
}

void BitTorrentSession::StartNatpmp()
{
	wxASSERT( m_libbtsession != NULL );

	if( m_config->GetEnableNatpmp() )
	{
		if( !m_natpmp_started )
		{
			lt::settings_pack settingsPack = m_libbtsession->get_settings();
			settingsPack.set_bool(lt::settings_pack::enable_natpmp, true );
			m_libbtsession->apply_settings( settingsPack );
		}

		m_natpmp_started = true;
	}
	else
	{
		lt::settings_pack settingsPack = m_libbtsession->get_settings();
		settingsPack.set_bool(lt::settings_pack::enable_natpmp, false );
		m_libbtsession->apply_settings( settingsPack );
		m_natpmp_started = false;
	}
}

void BitTorrentSession::AddTorrentToSession( shared_ptr<torrent_t>& torrent )
{
	lt::torrent_handle &handle = torrent->handle;
	wxLogWarning(( _T( "AddTorrent %s into session\n" ), torrent->name.c_str() ));
	wxString fastresumefile = wxGetApp().SaveTorrentsPath() + torrent->hash + _T( ".fastresume" );
	lt::entry resume_data;

	if( wxFileExists( fastresumefile ) )
	{
		wxLogInfo( _T( "%s: Read fast resume data\n" ), torrent->name.c_str() );
		std::ifstream fastresume_in( ( const char* )fastresumefile.mb_str( wxConvFile ), std::ios_base::binary );
		fastresume_in.unsetf( std::ios_base::skipws );

		try
		{
			resume_data = lt::bdecode( std::istream_iterator<char>( ( fastresume_in ) ), std::istream_iterator<char>() );
		}
		catch( std::exception& e )
		{
			wxLogError( wxString::FromUTF8( e.what() ) + _T("\n") );
		}
	}

	try
	{
		//XXX libtorrent updated with 3 mode
		//sparse (default)
		//allocate
		//compact
		//
		lt::add_torrent_params p;
		shared_ptr<lt::torrent_info> t(new lt::torrent_info(*(torrent->info)));
		p.ti = t;
		//p.url = wxFileSystem::FileNameToURL(const wxFileName &filename)
		p.save_path = ( const char* )torrent->config->GetDownloadPath().mb_str( wxConvUTF8 );

		if( resume_data.type() != lt::entry::undefined_t )
		{ bencode( std::back_inserter( p.resume_data ), resume_data ); }

		if (torrent->config->GetTorrentState() != TORRENT_STATE_PAUSE)
			p.flags |= lt::add_torrent_params::flag_paused;
		p.flags &= ~lt::add_torrent_params::flag_duplicate_is_error;
		p.flags |= lt::add_torrent_params::flag_auto_managed;

		wxString strStorageMode;
		enum lt::storage_mode_t eStorageMode = torrent->config->GetTorrentStorageMode();

		switch( eStorageMode )
		{
		case lt::storage_mode_allocate:
			strStorageMode = _( "Full" );
			break;

		case lt::storage_mode_sparse:
			strStorageMode = _( "Sparse" );
			break;

		default:
			eStorageMode = lt::storage_mode_sparse;
			strStorageMode = _( "Sparse" );
			break;
		}

		p.storage_mode = eStorageMode;
		wxLogInfo( _T( "%s: %s allocation mode\n" ), torrent->name.c_str(), strStorageMode.c_str() );
		m_libbtsession->async_add_torrent( p );
		m_queue_torrent_set.insert(wxString(torrent->hash));

		//enum torrent_state state = ( enum torrent_state ) torrent->config->GetTorrentState();
		//handle.pause();
		//ConfigureTorrent(torrent);
	}
	catch( std::exception& e )
	{
		//posibly duplicate torrent
		wxLogError( wxString::FromUTF8( e.what() ) + _T("\n") );
	}
}

bool BitTorrentSession::AddTorrent( shared_ptr<torrent_t>& torrent )
{
	wxLogWarning( _T( "Add Torrent %s, state %d\n" ),  torrent->name.c_str(), torrent->config->GetTorrentState() );

	try
	{
		{
			wxMutexLocker ml( m_torrent_queue_lock );
			if( find_torrent_from_hash( wxString(torrent->hash) ) >= 0 )
			{
				wxLogWarning( _T( "Torrent %s already exists\n" ), torrent->name.c_str() );
				return false;
			}
			
			m_torrent_queue.push_back( torrent );
			m_running_torrent_map.insert( std::pair<wxString, int>( wxString(torrent->hash), (int)(m_torrent_queue.size() - 1 )) );
		}
		enum torrent_state state = ( enum torrent_state ) torrent->config->GetTorrentState();
		
		//XXX redundant ? StartTorrent below will call the same thing
		if( ( state == TORRENT_STATE_START ) ||
				( state == TORRENT_STATE_FORCE_START ) ||
				( state == TORRENT_STATE_PAUSE ) )
		{
			AddTorrentToSession( torrent );
		}
	}
	catch( std::exception& e )
	{
		wxLogError( wxString::FromUTF8( e.what() ) + _T("\n") );
		return false;
	}

	return true;
}

void BitTorrentSession::RemoveTorrent( shared_ptr<torrent_t>& torrent, bool deletedata )
{
	WXLOGDEBUG(( _T( "%s: Removing Torrent\n" ), torrent->name.c_str() ));

	{
		wxMutexLocker ml( m_torrent_queue_lock );
		int idx = find_torrent_from_hash( wxString(torrent->hash) );

		if( idx < 0 )
		{
			wxLogError( _T( "RemoveTorrent %s: Torrent not found in queue\n" ), torrent->name.c_str() );
			return ;
		}
		torrents_t::iterator torrent_it = m_torrent_queue.begin() + idx;
		m_running_torrent_map.erase( wxString(( *torrent_it )->hash) );
		m_torrent_queue.erase( torrent_it );
	}

	lt::torrent_handle& h = torrent->handle;
	if( h.is_valid() )
	{
		h.pause();
		m_libbtsession->remove_torrent( h );
	}

	enum torrent_state state = ( enum torrent_state ) torrent->config->GetTorrentState();

	wxString app_prefix = wxGetApp().SaveTorrentsPath() +
							  wxString(torrent->hash);
	wxString fastresumefile = app_prefix + _T( ".fastresume" );
	wxString resumefile = app_prefix + _T( ".resume" );
	wxString torrentconffile = app_prefix + _T( ".conf" );
	wxString torrentfile = app_prefix + _T( ".torrent" );

	if( ( wxFileExists( fastresumefile ) ) &&
			( !wxRemoveFile( fastresumefile ) ) )
	{
		wxLogError( _T( "Error removing file %s\n" ), fastresumefile.c_str() );
	}

	if( ( wxFileExists( resumefile ) ) &&
			( !wxRemoveFile( resumefile ) ) )
	{
		wxLogError( _T( "Error removing file %s\n" ), resumefile.c_str() );
	}

	if( ( wxFileExists( torrentconffile ) ) &&
			( !wxRemoveFile( torrentconffile ) ) )
	{
		wxLogError( _T( "Error removing file %s\n" ), torrentconffile.c_str() );
	}

	if( ( wxFileExists( torrentfile ) ) &&
			( !wxRemoveFile( torrentfile ) ) )
	{
		wxLogError( _T( "Error removing file %s\n" ), torrentfile.c_str() );
	}

	if( deletedata )
	{
		wxString download_data = torrent->config->GetDownloadPath() + ( wxString )torrent->name.c_str() ;
		wxLogInfo( _T( "%s: Remove Data as well in %s\n" ), torrent->name.c_str(), download_data.c_str() );

		if( wxDirExists( download_data ) )
		{
			if( !RemoveDirectory( download_data ) )
			{
				wxLogError( _T( "%s: Error removing directory %s error %s\n" ), torrent->name.c_str(), download_data.c_str(), wxSysErrorMsg( wxSysErrorCode() ) );
			}
		}
		else if( wxFileExists( download_data ) )
		{
			if( !wxRemoveFile( download_data ) )
			{
				wxLogError( _T( "%s: Error removing file %s\n" ), torrent->name.c_str(), download_data.c_str() );
			}
		}
		else
		{ wxLogInfo( _T( "%s: No downloaded data to remove\n" ), torrent->name.c_str() ); }
	}

	if( ( state == TORRENT_STATE_START ) ||
			( state == TORRENT_STATE_FORCE_START ) ||
				( state == TORRENT_STATE_PAUSE ) )
	{
		PostQueueUpdateEvent();
	}
}

int BitTorrentSession::FindTorrent( const wxString &hash )
{
	wxMutexLocker ml(m_torrent_queue_lock);
	int idx = find_torrent_from_hash(hash);

	return idx;
}

shared_ptr<torrent_t> BitTorrentSession::GetTorrent( int idx )
{
	shared_ptr<torrent_t> torrent;

	wxMutexLocker ml( m_torrent_queue_lock );
	if( idx >= 0 && idx < m_torrent_queue.size())
	{
		torrent = m_torrent_queue.at( idx );
	}

	return torrent;
}

void BitTorrentSession::MergeTorrent( shared_ptr<torrent_t>& dst_torrent, shared_ptr<torrent_t>& src_torrent )
{
	lt::torrent_handle &torrent_handle = dst_torrent->handle;
	std::vector<lt::announce_entry> const& trackers = src_torrent->info->trackers();

	if(torrent_handle.is_valid())
	{
		for( size_t i = 0; i < trackers.size(); ++i )
		{
			torrent_handle.add_tracker( trackers.at( i ) );
		}

		std::vector<lt::web_seed_entry> const& web_seeds = src_torrent->info->web_seeds();

		for( std::vector<lt::web_seed_entry>::const_iterator i = web_seeds.begin(); i != web_seeds.end(); ++i )
		{
			if( i->type == lt::web_seed_entry::url_seed )
			{ torrent_handle.add_url_seed( i->url ); }
			else if( i->type == lt::web_seed_entry::http_seed )
			{ torrent_handle.add_http_seed( i->url ); }
		}
	}
}

void BitTorrentSession::MergeTorrent( shared_ptr<torrent_t>& dst_torrent, MagnetUri& src_magneturi )
{
	lt::torrent_handle &torrent_handle = dst_torrent->handle;
	std::vector<std::string> const& trackers = src_magneturi.trackers();

	if(torrent_handle.is_valid())
	{
		for( size_t i = 0; i < trackers.size(); ++i )
		{
			torrent_handle.add_tracker( lt::announce_entry( trackers.at( i ) ) );
		}

		std::vector<std::string> const& url_seeds = src_magneturi.urlSeeds();

		for( std::vector<std::string>::const_iterator i = url_seeds.begin(); i != url_seeds.end(); ++i )
		{
			torrent_handle.add_url_seed( *i );
		}
	}
}

void BitTorrentSession::ScanTorrentsDirectory( const wxString& dirname )
{
	wxASSERT( m_libbtsession != NULL );
	wxDir torrents_dir( dirname );
	wxString filename;
	//wxString fullpath;
	wxFileName fn;
	shared_ptr<torrent_t> torrent;

	if( !torrents_dir.IsOpened() )
	{
		wxLogWarning( _T( "Error opening directory %s for processing\n" ), wxGetApp().SaveTorrentsPath().c_str() );
		return ;
	}

	bool cont = torrents_dir.GetFirst( &filename, _T( "*.torrent" ), wxDIR_FILES );

	while( cont )
	{
		//fullpath = dirname + filename;
		fn.Assign(dirname, filename);
		WXLOGDEBUG(( _T( "Saved Torrent: %s\n" ), fn.GetFullPath().c_str() ));
		torrent =  ParseTorrent(fn.GetFullPath());

		if (torrent->config->GetTorrentState() == TORRENT_STATE_QUEUE) /*Fist time added torrent, it's not in libtorrent*/
			torrent->config->SetTorrentState(TORRENT_STATE_START);
		if( torrent->isvalid )
		{ AddTorrent( torrent ); }

		cont = torrents_dir.GetNext( &filename );
	}

	filename = wxGetApp().SaveTorrentsPath() + APPBINNAME + _T( ".magneturi" );
	wxTextFile magneturifile( filename );
	if( magneturifile.Exists() )
	{
		magneturifile.Open( wxConvFile );

		if( magneturifile.IsOpened() )
		{
			wxString url;
			for ( url = magneturifile.GetFirstLine(); !magneturifile.Eof(); url = magneturifile.GetNextLine() )
			{
				if(!url.IsEmpty())
				{
					MagnetUri magnetUri( url );

					if( magnetUri.isValid() )
						torrent = LoadMagnetUri( magnetUri );
				}
			}
		}
	}

	{
		wxMutexLocker ml( m_torrent_queue_lock );
		std::sort( m_torrent_queue.begin(), m_torrent_queue.end(),
				   BTQueueSortAsc() );
		m_running_torrent_map.clear();

		for( int i = 0; i < m_torrent_queue.size(); ++i )
		{
			m_running_torrent_map[m_torrent_queue.at( i )->hash] = i;
		}
	}
}

size_t BitTorrentSession::GetTorrentQueueSize()
{
	wxMutexLocker ml( m_torrent_queue_lock );
	return m_torrent_queue.size();
}

//find and return index of torrent handle from hash string
int BitTorrentSession::find_torrent_from_hash( const wxString& hash )
{
	int j = -1;
	wxMutexLocker ml( m_torrent_queue_lock );
	torrents_map::const_iterator it = m_running_torrent_map.find( hash );

	if( it != m_running_torrent_map.end() ) { j = it->second; }

	return j;
}

char const* timestamp()
{
	time_t t = std::time( 0 );
	tm* timeinfo = std::localtime( &t );
	static char str[200];
	std::strftime( str, 200, "%b %d %X", timeinfo );
	return str;
}
//save torrent fast resume data
//posibly called in interval fashion
void BitTorrentSession::SaveAllTorrent()
{
	WXLOGDEBUG(( _T( "Saving torrent info\n" ) ));
	// keep track of the number of resume data
	// alerts to wait for
	int num_paused = 0;
	int num_failed = 0;
	m_libbtsession->pause();
	/* take this opportunity to sync torrent index back to zero */
	int idx = 0;
	int num_outstanding_resume_data = 0;
	wxArrayString magneturi;

	{
		wxMutexLocker ml( m_torrent_queue_lock );
		for( torrents_t::iterator i = m_torrent_queue.begin();
				i != m_torrent_queue.end();
				++i )
		{
			shared_ptr<torrent_t> torrent = *i;
			torrent->config->SetQIndex( idx++ );
			torrent->config->Save();
			

			if( !torrent->handle.is_valid() || \
					!torrent->handle.status().has_metadata || \
					( !( ( torrent->handle.status() ).need_save_resume ) ) || \
					torrent->config->GetTorrentState() == TORRENT_STATE_STOP )
			{
				if((!torrent->magneturi.IsEmpty()) && torrent->handle.is_valid()
					&& !torrent->handle.status().has_metadata)
				{
					magneturi.Add(torrent->magneturi);
				}

				continue;
			}

			torrent->handle.pause();
			SaveTorrentResumeData( torrent );
			++num_outstanding_resume_data;
			m_libbtsession->remove_torrent( torrent->handle );
		}
	}

	while( num_outstanding_resume_data > 0 )
	{
		lt::alert const* a = m_libbtsession->wait_for_alert(lt::seconds( 10 ) );

		if( a == 0 ) { continue; }

		std::vector<lt::alert*> alerts;
		m_libbtsession->pop_alerts( &alerts );
		std::string now = timestamp();

		for( std::vector<lt::alert*>::iterator i = alerts.begin(),
			end( alerts.end() ); i != end; ++i )
		{
			// make sure to delete each alert
			lt::torrent_paused_alert const* tp = lt::alert_cast<lt::torrent_paused_alert>( *i );

			if( tp )
			{
				++num_paused;
				WXLOGDEBUG(( _T( "\nleft: %d failed: %d pause: %d\n" )
							, num_outstanding_resume_data, num_failed, num_paused ));
				continue;
			}

			if(lt::alert_cast<lt::save_resume_data_failed_alert>( *i ) )
			{
				++num_failed;
				--num_outstanding_resume_data;
				WXLOGDEBUG(( _T( "\nleft: %d failed: %d pause: %d\n" )
							, num_outstanding_resume_data, num_failed, num_paused ));
				continue;
			}

			lt::save_resume_data_alert const* rd = lt::alert_cast<lt::save_resume_data_alert>( *i );

			if( rd == 0 ) { continue; }

			--num_outstanding_resume_data;
			WXLOGDEBUG(( _T( "\nleft: %d failed: %d pause: %d\n" )
						, num_outstanding_resume_data, num_failed, num_paused ));

			if( !rd->resume_data ) { continue; }

			lt::torrent_handle h = rd->handle;
			lt::torrent_status st = h.status( lt::torrent_handle::query_save_path );
			//std::vector<char> out;
			//bencode(std::back_inserter(out), *rd->resume_data);
			wxString resumefile = wxGetApp().SaveTorrentsPath() + lt::to_hex( st.info_hash.to_string() ) + _T( ".resume" );
			std::ofstream out( ( const char* )resumefile.mb_str( wxConvFile ), std::ios_base::binary );
			out.unsetf( std::ios_base::skipws );
			bencode( std::ostream_iterator<char>( out ), *rd->resume_data );
		}
	}

	// save dead magnet uri
	size_t counts = magneturi.GetCount();
	wxString filename = wxGetApp().SaveTorrentsPath() + APPBINNAME + _T( ".magneturi" );
	wxTextFile magneturifile( filename );
	if(counts > 0)
	{
		wxString filename = wxGetApp().SaveTorrentsPath() + APPBINNAME + _T( ".magneturi" );
		wxTextFile magneturifile( filename );
		if( !magneturifile.Exists() )
		{
			magneturifile.Create();
			magneturifile.Open();
		}
		else
		{
			magneturifile.Open( wxConvFile );
		}

		if( magneturifile.IsOpened() )
		{
			magneturifile.Clear();
			for( size_t i = 0; i < counts; ++i )
			{
				magneturifile.AddLine( magneturi[i] );
			}
			
#ifdef __WXMAC__
			magneturifile.Write( wxTextFileType_Mac );
#else
			magneturifile.Write( wxTextFileType_Unix );
#endif
			magneturifile.Close();
		}
	}	
	else if( magneturifile.Exists() )
	{
		magneturifile.Open( wxConvFile );

		if( magneturifile.IsOpened() )
		{
			magneturifile.Clear();
			magneturifile.Close();
		}
	}
}

void BitTorrentSession::SaveTorrentResumeData( shared_ptr<torrent_t>& torrent )
{
	//new api, write to disk in save_resume_data_alert
	if (torrent->handle.is_valid())
	{
		lt::torrent_status status = torrent->handle.status();
		if(status.has_metadata)
		{ torrent->handle.save_resume_data(); }
	}
}

void BitTorrentSession::DumpTorrents()
{
	wxMutexLocker ml( m_torrent_queue_lock );
	for( torrents_t::iterator i = m_torrent_queue.begin();
			i != m_torrent_queue.end(); ++i )
	{
		shared_ptr<torrent_t> torrent = *i;
		wxLogMessage( _T( "[%s] %s" ), ( wxLongLong( torrent->config->GetQIndex() ).ToString() ).c_str(), torrent->name.c_str() );
	}
}

/* General function, parse torrent file into torrent_t */
shared_ptr<torrent_t> BitTorrentSession::ParseTorrent( const wxString& filename )
{
	shared_ptr<torrent_t> torrent( new torrent_t() );
	WXLOGDEBUG(( _T( "Parse Torrent: %s\n" ), filename.c_str() ));

	try
	{
		lt::error_code ec;
		shared_ptr<const lt::torrent_info> t(new lt::torrent_info(wxString(filename.mb_str(wxConvUTF8)).ToStdString(), ec));
		if(ec)
			wxLogError( wxString::FromUTF8( ec.message().c_str() ) );
		torrent->info = t;
		torrent->name = wxString( wxConvUTF8.cMB2WC( t->name().c_str() ) );
		torrent->hash = t->info_hash();
		torrent->config.reset( new TorrentConfig( wxString(torrent->hash) ) );

		if( torrent->config->GetTrackersURL().size() <= 0 )
		{
			std::vector<lt::announce_entry> trackers = t->trackers();
			torrent->config->SetTrackersURL( trackers );
		}

		torrent->isvalid = true;
	}
	catch( std::exception &e )
	{
		wxLogError( wxString::FromUTF8( e.what() ) + _T("\n") );
	}

	return torrent;
}

shared_ptr<torrent_t> BitTorrentSession::LoadMagnetUri( MagnetUri& magneturi )
{
	bool notfound = false;
	shared_ptr<torrent_t> torrent;
	{
		wxMutexLocker ml( m_torrent_queue_lock );
		notfound = (m_queue_torrent_set.find(wxString(magneturi.hash())) == m_queue_torrent_set.end());
	}
	if(notfound)
	{
		try
		{
			torrent.reset(new torrent_t());
			lt::add_torrent_params p = magneturi.addTorrentParams();
			// Limits
			//p.max_connections = maxConnectionsPerTorrent();
			//p.max_uploads = maxUploadsPerTorrent();
			//shared_ptr<lt::torrent_info> t(new lt::torrent_info( sha1_hash( magneturi.hash() ) ));
			//torrent->info = t;
			torrent->name = magneturi.name();
			torrent->hash = magneturi.hash();
			torrent->config.reset( new TorrentConfig( wxString(torrent->hash) ) );
			p.save_path = ( const char* )torrent->config->GetDownloadPath().mb_str( wxConvUTF8 );

			p.flags |= lt::add_torrent_params::flag_paused; // Start in pause
			p.flags &= ~lt::add_torrent_params::flag_auto_managed; // Because it is added in paused state
			p.flags &= ~lt::add_torrent_params::flag_duplicate_is_error; // Already checked
			// Solution to avoid accidental file writes
			p.flags |= lt::add_torrent_params::flag_upload_mode;

			// Adding torrent to BitTorrent session
			torrent->config->SetTorrentState( TORRENT_STATE_START );
			torrent->magneturi = magneturi.url();


			torrent->isvalid = false;
			m_libbtsession->async_add_torrent( p );
			wxMutexLocker ml( m_torrent_queue_lock );
			m_queue_torrent_set.insert(wxString(torrent->hash));
			m_torrent_queue.push_back( torrent );
			m_running_torrent_map.insert( std::pair<wxString, int>( wxString(torrent->hash), (int)(m_torrent_queue.size() - 1 )) );
		}
		catch( std::exception &e )
		{
			wxLogError( wxString::FromUTF8( e.what() ) + _T("\n") );
		}
	}

	return torrent;
}

bool BitTorrentSession::SaveTorrent( shared_ptr<torrent_t>& torrent, const wxString& filename )
{
	lt::create_torrent ct_torrent( *( torrent->info ) );

	try
	{
		std::ofstream out( ( const char* )filename.mb_str( wxConvFile ), std::ios_base::binary );
		out.unsetf( std::ios_base::skipws );
		bencode( std::ostream_iterator<char>( out ), ct_torrent.generate() );
	}
	catch( std::exception &e )
	{
		wxLogError( wxString::FromUTF8( e.what() ) + _T("\n") );
		return false;
	}

	return true;
}

void BitTorrentSession::StartTorrent( shared_ptr<torrent_t>& torrent, bool force )
{
	wxLogInfo( _T( "%s: Start %s\n" ), torrent->name.c_str(), force ? _T( "force" ) : _T( "" ) );
	lt::torrent_handle& handle = torrent->handle;

	//if(m_queue_torrent_set.find(wxString(torrent->hash)) != m_queue_torrent_set.end())
	//{
	//	return;
	//}

	if( !handle.is_valid() || ( ( handle.status(lt::torrent_handle::query_save_path).save_path).empty() ) )
	{
		AddTorrentToSession( torrent );
	}

	//lt::torrent_status st = handle.status();
	//if(st.state ==  )
	//{ handle.resume(); }

	if (handle.is_valid())
	{
		torrent->config->SetTorrentState(force ? TORRENT_STATE_FORCE_START : TORRENT_STATE_START);
		torrent->config->Save();
		ConfigureTorrent(torrent);
		if(handle.status().paused)
			handle.resume();
	}
}

void BitTorrentSession::StopTorrent( shared_ptr<torrent_t>& torrent )
{
	wxLogInfo( _T( "%s:Stop\n" ), torrent->name.c_str() );
	lt::torrent_handle& handle = torrent->handle;
	lt::torrent_handle invalid_handle;

	//Remove torrent from session
	//set property to STOP
	//
	if( handle.is_valid() )
	{
		handle.pause();
		SaveTorrentResumeData( torrent );
		m_libbtsession->remove_torrent( handle );
		torrent->handle = invalid_handle;
	}

	enum torrent_state state = ( enum torrent_state ) torrent->config->GetTorrentState();

	//XXX redundant ? StartTorrent below will call the same thing
	if( ( state == TORRENT_STATE_START ) ||
			( state == TORRENT_STATE_FORCE_START ))
	{
		PostQueueUpdateEvent();
	}

	torrent->config->SetTorrentState( TORRENT_STATE_STOP );
	torrent->config->Save();
}

void BitTorrentSession::QueueTorrent( shared_ptr<torrent_t>& torrent )
{
	//wxLogInfo(_T("%s: Queue"), torrent->name.c_str());
	lt::torrent_handle& handle = torrent->handle;

	if (handle.is_valid())
	{
		handle.pause();
	}
	else
	{
		AddTorrentToSession(torrent);
	}

	torrent->config->SetTorrentState( TORRENT_STATE_QUEUE );
	torrent->config->Save();
}

void BitTorrentSession::PauseTorrent( shared_ptr<torrent_t>& torrent )
{
	wxLogInfo( _T( "%s: Pause\n" ), torrent->name.c_str() );
	lt::torrent_handle& handle = torrent->handle;

	if( !handle.is_valid() )
	{
		AddTorrentToSession( torrent );
	}

	//if( !handle.is_paused() )
	handle.pause();

	torrent->config->SetTorrentState( TORRENT_STATE_PAUSE );
	torrent->config->Save();
}

void BitTorrentSession::MoveTorrentUp( shared_ptr<torrent_t>& torrent )
{
	int idx = -1;
	{
		wxMutexLocker ml( m_torrent_queue_lock );
		idx = find_torrent_from_hash( wxString(torrent->hash));
	}
	if( idx <= 0 )
	{
		if( idx == 0 )
		{
			wxLogInfo( _T( "Torrent is at top position %d-%s\n" ), idx, torrent->name.c_str() );
		}
		else
		{
			wxLogError( _T( "MoveTorrentUp %s: Torrent not found in queue\n" ), torrent->name.c_str() );
		}
		return ;
	}

	wxLogInfo( _T( "%s: Moving Torrent Up\n" ), torrent->name.c_str() );

	long qindex = torrent->config->GetQIndex();
	if(qindex < 0)
	{
		wxLogInfo( _T( "Torrent is not in download que %d-%s\n" ), idx, torrent->name.c_str() );
		return;
	}

	wxMutexLocker ml( m_torrent_queue_lock );
	torrents_t::iterator torrent_it = m_torrent_queue.begin() + idx - 1 ;
	shared_ptr<torrent_t> prev_torrent( *( torrent_it ) );
	WXLOGDEBUG(( _T( "Prev %d now %d\n" ), prev_torrent->config->GetQIndex(), torrent->config->GetQIndex() ));
	long prev_qindex = prev_torrent->config->GetQIndex();
	torrent->config->SetQIndex( prev_qindex );
	torrent->config->Save();
	if(torrent->handle.is_valid())
	{
		torrent->handle.queue_position_up();
	}
	prev_torrent->config->SetQIndex( qindex );
	prev_torrent->config->Save();
	m_torrent_queue[idx - 1] = torrent;
	m_torrent_queue[idx] = prev_torrent;
	m_running_torrent_map[wxString(torrent->hash)] = idx - 1;
	m_running_torrent_map[wxString(prev_torrent->hash)] = idx;
//m_torrent_queue.erase( torrent_it );
//torrent_it = m_torrent_queue.begin() + idx;
//m_torrent_queue.insert( torrent_it, prev_torrent );
}

void BitTorrentSession::MoveTorrentDown( shared_ptr<torrent_t>& torrent )
{
	int idx = -1;
	{
		wxMutexLocker ml( m_torrent_queue_lock );
		idx = find_torrent_from_hash( wxString(torrent->hash));
	}

	if( idx < 0 )
	{
		wxLogError( _T( "MoveTorrentDown %s: Torrent not found in queue\n" ), torrent->name.c_str() );
		return ;
	}

	/* we are going to delete it
	 * deference from queue
	 */
	//torrent_t *torrent = torrent;

	if( idx == ( m_torrent_queue.size() - 1 ) )
	{
		wxLogInfo( _T( "Torrent is at bottom position %d-%s\n" ), idx, torrent->name.c_str() );
		return;
	}

	long qindex = torrent->config->GetQIndex();
	if(qindex < 0)
	{
		wxLogInfo( _T( "Torrent is not in download que %d-%s\n" ), idx, torrent->name.c_str() );
		return;
	}

	wxLogInfo( _T( "%s: Moving Torrent Down\n" ), torrent->name.c_str() );
	
	wxMutexLocker ml( m_torrent_queue_lock );
	torrents_t::iterator torrent_it = m_torrent_queue.begin() + idx;
	shared_ptr<torrent_t> next_torrent( *( torrent_it + 1 ) );
	WXLOGDEBUG(( _T( "Next %d now %d\n" ), next_torrent->config->GetQIndex(), torrent->config->GetQIndex() ));
	long next_qindex = next_torrent->config->GetQIndex();
	
	if( next_qindex < 0 )
	{
		wxLogInfo( _T( "Torrent is at queue end %d-%s\n" ), idx, torrent->name.c_str() );
		return;
	}
	torrent->config->SetQIndex( next_qindex );
	torrent->config->Save();

	if(torrent->handle.is_valid())
	{
		torrent->handle.queue_position_down();
	}

	next_torrent->config->SetQIndex( qindex );
	next_torrent->config->Save();
	m_torrent_queue[idx] = next_torrent;
	m_torrent_queue[idx + 1] = torrent;
	m_running_torrent_map[wxString(torrent->hash)] = idx + 1;
	m_running_torrent_map[wxString(next_torrent->hash)] = idx;
//m_torrent_queue.erase( torrent_it );
//torrent_it = m_torrent_queue.begin() + idx + 1 ;
//m_torrent_queue.insert( torrent_it, torrent );
}

void BitTorrentSession::ReannounceTorrent( shared_ptr<torrent_t>& torrent )
{
	wxLogInfo( _T( "%s: Reannounce\n" ), torrent->name.c_str() );
	lt::torrent_handle &h = torrent->handle;

	if( h.is_valid() )
	{
		h.force_reannounce();
	}
}

void BitTorrentSession::RecheckTorrent( shared_ptr<torrent_t>& torrent )
{
	if(!torrent || !torrent->isvalid)
	{
		return;
	}
	enum torrent_state prev_state = ( enum torrent_state ) torrent->config->GetTorrentState();
	
	if( ( prev_state == TORRENT_STATE_START ) ||
			( prev_state == TORRENT_STATE_FORCE_START ) ||
			( prev_state == TORRENT_STATE_PAUSE ) )
	{
		wxLogInfo( _T( "%s: Stop for checking\n" ), torrent->name.c_str() );
		StopTorrent( torrent );
	}
	
	wxString fastresumefile = wxGetApp().SaveTorrentsPath() + wxString(torrent->hash) + _T( ".fastresume" );
	
	if( wxFileExists( fastresumefile ) )
	{
		wxLogWarning( _T( "%s: Remove fast resume data\n" ), torrent->name.c_str() );
	
		if( ! wxRemoveFile( fastresumefile ) )
		{
			wxLogError( _T( "%s: Remove fast resume data: failed\n" ), torrent->name.c_str() );
		}
	}
	
	if( ( prev_state == TORRENT_STATE_START ) ||
			( prev_state == TORRENT_STATE_FORCE_START ) )
	{
		wxLogInfo( _T( "%s: Stop for checking\n" ), torrent->name.c_str() );
		StartTorrent( torrent, ( prev_state == TORRENT_STATE_FORCE_START ) );
	}
	else
		if( prev_state == TORRENT_STATE_PAUSE )
		{
			PauseTorrent( torrent );
		}

}

void BitTorrentSession::ConfigureTorrentFilesPriority( shared_ptr<torrent_t>& torrent )
{
	bool nopriority = false;
	std::vector<int> filespriority = torrent->config->GetFilesPriorities();
	//XXX default priority is 4...
	// win32 4 triggers a assert
	std::vector<int> deffilespriority( torrent->info->num_files(), BITTORRENT_FILE_NORMAL );

	wxASSERT(torrent->info);
	if (filespriority.size() != torrent->info->num_files())
	{
		nopriority = true;
	}

	if( torrent->handle.is_valid() )
	{ torrent->handle.prioritize_files( nopriority ? deffilespriority : filespriority ); }
}

void BitTorrentSession::ConfigureTorrentTrackers( shared_ptr<torrent_t>& torrent )
{
	lt::torrent_handle& t_handle = torrent->handle;
	if( t_handle.is_valid() )
	{ t_handle.replace_trackers( torrent->config->GetTrackersURL() ); }
}

void BitTorrentSession::ConfigureTorrent( shared_ptr<torrent_t>& torrent )
{
	lt::torrent_handle &h = torrent->handle;
	WXLOGDEBUG(( _T( "%s: Configure\n" ), torrent->name.c_str() ));

	if(torrent->info)
	{
		wxASSERT(h.is_valid());
		std::string existdir = h.status(lt::torrent_handle::query_save_path).save_path;
		wxFileName oldpath, newpath;
		oldpath.AssignDir(wxString::FromUTF8( existdir.c_str() ) +
						   wxString( wxConvUTF8.cMB2WC( torrent->info->name().c_str() ) ));
		newpath.AssignDir(torrent->config->GetDownloadPath() + wxString( wxConvUTF8.cMB2WC( torrent->info->name().c_str() ) ));
		newpath.MakeAbsolute();
		oldpath.MakeAbsolute();

		if( oldpath.GetPath() != newpath.GetPath() )
		{
			bool copysuccess = true;
			/* move_storage already handled copying files */
			/* 2007-12-05 : Not to other drive/partition
			 * stop session, move it wx way restore session */
			enum torrent_state prev_state = ( enum torrent_state ) torrent->config->GetTorrentState();

			if( ( prev_state == TORRENT_STATE_START ) ||
					( prev_state == TORRENT_STATE_FORCE_START ) )
			{
				StopTorrent( torrent );
			}

			WXLOGDEBUG(( _T( "Old path %s, new path %s\n" ), oldpath.GetPath().c_str(), newpath.GetPath().c_str() ));

			if(oldpath.DirExists())
			{
				copysuccess = ( CopyDirectory( oldpath.GetPath( wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR ), newpath.GetPath( wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR ) ) != 0 );

				if( copysuccess )
				{
					oldpath.Rmdir();
				}
			}
			else if( oldpath.FileExists () )
			{
				copysuccess = wxCopyFile( oldpath.GetFullPath(), newpath.GetFullPath() );

				if( copysuccess )
				{
					wxRemoveFile( oldpath.GetFullPath() );
				}
			}
			else
			{
				wxLogWarning( _T( "Old path not exists %s\n" ), oldpath.GetFullPath().c_str() );
				copysuccess = false;
			}

			if( copysuccess )
			{
				if( ( prev_state == TORRENT_STATE_START ) ||
						( prev_state == TORRENT_STATE_FORCE_START ) )
				{
					lt::torrent_handle& h = torrent->handle;
					
					if( !h.is_valid() || ( ( h.status(lt::torrent_handle::query_save_path).save_path).empty() ) )
					{
						AddTorrentToSession( torrent );
					}
					/*if( h.is_valid() )
					{
						torrent->config->SetTorrentState(force ? TORRENT_STATE_FORCE_START : TORRENT_STATE_START);
						torrent->config->Save();
						h = torrent->handle;
						h.resume();
					}*/
				}
			}
			else //failed copy, restore path
			{
				wxLogError( _T( "Failed moving save path %s\n" ), torrent->config->GetDownloadPath().c_str() );
				wxLogWarning( _T( "Restoring download path %s\n" ), oldpath.GetFullPath().c_str() );
				
				wxFileName dpath;
				dpath.AssignDir(wxString::FromUTF8( existdir.c_str() ));
				torrent->config->SetDownloadPath( dpath.GetPathWithSep() );
				torrent->config->Save();
				if(oldpath.DirExists() && ( ( prev_state == TORRENT_STATE_START ) ||
						( prev_state == TORRENT_STATE_FORCE_START ) ))
				{
					if( !h.is_valid() || ( ( h.status(lt::torrent_handle::query_save_path).save_path).empty() ) )
					{
						AddTorrentToSession( torrent );
					}
				}
			}
		}
	}

	if( h.is_valid() )
	{
		h.set_upload_limit( torrent->config->GetTorrentUploadLimit() );
		h.set_download_limit( torrent->config->GetTorrentDownloadLimit() );
		h.set_max_uploads( torrent->config->GetTorrentMaxUploads() );
		h.set_max_connections( torrent->config->GetTorrentMaxConnections() );
		//h.set_ratio( torrent->config->GetTorrentRatio() );
		WXLOGDEBUG(( _T( "%s: Upload Limit %s download limit %s, max uploads %s, max connections %s, ratio %lf\n" ),
					torrent->name.c_str(),
					( wxLongLong( h.upload_limit() ).ToString() ).c_str(),
					( wxLongLong( h.download_limit() ).ToString() ).c_str(),
					( wxLongLong( torrent->config->GetTorrentMaxUploads() ).ToString() ).c_str(),
					( wxLongLong( torrent->config->GetTorrentMaxConnections() ).ToString() ).c_str(),
					( float )torrent->config->GetTorrentRatio() / 100 ));
		if(torrent->info)
		{
			ConfigureTorrentFilesPriority( torrent );
			ConfigureTorrentTrackers( torrent );
		}
	}
}

void BitTorrentSession::RemoveTorrent( int idx, bool deletedata )
{
	shared_ptr<torrent_t> torrent = GetTorrent( idx );
	if(torrent)
		RemoveTorrent( torrent, deletedata);
}

void BitTorrentSession::StartTorrent( int idx, bool force )
{
	shared_ptr<torrent_t> torrent = GetTorrent( idx );
	if(torrent && torrent->isvalid)
	{
		StartTorrent( torrent, force );
	}
}

void BitTorrentSession::StopTorrent( int idx )
{
	shared_ptr<torrent_t> torrent = GetTorrent( idx );
	if(torrent)
		StopTorrent( torrent );
}

void BitTorrentSession::QueueTorrent( int idx )
{
	shared_ptr<torrent_t> torrent = GetTorrent( idx );
	if(torrent && torrent->isvalid)
	{
		QueueTorrent( torrent );
	}
}

void BitTorrentSession::PauseTorrent( int idx )
{
	shared_ptr<torrent_t> torrent = GetTorrent( idx );
	if(torrent)
		PauseTorrent( torrent );
}

void BitTorrentSession::MoveTorrentUp( int idx )
{
	if( idx == 0 )
	{
		wxLogInfo( _T( "Torrent is at top position %d\n" ), idx );
		return;
	}
	shared_ptr<torrent_t> torrent = GetTorrent( idx );
	if(torrent)
		MoveTorrentUp( torrent );
}

void BitTorrentSession::MoveTorrentDown( int idx )
{
	{
		wxMutexLocker ml( m_torrent_queue_lock );
		bool CanNotMoveDown = (idx >= ( m_torrent_queue.size() - 1 ));
		if(CanNotMoveDown)
		{
			wxLogInfo( _T( "Torrent is at bottom position %d\n" ), idx );
			return;
		}
	}

	shared_ptr<torrent_t> torrent = GetTorrent( idx );
	if(torrent)
		MoveTorrentDown( torrent );
}

void BitTorrentSession::ReannounceTorrent( int idx )
{
	shared_ptr<torrent_t> torrent = GetTorrent( idx );
	if(torrent)
		ReannounceTorrent( torrent );
}

void BitTorrentSession::RecheckTorrent( int idx )
{
	if( idx < 0 || idx >= m_torrent_queue.size())
	{
		wxLogError( _T( "%d: Invalid torrent index\n" ), idx );
		return ;
	}

	RecheckTorrent( m_torrent_queue[idx] );
}

void BitTorrentSession::ConfigureTorrentFilesPriority( int idx )
{
	shared_ptr<torrent_t> torrent = GetTorrent( idx );
	if(torrent)
		ConfigureTorrentFilesPriority( torrent );
}

void BitTorrentSession::ConfigureTorrentTrackers( int idx )
{
	shared_ptr<torrent_t> torrent = GetTorrent( idx );
	if(torrent)
		ConfigureTorrentTrackers( torrent );
}

void BitTorrentSession::ConfigureTorrent( int idx )
{
	shared_ptr<torrent_t> torrent = GetTorrent( idx );
	if(torrent)
		ConfigureTorrent( torrent );
}

/* start or stop queued item based on several criteria
 * start:
 *  max number of torrent is  less than cfg->maxstart
 * XXX more criteria?
 *    start next torrent when download speed is less than x
 *
 * stop:
 *  share ratio is more than cfg->ratio
 * XXX more criteria?
 *    num of seed is less than x
 *    time elapsed is more than x
 */
void BitTorrentSession::CheckQueueItem()
{
	std::vector<int> start_torrents, queue_torrents;
	int idx = 0;
	
	int pause_count = 0;
	int start_count = 0;
	int i = 0;
	int maxstart = m_config->GetMaxStart();
	//WXLOGDEBUG((_T("CheckQueueItem %d\n"), m_torrent_queue.size());
	wxMutexLocker ml( m_torrent_queue_lock );

	for( idx = 0; idx < m_torrent_queue.size(); ++idx )
	{
		shared_ptr<torrent_t>& torrent = m_torrent_queue[idx];
		WXLOGDEBUG(( _T( "Processing torrent %s\n" ), torrent->name.c_str() ));
 
		switch( torrent->config->GetTorrentState() )
		{
		case TORRENT_STATE_START:
			{
				bool exseed = false;

				if( torrent->handle.is_valid() )
				{
					lt::torrent_status t_status = torrent->handle.status();

					if( ( t_status.state == lt::torrent_status::seeding ) ||
							( t_status.state == lt::torrent_status::finished ) ||
							( ( t_status.progress >= 1 ) &&
							  ( t_status.state != lt::torrent_status::checking_files ) ) )
					{
						//manually check ratio to stop torrent
						//XXX sometime it fails, have a look
						float seedratio = 0.0 ;

						if( t_status.total_payload_download > 0 )
						{ seedratio = ( t_status.total_payload_upload / t_status.total_payload_download ) * 100; }

						if( seedratio >= torrent->config->GetTorrentRatio() )
						{
							wxLogInfo( _T( "%s torrent finished\n" ), torrent->name.c_str() );
							StopTorrent( torrent );
							continue;
						}

						/* exclude seed from count as running job */
						if( m_config->GetExcludeSeed() )
						{ exseed = true; }
					}

#if 0
					//periodic save fastresume data
					//
					wxString fastresumefile = wxGetApp().SaveTorrentsPath() + torrent->hash + _T( ".fastresume" );
					time_t modtime = wxFileModificationTime( fastresumefile );

					if( modtime != -1 )
					{
						time_t now = wxDateTime::GetTimeNow();
						int timediff = now - modtime;

						if( timediff > m_config->GetFastResumeSaveTime() )
						{
							SaveTorrentResumeData( torrent );
						}
					}
					else
					{
						SaveTorrentResumeData( torrent );
					}
#endif
				}

				/* make this an option */
				if( !exseed )
				{
					start_torrents.push_back( idx );
					start_count++;
				}

				break;
			}

		case TORRENT_STATE_FORCE_START:
			{
#if 0
				//periodic save fastresume data
				//
				wxString fastresumefile = wxGetApp().SaveTorrentsPath() + torrent->hash + _T( ".fastresume" );
				time_t modtime = wxFileModificationTime( fastresumefile );

				if( modtime != -1 )
				{
					time_t now = wxDateTime::GetTimeNow();
					int timediff = now - modtime;

					if( timediff > m_config->GetFastResumeSaveTime() )
					{
						SaveTorrentResumeData( torrent );
					}
				}
				else
				{
					SaveTorrentResumeData( torrent );
				}
#endif
				if (!((m_torrent_queue[idx])->handle.is_valid()))
					StartTorrent((m_torrent_queue[idx]), true);

				break;
			}

		case TORRENT_STATE_QUEUE:
			{
				queue_torrents.push_back( idx );
				pause_count++;
				break;
			}

		default:
			//do nothing
			break;
		}
	}

	int check_count = start_count;
	if( check_count > maxstart ) check_count = maxstart;
	i = 0;
	while( i < check_count )
	{
		shared_ptr<torrent_t>& torrent = m_torrent_queue[start_torrents[i]];
		if(!torrent->handle.is_valid())
		{
			StartTorrent( torrent, false );
		}
		++i;
	}

	if( start_count > maxstart )
	{
		i = start_torrents.size() - 1;

		while( ( start_count-- > maxstart ) && ( i >= 0 ) )
		{
			QueueTorrent( m_torrent_queue[start_torrents[i]] );
			--i;
		}
	}
	else if( ( start_count < maxstart ) && ( queue_torrents.size() > 0 ) )
	{
		i = 0;

		while( ( start_count++ < maxstart ) && ( i <  queue_torrents.size() ) )
		{
			StartTorrent( m_torrent_queue[queue_torrents[i]], false );
			i++;
		}
	}
}

void BitTorrentSession::HandleTorrentAlert()
{
	wxString event_string;
	std::vector<lt::alert*> alerts;
	/*1--debug, 2--info, 3--warning, 4--error*/
	unsigned int log_severity = 2;

	m_libbtsession->pop_alerts(&alerts);
	for (std::vector<lt::alert*>::iterator it = alerts.begin()
		, end(alerts.end()); it != end; ++it)
	{
		TORRENT_TRY
		{
			event_string.Empty();
			switch ((*it)->type())
			{
				case lt::add_torrent_alert::alert_type:
					if( lt::add_torrent_alert* p = dynamic_cast<lt::add_torrent_alert*>( *it ) )
					{
						if (p->error)
						{
							log_severity = 4;
							event_string << _T("Couldn't add torrent: ") <<  wxString::FromUTF8(p->message().c_str());
						}
						else
						{
							HandleAddTorrentAlert(p);
							event_string << _T("Metadata: ") <<  wxString::FromUTF8(p->message().c_str());
						}
					}
					break;
				case lt::torrent_finished_alert::alert_type:
					if( lt::torrent_finished_alert* p = dynamic_cast<lt::torrent_finished_alert*>( *it ) )
					{
						lt::torrent_status st = (p->handle).status(lt::torrent_handle::query_name);
						event_string << st.name << _T(": ") << wxString::FromUTF8(p->message().c_str());
					}
					break;
				case lt::peer_ban_alert::alert_type:
					if( lt::peer_ban_alert* p = dynamic_cast<lt::peer_ban_alert*>( *it ) )
					{
						event_string << _T( "Peer: (" ) << p->ip.address().to_string() << _T( ")" ) << wxString::FromUTF8(p->message().c_str());
					}
					break;
				case lt::peer_error_alert::alert_type:
					if( lt::peer_error_alert* p = dynamic_cast<lt::peer_error_alert*>( *it ) )
					{
						log_severity = 4;
						event_string << _T( "Peer: " ) << identify_client( p->pid ) << _T( ": " ) << wxString::FromUTF8(p->message().c_str());
					}
					break;
				case lt::peer_blocked_alert::alert_type:
					if( lt::peer_blocked_alert* p = dynamic_cast<lt::peer_blocked_alert*>( *it ) )
					{
						log_severity = 3;
						event_string << _T( "Peer: (" ) << p->ip.to_string() << _T( ")" ) << wxString::FromUTF8(p->message().c_str());
					}
					break;
				case lt::invalid_request_alert::alert_type:
					if( lt::invalid_request_alert* p = dynamic_cast<lt::invalid_request_alert*>( *it ) )
					{
						log_severity = 4;
						event_string << identify_client( p->pid ) << _T( ": " ) << wxString::FromUTF8(p->message().c_str());
					}
					break;
				case lt::tracker_warning_alert::alert_type:
					if( lt::tracker_warning_alert* p = dynamic_cast<lt::tracker_warning_alert*>( *it ) )
					{
						log_severity = 3;
						event_string << _T( "Tracker: " ) << wxString::FromUTF8(p->message().c_str());
					}
					break;
				case lt::tracker_reply_alert::alert_type:
					if( lt::tracker_reply_alert* p = dynamic_cast<lt::tracker_reply_alert*>( *it ) )
					{
						event_string << _T( "Tracker:" ) << wxString::FromUTF8(p->message().c_str()) << p->num_peers << _T( " number of peers " );
					}
					break;
				case lt::url_seed_alert::alert_type:
					if( lt::url_seed_alert* p = dynamic_cast<lt::url_seed_alert*>( *it ) )
					{
						event_string << _T( "Url seed '" ) << p->server_url() << _T( "': " ) << wxString::FromUTF8(p->message().c_str());
					}
					break;
				case lt::listen_failed_alert::alert_type:
					if( lt::listen_failed_alert* p = dynamic_cast<lt::listen_failed_alert*>( *it ) )
					{
						log_severity = 4;
						event_string << _T( "Listen failed: " ) << wxString::FromUTF8(p->message().c_str());
					}
					break;
				case lt::portmap_error_alert::alert_type:
					if( lt::portmap_error_alert* p = dynamic_cast<lt::portmap_error_alert*>( *it ) )
					{
						log_severity = 4;
						event_string << _T( "Portmap error: " ) << wxString::FromUTF8(p->message().c_str());
					}
					break;
				case lt::portmap_alert::alert_type:
					if( lt::portmap_alert* p = dynamic_cast<lt::portmap_alert*>( *it ) )
					{
						event_string << _T( "Portmap: " ) << wxString::FromUTF8(p->message().c_str());
					}
					break;
				case lt::tracker_announce_alert::alert_type:
					if( lt::tracker_announce_alert* p = dynamic_cast<lt::tracker_announce_alert*>( *it ) )
					{
						event_string << _T( "Tracker: " ) << wxString::FromUTF8(p->message().c_str());
					}
					break;
				case lt::file_error_alert::alert_type:
					if( lt::file_error_alert* p = dynamic_cast<lt::file_error_alert*>( *it ) )
					{
						log_severity = 4;
						event_string << _T( "File error: " ) << wxString::FromUTF8(p->message().c_str());
					}
					break;
				case lt::hash_failed_alert::alert_type:
					if( lt::hash_failed_alert* p = dynamic_cast<lt::hash_failed_alert*>( *it ) )
					{
						log_severity = 4;
						event_string << _T( "Hash: " ) << wxString::FromUTF8(p->message().c_str());
					}
					break;
				case lt::metadata_failed_alert::alert_type:
					if( lt::metadata_failed_alert* p = dynamic_cast<lt::metadata_failed_alert*>( *it ) )
					{
						log_severity = 4;
						event_string << _T( "Metadata: " ) << wxString::FromUTF8(p->message().c_str());
					}
					break;
				case lt::metadata_received_alert::alert_type:
					if( lt::metadata_received_alert* p = dynamic_cast<lt::metadata_received_alert*>( *it ) )
					{
						event_string << _T( "Metadata: " ) << wxString::FromUTF8(p->message().c_str());
						HandleMetaDataAlert(p);
					}
					break;
				case lt::fastresume_rejected_alert::alert_type:
					if( lt::fastresume_rejected_alert* p = dynamic_cast<lt::fastresume_rejected_alert*>( *it ) )
					{
						log_severity = 3;
						event_string << _T( "Fastresume: " ) << wxString::FromUTF8(p->message().c_str());
					}
					break;
				case lt::save_resume_data_alert::alert_type:
					if( lt::save_resume_data_alert *p = dynamic_cast<lt::save_resume_data_alert*>( *it ) )
					{
						lt::torrent_handle& h = p->handle;

						if( p->resume_data )
						{
							InfoHash thash(h.info_hash());
							//save_resume_data to disk
							torrents_map::iterator it = m_running_torrent_map.find(wxString(thash));
							if(it != m_running_torrent_map.end())
							{
								shared_ptr<torrent_t> torrent = BitTorrentSession::GetTorrent( it->second );
								SaveTorrentResumeData(torrent);
							}
						}
						event_string << h.status(lt::torrent_handle::query_name).name << _T( ": " ) << wxString::FromUTF8(p->message().c_str());
					}
					break;
				case lt::session_stats_alert::alert_type:
					if (lt::session_stats_alert* p = dynamic_cast<lt::session_stats_alert *>(*it))
					{
						UpdateCounters(p->values, sizeof(p->values)/sizeof(p->values[0])
							, lt::duration_cast<lt::microseconds>(p->timestamp().time_since_epoch()).count());
						return;
					}
					break;
				case lt::stats_alert::alert_type:
				case lt::state_update_alert::alert_type:
				case lt::dht_stats_alert::alert_type:
				case lt::piece_finished_alert::alert_type:
				case lt::block_finished_alert::alert_type:
				case lt::block_downloading_alert::alert_type:
				case lt::udp_error_alert::alert_type:
				case lt::log_alert::alert_type:
					log_severity = 1;
				default:
					{
						event_string << _T("[") << (*it)->type() << _T("]") << wxString::FromUTF8((*it)->message().c_str());
						//if(event_string.IsEmpty()) return;
					}
					break;
			}
			//XXX include more alert from latest libtorrent
			//
			event_string << _T( '\0' );

			switch(log_severity)
			{
				case 2:
					{
						wxLogInfo( _T("%s\n"), event_string.c_str() );
					}
					break;
				case 3:
					{
						wxLogWarning( _T("%s\n"), event_string.c_str() );
					}
					break;
				case 4:
					{
						wxLogError( _T("%s\n"), event_string.c_str() );
					}
					break;
				default:
					wxLogDebug( _T("%s\n"), event_string.c_str() );
					break;
			}
		}
		TORRENT_CATCH(std::exception& e)
		{
			wxLogError( _T( "Exception: %s\n" ), e.what() );
		}
	}
}

/*Check error before call it. So, assume no p->error*/
bool BitTorrentSession::HandleAddTorrentAlert(lt::add_torrent_alert *p)
{
	try
	{
		//std::stringstream hash_stream;
		//hash_stream << p->handle.info_hash();
		InfoHash thash(p->handle.info_hash());
		wxString shash(thash);
		shared_ptr<torrent_t> torrent;
		wxMutexLocker ml( m_torrent_queue_lock );
		m_queue_torrent_set.erase(shash);
		int idx = find_torrent_from_hash( shash );

		if(idx >= 0)
		{
			torrent = m_torrent_queue.at(idx);
			//wxString torrent_backup = wxGetApp().SaveTorrentsPath() + torrent->hash + _T( ".torrent" );
			torrent->handle = p->handle;
			wxASSERT(p->handle.is_valid());
			torrent->isvalid = true;
			torrent->handle.auto_managed(false);
			torrent->handle.pause();
			enum torrent_state state = ( enum torrent_state ) torrent->config->GetTorrentState();
			wxLogError( _T( "HandleAddTorrentAlert %s\n" ), torrent->name.c_str() );

			if( torrent->config->GetQIndex() == -1 )
			{
				torrents_t::const_reverse_iterator it = m_torrent_queue.rbegin();

				int qidx = 0, index = m_torrent_queue.size();
				while( it != m_torrent_queue.rend() )
				{
					if((*it)->config->GetQIndex() != -1)
					{
						qidx = (*it)->config->GetQIndex() + 1;
						break;
					}
					++it;
					--index;
				}

				torrent->config->SetQIndex( qidx );

				if(( index != m_torrent_queue.size()) && (index != idx))
				{
					--it;
					m_torrent_queue[index] = torrent;
					m_torrent_queue[idx] = *it;
					m_running_torrent_map[wxString(torrent->hash)] = index;
					m_running_torrent_map[wxString((*it)->hash)] = idx;
				}
			}

			if(torrent->info)
			{
			 	bool nopriority = false;
				wxULongLong_t total_selected = 0;
				lt::torrent_info const& t_info = *(torrent->info);
				lt::file_storage const& allfiles = t_info.files();
				std::vector<int> filespriority = torrent->config->GetFilesPriorities();

				if (filespriority.size() != t_info.num_files())
				{
					nopriority = true;
				}
				/*0--unchecked_xpm*/
				/*1--checked_xpm*/
				/*2--unchecked_dis_xpm*/
				/*3--checked_dis_xpm*/

				for(int i = 0; i < t_info.num_files(); ++i)
				{
					if (nopriority || filespriority[i] != BITTORRENT_FILE_NONE)
					{
						total_selected += allfiles.file_size(i);
					}
				}
				torrent->config->SetSelectedSize(total_selected);
			}

			if( state == TORRENT_STATE_FORCE_START || state == TORRENT_STATE_START )
			{
				StartTorrent( torrent, (state == TORRENT_STATE_FORCE_START) );
			}
		}
		else
		{
			if( p->handle.is_valid() )
			{
				p->handle.pause();
				m_libbtsession->remove_torrent( p->handle );
			}
		}
	}
	catch( std::exception& e )
	{
		wxLogError( wxString::FromUTF8( e.what() ) + _T("\n") );
		return false;
	}

	return true;
}

bool BitTorrentSession::HandleMetaDataAlert(lt::metadata_received_alert *p)
{
	InfoHash thash(p->handle.info_hash());
	
	wxMutexLocker ml( m_torrent_queue_lock );
	torrents_map::iterator it = m_running_torrent_map.find(wxString(thash));
	if(it != m_running_torrent_map.end())
	{
		shared_ptr<torrent_t> torrent = GetTorrent(it->second);
		wxString torrent_backup = wxGetApp().SaveTorrentsPath() + wxString(torrent->hash) + _T( ".torrent" );
		torrent->handle = p->handle;
		torrent->info = p->handle.torrent_file();
		lt::torrent_status st = p->handle.status(lt::torrent_handle::query_name);
		torrent->name = st.name;
		SaveTorrent(torrent, torrent_backup );
		
		if(torrent->config->GetTrackersURL().size() <= 0 )
		{
			std::vector<lt::announce_entry> trackers = p->handle.trackers();
			torrent->config->SetTrackersURL( trackers );
		}
		StartTorrent(torrent, false);
		return true;
	}

	return false;
}

void BitTorrentSession::PostStatusUpdate()
{
	m_libbtsession->post_torrent_updates();
	m_libbtsession->post_session_stats();
	m_libbtsession->post_dht_stats();
}

void BitTorrentSession::UpdateCounters(boost::uint64_t* stats_counters
	, int num_cnt, boost::uint64_t t)
{
	// only update the previous counters if there's been enough
	// time since it was last updated
	if (t - m_timestamp[1] > 2000000)
	{
		m_cnt[1].swap(m_cnt[0]);
		m_timestamp[1] = m_timestamp[0];
	}

	m_cnt[0].assign(stats_counters, stats_counters + num_cnt);
	m_timestamp[0] = t;
	//render();
	
	float seconds = (m_timestamp[0] - m_timestamp[1]) / 1000000.f;

	if(seconds > 0.0)
	{
		m_download_rate = (m_cnt[0][m_recv_payload_idx] - m_cnt[1][m_recv_payload_idx])
			/ seconds;
		m_upload_rate = (m_cnt[0][m_sent_payload_idx] - m_cnt[1][m_sent_payload_idx])
			/ seconds;
	}
}

