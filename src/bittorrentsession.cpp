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

#include <cmath> 
#include <vector>
#include <algorithm> // for std::max
#include <fstream>
#include <wx/wx.h>
#include <wx/app.h>
#include <wx/dir.h>
#include <wx/log.h>
#include <wx/filesys.h>

#include <boost/bind.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <libtorrent/config.hpp>
#include <libtorrent/create_torrent.hpp>
//#include <libtorrent/session_status.hpp>

#include <libtorrent/torrent_info.hpp>
#include <libtorrent/announce_entry.hpp>
#include <libtorrent/entry.hpp>
#include <libtorrent/bencode.hpp>
#include <libtorrent/session.hpp>
#include <libtorrent/identify_client.hpp>
#include <libtorrent/alert_types.hpp>
#include <libtorrent/ip_filter.hpp>
#include <libtorrent/magnet_uri.hpp>
#include <libtorrent/peer_info.hpp>
#include <libtorrent/bdecode.hpp>
#include <libtorrent/add_torrent_params.hpp>
#include <libtorrent/time.hpp>
#include <libtorrent/read_resume_data.hpp>
#include <libtorrent/write_resume_data.hpp>
#include <libtorrent/disk_interface.hpp> // for open_file_state
#include "libtorrent/session_params.hpp"

//plugins
#include <libtorrent/extensions/ut_metadata.hpp>
#include <libtorrent/extensions/ut_pex.hpp>
#include <libtorrent/extensions/smart_ban.hpp>
#include <libtorrent/extensions/ut_pex.hpp>

#include "functions.h"
#include "magneturi.h"
#include "bitswash.h"
#include "bittorrentsession.h"

#ifdef TORRENT_DISABLE_DHT
	#error you must not disable DHT
#endif

BitwashStrSet BitTorrentSession::m_preview_ext_set;

std::string to_hex(lt::sha1_hash const& s)
{
	std::stringstream ret;
	ret << s;
	return ret.str();
}

bool load_file(std::string const& filename, std::vector<char>& v, int limit = 8000000)
{
	std::fstream f(filename, std::ios_base::in | std::ios_base::binary);
	f.seekg(0, std::ios_base::end);
	auto const s = f.tellg();
	if (s > limit || s < 0) return false;
	f.seekg(0, std::ios_base::beg);
	v.resize(static_cast<std::size_t>(s));
	if (s == std::fstream::pos_type(0)) return !f.fail();
	f.read(v.data(), int(v.size()));
	return !f.fail();
}

int save_file(std::string const& filename, std::vector<char> const& v)
{
	std::fstream f(filename, std::ios_base::trunc | std::ios_base::out | std::ios_base::binary);
	f.write(v.data(), int(v.size()));
	return !f.fail();
}

BitTorrentSession::BitTorrentSession( wxApp* pParent, Configuration* config, wxMutex *mutex/* = 0*/, wxCondition *condition/* = 0*/ )
	: wxThread( wxTHREAD_JOINABLE ), m_upnp_started( false ), m_natpmp_started( false ), m_lsd_started( false ), m_libbtsession( nullptr ), m_dht_changed(true),
	m_pParent(pParent), m_config(config), m_condition(condition), m_mutex(mutex)
{
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
	for( auto tms: m_timestamp )
		tms = 0;

	if(m_preview_ext_set.empty())
	{
        m_preview_ext_set.insert("3GP");
        m_preview_ext_set.insert("AAC");
        m_preview_ext_set.insert("AC3");
        m_preview_ext_set.insert("AIF");
        m_preview_ext_set.insert("AIFC");
        m_preview_ext_set.insert("AIFF");
        m_preview_ext_set.insert("ASF");
        m_preview_ext_set.insert("AU");
        m_preview_ext_set.insert("AVI");
        m_preview_ext_set.insert("FLAC");
        m_preview_ext_set.insert("FLV");
        m_preview_ext_set.insert("M3U");
        m_preview_ext_set.insert("M4A");
        m_preview_ext_set.insert("M4P");
        m_preview_ext_set.insert("M4V");
        m_preview_ext_set.insert("MID");
        m_preview_ext_set.insert("MKV");
        m_preview_ext_set.insert("MOV");
        m_preview_ext_set.insert("MP2");
        m_preview_ext_set.insert("MP3");
        m_preview_ext_set.insert("MP4");
        m_preview_ext_set.insert("MPC");
        m_preview_ext_set.insert("MPE");
        m_preview_ext_set.insert("MPEG");
        m_preview_ext_set.insert("MPG");
        m_preview_ext_set.insert("MPP");
        m_preview_ext_set.insert("OGG");
        m_preview_ext_set.insert("OGM");
        m_preview_ext_set.insert("OGV");
        m_preview_ext_set.insert("QT");
        m_preview_ext_set.insert("RA");
        m_preview_ext_set.insert("RAM");
        m_preview_ext_set.insert("RM");
        m_preview_ext_set.insert("RMV");
        m_preview_ext_set.insert("RMVB");
        m_preview_ext_set.insert("SWA");
        m_preview_ext_set.insert("SWF");
        m_preview_ext_set.insert("VOB");
        m_preview_ext_set.insert("WAV");
        m_preview_ext_set.insert("WMA");
        m_preview_ext_set.insert("WMV");
	}
}

BitTorrentSession::~BitTorrentSession()
{
}

static const char PEER_ID[] = "SW";

void *BitTorrentSession::Entry()
{
	WXLOGDEBUG(( _T( "BitTorrentSession Thread lt::entry pid 0x%lx priority %u." ), GetId(), GetPriority() ));
	ConfigureSession();

	// Resume main thread
	if(m_mutex != nullptr && m_condition != nullptr)
	{
		wxMutexLocker lock(*m_mutex);
		m_condition->Broadcast(); // same as Signal() here -- one waiter only
	}

	ScanTorrentsDirectory( wxGetApp().SaveTorrentsPath() );

	bts_event evt = BTS_EVENT_INVALID;
	wxMessageQueueError result = wxMSGQUEUE_NO_ERROR;
	time_t starttime = wxDateTime::GetTimeNow(), now;
	bool check_now = true;
	do
	{
		try
		{
			evt = BTS_EVENT_INVALID;
			result = m_evt_queue.ReceiveTimeout( 1000, evt );
			if(result == wxMSGQUEUE_NO_ERROR)
			{
				switch(evt)
				{
					case BTS_EVENT_ALERT:
					{
						HandleTorrentAlerts();
						break;
					}
					case BTS_EVENT_QUEUEUPDATE:
					{
						check_now = true;
						break;
					}
					default:
						wxASSERT(0);
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
		catch( std::exception& e )
		{
			wxLogFatalError( _T( "[%s]:%s! exiting..." ), __FUNCTION__, e.what() );
			break;
		}
		catch(...)
		{
			wxLogFatalError( _T( "[%s]:FATAL ERROR!! exiting..." ), __FUNCTION__ );
			break;
		}

		WXLOGDEBUG(( _T( "BitTorrentSession Thread msg queue result %d." ), result ));
	}
	while(result != wxMSGQUEUE_MISC_ERROR);

	Exit( 0 );
	return nullptr;
}

typedef std::function<void()> null_notify_func_t;
void BitTorrentSession::OnExit()
{
	//XXX have 1 soon
	//
	//wxCriticalSectionLocker locker(wxGetApp().m_btcritsect);
	//save DHT lt::entry
	WXLOGDEBUG(( _T( "BitTorrent Session exiting" ) ));

#ifndef TORRENT_DISABLE_DHT
	if( m_config->GetDHTEnabled() )
	{
		wxString dhtstatefile = wxGetApp().DHTStatePath();

		try
		{
			std::vector<char> dht_state = write_session_params_buf(m_libbtsession->session_state(lt::session::save_dht_state));
			std::fstream out( ( const char* )dhtstatefile.mb_str( wxConvFile ), std::ios_base::trunc | std::ios_base::out | std::ios_base::binary );
			out.write(dht_state.data(), int(dht_state.size()));
		}
		catch( std::exception& e )
		{
			wxLogFatalError( _T( "Save DHT error: %s" ), e.what() );
		}
		catch(...)
		{
			wxLogFatalError( _T( "Save DHT error!! exiting..." ) );
		}
	}
#endif
	m_libbtsession->pause();
	SaveAllTorrent();
	m_evt_queue.Clear();
	std::vector<lt::alert*> alerts;
	m_libbtsession->set_alert_notify(null_notify_func_t());
	m_libbtsession->pop_alerts( &alerts );
	delete m_libbtsession;
	{
		wxMutexLocker ml( m_torrent_queue_lock );
		m_torrent_queue.clear();
		m_torrent_queue.swap( m_torrent_queue );
	}
}

void BitTorrentSession::configurePeerClasses()
{
    lt::ip_filter f;
    f.add_rule(lt::address_v4::from_string("0.0.0.0")
               , lt::address_v4::from_string("255.255.255.255")
               , 1 << static_cast<std::uint32_t>(lt::session::global_peer_class_id));
#if TORRENT_USE_IPV6
    // IPv6 may not be available on OS and the parsing
    // would result in an exception -> abnormal program termination
    // Affects Windows XP
    try {
        f.add_rule(lt::address_v6::from_string("::0")
                   , lt::address_v6::from_string("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff")
                   , 1 << static_cast<std::uint32_t>(lt::session::global_peer_class_id));
    }
    catch (std::exception &) {}
#endif
    if (!m_config->GetRateLimitPeersOnLan()) {
        // local networks
        f.add_rule(lt::address_v4::from_string("10.0.0.0")
                   , lt::address_v4::from_string("10.255.255.255")
                   , 1 << static_cast<std::uint32_t>(lt::session::local_peer_class_id));
        f.add_rule(lt::address_v4::from_string("172.16.0.0")
                   , lt::address_v4::from_string("172.31.255.255")
                   , 1 << static_cast<std::uint32_t>(lt::session::local_peer_class_id));
        f.add_rule(lt::address_v4::from_string("192.168.0.0")
                   , lt::address_v4::from_string("192.168.255.255")
                   , 1 << static_cast<std::uint32_t>(lt::session::local_peer_class_id));
        // link local
        f.add_rule(lt::address_v4::from_string("169.254.0.0")
                   , lt::address_v4::from_string("169.254.255.255")
                   , 1 << static_cast<std::uint32_t>(lt::session::local_peer_class_id));
        // loopback
        f.add_rule(lt::address_v4::from_string("127.0.0.0")
                   , lt::address_v4::from_string("127.255.255.255")
                   , 1 << static_cast<std::uint32_t>(lt::session::local_peer_class_id));
#if TORRENT_USE_IPV6
        // IPv6 may not be available on OS and the parsing
        // would result in an exception -> abnormal program termination
        // Affects Windows XP
        try {
            // link local
            f.add_rule(lt::address_v6::from_string("fe80::")
                       , lt::address_v6::from_string("febf:ffff:ffff:ffff:ffff:ffff:ffff:ffff")
                       , 1 << static_cast<std::uint32_t>(lt::session::local_peer_class_id));
            // unique local addresses
            f.add_rule(lt::address_v6::from_string("fc00::")
                       , lt::address_v6::from_string("fdff:ffff:ffff:ffff:ffff:ffff:ffff:ffff")
                       , 1 << static_cast<std::uint32_t>(lt::session::local_peer_class_id));
            // loopback
            f.add_rule(lt::address_v6::from_string("::1")
                       , lt::address_v6::from_string("::1")
                       , 1 << static_cast<std::uint32_t>(lt::session::local_peer_class_id));
        }
        catch (std::exception &) {}
#endif
    }
    m_libbtsession->set_peer_class_filter(f);

    lt::peer_class_type_filter peerClassTypeFilter;
    peerClassTypeFilter.add(lt::peer_class_type_filter::tcp_socket, lt::session::tcp_peer_class_id);
    peerClassTypeFilter.add(lt::peer_class_type_filter::ssl_tcp_socket, lt::session::tcp_peer_class_id);
    peerClassTypeFilter.add(lt::peer_class_type_filter::i2p_socket, lt::session::tcp_peer_class_id);
    if (m_config->GetRateLimitUtp()) {
        peerClassTypeFilter.add(lt::peer_class_type_filter::utp_socket
            , lt::session::local_peer_class_id);
        peerClassTypeFilter.add(lt::peer_class_type_filter::utp_socket
            , lt::session::global_peer_class_id);
        peerClassTypeFilter.add(lt::peer_class_type_filter::ssl_utp_socket
            , lt::session::local_peer_class_id);
        peerClassTypeFilter.add(lt::peer_class_type_filter::ssl_utp_socket
            , lt::session::global_peer_class_id);
    }
    m_libbtsession->set_peer_class_type_filter(peerClassTypeFilter);
}

void BitTorrentSession::Configure(lt::settings_pack &settingsPack)
{
	std::string peerId = lt::generate_fingerprint(PEER_ID, BITSWASH_FINGERPRINT_VERSION);
	settingsPack.set_str( lt::settings_pack::user_agent, wxGetApp().UserAgent().ToStdString( ));

	// Speed limit
	settingsPack.set_int( lt::settings_pack::torrent_connect_boost, m_config->GetConnectBoostCounter() );
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
	//settingsPack.set_bool( lt::settings_pack::lazy_bitfields, m_config->GetLazyBitfields());
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

	lt::settings_pack::io_buffer_mode_t mode = m_config->GetUseOSCache() ? lt::settings_pack::enable_os_cache
															  : lt::settings_pack::disable_os_cache;
	settingsPack.set_int(lt::settings_pack::disk_io_read_mode, mode);
	settingsPack.set_int(lt::settings_pack::disk_io_write_mode, mode);
	settingsPack.set_bool(lt::settings_pack::anonymous_mode, m_config->GetAnonymousMode());

	// The most secure, rc4 only so that all streams are encrypted
	//settingsPack.set_int(lt::settings_pack::allowed_enc_level, ( lt::pe_settings::enc_level )( m_config->GetEncLevel()));
	settingsPack.set_bool(lt::settings_pack::prefer_rc4, m_config->GetEncEnabled());

    // The most secure, rc4 only so that all streams are encrypted
    settingsPack.set_int(lt::settings_pack::allowed_enc_level, lt::settings_pack::pe_rc4);
    settingsPack.set_bool(lt::settings_pack::prefer_rc4, true);
	if( m_config->GetEncEnabled() )
	{
		switch (m_config->GetEncPolicy()) {
		case 0: //Enabled
			settingsPack.set_int(lt::settings_pack::out_enc_policy, lt::settings_pack::pe_enabled);
			settingsPack.set_int(lt::settings_pack::in_enc_policy, lt::settings_pack::pe_enabled);
			break;
		case 1: // Forced
			settingsPack.set_int(lt::settings_pack::out_enc_policy, lt::settings_pack::pe_forced);
			settingsPack.set_int(lt::settings_pack::in_enc_policy, lt::settings_pack::pe_forced);
			break;
		default:
			wxASSERT(0);
			break;
		}
		wxLogMessage( _T( "Encryption enabled, policy %d" ), m_config->GetEncPolicy() );
	}
	else
	{
		settingsPack.set_int(lt::settings_pack::out_enc_policy, lt::settings_pack::pe_disabled);
		settingsPack.set_int(lt::settings_pack::in_enc_policy, lt::settings_pack::pe_disabled);
		wxLogMessage( _T( "Encryption is disabled" ) );
	}
	//case 1: // Forced
	//	settingsPack.set_int(lt::settings_pack::out_enc_policy, lt::settings_pack::pe_forced);
	//	settingsPack.set_int(lt::settings_pack::in_enc_policy, lt::settings_pack::pe_forced);

	settingsPack.set_int(lt::settings_pack::active_seeds, m_config->GetMaxActiveSeeds());
	settingsPack.set_bool(lt::settings_pack::dont_count_slow_torrents, m_config->GetIgnoreSlowTorrents());
	/*
	if (isQueueingSystemEnabled()) {
		adjustLimits(settingsPack);

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
	settingsPack.set_int(lt::settings_pack::alert_queue_size, 1000);
	//	10 * std::max(maxActiveTorrents() * 2, maxConnections())));

	// Outgoing ports
	settingsPack.set_int(lt::settings_pack::outgoing_port, m_config->GetPortMin());
	settingsPack.set_int(lt::settings_pack::num_outgoing_ports, m_config->GetPortMax() - m_config->GetPortMin() + 1);

	// Include overhead in transfer limits
	settingsPack.set_bool(lt::settings_pack::rate_limit_ip_overhead, m_config->GetRateLimitIpOverhead());
	settingsPack.set_bool(lt::settings_pack::prefer_udp_trackers, m_config->GetPreferUdpTrackers());
	// IP address to announce to trackers
	//settingsPack.set_str(lt::settings_pack::announce_ip, Utils::String::toStdString(announceIP()));
	// Super seeding
#if TORRENT_ABI_VERSION == 1
	settingsPack.set_bool(lt::settings_pack::strict_super_seeding, m_config->GetSupperSeeding());
#endif
	switch(m_config->GetEnabledProtocols())
	{
		case Configuration::PROTO_TCP:
			settingsPack.set_bool(lt::settings_pack::enable_incoming_tcp, true);
			settingsPack.set_bool(lt::settings_pack::enable_outgoing_tcp, true);
			settingsPack.set_bool(lt::settings_pack::enable_incoming_utp, false);
			settingsPack.set_bool(lt::settings_pack::enable_outgoing_utp, false);
			break;
		case Configuration::PROTO_UTP:
			settingsPack.set_bool(lt::settings_pack::enable_incoming_tcp, false);
			settingsPack.set_bool(lt::settings_pack::enable_outgoing_tcp, false);
			settingsPack.set_bool(lt::settings_pack::enable_incoming_utp, true);
			settingsPack.set_bool(lt::settings_pack::enable_outgoing_utp, true);
			break;
		default:
			settingsPack.set_bool(lt::settings_pack::enable_incoming_tcp, true);
			settingsPack.set_bool(lt::settings_pack::enable_outgoing_tcp, true);
			settingsPack.set_bool(lt::settings_pack::enable_incoming_utp, true);
			settingsPack.set_bool(lt::settings_pack::enable_outgoing_utp, true);
	}

	//settingsPack.set_bool(lt::settings_pack::apply_ip_filter_to_trackers, isTrackerFilteringEnabled());

	//Todo: new options=======================================

	settingsPack.set_int(lt::settings_pack::active_tracker_limit, -1);
	settingsPack.set_int(lt::settings_pack::active_dht_limit, -1);
	settingsPack.set_int(lt::settings_pack::active_lsd_limit, -1);

	// Set severity level of libtorrent session
	settingsPack.set_int(lt::settings_pack::alert_mask, lt::alert::error_notification
		| lt::alert::peer_notification
		| lt::alert::port_mapping_notification
		| lt::alert::storage_notification
		| lt::alert::tracker_notification
		| lt::alert::status_notification
		| lt::alert::ip_block_notification);
	settingsPack.set_str(lt::settings_pack::peer_fingerprint, peerId);
	settingsPack.set_bool(lt::settings_pack::listen_system_port_fallback, false);
	//settingsPack.set_bool(lt::settings_pack::upnp_ignore_nonrouters, true);
	settingsPack.set_bool(lt::settings_pack::use_dht_as_fallback, false);
	// Disable support for SSL torrents for now
	//settingsPack.set_int(lt::settings_pack::ssl_listen, 0);
	// To prevent ISPs from blocking seeding
	// Speed up exit
	settingsPack.set_int(lt::settings_pack::auto_scrape_interval, 1200); // 20 minutes
	settingsPack.set_int(lt::settings_pack::auto_scrape_min_interval, 900); // 15 minutes
	settingsPack.set_bool(lt::settings_pack::no_connect_privileged_ports, false);

	settingsPack.set_bool(lt::settings_pack::enable_dht, m_config->GetDHTEnabled());
    settingsPack.set_bool(lt::settings_pack::allow_multiple_connections_per_ip, m_config->GetAllowMultipleConnectionsPerIP());

	if( m_config->GetDHTEnabled() )
	{
		settingsPack.set_str(lt::settings_pack::dht_bootstrap_nodes, m_config->GetDhtBootstrapNodes().ToStdString());
	}
	settingsPack.set_bool(lt::settings_pack::enable_lsd, m_config->GetEnableLsd());

	switch (m_config->GetChokingAlgorithm()) {
	case Configuration::FIXED_SLOTS:
	default:
		settingsPack.set_int(lt::settings_pack::choking_algorithm, lt::settings_pack::fixed_slots_choker);
		break;
	case Configuration::RATE_BASED:
		settingsPack.set_int(lt::settings_pack::choking_algorithm, lt::settings_pack::rate_based_choker);
		break;
#if TORRENT_ABI_VERSION == 1
	case Configuration::BITTYRANT:
		settingsPack.set_int(lt::settings_pack::choking_algorithm, lt::settings_pack::bittyrant_choker);
		break;
#endif
	}

	switch (m_config->GetSeedChokingAlgorithm()) {
	case Configuration::ROUND_ROBIN:
		settingsPack.set_int(lt::settings_pack::seed_choking_algorithm, lt::settings_pack::round_robin);
		break;
	case Configuration::FASTEST_UPLOAD:
	default:
		settingsPack.set_int(lt::settings_pack::seed_choking_algorithm, lt::settings_pack::fastest_upload);
		break;
	case Configuration::ANTI_LEECH:
		settingsPack.set_int(lt::settings_pack::seed_choking_algorithm, lt::settings_pack::anti_leech);
		break;
	}
}

void BitTorrentSession::ConfigureSession()
{
	lt::session_params params;

	if( m_config->GetDHTEnabled() )
	{
		wxString dhtstatefile = wxGetApp().DHTStatePath();

		if( wxFileExists( dhtstatefile ) )
		{
			wxLogInfo( _T( "Restoring previous DHT state " ) + dhtstatefile );
#ifndef TORRENT_DISABLE_DHT
			std::vector<char> in;
			if (load_file(( const char* )dhtstatefile.mb_str( wxConvFile ), in))
				params = lt::read_session_params(in, lt::session_handle::save_dht_state);

			params.settings.set_bool(lt::settings_pack::dht_privacy_lookups, true);
			params.settings.set_int(lt::settings_pack::dht_max_peers_reply, m_config->GetDHTMaxPeers());
			params.settings.set_int(lt::settings_pack::dht_search_branching, m_config->GetDHTSearchBranching());
			params.settings.set_int(lt::settings_pack::dht_max_fail_count, m_config->GetDHTMaxFail());
#endif
		}
		else
		{
			wxLogWarning( _T( "Previous DHT state not found " ) + dhtstatefile );
		}
	}

	SetConnection();
	Configure(params.settings);

	m_libbtsession = new lt::session(std::move(params));

	m_libbtsession->set_alert_notify(boost::bind(&BitTorrentSession::PostLibTorrentAlertEvent, this));

	configurePeerClasses();

	//Network settings
	StartExtensions();
	StartUpnp();
	StartNatpmp();
}

void BitTorrentSession::SetConnection()
{
	long portmin, portmax;
	portmin = m_config->GetPortMin();
	portmax = m_config->GetPortMax();

	if( ( portmin < 0 ) || ( portmin > 65535 ) )
	{
		portmin = 6881 ;
	}
	m_config->SetPortMin( portmin );

	if( ( portmax < 0 ) || ( portmax > 65535 ) || ( portmax < portmin ) )
	{
		portmax = portmin + 8;
	}
	m_config->SetPortMax( portmax );
}

void BitTorrentSession::StartExtensions()
{
	wxASSERT( m_libbtsession != nullptr );

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
	wxASSERT( m_libbtsession != nullptr );
	bool enable_upnp = m_config->GetEnableUpnp();
	lt::settings_pack settingsPack = m_libbtsession->get_settings();
	settingsPack.set_bool(lt::settings_pack::enable_upnp, enable_upnp );
	m_libbtsession->apply_settings( settingsPack );
	m_upnp_started = enable_upnp;
}

void BitTorrentSession::StartNatpmp()
{
	wxASSERT( m_libbtsession != nullptr );
	bool enable_natpmp = m_config->GetEnableNatpmp();
	lt::settings_pack settingsPack = m_libbtsession->get_settings();
	settingsPack.set_bool(lt::settings_pack::enable_natpmp, enable_natpmp );
	m_libbtsession->apply_settings( settingsPack );
	m_natpmp_started = enable_natpmp;
}

void BitTorrentSession::AddTorrentToSession( std::shared_ptr<torrent_t>& torrent )
{
	lt::torrent_handle &handle = torrent->handle;
	wxString thash(torrent->hash);
	wxLogDebug(( _T( "AddTorrent %s into session" ), torrent->name.c_str() ));
	wxASSERT_MSG((thash != _T("0000000000000000000000000000000000000000")), _("Torrent hash is invalid! ")+torrent->name);
	wxString fastresumefile = wxGetApp().SaveTorrentsPath() + thash + _T( ".resume" );

	try
	{
		//XXX libtorrent updated with 3 mode
		//sparse (default)
		//allocate
		//compact
		//
		lt::add_torrent_params p;
		lt::error_code ec;

		if( wxFileExists( fastresumefile ) )
		{
			wxLogInfo( _T( "%s: Read fast resume data" ), torrent->name.c_str() );
			std::ifstream fastresume_in( ( const char * )fastresumefile.mb_str( wxConvFile ), std::ios_base::binary );
			if(fastresume_in)
			{
				fastresume_in.unsetf( std::ios_base::skipws );
				// get length of file:
				fastresume_in.seekg (0, fastresume_in.end);
				int length = fastresume_in.tellg();
				fastresume_in.seekg (0, fastresume_in.beg);
				if(length > 0)
				{
					std::vector<char> resume_data;
					resume_data.resize(length);
					fastresume_in.read (&resume_data[0],length);
					p = lt::read_resume_data(resume_data, ec);
				}
			}
		}

		wxASSERT(torrent->info);
		p.ti = torrent->info;
		//p.ti.reset(const_cast<lt::torrent_info *>(torrent->info.get()));
		p.save_path = std::move(std::string((const char*)((torrent->config->GetDownloadPath().ToUTF8())).data()));
		p.max_connections = m_config->GetMaxConnections();
		p.max_uploads = m_config->GetMaxUploads();
		p.upload_limit = m_config->GetDefaultUploadLimit();
		p.download_limit = m_config->GetDefaultDownloadLimit();

		if (torrent->config->GetTorrentState() == TORRENT_STATE_PAUSE)
			p.flags |= lt::torrent_flags::paused;
		p.flags &= ~lt::torrent_flags::duplicate_is_error;
		p.flags &= ~lt::torrent_flags::auto_managed;

		wxString strStorageMode;
		enum lt::storage_mode_t eStorageMode = torrent->config->GetTorrentStorageMode();

		switch( eStorageMode )
		{
		case lt::storage_mode_allocate:
			strStorageMode = _( "Full" );
			break;

		case lt::storage_mode_sparse:
		default:
			eStorageMode = lt::storage_mode_sparse;
			strStorageMode = _( "Sparse" );
			break;
		}

		p.storage_mode = eStorageMode;
		wxLogInfo( _T( "%s: %s allocation mode" ), torrent->name.c_str(), strStorageMode.c_str() );
		m_libbtsession->async_add_torrent(std::move(p));

		wxMutexLocker ml( m_torrent_queue_lock );
		m_queue_torrent_set.insert(thash);
		//enum torrent_state state = ( enum torrent_state ) torrent->config->GetTorrentState();
		//handle.pause();
		//ConfigureTorrent(torrent);
	}
	catch( std::exception& e )
	{
		//posibly duplicate torrent
		wxLogFatalError( _T("Exception: [%s]:%s"), __FUNCTION__, wxString::FromUTF8(e.what()).c_str() );
	}
	catch (...)
	{
		wxLogFatalError(_T("Unknown Exception: %s"), __FUNCTION__ );
	}
}

void BitTorrentSession::AddMagnetUriToSession(std::shared_ptr<torrent_t>& torrent)
{
	try
	{
		wxASSERT(torrent);
		wxASSERT(torrent->config);
		const wxString & url = torrent->config->GetTorrentMagnetUri();
		if(!url.IsEmpty())
		{
			lt::error_code ec;
			lt::add_torrent_params p = lt::parse_magnet_uri(std::string((const char *)(url.ToUTF8().data())), ec);
			if (ec)
			{
				wxLogError( wxString::FromUTF8( ec.message().c_str() ) );
			}
			else
			{
				wxString thash(torrent->hash);
				if( thash != _T("0000000000000000000000000000000000000000"))
				{
					p.save_path = std::move(std::string(( const char* )((torrent->config->GetDownloadPath().ToUTF8( ))).data()));
			
					p.flags |= lt::torrent_flags::paused; // Start in pause
					p.flags &= ~lt::torrent_flags::auto_managed; // Because it is added in paused state
					p.flags &= ~lt::torrent_flags::duplicate_is_error; // Already checked
					// Solution to avoid accidental file writes
					p.flags |= lt::torrent_flags::upload_mode;
					p.max_connections = m_config->GetMaxConnections();
					p.max_uploads = m_config->GetMaxUploads();
					p.upload_limit = m_config->GetDefaultUploadLimit();
					p.download_limit = m_config->GetDefaultDownloadLimit();
			
					// Adding torrent to BitTorrent session
					m_libbtsession->async_add_torrent(std::move(p));
					wxMutexLocker ml( m_torrent_queue_lock );
					m_queue_torrent_set.insert(thash);
				}
				else
				{
					wxMessageBox( _( "Parse magnet URI failed:The torrent is invalid!" ), _( "Error" ),  wxOK | wxICON_ERROR );
				}
			}
		}
	}
	catch( std::exception &e )
	{
		wxLogFatalError( _T("Exception: [%s]:%s"), __FUNCTION__, wxString::FromUTF8(e.what()).c_str() );
	}
	catch (...)
	{
		wxLogFatalError(_T("Unknown Exception: %s"), __FUNCTION__ );
	}
}

bool BitTorrentSession::AddTorrent( std::shared_ptr<torrent_t>& torrent )
{
	wxLogDebug( _T( "Add Torrent %s, state %d" ),  torrent->name.c_str(), torrent->config->GetTorrentState() );

	try
	{
		wxString thash(torrent->hash);
		{
			wxMutexLocker ml( m_torrent_queue_lock );
			if( find_torrent_by_hash( thash ) >= 0 )
			{
				wxLogWarning( _T( "Torrent %s already exists" ), torrent->name.c_str() );
				return false;
			}

			int idx = (int)m_torrent_queue.size();
			//wxASSERT(idx >= 0);

			m_torrent_queue.push_back( torrent );
			m_running_torrent_map.insert( std::pair<wxString, int>( thash, idx) );
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
		wxLogFatalError( _T("Exception: [%s]:%s"), __FUNCTION__, wxString::FromUTF8(e.what()).c_str() );
		return false;
	}
	catch (...)
	{
		wxLogFatalError(_T("Unknown Exception: %s"), __FUNCTION__ );
		return false;
	}

	return true;
}

void BitTorrentSession::RemoveTorrent( std::shared_ptr<torrent_t>& torrent, bool deletedata )
{
	WXLOGDEBUG(( _T( "%s: Removing Torrent" ), torrent->name.c_str() ));
	wxASSERT(torrent);
	wxString thash(torrent->hash);
	{
		wxMutexLocker ml( m_torrent_queue_lock );
		int idx = find_torrent_by_hash( thash );

		if( idx < 0 )
		{
			wxLogError( _T( "RemoveTorrent %s: Torrent not found in queue" ), torrent->name.c_str() );
			return ;
		}
		auto torrent_it = m_torrent_queue.begin() + idx;
		m_queue_torrent_set.erase( thash );
		m_running_torrent_map.erase( thash );
		m_torrent_queue.erase( torrent_it );
		for(size_t i = idx; i < m_torrent_queue.size(); ++i)
		{
			m_running_torrent_map[ wxString(m_torrent_queue[i]->hash) ] = i;
		}
	}

	lt::torrent_handle& h = torrent->handle;
	if( h.is_valid() )
	{
		h.pause();
		lt::remove_flags_t options;
		if( deletedata ) options = lt::session::delete_files;
		m_libbtsession->remove_torrent( h, options );
	}

	wxASSERT(torrent->config);
	enum torrent_state state = ( enum torrent_state ) torrent->config->GetTorrentState();

	wxString app_prefix = wxGetApp().SaveTorrentsPath() + thash;
	wxString fastresumefile = app_prefix + _T( ".resume" );
	//wxString resumefile = app_prefix + _T( ".resume" );
	wxString torrentconffile = app_prefix + _T( ".conf" );
	wxString torrentfile = app_prefix + _T( ".torrent" );
	wxString partsfile = wxGetApp().SaveTorrentsPath() + _T(".") + thash + _T( ".parts" );

	if( ( wxFileExists( fastresumefile ) ) &&
			( !wxRemoveFile( fastresumefile ) ) )
	{
		wxLogError( _T( "Error removing file %s" ), fastresumefile.c_str() );
	}

	if( ( wxFileExists( torrentconffile ) ) &&
			( !wxRemoveFile( torrentconffile ) ) )
	{
		wxLogError( _T( "Error removing file %s" ), torrentconffile.c_str() );
	}

	if( ( wxFileExists( torrentfile ) ) &&
			( !wxRemoveFile( torrentfile ) ) )
	{
		wxLogError( _T( "Error removing file %s" ), torrentfile.c_str() );
	}

	if( ( wxFileExists( partsfile ) ) &&
			( !wxRemoveFile( partsfile ) ) )
	{
		wxLogError( _T( "Error removing file %s" ), partsfile.c_str() );
	}

	if( deletedata )
	{
		wxString download_data( torrent->config->GetDownloadPath() + torrent->name );
		wxLogInfo( _T( "%s: Remove Data as well in %s" ), torrent->name.c_str(), download_data.c_str() );

		if( wxDirExists( download_data ) )
		{
			wxFileName fn(torrent->config->GetDownloadPath());
			wxFileName cwd(wxFileName::GetCwd(fn.GetVolume()));
			if(fn.GetLongPath() == cwd.GetLongPath())
			{
				wxSetWorkingDirectory (fn.GetVolume() + wxFileName::GetPathSeparator());
			}
			if( !RemoveDirectory( download_data ) )
			{
				wxLogError( _T( "%s: Error removing directory %s error %s" ), torrent->name.c_str(), download_data.c_str(), wxSysErrorMsg( wxSysErrorCode() ) );
			}
		}
		else if( wxFileExists( download_data ) )
		{
			if( !wxRemoveFile( download_data ) )
			{
				wxLogError( _T( "%s: Error removing file %s" ), torrent->name.c_str(), download_data.c_str() );
			}
		}
		else
		{ wxLogInfo( _T( "%s: No downloaded data to remove" ), torrent->name.c_str() ); }
	}

	if (torrent->info && torrent->info->is_valid())
	{
		const std::vector<lt::download_priority_t>& fp = torrent->config->GetFilesPriorities();
		const std::vector<lt::download_priority_t>& filespriority = torrent->config->GetFilesPriorities();
		int nfiles = torrent->info->num_files();
		const lt::file_storage &allfiles = torrent->info->files();
		wxString fname;
		wxASSERT(nfiles == fp.size());

		for (int i = 0; i < nfiles; ++i)
		{
			if (filespriority[i] == TorrentConfig::file_none)
			{
				lt::file_index_t idx(i);
				fname = (torrent->config->GetDownloadPath() + wxString::FromUTF8((allfiles.file_path(idx)).c_str()));
				if (wxFileExists(fname) && !wxRemoveFile(fname))
					wxLogError(_T("Error removing file %s"), fname.c_str());
			}
		}
	}

	wxString uri = torrent->config->GetTorrentMagnetUri();
	if(!uri.IsEmpty())
	{
		wxString urifname = wxGetApp().SaveTorrentsPath() + APPBINNAME + _T( ".magneturi" );
		wxTextFile magneturifile(urifname);
		if( magneturifile.Exists() )
		{
			magneturifile.Open( wxConvFile );
			if( magneturifile.IsOpened() )
			{
				size_t n = 0;
				wxString url;
				for ( url = magneturifile.GetFirstLine(); !magneturifile.Eof(); url = magneturifile.GetNextLine() )
				{
					if(!url.IsEmpty())
					{
						if(url == uri)
						{
							magneturifile.RemoveLine(n);
							break;
						}
					}
					++n;
				}
			}
		}
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
	int idx = find_torrent_by_hash(hash);

	return idx;
}

std::shared_ptr<torrent_t> BitTorrentSession::GetTorrent( const wxString &hash )
{
	wxMutexLocker ml(m_torrent_queue_lock);
	int idx = find_torrent_by_hash(hash);

	if( idx >= 0 )
		return m_torrent_queue.at( idx );
	else
		return {};
}

std::shared_ptr<torrent_t> BitTorrentSession::GetTorrent( int idx )
{
	std::shared_ptr<torrent_t> torrent;

	wxMutexLocker ml( m_torrent_queue_lock );
	if( idx >= 0 && idx < m_torrent_queue.size())
	{
		torrent = m_torrent_queue.at( idx );
	}

	return torrent;
}

void BitTorrentSession::MergeTorrent( std::shared_ptr<torrent_t>& dst_torrent, std::shared_ptr<torrent_t>& src_torrent )
{
	lt::torrent_handle &torrent_handle = dst_torrent->handle;

	if(torrent_handle.is_valid())
	{
		std::vector<lt::announce_entry> const& trackers = src_torrent->info->trackers();
		for(auto tracker : trackers)
		{
			torrent_handle.add_tracker( tracker );
		}

		std::vector<lt::web_seed_entry> const& web_seeds = src_torrent->info->web_seeds();
		for(auto web_seed : web_seeds)
		{
			if( web_seed.type == lt::web_seed_entry::url_seed )
			{ torrent_handle.add_url_seed( web_seed.url ); }
			else if( web_seed.type == lt::web_seed_entry::http_seed )
			{ torrent_handle.add_http_seed( web_seed.url ); }
			else
			{wxLogError(_T("Unsupported web seed type %d"), web_seed.type);}
				
		}
	}
}

void BitTorrentSession::MergeTorrent( std::shared_ptr<torrent_t>& dst_torrent, MagnetUri& src_magneturi )
{
	lt::torrent_handle &torrent_handle = dst_torrent->handle;

	if(torrent_handle.is_valid())
	{
		std::vector<std::string> const& trackers = src_magneturi.trackers();
		for(auto tracker : trackers)
		{
			torrent_handle.add_tracker( lt::announce_entry( tracker ) );
		}

		std::vector<std::string> const& url_seeds = src_magneturi.urlSeeds();
		for(auto url_seed : url_seeds)
		{
			torrent_handle.add_url_seed( url_seed );
		}
	}
}

void BitTorrentSession::ScanTorrentsDirectory( const wxString& dirname )
{
	wxASSERT( m_libbtsession != nullptr );
	wxDir torrents_dir( dirname );
	wxString filename;
	//wxString fullpath;
	wxFileName fn;
	std::shared_ptr<torrent_t> torrent;

	if( !torrents_dir.IsOpened() )
	{
		wxLogWarning( _( "Error opening directory %s for processing" ), wxGetApp().SaveTorrentsPath().c_str() );
		return ;
	}

	bool cont = torrents_dir.GetFirst( &filename, _T( "*.torrent" ), wxDIR_FILES );

	while( cont )
	{
		//fullpath = dirname + filename;
		fn.Assign(dirname, filename);
		WXLOGDEBUG(( _T( "Saved Torrent: %s" ), fn.GetFullPath().c_str() ));
		torrent = ParseTorrent(fn.GetFullPath());

		if( torrent && torrent->isvalid )
		{
            if (torrent->config->GetTorrentState() == TORRENT_STATE_QUEUE) /*Fist time added torrent, it's not in libtorrent*/
        	{
                torrent->config->SetTorrentState(TORRENT_STATE_START);
        	}
			{
				wxString thash(torrent->hash);
				wxMutexLocker ml( m_torrent_queue_lock );
				if( find_torrent_by_hash( thash ) >= 0 )
				{
					wxLogWarning( _T( "Torrent %s already exists" ), torrent->name.c_str() );
				}
				else
				{
					m_torrent_queue.push_back( torrent );
				}
			}
		}

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

	std::vector<size_t> start_torrents;
	{
		int maxstart = m_config->GetMaxStart(), started = 0;
		wxMutexLocker ml( m_torrent_queue_lock );
		m_running_torrent_map.clear();
		std::sort( m_torrent_queue.begin(), m_torrent_queue.end(), [&] ( const std::shared_ptr<torrent_t>& torrent_start, const std::shared_ptr<torrent_t>& torrent_end )
			{
				return ((torrent_end->config->GetQIndex() >= 0 && torrent_start->config->GetQIndex() >= 0) && (torrent_start->config->GetQIndex() < torrent_end->config->GetQIndex()));
			});

		for( size_t i = 0; i < m_torrent_queue.size(); ++i )
		{
			auto & torrent = m_torrent_queue.at( i );
			wxASSERT(torrent);
			torrent->config->SetQIndex(i); //Make sure
			m_running_torrent_map[wxString(torrent->hash)] = i;

			//XXX redundant ? StartTorrent below will call the same thing
			if((started < maxstart) && torrent->isvalid )
			{
				wxASSERT(torrent->config);
				if(torrent->config)
				{
					enum torrent_state state = ( enum torrent_state ) torrent->config->GetTorrentState();
					if(( ( state == TORRENT_STATE_START ) ||
							( state == TORRENT_STATE_FORCE_START ) ||
							( state == TORRENT_STATE_PAUSE ) ))
					{
						if((torrent->info) && torrent->info->is_valid())
						{
							start_torrents.push_back(i);
							++started; //Do NOT break!
						}
					}
				}
				else
				{
					wxLogError( _T( "Torrent %s doesn't have config" ), torrent->name.c_str() );
				}
			}
		}
	}

	for(auto i : start_torrents)
		AddTorrentToSession( m_torrent_queue.at( i ) );
}

size_t BitTorrentSession::GetTorrentQueueSize()
{
	wxMutexLocker ml( m_torrent_queue_lock );
	return m_torrent_queue.size();
}

//find and return index of torrent handle from hash string
int BitTorrentSession::find_torrent_by_hash( const wxString& hash )
{
	int j = -1;
	wxMutexLocker ml( m_torrent_queue_lock );
	auto it = m_running_torrent_map.find( hash );

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
	WXLOGDEBUG(( _T( "Saving torrent info" ) ));
	// keep track of the number of resume data
	// alerts to wait for
	int num_paused = 0;
	int num_failed = 0;
	/* take this opportunity to sync torrent index back to zero */
	int num_outstanding_resume_data = 0;
	wxArrayString magneturis;

	{
		int idx = 0;
		wxMutexLocker ml( m_torrent_queue_lock );
		for( auto & torrent : m_torrent_queue )
		{
			torrent->config->SetQIndex( idx++ );
			torrent->config->Save();


			if( !torrent->handle.is_valid() || \
					!torrent->handle.status().has_metadata || \
					( !( ( torrent->handle.status() ).need_save_resume ) ) || \
					torrent->config->GetTorrentState() == TORRENT_STATE_STOP )
			{
				wxString magneturi(torrent->config->GetTorrentMagnetUri());
				if ((!magneturi.IsEmpty()) && torrent->handle.is_valid()
					&& !torrent->handle.status().has_metadata)
				{
					magneturis.Add(magneturi);
				}

				continue;
			}

			if (torrent->handle.is_valid())
			{
				lt::torrent_status status = torrent->handle.status();
				if(status.has_metadata)
				{
					torrent->handle.save_resume_data(lt::torrent_handle::save_info_dict);
					++num_outstanding_resume_data;
				}
			}
			m_libbtsession->remove_torrent( torrent->handle );
		}
	}

	while( num_outstanding_resume_data > 0 )
	{
		lt::alert const* a = m_libbtsession->wait_for_alert(lt::seconds( m_config->GetExitWaitingTime() ) );

		if( a == 0 ) { continue; }

		std::vector<lt::alert*> alerts;
		m_libbtsession->pop_alerts( &alerts );
		std::string now = timestamp();

		for( auto const& i : alerts )
		{
			// make sure to delete each alert
			lt::torrent_paused_alert const* tp = lt::alert_cast<lt::torrent_paused_alert>( i );

			if( tp )
			{
				++num_paused;
				WXLOGDEBUG(( _T( "left: %d failed: %d pause: %d" )
							, num_outstanding_resume_data, num_failed, num_paused ));
			}
			else if(lt::alert_cast<lt::save_resume_data_failed_alert>( i ) )
			{
				++num_failed;
				--num_outstanding_resume_data;
				WXLOGDEBUG(( _T( "left: %d failed: %d pause: %d" )
							, num_outstanding_resume_data, num_failed, num_paused ));
			}
			else if (lt::save_resume_data_alert * p = lt::alert_cast<lt::save_resume_data_alert>( i ) )
			{
				--num_outstanding_resume_data;
				WXLOGDEBUG(( _T( "left: %d failed: %d pause: %d" )
							, num_outstanding_resume_data, num_failed, num_paused ));

				SaveTorrentResumeData(p);
			}
		}
	}

	// save dead magnet uri
	size_t counts = magneturis.GetCount();
	wxString filename( wxGetApp().SaveTorrentsPath() + APPBINNAME + _T( ".magneturi" ));
	wxTextFile magneturifile( filename );
	if(counts > 0)
	{
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
				magneturifile.AddLine( magneturis[i] );
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

void BitTorrentSession::SaveTorrentResumeData( lt::save_resume_data_alert * p )
{
	//new api, write to disk in save_resume_data_alert
	lt::torrent_handle& h = p->handle;

	if (h.is_valid())
	{
		lt::torrent_status st = h.status(/*lt::torrent_handle::query_save_path*/);
		if (st.has_metadata)
		{
			wxString resumefile = wxGetApp().SaveTorrentsPath() + to_hex(p->params.info_hashes.get_best()) + _T(".resume");
			auto const buf = write_resume_data_buf(p->params);
			save_file((const char*)resumefile.mb_str(wxConvFile), buf);
		}
	}
}

void BitTorrentSession::DumpTorrents()
{
	wxMutexLocker ml( m_torrent_queue_lock );
	for( auto & torrent : m_torrent_queue )
	{
		wxLogMessage( _T( "[%s] %s" ), ( wxLongLong( torrent->config->GetQIndex() ).ToString() ).c_str(), torrent->name.c_str() );
	}
}

/* General function, parse torrent file into torrent_t */
std::shared_ptr<torrent_t> BitTorrentSession::ParseTorrent( const wxString& filename )
{
	std::shared_ptr<torrent_t> torrent;
	wxLogInfo(( _T( "Parse Torrent: %s" ), filename.c_str() ));

	try
	{
		lt::error_code ec;
		std::string cpath((const char *)(filename.ToUTF8()).data());
		std::shared_ptr<lt::torrent_info> t(new lt::torrent_info(cpath, ec));
		if(ec)
			wxLogError( wxString::FromUTF8( ec.message().c_str() ) );
		else
		{
		    torrent.reset( new torrent_t() );
			torrent->info = t;
			torrent->name = wxString( wxConvUTF8.cMB2WC( t->name().c_str() ) );
			torrent->hash = InfoHash(t->info_hash());
			wxString thash(torrent->hash);
			if( thash != _T("0000000000000000000000000000000000000000"))
			{
				torrent->config.reset( new TorrentConfig( thash ) );

				if( torrent->config->GetTrackersURL().size() <= 0 )
				{
					std::vector<lt::announce_entry> trackers = t->trackers();
					torrent->config->SetTrackersURL( trackers );
				}

				torrent->isvalid = true;
				UpdateTorrentFileSize(torrent);
			}
			else
			{
				wxMessageBox( _( "Parse torrent failed:The torrent is invalid!" ), _( "Error" ),  wxOK | wxICON_ERROR );
			}
		}
	}
	catch( std::exception &e )
	{
		wxLogFatalError( _T("Exception: [%s]:%s"), __FUNCTION__, wxString::FromUTF8(e.what()).c_str() );
	}
	catch (...)
	{
		wxLogFatalError(_T("Unknown Exception: %s"), __FUNCTION__ );
	}

	return torrent;
}

std::shared_ptr<torrent_t> BitTorrentSession::LoadMagnetUri( MagnetUri& magneturi )
{
	std::shared_ptr<torrent_t> torrent;
	{
		bool notfound = true;
		wxString thash(magneturi.hash());
		wxLogInfo(( _T( "LoadMagnetUri: hash %s" ), thash.c_str() ));
		{
			wxMutexLocker ml(m_torrent_queue_lock);
			notfound = ((m_queue_torrent_set.find( thash ) == m_queue_torrent_set.end())
				&& (find_torrent_by_hash( thash ) < 0));
		}
		if(notfound)
		{
			try
			{
				torrent.reset(new torrent_t());
				lt::add_torrent_params p = magneturi.addTorrentParams();
				// Limits
				torrent->name = magneturi.name();
				torrent->hash = magneturi.hash();
				if( wxString(torrent->hash) != _T("0000000000000000000000000000000000000000"))
				{
					torrent->config.reset( new TorrentConfig( thash ) );
					torrent->config->SetTorrentMagnetUri(magneturi.url());
					torrent->config->SetTorrentState( TORRENT_STATE_START );

					std::vector<wxString>&historypath = m_config->GetSavePathHistory();
					std::vector<wxString>::reverse_iterator path  = historypath.rbegin();
					wxString spath(wxEmptyString);
					if(path != historypath.rend())
					{
						spath = *path;
					}
					if(spath == wxEmptyString)
					{
						spath = wxFileName::GetTempDir();
					}
					torrent->config->SetDownloadPath(spath);

					torrent->isvalid = false;
					AddMagnetUriToSession(torrent);
					wxMutexLocker ml(m_torrent_queue_lock);
					int idx = (int)m_torrent_queue.size();
					m_torrent_queue.push_back( torrent );
					m_running_torrent_map.insert(std::pair<wxString, int>( thash, idx));
				}
				else
				{
					wxMessageBox( _( "Parse magnet URI failed:The torrent is invalid!" ), _( "Error" ),  wxOK | wxICON_ERROR );
				}
			}
			catch( std::exception &e )
			{
				wxLogFatalError( _T("Exception: [%s]:%s"), __FUNCTION__, wxString::FromUTF8(e.what()).c_str() );
			}
			catch (...)
			{
				wxLogFatalError(_T("Unknown Exception: %s"), __FUNCTION__ );
			}
		}
	}

	return torrent;
}

void BitTorrentSession::UpdateTorrentFileSize(std::shared_ptr<torrent_t>& torrent)
{
	if(torrent && torrent->info && torrent->info->is_valid())
	{
		bool nopriority = false;
		wxULongLong_t total_selected = 0, total = 0;
		lt::torrent_info const& t_info = *(torrent->info);
		lt::file_storage const& allfiles = t_info.files();
		std::vector<lt::download_priority_t> filespriority = torrent->config->GetFilesPriorities();
		int totalfiles = t_info.num_files();

		if (filespriority.size() != totalfiles)
		{
			nopriority = true;
		}

		for(int i = 0; i < totalfiles; ++i)
		{

			lt::file_index_t idx(i);
			if (nopriority || filespriority[i] != TorrentConfig::file_none)
			{
				total_selected += allfiles.file_size(idx);
			}
			total += allfiles.file_size(idx);
		}

		torrent->config->SetSelectedSize(total_selected);
		torrent->config->SetTotalSize(total);
	}
}

bool BitTorrentSession::SaveTorrent( std::shared_ptr<torrent_t>& torrent, const wxString& filename )
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
		wxLogFatalError( _T("Exception: [%s]:%s"), __FUNCTION__, wxString::FromUTF8(e.what()).c_str() );
		return false;
	}
	catch (...)
	{
		wxLogFatalError(_T("Unknown Exception: %s"), __FUNCTION__ );
		return false;
	}

	return true;
}

void BitTorrentSession::StartTorrent( std::shared_ptr<torrent_t>& torrent, bool force )
{
	wxLogInfo( _T( "%s: Start %s" ), torrent->name.c_str(), force ? _T( "force" ) : _T( "" ) );
	lt::torrent_handle& handle = torrent->handle;

	if (handle.is_valid())
	{
		ConfigureTorrent(torrent);
		if(force) handle.clear_error();
		if(handle.status().flags & lt::torrent_flags::paused)
			handle.resume();
	}

	wxASSERT(torrent->config);
	if(torrent->config->GetTorrentState() != TORRENT_STATE_FORCE_START)
	{
		torrent->config->SetTorrentState(force ? TORRENT_STATE_FORCE_START : TORRENT_STATE_START);
		torrent->config->Save();
	}

	if( !handle.is_valid() || ( ( handle.status(lt::torrent_handle::query_save_path).save_path).empty() ) )
	{
		if(torrent->info)
			AddTorrentToSession( torrent );
		else
			AddMagnetUriToSession(torrent);
	}
}

void BitTorrentSession::StopTorrent( std::shared_ptr<torrent_t>& torrent )
{
	wxLogInfo( _T( "%s:Stop" ), torrent->name.c_str() );
	wxASSERT(torrent);
	wxASSERT(torrent->isvalid);
	wxASSERT(torrent->config);
	lt::torrent_handle& handle = torrent->handle;

	//Remove torrent from session
	//set property to STOP
	//
	if( handle.is_valid() )
	{
		handle.pause(lt::torrent_handle::graceful_pause);
		lt::torrent_status status = torrent->handle.status();

		if (torrent->config)
		{
			stats_t& torrentstats = torrent->config->GetTorrentStats();
			torrentstats.progress = status.progress;
			torrentstats.total_download = status.total_payload_download;
			torrentstats.total_upload = status.total_payload_upload;
		}

		try
		{
			if (status.has_metadata && !((status.state == lt::torrent_status::seeding) || (status.state == lt::torrent_status::finished)))
			{
				torrent->handle.save_resume_data(lt::torrent_handle::save_info_dict);
			}
			else
				m_libbtsession->remove_torrent(handle);
		}
		catch (std::exception& e)
		{
			//posibly duplicate torrent
			wxLogFatalError( _T("Exception: [%s]:%s"), __FUNCTION__, wxString::FromUTF8(e.what()).c_str() );
		}
		catch (...)
		{
			wxLogFatalError(_T("Unknown Exception: %s"), __FUNCTION__);
		}
	}

	if(torrent->config)
	{
		enum torrent_state state = ( enum torrent_state ) torrent->config->GetTorrentState();

		torrent->config->SetTorrentState( TORRENT_STATE_STOP );
		torrent->config->Save();

		//XXX redundant ? StartTorrent below will call the same thing
		if( ( state == TORRENT_STATE_START ) ||
				( state == TORRENT_STATE_FORCE_START ))
		{
			PostQueueUpdateEvent();
		}
	}
}

void BitTorrentSession::QueueTorrent( std::shared_ptr<torrent_t>& torrent )
{
	QueueTorrent(torrent, TORRENT_STATE_QUEUE);
	wxLogInfo( _T( "%s: Queueed" ), torrent->name.c_str() );
}

void BitTorrentSession::QueueTorrent(std::shared_ptr<torrent_t>& torrent, int state)
{
	wxASSERT(torrent->isvalid);
	wxASSERT(torrent->config);
	lt::torrent_handle& handle = torrent->handle;

	torrent->config->SetTorrentState( state );
	torrent->config->Save();
	if (handle.is_valid())
	{
		lt::torrent_status status = torrent->handle.status(lt::torrent_handle::query_distributed_copies);

		stats_t& torrentstats = torrent->config->GetTorrentStats();
		torrentstats.progress = status.progress;
		torrentstats.total_download = status.total_payload_download;
		torrentstats.total_upload = status.total_payload_upload;
		handle.pause(lt::torrent_handle::graceful_pause);
	}
	else
	{
		bool notfound = true;
		{
			wxMutexLocker ml(m_torrent_queue_lock);
			notfound = (m_queue_torrent_set.find(wxString(torrent->hash)) == m_queue_torrent_set.end());
		}
		if(notfound)
		{
			if(torrent->info)
				AddTorrentToSession( torrent );
			else
				AddMagnetUriToSession( torrent );
		}
	}
}

void BitTorrentSession::PauseTorrent( std::shared_ptr<torrent_t>& torrent )
{
	QueueTorrent(torrent, TORRENT_STATE_PAUSE);
	wxLogInfo( _T( "%s: Paused" ), torrent->name.c_str() );
}

void BitTorrentSession::MoveTorrentUp( std::shared_ptr<torrent_t>& torrent )
{
	wxString thash( torrent->hash );
	wxMutexLocker ml( m_torrent_queue_lock );
	int idx = find_torrent_by_hash( thash );
	if( idx <= 0 )
	{
		if( idx == 0 )
		{
			wxLogInfo( _T( "Torrent is at top position %d-%s" ), idx, torrent->name.c_str() );
		}
		else
		{
			wxLogError( _T( "MoveTorrentUp %s: Torrent not found in queue" ), torrent->name.c_str() );
		}
		return ;
	}

	long qindex = torrent->config->GetQIndex();
	if(qindex < 0)
	{
		wxLogInfo( _T( "Torrent is not in download queue %d-%s" ), idx, torrent->name.c_str() );
		return;
	}

	torrents_t::iterator torrent_it = m_torrent_queue.begin() + idx - 1;
	std::shared_ptr<torrent_t> prev_torrent( *( torrent_it ) );
	WXLOGDEBUG(( _T( "Prev %d now %d" ), prev_torrent->config->GetQIndex(), torrent->config->GetQIndex() ));
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
	m_running_torrent_map[thash] = idx - 1;
	m_running_torrent_map[wxString(prev_torrent->hash)] = idx;
//m_torrent_queue.erase( torrent_it );
//torrent_it = m_torrent_queue.begin() + idx;
//m_torrent_queue.insert( torrent_it, prev_torrent );
}

void BitTorrentSession::MoveTorrentDown( std::shared_ptr<torrent_t>& torrent )
{
	wxString thash( torrent->hash );
	wxMutexLocker ml( m_torrent_queue_lock );
	int idx = find_torrent_by_hash( thash );

	if( idx < 0 )
	{
		wxLogError( _T( "MoveTorrentDown %s: Torrent not found in queue" ), torrent->name.c_str() );
		return ;
	}

	/* we are going to delete it
	 * deference from queue
	 */
	//torrent_t *torrent = torrent;

	if( idx == ( m_torrent_queue.size() - 1 ) )
	{
		wxLogInfo( _T( "Torrent is at bottom position %d-%s" ), idx, torrent->name.c_str() );
		return;
	}

	long qindex = torrent->config->GetQIndex();
	if(qindex < 0)
	{
		wxLogInfo( _T( "Torrent is not in download que %d-%s" ), idx, torrent->name.c_str() );
		return;
	}

	wxLogInfo( _T( "%s: Moving Torrent Down" ), torrent->name.c_str() );
	torrents_t::iterator torrent_it = m_torrent_queue.begin() + idx;
	std::shared_ptr<torrent_t> next_torrent( *( torrent_it + 1 ) );
	WXLOGDEBUG(( _T( "Next %d now %d" ), next_torrent->config->GetQIndex(), torrent->config->GetQIndex() ));
	long next_qindex = next_torrent->config->GetQIndex();

	if( next_qindex < 0 )
	{
		wxLogInfo( _T( "Torrent is at queue end %d-%s" ), idx, torrent->name.c_str() );
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
	m_running_torrent_map[thash] = idx + 1;
	m_running_torrent_map[wxString(next_torrent->hash)] = idx;
//m_torrent_queue.erase( torrent_it );
//torrent_it = m_torrent_queue.begin() + idx + 1 ;
//m_torrent_queue.insert( torrent_it, torrent );
}

void BitTorrentSession::MoveTorrentBottom( std::shared_ptr<torrent_t>& torrent )
{
	wxString thash( torrent->hash );
	wxMutexLocker ml( m_torrent_queue_lock );
	int idx = find_torrent_by_hash( thash );
	if( idx < 0 )
	{
		wxLogError( _T( "MoveTorrentDown %s: Torrent not found in queue" ), torrent->name.c_str() );
		return ;
	}
	if( idx == ( m_torrent_queue.size() - 1 ) )
	{
		wxLogInfo( _T( "Torrent is at bottom position %d-%s" ), idx, torrent->name.c_str() );
		return;
	}

	long qindex = torrent->config->GetQIndex();
	if(qindex < 0)
	{
		wxLogInfo( _T( "Torrent is not in download que %d-%s" ), idx, torrent->name.c_str() );
		return;
	}

	wxLogInfo( _T( "%s: Moving Torrent Down" ), torrent->name.c_str() );

	//torrents_t::iterator torrent_it = m_torrent_queue.begin() + idx;
	size_t i = 0;
	long bottom_qindex = -1, next_qindex = -1;
	for(i = idx + 1; i < m_torrent_queue.size(); ++i)
	{
		next_qindex = m_torrent_queue[i]->config->GetQIndex();
		if( next_qindex < 0 )
		{
			wxLogInfo( _T( "Torrent is at queue end %d" ), i );
			break;
		}
		bottom_qindex = next_qindex;
	}

	if(bottom_qindex > idx)
	{
		m_torrent_queue.erase(m_torrent_queue.begin() + idx);
		m_torrent_queue.insert(m_torrent_queue.begin() + bottom_qindex, torrent);
		for(i = idx; i <= bottom_qindex; ++i)
		{
			m_torrent_queue[i]->config->SetQIndex( i );
			m_torrent_queue[i]->config->Save();
			m_running_torrent_map[wxString(m_torrent_queue[i]->hash)] = i;
		}
	}

	if(torrent->handle.is_valid())
	{
		torrent->handle.queue_position_bottom();
	}
}

void BitTorrentSession::MoveTorrentTop( std::shared_ptr<torrent_t>& torrent )
{
	wxMutexLocker ml( m_torrent_queue_lock );
	int idx = find_torrent_by_hash( wxString(torrent->hash));
	if( idx <= 0 )
	{
		if( idx == 0 )
		{
			wxLogInfo( _T( "Torrent is at top position %d-%s" ), idx, torrent->name.c_str() );
		}
		else
		{
			wxLogError( _T( "MoveTorrentUp %s: Torrent not found in queue" ), torrent->name.c_str() );
		}
		return ;
	}

	wxLogInfo( _T( "%s: Moving Torrent Up" ), torrent->name.c_str() );

	long qindex = torrent->config->GetQIndex();
	if(qindex < 0)
	{
		wxLogInfo( _T( "Torrent is not in download que %d-%s" ), idx, torrent->name.c_str() );
		return;
	}

	m_torrent_queue.erase(m_torrent_queue.begin() + idx);
	m_torrent_queue.insert(m_torrent_queue.begin(), torrent);
	for(size_t i = 0; i <= idx; ++i)
	{
		m_torrent_queue[i]->config->SetQIndex( i );
		m_torrent_queue[i]->config->Save();
		m_running_torrent_map[wxString(m_torrent_queue[i]->hash)] = i;
	}

	if(torrent->handle.is_valid())
	{
		torrent->handle.queue_position_top();
	}
//m_torrent_queue.erase( torrent_it );
//torrent_it = m_torrent_queue.begin() + idx;
//m_torrent_queue.insert( torrent_it, prev_torrent );
}

void BitTorrentSession::ReannounceTorrent( std::shared_ptr<torrent_t>& torrent )
{
	wxLogInfo( _T( "%s: Reannounce" ), torrent->name.c_str() );
	lt::torrent_handle &h = torrent->handle;

	if( h.is_valid() )
	{
		h.force_reannounce();
	}
}

void BitTorrentSession::RecheckTorrent( std::shared_ptr<torrent_t>& torrent )
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
		wxLogInfo( _T( "%s: Stop for checking" ), torrent->name.c_str() );
		StopTorrent( torrent );
	}

	wxString fastresumefile = wxGetApp().SaveTorrentsPath() + wxString(torrent->hash) + _T( ".resume" );

	if( wxFileExists( fastresumefile ) )
	{
		wxLogWarning( _T( "%s: Remove fast resume data" ), torrent->name.c_str() );

		if( ! wxRemoveFile( fastresumefile ) )
		{
			wxLogError( _T( "%s: Remove fast resume data: failed" ), torrent->name.c_str() );
		}
	}

	if( ( prev_state == TORRENT_STATE_START ) ||
			( prev_state == TORRENT_STATE_FORCE_START ) )
	{
		wxLogInfo( _T( "%s: Stop for checking" ), torrent->name.c_str() );
		StartTorrent( torrent, ( prev_state == TORRENT_STATE_FORCE_START ) );
	}
	else
		if( prev_state == TORRENT_STATE_PAUSE )
		{
			PauseTorrent( torrent );
		}
}

void BitTorrentSession::ConfigureTorrentFilesPriority( std::shared_ptr<torrent_t>& torrent )
{
	std::vector<lt::download_priority_t>& filespriority = torrent->config->GetFilesPriorities();
	wxASSERT(torrent->info);
	//XXX default priority is 4...
	// win32 4 triggers a assert

	int nfiles = torrent->info->num_files();
	wxASSERT(nfiles > 0);

	if( torrent->handle.is_valid() )
	{
		if (filespriority.size() != nfiles)
		{
			std::vector<lt::download_priority_t> deffilespriority( nfiles, TorrentConfig::file_normal );
			filespriority.swap(deffilespriority);
		}

		torrent->handle.prioritize_files(filespriority);
		bool set = torrent->config->GetEnableVideoPreview();

		if(set)
		{
			int piece_len = torrent->info->piece_length();
			std::vector<lt::download_priority_t> pp(torrent->handle.get_piece_priorities());
			const lt::file_storage &allfiles = torrent->info->files();
			bool ppchanged = false;

			wxFileName filename;
			for(int i = 0; i < nfiles; ++i)
			{
				lt::file_index_t idx(i);
				filename.Assign(wxString::FromUTF8((allfiles.file_name(idx)).to_string().c_str()));
				if(filespriority[i] != TorrentConfig::file_none && (m_preview_ext_set.count(filename.GetExt()) != 0))
				{
					const auto fileSize = allfiles.file_size(idx);
					const auto firstOffset = allfiles.file_offset(idx);
					int first = static_cast<int>(firstOffset / piece_len);
					int last = static_cast<int>((firstOffset + fileSize - 1) / piece_len);
					lt::download_priority_t prio(TorrentConfig::file_highest);
					int nNumPieces = ceil(fileSize * 0.01 / piece_len);
					for (int j = 0; j < nNumPieces; ++j)
					{
						pp[first + j] = prio;
						pp[last - j] = prio;
					}

					ppchanged = true;
				}
			}

			if(ppchanged)
				torrent->handle.prioritize_pieces(pp);
		}

		torrent->config->Save();
	}
}

void BitTorrentSession::ConfigureTorrentTrackers( std::shared_ptr<torrent_t>& torrent )
{
	auto & t_handle = torrent->handle;
	if( t_handle.is_valid() )
	{ t_handle.replace_trackers( torrent->config->GetTrackersURL() ); }
}

void BitTorrentSession::ConfigureTorrent( std::shared_ptr<torrent_t>& torrent )
{
	auto &h = torrent->handle;
	WXLOGDEBUG(( _T( "%s: Configure" ), torrent->name.c_str() ));

	if(torrent->info && torrent->info->is_valid())
	{
		wxASSERT(h.is_valid());
		std::string existdir = h.status(lt::torrent_handle::query_save_path).save_path;
		wxFileName oldpath, newpath;
		oldpath.AssignDir(wxString::FromUTF8( existdir.c_str() ) +
						   wxString( wxConvUTF8.cMB2WC( torrent->info->name().c_str() ) ));
		newpath.AssignDir(torrent->config->GetDownloadPath() + wxString( wxConvUTF8.cMB2WC( torrent->info->name().c_str() ) ));
		newpath.MakeAbsolute();
		oldpath.MakeAbsolute();

#if defined(__WXMSW__)
		if((oldpath.GetPath()).CmpNoCase(newpath.GetPath()) != 0)
#else
		if( oldpath.GetPath() != newpath.GetPath() )
#endif
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

			WXLOGDEBUG(( _T( "Old path %s, new path %s" ), oldpath.GetPath().c_str(), newpath.GetPath().c_str() ));

			if(oldpath.DirExists())
			{
				copysuccess = ( CopyDirectory( oldpath.GetPath( wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR ), newpath.GetPath( wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR ) ) != 0 );

				if( copysuccess )
				{
					oldpath.Rmdir();
				}
				else
					wxMessageBox( wxString::Format(_( "Failed to copy %s to %s!" ), oldpath.GetFullPath().c_str(), newpath.GetFullPath().c_str()), _( "Error" ),  wxOK | wxICON_ERROR );
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
				wxLogWarning( _T( "Old path not exists %s" ), oldpath.GetFullPath().c_str() );
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
				}
			}
			else //failed copy, restore path
			{
				wxLogError( _T( "Failed moving save path %s" ), torrent->config->GetDownloadPath().c_str() );
				wxLogWarning( _T( "Restoring download path %s" ), oldpath.GetFullPath().c_str() );

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
		WXLOGDEBUG(( _T( "%s: Upload Limit %s download limit %s, max uploads %s, max connections %s, ratio %lf" ),
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
	std::shared_ptr<torrent_t> torrent = GetTorrent( idx );
	if(torrent)
	{
		RemoveTorrent( torrent, deletedata);
	}
}

void BitTorrentSession::StartTorrent( int idx, bool force )
{
	std::shared_ptr<torrent_t> torrent = GetTorrent( idx );
	if(torrent && torrent->isvalid)
	{
		StartTorrent( torrent, force );
	}
}

void BitTorrentSession::StopTorrent( int idx )
{
	std::shared_ptr<torrent_t> torrent = GetTorrent( idx );
	if(torrent)
		StopTorrent( torrent );
}

void BitTorrentSession::QueueTorrent( int idx )
{
	std::shared_ptr<torrent_t> torrent = GetTorrent( idx );
	if(torrent && torrent->isvalid)
	{
		QueueTorrent( torrent );
	}
}

void BitTorrentSession::PauseTorrent( int idx )
{
	std::shared_ptr<torrent_t> torrent = GetTorrent( idx );
	if(torrent)
		PauseTorrent( torrent );
}

void BitTorrentSession::MoveTorrentUp( int idx )
{
	if( idx == 0 )
	{
		wxLogInfo( _T( "Torrent is at top position %d" ), idx );
		return;
	}
	std::shared_ptr<torrent_t> torrent = GetTorrent( idx );
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
			wxLogInfo( _T( "Torrent is at bottom position %d" ), idx );
			return;
		}
	}

	std::shared_ptr<torrent_t> torrent = GetTorrent( idx );
	if(torrent)
		MoveTorrentDown( torrent );
}

void BitTorrentSession::ReannounceTorrent( int idx )
{
	std::shared_ptr<torrent_t> torrent = GetTorrent( idx );
	if(torrent)
		ReannounceTorrent( torrent );
}

void BitTorrentSession::RecheckTorrent( int idx )
{
	if( idx < 0 || idx >= m_torrent_queue.size())
	{
		wxLogError( _T( "%d: Invalid torrent index" ), idx );
		return ;
	}

	RecheckTorrent( m_torrent_queue[idx] );
}

void BitTorrentSession::ConfigureTorrentFilesPriority( int idx )
{
	std::shared_ptr<torrent_t> torrent = GetTorrent( idx );
	if(torrent)
		ConfigureTorrentFilesPriority( torrent );
}

void BitTorrentSession::ConfigureTorrentTrackers( int idx )
{
	std::shared_ptr<torrent_t> torrent = GetTorrent( idx );
	if(torrent)
		ConfigureTorrentTrackers( torrent );
}

void BitTorrentSession::ConfigureTorrent( int idx )
{
	std::shared_ptr<torrent_t> torrent = GetTorrent( idx );
	if(torrent)
		ConfigureTorrent( torrent );
}

bool BitTorrentSession::GetTorrentMagnetUri( int idx, wxString& magneturi )
{
	bool ok = false;
	std::shared_ptr<torrent_t> torrent = GetTorrent( idx );
	if(torrent)
	{
		if(torrent->isvalid)
		{
			wxASSERT(torrent->config);
			magneturi = torrent->config->GetTorrentMagnetUri();//wxString::FromUTF8((lt::make_magnet_uri(torrent->handle)).c_str());
			if(!magneturi.IsEmpty()) ok = true;
		}
	}
	return ok;
}

/* start or stop queued item based on several criteria
 * start:
 *  max number of torrent is  less than cfg->maxstart
 * XXX more criteria?
 *	start next torrent when download speed is less than x
 *
 * stop:
 *  share ratio is more than cfg->ratio
 * XXX more criteria?
 *	num of seed is less than x
 *	time elapsed is more than x
 */
void BitTorrentSession::CheckQueueItem()
{
	std::vector<int> start_torrents, queue_torrents;
	int idx = 0;
	int pause_count = 0;
	int start_count = 0;
	int i = 0;
	int maxstart = m_config->GetMaxStart();
	//WXLOGDEBUG((_T("CheckQueueItem %d"), m_torrent_queue.size());
	wxMutexLocker ml( m_torrent_queue_lock );

	for( idx = 0; idx < m_torrent_queue.size(); ++idx )
	{
		std::shared_ptr<torrent_t>& torrent = m_torrent_queue[idx];
		WXLOGDEBUG(( _T( "Processing torrent %s" ), torrent->name.c_str() ));
		wxASSERT(torrent);
		wxASSERT(torrent->config);
		switch( torrent->config->GetTorrentState() )
		{
		case TORRENT_STATE_START:
			{
				bool exseed = false;

				if( torrent->handle.is_valid() && torrent->info && torrent->info->is_valid())
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
							wxLogInfo( _T( "%s torrent finished" ), torrent->name.c_str() );
							StopTorrent( torrent );
							continue;
						}

						if(torrent->handle.max_connections() == torrent->config->GetTorrentMaxConnections())
						{
							torrent->handle.set_max_connections( torrent->config->GetTorrentMaxConnections() / 2 );
						}

						/* exclude seed from count as running job */
						if( m_config->GetExcludeSeed() )
						{ exseed = true; }
					}
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
				if ((!((m_torrent_queue[idx])->handle.is_valid()) && torrent->info && torrent->info->is_valid()))
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
		std::shared_ptr<torrent_t>& torrent = m_torrent_queue[start_torrents[i]];
		if(!torrent->handle.is_valid())
		{
			StartTorrent( torrent, false );
		}
		++i;
	}

	if( start_count > maxstart )
	{
		i = start_torrents.size() - 1;

		while( ( start_count > maxstart ) && ( i >= 0 ) )
		{
			if(m_queue_torrent_set.find(wxString(m_torrent_queue[start_torrents[i]]->hash)) == m_queue_torrent_set.end())
			{         
				QueueTorrent( m_torrent_queue[start_torrents[i]] );
				--start_count;
			}
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

void BitTorrentSession::HandleTorrentAlerts()
{
	wxString event_string;
	std::vector<lt::alert*> alerts;
	/*1--debug, 2--info, 3--warning, 4--error*/
	unsigned int log_severity = 2;

	m_libbtsession->pop_alerts(&alerts);
	for (auto & alert : alerts)
	{
		TORRENT_TRY
		{
			event_string.Empty();
			switch ((alert)->type())
			{
				case lt::add_torrent_alert::alert_type:
					if( lt::add_torrent_alert* p = lt::alert_cast<lt::add_torrent_alert>( alert ) )
					{
						if (p->error)
						{
							log_severity = 4;
							event_string << _T("Couldn't add torrent: ") <<  wxString::FromUTF8(p->message().c_str());
						}
						else
						{
							TORRENT_TRY
							{
								HandleTorrentAddAlert(p);
							}
							TORRENT_CATCH(std::exception& e)
							{
								wxLogError( _T("Exception: [add_torrent_alert]:%s"),  wxString::FromUTF8(e.what()).c_str() );
							}
							TORRENT_CATCH(...)
							{
								wxLogError( _T( "Unknown Exception: [%s]:#%d" ), __FUNCTION__, __LINE__ );
							}
							
							event_string << _T("Metadata: ") <<  wxString::FromUTF8(p->message().c_str());
						}
					}
					break;
				case lt::torrent_finished_alert::alert_type:
					if( lt::torrent_finished_alert* p = lt::alert_cast<lt::torrent_finished_alert>( alert ) )
					{
						auto st = (p->handle).status(lt::torrent_handle::query_name);
						event_string << st.name << _T(": ") << wxString::FromUTF8(p->message().c_str());
						wxASSERT(st.has_metadata == true);
						TORRENT_TRY
						{
							(p->handle).save_resume_data(lt::torrent_handle::save_info_dict);
						}
						TORRENT_CATCH(std::exception& e)
						{
							wxLogError( _T("Exception: [torrent_finished_alert]:%s"),  wxString::FromUTF8(e.what()).c_str() );
						}
						TORRENT_CATCH(...)
						{
							wxLogError( _T( "Unknown Exception: [%s]:#%d" ), __FUNCTION__, __LINE__ );
						}
						//(p->handle).set_max_connections(max_connections_per_torrent / 2);
					}
					break;
				case lt::save_resume_data_failed_alert::alert_type:
					if (lt::save_resume_data_failed_alert* p = lt::alert_cast<lt::save_resume_data_failed_alert>( alert ))
					{
						auto & h = p->handle;
						log_severity = 1;
						event_string << _T("FAILED TO SAVE RESUME DATA:: ") << wxString::FromUTF8(p->message().c_str());

						if (h.is_valid() && p->error != lt::errors::resume_data_not_modified)
						{
							TORRENT_TRY
							{
								log_severity = 4;
								InfoHash thash(h.info_hash());
								wxMutexLocker ml(m_torrent_queue_lock);
								auto it = m_running_torrent_map.find(wxString(thash));
								if (it != m_running_torrent_map.end())
								{
									m_libbtsession->remove_torrent(h);
									std::shared_ptr<torrent_t> torrent = BitTorrentSession::GetTorrent(it->second);
									wxASSERT(torrent);
									torrent->config->SetTorrentState(TORRENT_STATE_STOP);
									torrent->config->Save();
								}
							}							
							TORRENT_CATCH(std::exception& e)
							{
								wxLogError( _T("Exception: [save_resume_data_failed_alert]:%s"),  wxString::FromUTF8(e.what()).c_str() );
							}
							TORRENT_CATCH(...)
							{
								wxLogError( _T( "Unknown Exception: [%s]:#%d" ), __FUNCTION__, __LINE__ );
							}
						}
					}
					break;
				case lt::session_stats_alert::alert_type:
					if (lt::session_stats_alert* p = lt::alert_cast<lt::session_stats_alert>( alert ))
					{
						UpdateCounters(p->counters(), lt::duration_cast<lt::microseconds>(p->timestamp().time_since_epoch()).count());
						return;
					}
					break;
				case lt::torrent_paused_alert::alert_type:
					if (lt::torrent_paused_alert* p = lt::alert_cast<lt::torrent_paused_alert>( alert ))
					{
						auto& h = p->handle;
						if (h.is_valid())
						{
							TORRENT_TRY
							{
								auto st = h.status(lt::torrent_handle::query_name);
								if (st.has_metadata && !((st.state == lt::torrent_status::seeding) || (st.state == lt::torrent_status::finished)))
									h.save_resume_data(lt::torrent_handle::save_info_dict);
							}
							TORRENT_CATCH(std::exception& e)
							{
								wxLogError( _T("Exception: [torrent_paused_alert]:%s"),  wxString::FromUTF8(e.what()).c_str() );
							}
							TORRENT_CATCH(...)
							{
								wxLogError( _T( "Unknown Exception: [%s]:#%d" ), __FUNCTION__, __LINE__ );
							}
						}
						log_severity = 1;
						event_string << _T("Torrent paused: ") << wxString::FromUTF8(p->message().c_str());
					}
					break;
				case lt::metadata_received_alert::alert_type:
					if( lt::metadata_received_alert* p = lt::alert_cast<lt::metadata_received_alert>( alert ) )
					{
						event_string << _T( "Metadata: " ) << wxString::FromUTF8(p->message().c_str());
						HandleTorrentMetaDataAlert(p);
					}
					break;
				case lt::save_resume_data_alert::alert_type:
					if( lt::save_resume_data_alert *p = lt::alert_cast<lt::save_resume_data_alert>( alert ) )
					{
						SaveTorrentResumeData(p);
						auto& h = p->handle;
						if (h.is_valid())
						{
							TORRENT_TRY
							{
								InfoHash thash(h.info_hash());
								wxMutexLocker ml(m_torrent_queue_lock);
								auto it = m_running_torrent_map.find(wxString(thash));
								if (it != m_running_torrent_map.end())
								{
									std::shared_ptr<torrent_t> torrent = BitTorrentSession::GetTorrent(it->second);
									wxASSERT(torrent);
									if (torrent->config->GetTorrentState() == TORRENT_STATE_STOP && h.is_valid())
									{
										m_libbtsession->remove_torrent(h);
									}
								}
							}
								TORRENT_CATCH(std::exception& e)
							{
								wxLogError(_T("Exception: [save_resume_data_alert]:%s"), wxString::FromUTF8(e.what()).c_str());
							}
							TORRENT_CATCH(...)
							{
								wxLogError(_T("Unknown Exception: [%s]:#%d"), __FUNCTION__, __LINE__);
							}
						}

						event_string << h.status(lt::torrent_handle::query_name).name << _T(": ") << wxString::FromUTF8(p->message().c_str());
					}
					break;
				case lt::dht_stats_alert::alert_type:
					if (lt::dht_stats_alert* p = lt::alert_cast<lt::dht_stats_alert>(alert))
					{
						{
							wxMutexLocker ml(m_dht_status_lock);
							m_dht_changed = true;
							m_dht_active_requests = p->active_requests;
							m_dht_routing_table = p->routing_table;
						}
						log_severity = 1;
						event_string << _T("[") << (alert)->type() << _T("]") << wxString::FromUTF8((alert)->message().c_str());
							//if(event_string.IsEmpty()) return;
					}
					break;
				case lt::peer_ban_alert::alert_type:
					if( lt::peer_ban_alert* p = lt::alert_cast<lt::peer_ban_alert>( alert ) )
					{
						event_string << wxString::FromUTF8(p->message().c_str());
					}
					break;
				case lt::peer_error_alert::alert_type:
					if( lt::peer_error_alert* p = lt::alert_cast<lt::peer_error_alert>( alert ) )
					{
						log_severity = 4;
						event_string << _T( "Peer: " ) << lt::aux::identify_client_impl( p->pid ) << _T( ": " ) << wxString::FromUTF8(p->message().c_str());
					}
					break;
				case lt::peer_blocked_alert::alert_type:
					if( lt::peer_blocked_alert* p = lt::alert_cast<lt::peer_blocked_alert>( alert ) )
					{
						log_severity = 3;
						event_string << wxString::FromUTF8(p->message().c_str());
					}
					break;
				case lt::invalid_request_alert::alert_type:
					if( lt::invalid_request_alert* p = lt::alert_cast<lt::invalid_request_alert>( alert ) )
					{
						log_severity = 4;
						event_string << lt::aux::identify_client_impl( p->pid ) << _T( ": " ) << wxString::FromUTF8(p->message().c_str());
					}
					break;
				case lt::tracker_warning_alert::alert_type:
					if( lt::tracker_warning_alert* p = lt::alert_cast<lt::tracker_warning_alert>( alert ) )
					{
						log_severity = 3;
						event_string << _T( "Tracker: " ) << wxString::FromUTF8(p->message().c_str());
					}
					break;
				case lt::tracker_reply_alert::alert_type:
					if( lt::tracker_reply_alert* p = lt::alert_cast<lt::tracker_reply_alert>( alert ) )
					{
						event_string << _T( "Tracker:" ) << wxString::FromUTF8(p->message().c_str()) << p->num_peers << _T( " number of peers " );
					}
					break;
				case lt::url_seed_alert::alert_type:
					if( lt::url_seed_alert* p = lt::alert_cast<lt::url_seed_alert>( alert ) )
					{
						event_string << _T( "Url seed '" ) << p->server_url() << _T( "': " ) << wxString::FromUTF8(p->message().c_str());
					}
					break;
				case lt::listen_failed_alert::alert_type:
					if( lt::listen_failed_alert* p = lt::alert_cast<lt::listen_failed_alert>( alert ) )
					{
						log_severity = 4;
						event_string << _T( "Listen failed: " ) << wxString::FromUTF8(p->message().c_str());
					}
					break;
				case lt::portmap_error_alert::alert_type:
					if( lt::portmap_error_alert* p = lt::alert_cast<lt::portmap_error_alert>( alert ) )
					{
						log_severity = 4;
						event_string << _T( "Portmap error: " ) << wxString::FromUTF8(p->message().c_str());
					}
					break;
				case lt::portmap_alert::alert_type:
					if( lt::portmap_alert* p = lt::alert_cast<lt::portmap_alert>( alert ) )
					{
						event_string << _T( "Portmap: " ) << wxString::FromUTF8(p->message().c_str());
					}
					break;
				case lt::tracker_announce_alert::alert_type:
					if( lt::tracker_announce_alert* p = lt::alert_cast<lt::tracker_announce_alert>( alert ) )
					{
						event_string << _T( "Tracker: " ) << wxString::FromUTF8(p->message().c_str());
					}
					break;
				case lt::file_error_alert::alert_type:
					if( lt::file_error_alert* p = lt::alert_cast<lt::file_error_alert>( alert ) )
					{
						log_severity = 4;
						event_string << _T( "File error: " ) << wxString::FromUTF8(p->message().c_str());
					}
					break;
				case lt::hash_failed_alert::alert_type:
					if( lt::hash_failed_alert* p = lt::alert_cast<lt::hash_failed_alert>( alert ) )
					{
						log_severity = 4;
						event_string << _T( "Hash: " ) << wxString::FromUTF8(p->message().c_str());
					}
					break;
				case lt::metadata_failed_alert::alert_type:
					if( lt::metadata_failed_alert* p = lt::alert_cast<lt::metadata_failed_alert>( alert ) )
					{
						log_severity = 4;
						event_string << _T( "Metadata: " ) << wxString::FromUTF8(p->message().c_str());
					}
					break;
				case lt::fastresume_rejected_alert::alert_type:
					if( lt::fastresume_rejected_alert* p = lt::alert_cast<lt::fastresume_rejected_alert>( alert ) )
					{
						log_severity = 3;
						event_string << _T( "Fastresume: " ) << wxString::FromUTF8(p->message().c_str());
					}
					break;
				//case lt::stats_alert::alert_type:
				case lt::state_update_alert::alert_type:
				case lt::piece_finished_alert::alert_type:
				case lt::block_finished_alert::alert_type:
				case lt::block_downloading_alert::alert_type:
				case lt::udp_error_alert::alert_type:
				case lt::log_alert::alert_type:
				default:
					log_severity = 1;
					{
						event_string << _T("[") << (alert)->type() << _T("]") << wxString::FromUTF8(( alert )->message().c_str());
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
						wxLogInfo( _T("%s"), event_string.c_str() );
					}
					break;
				case 3:
					{
						wxLogWarning( _T("%s"), event_string.c_str() );
					}
					break;
				case 4:
					{
						wxLogError( _T("%s"), event_string.c_str() );
					}
					break;
				default:
					wxLogDebug( _T("%s"), event_string.c_str() );
					break;
			}
		}
		TORRENT_CATCH(std::exception& e)
		{
			wxLogError( _T("Exception: [%s]:%s"), __FUNCTION__, wxString::FromUTF8(e.what()).c_str() );
		}
		TORRENT_CATCH(...)
		{
			wxLogError( _T( "Unknown Exception: [%s]" ), __FUNCTION__ );
		}
	}
}

/*Check error before call it. So, assume no p->error*/
bool BitTorrentSession::HandleTorrentAddAlert(lt::add_torrent_alert * p)
{
	try
	{
		//std::stringstream hash_stream;
		//hash_stream << p->handle.info_hash();
		static int maxQindex = -1;
		InfoHash thash(p->handle.info_hash());
		wxString shash(thash);
		std::shared_ptr<torrent_t> torrent;
		wxMutexLocker ml( m_torrent_queue_lock );
		m_queue_torrent_set.erase(shash);
		int idx = find_torrent_by_hash( shash );

		if(idx >= 0)
		{
			torrent = m_torrent_queue.at(idx);
			//wxString torrent_backup = wxGetApp().SaveTorrentsPath() + torrent->hash + _T( ".torrent" );
			wxASSERT(p->handle.is_valid());
			torrent->handle = p->handle;
			lt::torrent_status st = (p->handle).status(lt::torrent_handle::query_name);
			if(st.has_metadata)
				p->handle.save_resume_data(lt::torrent_handle::save_info_dict | lt::torrent_handle::only_if_modified);
			torrent->isvalid = true;
			if((torrent->config->GetTorrentMagnetUri()).IsEmpty())
				torrent->config->SetTorrentMagnetUri(wxString::FromUTF8((lt::make_magnet_uri(p->handle)).c_str()));
			torrent->handle.unset_flags(lt::torrent_flags::auto_managed);
			torrent->handle.pause(lt::torrent_handle::graceful_pause);
			enum torrent_state state = ( enum torrent_state ) torrent->config->GetTorrentState();
			//wxLogInfo( _T( "HandleAddTorrentAlert %s" ), torrent->name.c_str() );

			if( torrent->config->GetQIndex() == -1 )
			{
				auto it = m_torrent_queue.rbegin();

				int qidx = 0, index = m_torrent_queue.size();
				while( it != m_torrent_queue.rend() )
				{
					int lastqidx = (*it)->config->GetQIndex();
					if(lastqidx != -1)
					{
						qidx = lastqidx + 1;
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

			if( state == TORRENT_STATE_FORCE_START || state == TORRENT_STATE_START )
			{
				StartTorrent( torrent, (state == TORRENT_STATE_FORCE_START) );
			}
			else if (state == TORRENT_STATE_PAUSE)
			{
				if (p->handle.is_valid())
				{
					p->handle.pause(lt::torrent_handle::graceful_pause);
				}
			}
		}
		else
		{
			if( p->handle.is_valid() )
			{
				wxLogError( _T("Torrent Add Alert: %s was not found in torrent list, maybe deleted?"), shash.c_str());
				p->handle.pause();
				m_libbtsession->remove_torrent( p->handle );
			}
		}
	}
	catch( std::exception& e )
	{
		wxLogFatalError( _T("Exception: [%s]:%s"), __FUNCTION__, wxString::FromUTF8(e.what()).c_str() );
		return false;
	}
	catch (...)
	{
		wxLogFatalError(_T("Unknown Exception: %s"), __FUNCTION__ );
		return false;
	}

	return true;
}

extern void TorrentMetadataNotify( );
bool BitTorrentSession::HandleTorrentMetaDataAlert(lt::metadata_received_alert *p)
{
	wxASSERT(p->handle.is_valid());
	p->handle.pause(lt::torrent_handle::graceful_pause);
	wxString hashstr(InfoHash(p->handle.info_hash()));

	wxMutexLocker ml( m_torrent_queue_lock );
	auto it = m_running_torrent_map.find(hashstr);
	if(it != m_running_torrent_map.end())
	{
		std::shared_ptr<torrent_t> torrent = GetTorrent(it->second);

		wxASSERT(torrent);
		if(torrent)
		{
			lt::torrent_status st = (p->handle).status(lt::torrent_handle::query_name);
			wxASSERT(st.has_metadata);
			p->handle.save_resume_data(lt::torrent_handle::save_info_dict);
			torrent->handle = p->handle;
			torrent->info.reset(new lt::torrent_info(*(p->handle.torrent_file())));
			torrent->name = st.name;

			{
				wxMutexLocker ml( m_metadata_queue_lock );
				m_metadata_queue.push_back(hashstr);
			}
			TorrentMetadataNotify();

			return true;
		}
		else
		{
			wxLogError(_T("Invalid torrent: %s"), __FUNCTION__ );
			m_running_torrent_map.erase(it);
		}
	}

	return false;
}

void BitTorrentSession::GetPendingMetadata(metadata_t & mdq)
{
	mdq.clear();
	wxMutexLocker ml( m_metadata_queue_lock );
	if(!m_metadata_queue.empty())
	{
		mdq.swap(m_metadata_queue);
		//m_metadata_queue.clear();
	}
}

void BitTorrentSession::PostStatusUpdate()
{
	m_libbtsession->post_torrent_updates();
	m_libbtsession->post_session_stats();
	m_libbtsession->post_dht_stats();
}

void BitTorrentSession::UpdateCounters(lt::span<std::int64_t const>& stats_counters, boost::uint64_t t)
{
	// only update the previous counters if there's been enough
	// time since it was last updated
	if (t - m_timestamp[1] > 2000000)
	{
		m_cnt[1].swap(m_cnt[0]);
		m_timestamp[1] = m_timestamp[0];
	}

	m_cnt[0].assign(stats_counters.begin(), stats_counters.end());
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

bool BitTorrentSession::DHTStatusToString(wxString & status)
{
	static wxString lastStatus;
	bool changed = false;
	wxMutexLocker ml(m_dht_status_lock);
	changed = m_dht_changed;
	if (m_dht_changed == true)
	{
        int bucket = 0;
		lastStatus = wxT("\n");
		for (lt::dht_routing_bucket const& n : m_dht_routing_table)
		{
			static char const* progress_bar =
				"################################"
				"################################"
				"################################"
				"################################";
			static char const* short_progress_bar = "--------";
			lastStatus += wxString::Format(wxT("%3d [%3d, %d] %s%s\n")
				, bucket, n.num_nodes, n.num_replacements
				, progress_bar + (128 - n.num_nodes)
				, short_progress_bar + (8 - (std::min)(8, n.num_replacements)));
		}

		for (lt::dht_lookup const& l : m_dht_active_requests)
		{
			lastStatus += wxString::Format(wxT("  %10s target: %s "
				"[limit: %2d] "
				"in-flight: %-2d "
				"left: %-3d "
				"1st-timeout: %-2d "
				"timeouts: %-2d "
				"responses: %-2d "
				"last_sent: %-2d\n")
				, l.type
				, to_hex(l.target).c_str()
				, l.branch_factor
				, l.outstanding_requests
				, l.nodes_left
				, l.first_timeout
				, l.timeouts
				, l.responses
				, l.last_sent);
		}

		m_dht_changed = false;
		status = lastStatus;
	}

	return changed;
}

