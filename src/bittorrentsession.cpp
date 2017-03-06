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
#include <wx/wx.h>
#include <wx/app.h>
#include <wx/dir.h>
#include <wx/log.h>

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

#ifdef TORRENT_DISABLE_RESOLVE_COUNTRIES
	#error you must not disable RESOLVE COUNTRIES
#endif

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

using namespace libtorrent;

BitTorrentSession::BitTorrentSession( wxApp* pParent, Configuration* config )
	: wxThread( wxTHREAD_JOINABLE ), m_upnp_started( false ), m_natpmp_started( false ), m_lsd_started( false ), m_libbtsession( 0 )
{
	m_pParent = pParent;
	m_config = config;
	//boost::filesystem::path::default_name_check(boost::filesystem::no_check);
}

BitTorrentSession::~BitTorrentSession()
{
}

static const char PEER_ID[] = "SW";

void *BitTorrentSession::Entry()
{
	session_settings btsetting;
	wxLogDebug( _T( "BitTorrentSession Thread Entry pid 0x%lx priority %u.\n" ), GetId(), GetPriority() );
	
#if LIBTORRENT_VERSION_NUM < 10100
	m_libbtsession = new session( fingerprint( "sw", BITSWASH_FINGERPRINT_VERSION ) );
	//XXX: session_settings contain lots more like proxy and misc
	btsetting.user_agent = wxGetApp().UserAgent().mb_str( wxConvUTF8 );
	btsetting.tracker_completion_timeout = m_config->GetTrackerCompletionTimeout();
	btsetting.tracker_receive_timeout = m_config->GetTrackerReceiveTimeout();
	btsetting.stop_tracker_timeout = m_config->GetStopTrackerTimeout();
	btsetting.tracker_maximum_response_length = m_config->GetTrackerMaximumResponseLength();
	btsetting.piece_timeout = m_config->GetPieceTimeout();
	btsetting.request_queue_time = m_config->GetRequestQueueTime();
	btsetting.max_allowed_in_request_queue = m_config->GetMaxAllowedInRequestQueue();
	btsetting.max_out_request_queue = m_config->GetMaxOutRequestQueue();
	btsetting.whole_pieces_threshold = m_config->GetWholePiecesThreshold();
	btsetting.peer_timeout = m_config->GetPeerTimeout();
	btsetting.urlseed_timeout = m_config->GetUrlseedTimeout();
	btsetting.urlseed_pipeline_size = m_config->GetUrlseedPipelineSize();
	btsetting.file_pool_size = m_config->GetFilePoolSize();
	btsetting.allow_multiple_connections_per_ip = m_config->GetAllowMultipleConnectionsPerIP();
	btsetting.max_failcount = m_config->GetMaxFailCount();
	btsetting.min_reconnect_time = m_config->GetMinReconnectTime();
	btsetting.peer_connect_timeout = m_config->GetPeerConnectTimeout();
	btsetting.ignore_limits_on_local_network = m_config->GetIgnoreLimitsOnLocalNetwork();
	btsetting.connection_speed = m_config->GetConnectionSpeed();
	btsetting.send_redundant_have = m_config->GetSendRedundantHave();
	btsetting.lazy_bitfields = m_config->GetLazyBitfields();
	btsetting.inactivity_timeout = m_config->GetInactivityTimeout();
	btsetting.unchoke_interval = m_config->GetUnchokeInterval();
	//  btsetting.optimistic_unchoke_multiplier = m_config->GetOptimisticUnchokeMultiplier();
	btsetting.num_want = m_config->GetNumWant();
	btsetting.initial_picker_threshold = m_config->GetInitialPickerThreshold();
	btsetting.allowed_fast_set_size = m_config->GetAllowedFastSetSize();
	//  btsetting.max_outstanding_disk_bytes_per_connection = m_config->GetMaxOutstandingDiskBytesPerConnection();
	btsetting.handshake_timeout = m_config->GetHandshakeTimeout();
	btsetting.use_dht_as_fallback = m_config->GetUseDhtAsFallback();
	btsetting.free_torrent_hashes = m_config->GetFreeTorrentHashes();
	btsetting.announce_to_all_tiers = true;
	btsetting.announce_to_all_trackers = true;

	
    btsetting.upnp_ignore_nonrouters = true;
    btsetting.use_dht_as_fallback = false;
    // Disable support for SSL torrents for now
    //btsetting.ssl_listen = 0;
    // To prevent ISPs from blocking seeding
    btsetting.lazy_bitfields = true;
    // Speed up exit
    //btsetting.stop_tracker_timeout = 1;
    btsetting.auto_scrape_interval = 1200; // 20 minutes
    btsetting.auto_scrape_min_interval = 900; // 15 minutes
    //btsetting.connection_speed = 20; // default is 10
    btsetting.no_connect_privileged_ports = false;
    btsetting.seed_choking_algorithm = libtorrent::session_settings::fastest_upload;
	m_libbtsession->set_settings( btsetting );
#else
	libtorrent::settings_pack pack;

    Configure(pack);
    m_libbtsession = new libtorrent::session(pack, 0);
	wxASSERT(m_libbtsession != 0);

	if( m_config->GetDHTEnabled() )
	{
		entry dht_state;
		long dhtport = m_config->GetDHTPort();
		wxString dhtstatefile = wxGetApp().DHTStatePath();
		struct dht_settings DHTSettings;

		//XXX set other dht settings
		if( dhtport < 1 || dhtport > 65535 )
		{ DHTSettings.service_port = dhtport; }
		else //use TCP port
		{ DHTSettings.service_port = m_config->GetPortMax(); }

		DHTSettings.max_peers_reply = m_config->GetDHTMaxPeers();
		DHTSettings.search_branching = m_config->GetDHTSearchBranching();
		DHTSettings.max_fail_count = m_config->GetDHTMaxFail();
		m_libbtsession->set_dht_settings( DHTSettings );

		if( wxFileExists( dhtstatefile ) )
		{
			wxLogInfo( _T( "Restoring previous DHT state \n" ) + dhtstatefile );
			std::ifstream in( ( const char* )dhtstatefile.mb_str( wxConvFile ), std::ios_base::binary );
			in.unsetf( std::ios_base::skipws );

			try
			{
				dht_state = bdecode( std::istream_iterator<char>( in ), std::istream_iterator<char>() );
			}
			catch( std::exception& e )
			{
				wxLogWarning( _T( "Unable to restore dht state file %s - %s\n" ), dhtstatefile.c_str(), wxString::FromAscii( e.what() ).c_str() );
			}
		}
		else
		{
			wxLogWarning( _T( "Previous DHT state not found \n" ) + dhtstatefile );
		}
	}
#endif

	ConfigureSession();
	ScanTorrentsDirectory( wxGetApp().SaveTorrentsPath() );
	( ( BitSwash* )m_pParent )->BTInitDone();

	while( 1 )
	{
		//DumpTorrents();
		if( TestDestroy() )
		{ break; }

		wxThread::Sleep( 1000 );
		CheckQueueItem();
	}

	Exit( 0 );
	return NULL;
}

void BitTorrentSession::OnExit()
{
	//XXX have 1 soon
	//
	//wxCriticalSectionLocker locker(wxGetApp().m_btcritsect);
	//save DHT entry
	wxLogDebug( _T( "BitTorrent Session exiting\n" ) );

	if( m_config->GetDHTEnabled() )
	{
		wxString dhtstatefile = wxGetApp().DHTStatePath();

		try
		{
			entry dht_state = m_libbtsession->dht_state();
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
	std::deque<alert*> alerts;
	m_libbtsession->pop_alerts( &alerts );
	delete m_libbtsession;
	{
		wxMutexLocker ml( m_torrent_queue_lock );
		m_torrent_queue.empty();
		m_torrent_queue.swap( m_torrent_queue );
	}
}

void BitTorrentSession::Configure(libtorrent::settings_pack &settingsPack)
{
	// Set severity level of libtorrent session
	int alertMask = libtorrent::alert::error_notification
		| libtorrent::alert::peer_notification
		| libtorrent::alert::port_mapping_notification
		| libtorrent::alert::storage_notification
		| libtorrent::alert::tracker_notification
		| libtorrent::alert::status_notification
		| libtorrent::alert::ip_block_notification
		| libtorrent::alert::progress_notification
		| libtorrent::alert::stats_notification;

	std::string peerId = libtorrent::generate_fingerprint(PEER_ID, BITSWASH_FINGERPRINT_VERSION);
	settingsPack.set_str( libtorrent::settings_pack::user_agent, wxGetApp().UserAgent().ToStdString( ));

	// Speed limit
	//const bool altSpeedLimitEnabled = isAltGlobalSpeedLimitEnabled();
	//settingsPack.set_int(libtorrent::settings_pack::download_rate_limit, altSpeedLimitEnabled ? altGlobalDownloadSpeedLimit() : globalDownloadSpeedLimit());
	//settingsPack.set_int(libtorrent::settings_pack::upload_rate_limit, altSpeedLimitEnabled ? altGlobalUploadSpeedLimit() : globalUploadSpeedLimit());
	settingsPack.set_int( libtorrent::settings_pack::upload_rate_limit, m_config->GetGlobalUploadLimit() );
	settingsPack.set_int( libtorrent::settings_pack::download_rate_limit, m_config->GetGlobalDownloadLimit() );
	// * Max Half-open connections
	settingsPack.set_int(libtorrent::settings_pack::half_open_limit, m_config->GetGlobalMaxHalfConnect());
	// * Max connections limit
	settingsPack.set_int(libtorrent::settings_pack::connections_limit, m_config->GetGlobalMaxConnections());
	// * Global max upload slots
	settingsPack.set_int(libtorrent::settings_pack::unchoke_slots_limit, m_config->GetGlobalMaxUploads());
	// uTP

	settingsPack.set_int( libtorrent::settings_pack::tracker_completion_timeout, m_config->GetTrackerCompletionTimeout());
	settingsPack.set_int( libtorrent::settings_pack::tracker_receive_timeout, m_config->GetTrackerReceiveTimeout());
	settingsPack.set_int( libtorrent::settings_pack::stop_tracker_timeout, m_config->GetStopTrackerTimeout());
	settingsPack.set_int( libtorrent::settings_pack::tracker_maximum_response_length, m_config->GetTrackerMaximumResponseLength());
	settingsPack.set_int( libtorrent::settings_pack::piece_timeout, m_config->GetPieceTimeout());
	settingsPack.set_int( libtorrent::settings_pack::request_queue_time, m_config->GetRequestQueueTime());
	settingsPack.set_int( libtorrent::settings_pack::max_allowed_in_request_queue, m_config->GetMaxAllowedInRequestQueue());
	settingsPack.set_int( libtorrent::settings_pack::max_out_request_queue, m_config->GetMaxOutRequestQueue());
	settingsPack.set_int( libtorrent::settings_pack::whole_pieces_threshold, m_config->GetWholePiecesThreshold());
	settingsPack.set_int( libtorrent::settings_pack::peer_timeout, m_config->GetPeerTimeout());
	settingsPack.set_int( libtorrent::settings_pack::urlseed_timeout, m_config->GetUrlseedTimeout());
	settingsPack.set_int( libtorrent::settings_pack::urlseed_pipeline_size, m_config->GetUrlseedPipelineSize());
	settingsPack.set_int( libtorrent::settings_pack::file_pool_size, m_config->GetFilePoolSize());
	settingsPack.set_bool( libtorrent::settings_pack::allow_multiple_connections_per_ip, m_config->GetAllowMultipleConnectionsPerIP());
	settingsPack.set_int( libtorrent::settings_pack::max_failcount, m_config->GetMaxFailCount());
	settingsPack.set_int( libtorrent::settings_pack::min_reconnect_time, m_config->GetMinReconnectTime());
	settingsPack.set_int( libtorrent::settings_pack::peer_connect_timeout, m_config->GetPeerConnectTimeout());
	// Ignore limits on LAN
	settingsPack.set_bool( libtorrent::settings_pack::ignore_limits_on_local_network, m_config->GetIgnoreLimitsOnLocalNetwork());
	settingsPack.set_int( libtorrent::settings_pack::connection_speed, m_config->GetConnectionSpeed());
	settingsPack.set_bool( libtorrent::settings_pack::send_redundant_have, m_config->GetSendRedundantHave());
	settingsPack.set_bool( libtorrent::settings_pack::lazy_bitfields, m_config->GetLazyBitfields());
	settingsPack.set_int( libtorrent::settings_pack::inactivity_timeout, m_config->GetInactivityTimeout());
	settingsPack.set_int( libtorrent::settings_pack::unchoke_interval, m_config->GetUnchokeInterval());
	//	settingsPack.set_int( libtorrent::settings_pack::optimistic_unchoke_multiplier, m_config->GetOptimisticUnchokeMultiplier());
	settingsPack.set_int( libtorrent::settings_pack::num_want, m_config->GetNumWant());
	settingsPack.set_int( libtorrent::settings_pack::initial_picker_threshold, m_config->GetInitialPickerThreshold());
	settingsPack.set_int( libtorrent::settings_pack::allowed_fast_set_size, m_config->GetAllowedFastSetSize());
	//	settingsPack.set_int( libtorrent::settings_pack::max_outstanding_disk_bytes_per_connection, m_config->GetMaxOutstandingDiskBytesPerConnection());
	settingsPack.set_int( libtorrent::settings_pack::handshake_timeout, m_config->GetHandshakeTimeout());
	settingsPack.set_bool( libtorrent::settings_pack::use_dht_as_fallback, m_config->GetUseDhtAsFallback());
	//settingsPack.set_bool( libtorrent::settings_pack::free_torrent_hashes, m_config->GetFreeTorrentHashes());

	//Todo: new options=======================================
	settingsPack.set_bool( libtorrent::settings_pack::announce_to_all_tiers, true);
	settingsPack.set_bool( libtorrent::settings_pack::announce_to_all_trackers, true);

	libtorrent::settings_pack::io_buffer_mode_t mode = /*useOSCache() ? */libtorrent::settings_pack::enable_os_cache;
                                                              //: libtorrent::settings_pack::disable_os_cache;
	settingsPack.set_int(libtorrent::settings_pack::disk_io_read_mode, mode);
	settingsPack.set_int(libtorrent::settings_pack::disk_io_write_mode, mode);
	settingsPack.set_bool(libtorrent::settings_pack::anonymous_mode, false);

	
    // The most secure, rc4 only so that all streams are encrypted
    settingsPack.set_int(libtorrent::settings_pack::allowed_enc_level, ( libtorrent::pe_settings::enc_level )( m_config->GetEncLevel()));
    settingsPack.set_bool(libtorrent::settings_pack::prefer_rc4, m_config->GetEncEnabled());
	
	if( m_config->GetEncEnabled() )
	{
		settingsPack.set_int(libtorrent::settings_pack::out_enc_policy,  (libtorrent::pe_settings::enc_policy )( m_config->GetEncPolicy()));
		settingsPack.set_int(libtorrent::settings_pack::in_enc_policy, ( libtorrent::pe_settings::enc_policy )( m_config->GetEncPolicy()));
		wxLogMessage( _T( "Encryption enabled, policy %d\n" ), m_config->GetEncPolicy() );
	}
	else
	{
		settingsPack.set_int(libtorrent::settings_pack::out_enc_policy, libtorrent::settings_pack::pe_disabled);
		settingsPack.set_int(libtorrent::settings_pack::in_enc_policy, libtorrent::settings_pack::pe_disabled);
		wxLogMessage( _T( "Encryption is disabled\n" ) );
	}
    //case 1: // Forced
    //    settingsPack.set_int(libtorrent::settings_pack::out_enc_policy, libtorrent::settings_pack::pe_forced);
    //    settingsPack.set_int(libtorrent::settings_pack::in_enc_policy, libtorrent::settings_pack::pe_forced);
	/*
	if (isQueueingSystemEnabled()) {
        adjustLimits(settingsPack);

        settingsPack.set_int(libtorrent::settings_pack::active_seeds, maxActiveUploads());
        settingsPack.set_bool(libtorrent::settings_pack::dont_count_slow_torrents, ignoreSlowTorrentsForQueueing());
    }
    else {
        settingsPack.set_int(libtorrent::settings_pack::active_downloads, -1);
        settingsPack.set_int(libtorrent::settings_pack::active_seeds, -1);
        settingsPack.set_int(libtorrent::settings_pack::active_limit, -1);
    }
	*/
	// 1 active torrent force 2 connections. If you have more active torrents * 2 than connection limit,
	// connection limit will get extended. Multiply max connections or active torrents by 10 for queue.
	// Ignore -1 values because we don't want to set a max int message queue
	//settingsPack.set_int(libtorrent::settings_pack::alert_queue_size, std::max(1000,
	//	10 * std::max(maxActiveTorrents() * 2, maxConnections())));
	
	// Outgoing ports
	//settingsPack.set_int(libtorrent::settings_pack::outgoing_port, m_config->GetPortMin());
	//settingsPack.set_int(libtorrent::settings_pack::num_outgoing_ports, m_config->GetPortMax() - m_config->GetPortMin() + 1);
	
	// Include overhead in transfer limits
	settingsPack.set_bool(libtorrent::settings_pack::rate_limit_ip_overhead, true);
	// IP address to announce to trackers
	//settingsPack.set_str(libtorrent::settings_pack::announce_ip, Utils::String::toStdString(announceIP()));
	// Super seeding
	settingsPack.set_bool(libtorrent::settings_pack::strict_super_seeding, false);
	settingsPack.set_bool(libtorrent::settings_pack::enable_incoming_utp, true);
	settingsPack.set_bool(libtorrent::settings_pack::enable_outgoing_utp, true);
	// uTP rate limiting
	//settingsPack.set_bool(libtorrent::settings_pack::rate_limit_utp, isUTPRateLimited());
	//settingsPack.set_int(libtorrent::settings_pack::mixed_mode_algorithm, isUTPRateLimited()
	//					 ? libtorrent::settings_pack::prefer_tcp
	//					 : libtorrent::settings_pack::peer_proportional);
	
	//settingsPack.set_bool(libtorrent::settings_pack::apply_ip_filter_to_trackers, isTrackerFilteringEnabled());

	//Todo: new options=======================================

    settingsPack.set_int(libtorrent::settings_pack::active_tracker_limit, -1);
    settingsPack.set_int(libtorrent::settings_pack::active_dht_limit, -1);
    settingsPack.set_int(libtorrent::settings_pack::active_lsd_limit, -1);

	settingsPack.set_int(libtorrent::settings_pack::alert_mask, alertMask);
	settingsPack.set_str(libtorrent::settings_pack::peer_fingerprint, peerId);
	settingsPack.set_bool(libtorrent::settings_pack::listen_system_port_fallback, false);
	settingsPack.set_bool(libtorrent::settings_pack::upnp_ignore_nonrouters, true);
	settingsPack.set_bool(libtorrent::settings_pack::use_dht_as_fallback, false);
	// Disable support for SSL torrents for now
	settingsPack.set_int(libtorrent::settings_pack::ssl_listen, 0);
	// To prevent ISPs from blocking seeding
	// Speed up exit
	settingsPack.set_int(libtorrent::settings_pack::auto_scrape_interval, 1200); // 20 minutes
	settingsPack.set_int(libtorrent::settings_pack::auto_scrape_min_interval, 900); // 15 minutes
	settingsPack.set_bool(libtorrent::settings_pack::no_connect_privileged_ports, false);
	settingsPack.set_int(libtorrent::settings_pack::seed_choking_algorithm, libtorrent::settings_pack::fastest_upload);

	if( m_config->GetDHTEnabled() )
	{
		settingsPack.set_str(libtorrent::settings_pack::dht_bootstrap_nodes, "dht.libtorrent.org:25401,router.bittorrent.com:6881,router.utorrent.com:6881,dht.transmissionbt.com:6881,dht.aelitis.com:6881");
	}
	settingsPack.set_bool(libtorrent::settings_pack::enable_lsd, m_config->GetEnableLsd());
}

void BitTorrentSession::ConfigureSession()
{
	wxASSERT( m_libbtsession != NULL );
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
	m_libbtsession->set_severity_level( ( alert::severity_t )m_config->GetLogSeverity() );
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

	error_code ec;
	m_libbtsession->listen_on( std::make_pair( portmin, portmax ), ec,
							   m_config->m_local_ip.IPAddress().ToAscii() );
}

void BitTorrentSession::StartExtensions()
{
	wxASSERT( m_libbtsession != NULL );

	/* XXX no way to remove extension currently
	 * restart is needed
	 */
	if( m_config->GetEnableMetadata() )
	{ m_libbtsession->add_extension( &libtorrent::create_ut_metadata_plugin ); }

	if( m_config->GetEnablePex() )
	{ m_libbtsession->add_extension( &libtorrent::create_ut_pex_plugin ); }

	m_libbtsession->add_extension( &libtorrent::create_smart_ban_plugin );
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
			settings_pack settingsPack = m_libbtsession->get_settings();
			settingsPack.set_bool( settings_pack::enable_upnp, true );
			m_libbtsession->apply_settings( settingsPack );
			m_upnp_started = true;
		}
	}
	else
	{
		settings_pack settingsPack = m_libbtsession->get_settings();
		settingsPack.set_bool( settings_pack::enable_upnp, false );
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
			settings_pack settingsPack = m_libbtsession->get_settings();
			settingsPack.set_bool( settings_pack::enable_natpmp, true );
			m_libbtsession->apply_settings( settingsPack );
		}

		m_natpmp_started = true;
	}
	else
	{
		settings_pack settingsPack = m_libbtsession->get_settings();
		settingsPack.set_bool( settings_pack::enable_natpmp, false );
		m_libbtsession->apply_settings( settingsPack );
		m_natpmp_started = false;
	}
}

void BitTorrentSession::AddTorrentSession( shared_ptr<torrent_t>& torrent )
{
	torrent_handle &handle = torrent->handle;
	shared_ptr<torrent_info> t = torrent->info;
	wxLogDebug( _T( "AddTorrent %s into session\n" ), torrent->name.c_str() );
	wxString fastresumefile = wxGetApp().SaveTorrentsPath() + wxGetApp().PathSeparator() + torrent->hash + _T( ".fastresume" );
	entry resume_data;

	if( wxFileExists( fastresumefile ) )
	{
		wxLogInfo( _T( "%s: Read fast resume data\n" ), torrent->name.c_str() );
		std::ifstream fastresume_in( ( const char* )fastresumefile.mb_str( wxConvFile ), std::ios_base::binary );
		fastresume_in.unsetf( std::ios_base::skipws );

		try
		{
			resume_data = bdecode( std::istream_iterator<char>( ( fastresume_in ) ), std::istream_iterator<char>() );
		}
		catch( std::exception& e )
		{
			wxLogError( wxString::FromAscii( e.what() ) + _T( "\n" ) );
		}
	}

	try
	{
		//XXX libtorrent updated with 3 mode
		//sparse (default)
		//allocate
		//compact
		//
		libtorrent::add_torrent_params p;
		p.ti = t;
		p.save_path = ( const char* )torrent->config->GetDownloadPath().mb_str( wxConvUTF8 );

		if( resume_data.type() != entry::undefined_t )
		{ bencode( std::back_inserter( p.resume_data ), resume_data ); }

		//p.resume_data = resume_data.preformatted();
		p.paused = ( torrent->config->GetTorrentState() != TORRENT_STATE_PAUSE );
		p.duplicate_is_error = true;
		p.auto_managed = true;

		/*if( torrent->config->GetTorrentCompactAlloc() )
		{
			wxLogInfo( _T( "%s: Compact allocation mode\n" ), torrent->name.c_str() );
			p.storage_mode = libtorrent::storage_mode_compact;
			handle = m_libbtsession->add_torrent( p );
		}
		else*/
		{
			wxString strStorageMode;
			enum libtorrent::storage_mode_t eStorageMode = torrent->config->GetTorrentStorageMode();

			switch( eStorageMode )
			{
			case libtorrent::storage_mode_allocate:
				strStorageMode = _( "Full" );
				break;

			case libtorrent::storage_mode_sparse:
				strStorageMode = _( "Sparse" );
				break;

			/*case libtorrent::storage_mode_compact:
				strStorageMode = _( "Compact" );
				break;*/

			default:
				eStorageMode = libtorrent::storage_mode_sparse;
				strStorageMode = _( "Sparse" );
				break;
			}

			p.storage_mode = eStorageMode;
			wxLogInfo( _T( "%s: %s allocation mode\n" ), torrent->name.c_str(), strStorageMode.c_str() );
			handle = m_libbtsession->add_torrent( p );
		}

		handle.resolve_countries( true );
		enum torrent_state state = ( enum torrent_state ) torrent->config->GetTorrentState();
		//handle.pause();
		//ConfigureTorrent(torrent);
	}
	catch( std::exception& e )
	{
		//posibly duplicate torrent
		wxLogError( wxString::FromAscii( e.what() ) + _T( "\n" ) );
	}
}

bool BitTorrentSession::AddTorrent( shared_ptr<torrent_t>& torrent )
{
	wxLogDebug( _T( "Add Torrent %s\n" ),  torrent->name.c_str() );

	try
	{
		if( find_torrent_from_hash( torrent->hash ) >= 0 )
		{
			wxLogWarning( _T( "Torrent %s already exists\n" ), torrent->name.c_str() );
			return false;
		}

		enum torrent_state state = ( enum torrent_state ) torrent->config->GetTorrentState();

		//XXX redundant ? StartTorrent below will call the same thing
		if( ( state == TORRENT_STATE_START ) ||
				( state == TORRENT_STATE_FORCE_START ) ||
				( state == TORRENT_STATE_PAUSE ) )
		{
			AddTorrentSession( torrent );
		}

		if( torrent->config->GetQIndex() == -1 )
		{
			torrents_t::const_reverse_iterator i = m_torrent_queue.rbegin();

			if( i == m_torrent_queue.rend() )
			{
				torrent->config->SetQIndex( 0 );
			}
			else
			{
				shared_ptr<torrent_t> last_handle = *i;
				torrent->config->SetQIndex( last_handle->config->GetQIndex() + 1 );
			}
		}

	 	bool nopriority = false;
		wxULongLong_t total_selected = 0;
		libtorrent::file_entry f_entry;
		libtorrent::torrent_info const& torrent_info = *(torrent->info);
		std::vector<int> filespriority = torrent->config->GetFilesPriorities();

		if (filespriority.size() != torrent_info.num_files())
		{
			nopriority = true;
		}
		/*0--unchecked_xpm*/
		/*1--checked_xpm*/
		/*2--unchecked_dis_xpm*/
		/*3--checked_dis_xpm*/

		for(int i = 0; i < torrent_info.num_files(); ++i)
		{
			if (nopriority || filespriority[i] != BITTORRENT_FILE_NONE)
			{
				f_entry = torrent_info.file_at( i );
				total_selected += f_entry.size;
			}
		}
		torrent->config->SetSelectedSize(total_selected);

		{
			wxMutexLocker ml( m_torrent_queue_lock );
			m_torrent_queue.push_back( torrent );
			m_torrent_map.insert( std::pair<wxString, int>( torrent->hash, m_torrent_queue.size() - 1 ) );
		}

		if( state == TORRENT_STATE_FORCE_START || state == TORRENT_STATE_START )
		{
			StartTorrent( torrent, ( state == TORRENT_STATE_FORCE_START ) );
		}
	}
	catch( std::exception& e )
	{
		wxLogError( wxString::FromAscii( e.what() ) );
		return false;
	}

	return true;
}

void BitTorrentSession::RemoveTorrent( shared_ptr<torrent_t>& torrent, bool deletedata )
{
	int idx = find_torrent_from_hash( torrent->hash );

	if( idx < 0 )
	{
		wxLogError( _T( "QueueTorrent %s: Torrent not found in queue\n" ), torrent->name.c_str() );
		return ;
	}
	wxLogInfo( _T( "%s: Removing Torrent\n" ), torrent->name.c_str() );

	libtorrent::torrent_handle& h = torrent->handle;
	if( h.is_valid() )
	{
		h.pause();
		m_libbtsession->remove_torrent( h );
	}

	wxString fastresumefile = wxGetApp().SaveTorrentsPath() +
							  wxGetApp().PathSeparator() +
							  torrent->hash + _T( ".fastresume" );
	wxString torrentconffile = wxGetApp().SaveTorrentsPath() +
							   wxGetApp().PathSeparator() +
							   torrent->hash + _T( ".conf" );
	wxString torrentfile = wxGetApp().SaveTorrentsPath() +
						   wxGetApp().PathSeparator() +
						   torrent->hash + _T( ".torrent" );

	if( ( wxFileExists( fastresumefile ) ) &&
			( !wxRemoveFile( fastresumefile ) ) )
	{
		wxLogError( _T( "Error removing file %s\n" ), fastresumefile.c_str() );
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
		wxString download_data = torrent->config->GetDownloadPath() +
								 wxGetApp().PathSeparator() + ( wxString )torrent->name.c_str() ;
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

	torrents_t::iterator torrent_it = m_torrent_queue.begin() + idx;
	{
		wxMutexLocker ml( m_torrent_queue_lock );
		m_torrent_map.erase( ( *torrent_it )->hash );
		m_torrent_queue.erase( torrent_it );
	}
}

shared_ptr<torrent_t> BitTorrentSession::FindTorrent( const wxString &hash ) const
{
	shared_ptr<torrent_t> torrent;
	int idx = find_torrent_from_hash( hash );

	if( idx >= 0 ) { torrent = m_torrent_queue.at( idx ); }

	return torrent;
}

shared_ptr<torrent_t> BitTorrentSession::GetTorrent( int idx ) const
{
	shared_ptr<torrent_t> torrent;
	if( idx >= 0 && idx < m_torrent_queue.size()) { torrent = m_torrent_queue.at( idx ); }

	return torrent;
}

void BitTorrentSession::MergeTorrent( shared_ptr<torrent_t>& dst_torrent, shared_ptr<torrent_t>& src_torrent )
{
	libtorrent::torrent_handle &torrent_handle = dst_torrent->handle;
	std::vector<libtorrent::announce_entry> const& trackers = src_torrent->info->trackers();

	for( size_t i = 0; i < trackers.size(); ++i )
	{
		torrent_handle.add_tracker( trackers.at( i ) );
	}

	std::vector<libtorrent::web_seed_entry> const& web_seeds = src_torrent->info->web_seeds();

	for( std::vector<libtorrent::web_seed_entry>::const_iterator i = web_seeds.begin(); i != web_seeds.end(); ++i )
	{
		if( i->type == libtorrent::web_seed_entry::url_seed )
		{ torrent_handle.add_url_seed( i->url ); }
		else if( i->type == libtorrent::web_seed_entry::http_seed )
		{ torrent_handle.add_http_seed( i->url ); }
	}
}

void BitTorrentSession::MergeTorrent( shared_ptr<torrent_t>& dst_torrent, MagnetUri& src_magneturi )
{
	libtorrent::torrent_handle &torrent_handle = dst_torrent->handle;
	std::vector<std::string> const& trackers = src_magneturi.trackers();

	for( size_t i = 0; i < trackers.size(); ++i )
	{
		torrent_handle.add_tracker( libtorrent::announce_entry( trackers.at( i ) ) );
	}

	std::vector<std::string> const& url_seeds = src_magneturi.urlSeeds();

	for( std::vector<std::string>::const_iterator i = url_seeds.begin(); i != url_seeds.end(); ++i )
	{
		torrent_handle.add_url_seed( *i );
	}
}
void BitTorrentSession::ScanTorrentsDirectory( const wxString& dirname )
{
	wxASSERT( m_libbtsession != NULL );
	wxDir torrents_dir( dirname );
	wxString filename;
	wxString fullpath;
	shared_ptr<torrent_t> torrent;

	if( !torrents_dir.IsOpened() )
	{
		wxLogWarning( _T( "Error opening directory %s for processing\n" ), wxGetApp().SaveTorrentsPath().c_str() );
		return ;
	}

	bool cont = torrents_dir.GetFirst( &filename, _T( "*.torrent" ), wxDIR_FILES );

	while( cont )
	{
		fullpath = dirname + wxGetApp().PathSeparator() + filename;
		wxLogDebug( _T( "Saved Torrent: %s\n" ), fullpath.c_str() );
		torrent =  ParseTorrent( fullpath );

		if( torrent->isvalid )
		{ AddTorrent( torrent ); }

		cont = torrents_dir.GetNext( &filename );
	}

	filename = wxGetApp().SaveTorrentsPath() + wxGetApp().PathSeparator() + APPBINNAME + _T( ".magneturi" );
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
		m_torrent_map.clear();

		for( int i = 0; i < m_torrent_queue.size(); ++i )
		{
			m_torrent_map[m_torrent_queue.at( i )->hash] = i;
		}
	}
}

//find and return index of torrent handle from hash string
int BitTorrentSession::find_torrent_from_hash( const wxString& hash ) const
{
	int j = -1;
	/*for( torrents_t::iterator i = m_torrent_queue.begin();
	        i != m_torrent_queue.end(); ++i )
	{
	    torrent_t *torrent = *i;
	    wxLogDebug( _T( "hash %s - %s\n" ), torrent->hash.c_str(), hash.c_str() );

	    if( torrent->hash.Cmp( hash ) == 0 )
	    { return j; }

	    j++;
	}*/
	torrents_map::const_iterator it = m_torrent_map.find( hash );

	if( it != m_torrent_map.end() ) { j = it->second; }

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
	wxLogDebug( _T( "Saving torrent info\n" ) );
	// keep track of the number of resume data
	// alerts to wait for
	int num_paused = 0;
	int num_failed = 0;
	m_libbtsession->pause();
	/* take this opportunity to sync torrent index back to zero */
	int idx = 0;
	int num_outstanding_resume_data = 0;
	wxArrayString magneturi;

	for( torrents_t::iterator i = m_torrent_queue.begin();
			i != m_torrent_queue.end();
			++i )
	{
		shared_ptr<torrent_t> torrent = *i;
		torrent->config->SetQIndex( idx++ );
		torrent->config->Save();

		if( !torrent->handle.is_valid() || \
				!torrent->handle.has_metadata() || \
				( !( ( torrent->handle.status() ).need_save_resume ) ) || \
				torrent->config->GetTorrentState() == TORRENT_STATE_STOP )
		{
			if((!torrent->magneturi.IsEmpty()) && torrent->handle.is_valid()
				&& !torrent->handle.has_metadata())
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

	while( num_outstanding_resume_data > 0 )
	{
		alert const* a = m_libbtsession->wait_for_alert( seconds( 10 ) );

		if( a == 0 ) { continue; }

		std::deque<alert*> alerts;
		m_libbtsession->pop_alerts( &alerts );
		std::string now = timestamp();

		for( std::deque<alert*>::iterator i = alerts.begin(),
			end( alerts.end() ); i != end; ++i )
		{
			// make sure to delete each alert
			std::auto_ptr<alert> a( *i );
			torrent_paused_alert const* tp = alert_cast<torrent_paused_alert>( *i );

			if( tp )
			{
				++num_paused;
				wxLogDebug( _T( "\nleft: %d failed: %d pause: %d " )
							, num_outstanding_resume_data, num_failed, num_paused );
				continue;
			}

			if( alert_cast<save_resume_data_failed_alert>( *i ) )
			{
				++num_failed;
				--num_outstanding_resume_data;
				wxLogDebug( _T( "\nleft: %d failed: %d pause: %d " )
							, num_outstanding_resume_data, num_failed, num_paused );
				continue;
			}

			save_resume_data_alert const* rd = alert_cast<save_resume_data_alert>( *i );

			if( !rd ) { continue; }

			--num_outstanding_resume_data;
			wxLogDebug( _T( "\nleft: %d failed: %d pause: %d " )
						, num_outstanding_resume_data, num_failed, num_paused );

			if( !rd->resume_data ) { continue; }

			torrent_handle h = rd->handle;
			torrent_status st = h.status( torrent_handle::query_save_path );
			//std::vector<char> out;
			//bencode(std::back_inserter(out), *rd->resume_data);
			wxString resumefile = wxGetApp().SaveTorrentsPath() + wxGetApp().PathSeparator() + to_hex( st.info_hash.to_string() ) + _T( ".resume" );
			std::ofstream out( ( const char* )resumefile.mb_str( wxConvFile ), std::ios_base::binary );
			out.unsetf( std::ios_base::skipws );
			bencode( std::ostream_iterator<char>( out ), *rd->resume_data );
		}
	}

	// save dead magnet uri
	size_t counts = magneturi.GetCount();
	wxString filename = wxGetApp().SaveTorrentsPath() + wxGetApp().PathSeparator() + APPBINNAME + _T( ".magneturi" );
	wxTextFile magneturifile( filename );
	if(counts > 0)
	{
		wxString filename = wxGetApp().SaveTorrentsPath() + wxGetApp().PathSeparator() + APPBINNAME + _T( ".magneturi" );
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
	wxLogDebug( _T( "%s: SaveTorrentResumeData\n" ), torrent->name.c_str() );
	wxString fastresumefile = wxGetApp().SaveTorrentsPath() + wxGetApp().PathSeparator() + torrent->hash + _T( ".fastresume" );

	//new api, write to disk in save_resume_data_alert
	if( torrent->handle.has_metadata() )
	{ torrent->handle.save_resume_data(); }

#if 0
	entry data = torrent->handle.write_resume_data();
	std::ofstream out( ( const char* )fastresumefile.mb_str( wxConvFile ), std::ios_base::binary );
	out.unsetf( std::ios_base::skipws );
	bencode( std::ostream_iterator<char>( out ), data );
#endif
}

void BitTorrentSession::DumpTorrents()
{
	for( torrents_t::iterator i = m_torrent_queue.begin();
			i != m_torrent_queue.end(); ++i )
	{
		shared_ptr<torrent_t> torrent = *i;
		wxLogMessage( _T( "[%s] %s\n" ), ( wxLongLong( torrent->config->GetQIndex() ).ToString() ).c_str(), torrent->name.c_str() );
	}
}

/* General function, parse torrent file into torrent_t */
shared_ptr<torrent_t> BitTorrentSession::ParseTorrent( const wxString& filename )
{
	shared_ptr<torrent_t> torrent( new torrent_t() );
	wxLogDebug( _T( "Parse Torrent: %s\n" ), filename.c_str() );

	try
	{
		std::ifstream in( ( const char* )filename.mb_str( wxConvFile ), std::ios_base::binary );
		in.unsetf( std::ios_base::skipws );
		entry e = bdecode( std::istream_iterator<char>( in ), std::istream_iterator<char>() );
		shared_ptr<libtorrent::torrent_info> t(new torrent_info( e ));
		torrent->info = t;
		torrent->name = wxString( wxConvUTF8.cMB2WC( t->name().c_str() ) );
		std::stringstream hash_stream;
		hash_stream << t->info_hash();
		torrent->hash = wxString::FromAscii( hash_stream.str().c_str() );
		torrent->config.reset( new TorrentConfig( torrent->hash ) );

		if( torrent->config->GetTrackersURL().size() <= 0 )
		{
			std::vector<libtorrent::announce_entry> trackers = t->trackers();
			torrent->config->SetTrackersURL( trackers );
		}

		torrent->isvalid = true;
	}
	catch( std::exception &e )
	{
		wxLogError( wxString::FromAscii( e.what() ) + _T( "\n" ) );
	}

	return torrent;
}

shared_ptr<torrent_t> BitTorrentSession::LoadMagnetUri( MagnetUri& magneturi )
{
	shared_ptr<torrent_t> torrent( new torrent_t() );

	try
	{
		libtorrent::add_torrent_params& p = magneturi.addTorrentParams();
		libtorrent::error_code ec;
		// Limits
		//p.max_connections = maxConnectionsPerTorrent();
		//p.max_uploads = maxUploadsPerTorrent();
		shared_ptr<libtorrent::torrent_info> t(new torrent_info( sha1_hash( magneturi.hash() ) ));
		torrent->info = t;
		torrent->name = magneturi.name();
		torrent->hash = magneturi.hash();
		torrent->config.reset( new TorrentConfig( torrent->hash ) );
		p.save_path = ( const char* )torrent->config->GetDownloadPath().mb_str( wxConvUTF8 );
		p.duplicate_is_error = true;
		p.auto_managed = true;
		// Forced start
		p.flags &= ~libtorrent::add_torrent_params::flag_paused;
		p.flags &= ~libtorrent::add_torrent_params::flag_auto_managed;
		// Solution to avoid accidental file writes
		p.flags |= libtorrent::add_torrent_params::flag_upload_mode;
		// Adding torrent to BitTorrent session
		//torrent->handle = m_libbtsession->add_torrent(p, ec);
		torrent->config->SetTorrentState( TORRENT_STATE_START );
		torrent->handle.resolve_countries( true );
		torrent->isvalid = AddTorrent( torrent );
		if(torrent->isvalid && !torrent->handle.has_metadata())
		{
			torrent->magneturi = magneturi.url();
			m_torrent_handle_map.insert(std::pair<libtorrent::torrent_handle, shared_ptr<torrent_t> >(torrent->handle, torrent));
		}
	}
	catch( std::exception &e )
	{
		wxLogError( wxString::FromAscii( e.what() ) + _T( "\n" ) );
	}

	return torrent;
}

bool BitTorrentSession::SaveTorrent( shared_ptr<torrent_t>& torrent, const wxString& filename )
{
	libtorrent::create_torrent ct_torrent( *( torrent->info ) );

	try
	{
		std::ofstream out( ( const char* )filename.mb_str( wxConvFile ), std::ios_base::binary );
		out.unsetf( std::ios_base::skipws );
		bencode( std::ostream_iterator<char>( out ), ct_torrent.generate() );
	}
	catch( std::exception &e )
	{
		wxLogError( wxString::FromAscii( e.what() ) + _T( "\n" ) );
		return false;
	}

	return true;
}

void BitTorrentSession::StartTorrent( shared_ptr<torrent_t>& torrent, bool force )
{
	wxLogInfo( _T( "%s: Start %s\n" ), torrent->name.c_str(), force ? _T( "force" ) : _T( "" ) );
	torrent_handle& handle = torrent->handle;

	if( !handle.is_valid() || ( ( torrent->handle.save_path() ).empty() ) )
	{
		AddTorrentSession( torrent );
	}

	if( handle.is_paused() )
	{ handle.resume(); }

	torrent->config->SetTorrentState( force ? TORRENT_STATE_FORCE_START : TORRENT_STATE_START );
	torrent->config->Save();
	ConfigureTorrent( torrent );
}

void BitTorrentSession::StopTorrent( shared_ptr<torrent_t>& torrent )
{
	wxLogInfo( _T( "%s:Stop\n" ), torrent->name.c_str() );
	torrent_handle& handle = torrent->handle;
	torrent_handle invalid_handle;

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

	torrent->config->SetTorrentState( TORRENT_STATE_STOP );
	torrent->config->Save();
}

void BitTorrentSession::QueueTorrent( shared_ptr<torrent_t>& torrent )
{
	//wxLogInfo(_T("%s: Queue\n"), torrent->name.c_str());
	torrent_handle& handle = torrent->handle;
	/*
	if(!handle.is_valid())
	{
	    AddTorrentSession(torrent);
	}

	if(!handle.is_paused() )
	    handle.pause();
	*/
	torrent->config->SetTorrentState( TORRENT_STATE_QUEUE );
	torrent->config->Save();
}

void BitTorrentSession::PauseTorrent( shared_ptr<torrent_t>& torrent )
{
	wxLogInfo( _T( "%s: Pause\n" ), torrent->name.c_str() );
	torrent_handle& handle = torrent->handle;

	if( !handle.is_valid() )
	{
		AddTorrentSession( torrent );
	}

	if( !handle.is_paused() )
	{ handle.pause(); }

	torrent->config->SetTorrentState( TORRENT_STATE_PAUSE );
	torrent->config->Save();
}

void BitTorrentSession::MoveTorrentUp( shared_ptr<torrent_t>& torrent )
{
	int idx = find_torrent_from_hash( torrent->hash );

	if( idx < 0 )
	{
		wxLogError( _T( "MoveTorrentUp %s: Torrent not found in queue\n" ), torrent->name.c_str() );
		return ;
	}

	wxLogInfo( _T( "%s: Moving Torrent Up\n" ), torrent->name.c_str() );

	if( idx == 0 )
	{
		wxLogInfo( _T( "Torrent is at top position %d-%s\n" ), idx, torrent->name.c_str() );
		return;
	}

	torrents_t::iterator torrent_it = m_torrent_queue.begin() + idx - 1 ;
	shared_ptr<torrent_t> prev_torrent( *( torrent_it ) );
	wxLogDebug( _T( "Prev %d now %d\n" ), prev_torrent->config->GetQIndex(), torrent->config->GetQIndex() );
	long prev_qindex = prev_torrent->config->GetQIndex();
	long qindex = torrent->config->GetQIndex();
	torrent->config->SetQIndex( prev_qindex );
	torrent->config->Save();
	prev_torrent->config->SetQIndex( qindex );
	prev_torrent->config->Save();
	{
		wxMutexLocker ml( m_torrent_queue_lock );
		m_torrent_queue[idx - 1] = torrent;
		m_torrent_queue[idx] = prev_torrent;
		m_torrent_map[torrent->hash] = idx - 1;
		m_torrent_map[prev_torrent->hash] = idx;
	}
	//m_torrent_queue.erase( torrent_it );
	//torrent_it = m_torrent_queue.begin() + idx;
	//m_torrent_queue.insert( torrent_it, prev_torrent );
}

void BitTorrentSession::MoveTorrentDown( shared_ptr<torrent_t>& torrent )
{
	int idx = find_torrent_from_hash( torrent->hash );

	if( idx < 0 )
	{
		wxLogError( _T( "MoveTorrentDown %s: Torrent not found in queue\n" ), torrent->name.c_str() );
		return ;
	}

	/* we are going to delete it
	 * deference from queue
	 */
	//torrent_t *torrent = torrent;
	wxLogInfo( _T( "%s: Moving Torrent Down\n" ), torrent->name.c_str() );

	if( idx == ( m_torrent_queue.size() - 1 ) )
	{
		wxLogInfo( _T( "Torrent is at bottom position %d-%s\n" ), idx, torrent->name.c_str() );
		return;
	}

	torrents_t::iterator torrent_it = m_torrent_queue.begin() + idx  ;
	shared_ptr<torrent_t> next_torrent( *( torrent_it + 1 ) );
	wxLogDebug( _T( "Next %d now %d\n" ), next_torrent->config->GetQIndex(), torrent->config->GetQIndex() );
	long next_qindex = next_torrent->config->GetQIndex();
	long qindex = torrent->config->GetQIndex();
	torrent->config->SetQIndex( next_qindex );
	torrent->config->Save();
	next_torrent->config->SetQIndex( qindex );
	next_torrent->config->Save();
	{
		wxMutexLocker ml( m_torrent_queue_lock );
		m_torrent_queue[idx] = next_torrent;
		m_torrent_queue[idx + 1] = torrent;
		m_torrent_map[torrent->hash] = idx + 1;
		m_torrent_map[next_torrent->hash] = idx;
	}
	//m_torrent_queue.erase( torrent_it );
	//torrent_it = m_torrent_queue.begin() + idx + 1 ;
	//m_torrent_queue.insert( torrent_it, torrent );
}

void BitTorrentSession::ReannounceTorrent( shared_ptr<torrent_t>& torrent )
{
	wxLogInfo( _T( "%s: Reannounce\n" ), torrent->name.c_str() );
	torrent_handle &h = torrent->handle;

	if( h.is_valid() )
	{
		h.force_reannounce();
	}
}

void BitTorrentSession::ConfigureTorrentFilesPriority( shared_ptr<torrent_t>& torrent )
{
	int nopriority = 0;
	std::vector<int> filespriority = torrent->config->GetFilesPriorities();
	//XXX default priority is 4...
	// win32 4 triggers a assert
	std::vector<int> deffilespriority( torrent->info->num_files(), 5 );

	if( filespriority.size() != torrent->info->num_files() )
	{
		nopriority = 1;
	}

	if( torrent->handle.is_valid() )
	{ torrent->handle.prioritize_files( nopriority ? deffilespriority : filespriority ); }
}

void BitTorrentSession::ConfigureTorrentTrackers( shared_ptr<torrent_t>& torrent )
{
	libtorrent::torrent_handle& t_handle = torrent->handle;
	if( t_handle.is_valid() )
	{ t_handle.replace_trackers( torrent->config->GetTrackersURL() ); }
}

void BitTorrentSession::ConfigureTorrent( shared_ptr<torrent_t>& torrent )
{
	torrent_handle &h = torrent->handle;
	wxLogDebug( _T( "%s: Configure\n" ), torrent->name.c_str() );
	std::string existdir = h.save_path()/*.string()*/;
	std::string newdir( torrent->config->GetDownloadPath().mb_str( wxConvUTF8 ) );
	wxString oldpath = wxString::FromAscii( existdir.c_str() ) +
					   wxFileName::GetPathSeparator( wxPATH_NATIVE ) +
					   wxString( wxConvUTF8.cMB2WC( torrent->info->name().c_str() ) )/*.c_str()*/;
#ifdef __WXMSW__
	oldpath.Replace( _T( "/" ), _T( "\\" ), true );
#endif
	wxString newpath = torrent->config->GetDownloadPath() +
					   wxFileName::GetPathSeparator( wxPATH_NATIVE ) +
					   wxString( wxConvUTF8.cMB2WC( torrent->info->name().c_str() ) )/*.c_str()*/;

	if( oldpath.Cmp( newpath ) != 0 )
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

		wxLogDebug( _T( "Old path %s, new path %s\n" ), oldpath.c_str(), newpath.c_str() );

		if( wxDirExists( oldpath ) )
		{
			copysuccess = ( CopyDirectory( oldpath, newpath ) != 0 );

			if( copysuccess )
			{
				RemoveDirectory( oldpath );
			}
		}
		else if( wxFileExists( oldpath ) )
		{
			copysuccess = wxCopyFile( oldpath, newpath );

			if( copysuccess )
			{
				wxRemoveFile( oldpath );
			}
		}
		else
		{
			wxLogWarning( _T( "Old path not exists %s\n" ), oldpath.c_str() );
			copysuccess = false;
		}

		//failed copy, restore path
		if( copysuccess )
		{
			if( ( prev_state == TORRENT_STATE_START ) ||
					( prev_state == TORRENT_STATE_FORCE_START ) )
			{
				StartTorrent( torrent, prev_state == ( TORRENT_STATE_FORCE_START ) );
				h = torrent->handle;
			}
		}
		else
		{
			wxLogError( _T( "Failed moving save path %s\n" ), torrent->config->GetDownloadPath().c_str() );
			wxLogWarning( _T( "Restoring download path %s\n" ), oldpath.c_str() );
			torrent->config->SetDownloadPath( wxString::FromAscii( existdir.c_str() ) );
			torrent->config->Save();
		}
	}

	if( h.is_valid() )
	{
		bool ispaused = h.is_paused();
		h.set_upload_limit( torrent->config->GetTorrentUploadLimit() );
		h.set_download_limit( torrent->config->GetTorrentDownloadLimit() );
		h.set_max_uploads( torrent->config->GetTorrentMaxUploads() );
		h.set_max_connections( torrent->config->GetTorrentMaxConnections() );
		h.set_ratio( torrent->config->GetTorrentRatio() );
		wxLogDebug( _T( "%s: Upload Limit %s download limit %s, max uploads %s, max connections %s, ratio %lf\n" ),
					torrent->name.c_str(),
					( wxLongLong( h.upload_limit() ).ToString() ).c_str(),
					( wxLongLong( h.download_limit() ).ToString() ).c_str(),
					( wxLongLong( torrent->config->GetTorrentMaxUploads() ).ToString() ).c_str(),
					( wxLongLong( torrent->config->GetTorrentMaxConnections() ).ToString() ).c_str(),
					( float )torrent->config->GetTorrentRatio() / 100 );
		ConfigureTorrentFilesPriority( torrent );
		ConfigureTorrentTrackers( torrent );
	}
}

void BitTorrentSession::RemoveTorrent( int idx, bool deletedata )
{
	if( idx < 0 || idx >= m_torrent_queue.size())
	{
		wxLogError( _T( "%d: Invalid torrent index\n" ), idx );
		return ;
	}

	RemoveTorrent( m_torrent_queue[idx], deletedata);
}

void BitTorrentSession::StartTorrent( int idx, bool force )
{
	if( idx < 0 || idx >= m_torrent_queue.size())
	{
		wxLogError( _T( "%d: Invalid torrent index\n" ), idx );
		return ;
	}

	StartTorrent( m_torrent_queue[idx], force );
}

void BitTorrentSession::StopTorrent( int idx )
{
	if( idx < 0 || idx >= m_torrent_queue.size())
	{
		wxLogError( _T( "%d: Invalid torrent index\n" ), idx );
		return ;
	}

	StopTorrent( m_torrent_queue[idx] );
}

void BitTorrentSession::QueueTorrent( int idx )
{
	if( idx < 0 || idx >= m_torrent_queue.size())
	{
		wxLogError( _T( "%d: Invalid torrent index\n" ), idx );
		return ;
	}

	QueueTorrent( m_torrent_queue[idx] );
}

void BitTorrentSession::PauseTorrent( int idx )
{
	if( idx < 0 || idx >= m_torrent_queue.size())
	{
		wxLogError( _T( "%d: Invalid torrent index\n" ), idx );
		return ;
	}

	PauseTorrent( m_torrent_queue[idx] );
}

void BitTorrentSession::MoveTorrentUp( int idx )
{
	if( idx < 0 || idx >= m_torrent_queue.size())
	{
		wxLogError( _T( "%d: Invalid torrent index\n" ), idx );
		return ;
	}

	if( idx == 0 )
	{
		wxLogInfo( _T( "Torrent is at top position %d\n" ), idx );
		return;
	}
	MoveTorrentUp( m_torrent_queue[idx] );
}

void BitTorrentSession::MoveTorrentDown( int idx )
{
	if( idx < 0 || idx >= m_torrent_queue.size())
	{
		wxLogError( _T( "%d: Invalid torrent index\n" ), idx );
		return ;
	}

	if( idx == ( m_torrent_queue.size() - 1 ) )
	{
		wxLogInfo( _T( "Torrent is at bottom position %d\n" ), idx );
		return;
	}

	MoveTorrentDown( m_torrent_queue[idx] );
}

void BitTorrentSession::ReannounceTorrent( int idx )
{
	if( idx < 0 || idx >= m_torrent_queue.size())
	{
		wxLogError( _T( "%d: Invalid torrent index\n" ), idx );
		return ;
	}

	ReannounceTorrent( m_torrent_queue[idx] );
}

void BitTorrentSession::ConfigureTorrentFilesPriority( int idx )
{
	if( idx < 0 || idx >= m_torrent_queue.size())
	{
		wxLogError( _T( "%d: Invalid torrent index\n" ), idx );
		return ;
	}

	ConfigureTorrentFilesPriority( m_torrent_queue[idx] );
}

void BitTorrentSession::ConfigureTorrentTrackers( int idx )
{
	if( idx < 0 || idx >= m_torrent_queue.size())
	{
		wxLogError( _T( "%d: Invalid torrent index\n" ), idx );
		return ;
	}

	ConfigureTorrentTrackers( m_torrent_queue[idx] );
}

void BitTorrentSession::ConfigureTorrent( int idx )
{
	if( idx < 0 || idx >= m_torrent_queue.size())
	{
		wxLogError( _T( "%d: Invalid torrent index\n" ), idx );
		return ;
	}

	ConfigureTorrent( m_torrent_queue[idx] );
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
	torrents_t start_torrents;
	torrents_t queue_torrents;
	torrents_t::iterator torrent_it;
	int pause_count = 0;
	int start_count = 0;
	int i = 0;
	int maxstart = m_config->GetMaxStart();
	//wxLogDebug(_T("CheckQueueItem %d\n"), m_torrent_queue.size());
	wxMutexLocker ml( m_torrent_queue_lock );

	for( torrent_it = m_torrent_queue.begin(); torrent_it != m_torrent_queue.end(); ++torrent_it )
	{
		shared_ptr<torrent_t>& torrent = *torrent_it;
		wxLogDebug( _T( "Processing torrent %s\n" ), torrent->name.c_str() );

		switch( torrent->config->GetTorrentState() )
		{
		case TORRENT_STATE_START:
			{
				bool exseed = false;

				if( torrent->handle.is_valid() )
				{
					torrent_status t_status = torrent->handle.status();

					if( ( t_status.state == torrent_status::seeding ) ||
							( t_status.state == torrent_status::finished ) ||
							( ( t_status.progress >= 1 ) &&
							  ( t_status.state != torrent_status::checking_files ) ) )
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

					//periodic save fastresume data
					//
					wxString fastresumefile = wxGetApp().SaveTorrentsPath() + wxGetApp().PathSeparator() + torrent->hash + _T( ".fastresume" );
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
				}

				/* make this an option */
				if( !exseed )
				{
					start_torrents.push_back( torrent );
					start_count++;
				}

				break;
			}

		case TORRENT_STATE_FORCE_START:
			{
				//periodic save fastresume data
				//
				wxString fastresumefile = wxGetApp().SaveTorrentsPath() + wxGetApp().PathSeparator() + torrent->hash + _T( ".fastresume" );
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

				break;
			}

		case TORRENT_STATE_QUEUE:
			{
				queue_torrents.push_back( torrent );
				pause_count++;
				break;
			}

		default:
			//do nothing
			break;
		}
	}

	if( start_count > maxstart )
	{
		i = start_torrents.size() - 1;

		while( ( start_count-- > maxstart ) && ( i >= 0 ) )
		{
			QueueTorrent( start_torrents[i--] );
		}
	}
	else if( ( start_count < maxstart ) && ( queue_torrents.size() > 0 ) )
	{
		i = 0;

		while( ( start_count++ < maxstart ) && ( i <  queue_torrents.size() ) )
		{
			StartTorrent( queue_torrents[i++], false );
		}
	}
}

void BitTorrentSession::GetTorrentLog()
{
	wxString event_string;
	std::vector<alert*> alerts;
	m_libbtsession->pop_alerts(&alerts);
	//std::string now = timestamp();
	for (std::vector<alert*>::iterator it = alerts.begin()
		, end(alerts.end()); it != end; ++it)
	{
		TORRENT_TRY
		{
			event_string.Empty();
			switch ((*it)->type())
			{
				case torrent_finished_alert::alert_type:
					if( torrent_finished_alert* p = dynamic_cast<torrent_finished_alert*>( *it ) )
					{
						event_string << p->handle.get_torrent_info().name() << _T( ": " ) << (*it)->message();
						break;
					}
				case peer_ban_alert::alert_type:
					if( peer_ban_alert* p = dynamic_cast<peer_ban_alert*>( *it ) )
					{
						event_string << _T( "Peer: (" ) << p->ip.address().to_string() << _T( ")" ) << (*it)->message();
						break;
					}
				case peer_error_alert::alert_type:
					if( peer_error_alert* p = dynamic_cast<peer_error_alert*>( *it ) )
					{
						event_string << _T( "Peer: " ) << identify_client( p->pid ) << _T( ": " ) << (*it)->message();
						break;
					}
				case peer_blocked_alert::alert_type:
					if( peer_blocked_alert* p = dynamic_cast<peer_blocked_alert*>( *it ) )
					{
						event_string << _T( "Peer: (" ) << p->ip.to_string() << _T( ")" ) << (*it)->message();
						break;
					}
				case invalid_request_alert::alert_type:
					if( invalid_request_alert* p = dynamic_cast<invalid_request_alert*>( *it ) )
					{
						event_string << identify_client( p->pid ) << _T( ": " ) << (*it)->message();
						break;
					}
				case tracker_warning_alert::alert_type:
					if( tracker_warning_alert* p = dynamic_cast<tracker_warning_alert*>( *it ) )
					{
						event_string << _T( "Tracker: " ) << p->message();
						break;
					}
				case tracker_reply_alert::alert_type:
					if( tracker_reply_alert* p = dynamic_cast<tracker_reply_alert*>( *it ) )
					{
						event_string << _T( "Tracker:" ) << p->message() << p->num_peers << _T( " number of peers " );
						break;
					}
				case url_seed_alert::alert_type:
					if( url_seed_alert* p = dynamic_cast<url_seed_alert*>( *it ) )
					{
						event_string << _T( "Url seed '" ) << p->url << _T( "': " ) << p->message();
						break;
					}
				case listen_failed_alert::alert_type:
					if( listen_failed_alert* p = dynamic_cast<listen_failed_alert*>( *it ) )
					{
						event_string << _T( "Listen failed: " ) << p->message();
						break;
					}
				case portmap_error_alert::alert_type:
					if( portmap_error_alert* p = dynamic_cast<portmap_error_alert*>( *it ) )
					{
						event_string << _T( "Portmap error: " ) << p->message();
						break;
					}
				case portmap_alert::alert_type:
					if( portmap_alert* p = dynamic_cast<portmap_alert*>( *it ) )
					{
						event_string << _T( "Portmap: " ) << p->message();
						break;
					}
				case tracker_announce_alert::alert_type:
					if( tracker_announce_alert* p = dynamic_cast<tracker_announce_alert*>( *it ) )
					{
						event_string << _T( "Tracker: " ) << p->message();
						break;
					}
				case file_error_alert::alert_type:
					if( file_error_alert* p = dynamic_cast<file_error_alert*>( *it ) )
					{
						event_string << _T( "File error: " ) << p->message();
						break;
					}
				case hash_failed_alert::alert_type:
					if( hash_failed_alert* p = dynamic_cast<hash_failed_alert*>( *it ) )
					{
						event_string << _T( "Hash: " ) << p->message();
						break;
					}
				case metadata_failed_alert::alert_type:
					if( metadata_failed_alert* p = dynamic_cast<metadata_failed_alert*>( *it ) )
					{
						event_string << _T( "Metadata: " ) << p->message();
						break;
					}
				case metadata_received_alert::alert_type:
					if( metadata_received_alert* p = dynamic_cast<metadata_received_alert*>( *it ) )
					{
						event_string << _T( "Metadata: " ) << p->message();

						torrent_handle_map::iterator it = m_torrent_handle_map.find(p->handle);
						if(it != m_torrent_handle_map.end())
						{
							wxString torrent_backup = wxGetApp().SaveTorrentsPath() + wxGetApp().PathSeparator() + it->second->hash + _T( ".torrent" );
							SaveTorrent(it->second, torrent_backup );
							m_torrent_handle_map.erase(it);
							
							if( it->second->config->GetTrackersURL().size() <= 0 )
							{
								std::vector<libtorrent::announce_entry> trackers = p->handle.trackers();
								it->second->config->SetTrackersURL( trackers );
							}
						}
						break;
					}
				case fastresume_rejected_alert::alert_type:
					if( fastresume_rejected_alert* p = dynamic_cast<fastresume_rejected_alert*>( *it ) )
					{
						event_string << _T( "Fastresume: " ) << p->message();
						break;
					}
				case save_resume_data_alert::alert_type:
					if( save_resume_data_alert *p = dynamic_cast<save_resume_data_alert*>( *it ) )
					{
						torrent_handle& h = p->handle;

						if( p->resume_data )
						{
							std::stringstream hash_stream;
							hash_stream << h.info_hash();
							wxString thash = wxString::FromAscii( hash_stream.str().c_str() );
							wxString fastresumefile = wxGetApp().SaveTorrentsPath() + wxGetApp().PathSeparator() + thash + _T( ".fastresume" );
							std::ofstream out( ( const char* )fastresumefile.mb_str( wxConvFile ), std::ios_base::binary );
							out.unsetf( std::ios_base::skipws );
							bencode( std::ostream_iterator<char>( out ), *p->resume_data );
							//save_resume_data to disk
						}

						event_string << h.name() << _T( ": " ) << p->message();
					}
				default:
					{
						event_string << (*it)->message();
						if(event_string.IsEmpty()) return;
						break;
					}
			}
			//XXX include more alert from latest libtorrent
			//
			event_string << _T( '\n' ) << _T( '\0' );

			if( (*it)->severity() == alert::fatal )
			{
				wxLogError( _T("%s"), event_string.c_str() );
			}
			else if( (*it)->severity() == alert::critical )
			{
				wxLogError( _T("%s"), event_string.c_str() );
			}
			else if( (*it)->severity() == alert::warning )
			{
				wxLogWarning( _T("%s"), event_string.c_str() );
			}
			else if( (*it)->severity() == alert::info )
			{
				wxLogInfo( _T("%s"), event_string.c_str() );
			}
			else if( (*it)->severity() == alert::debug )
			{
				wxLogDebug( _T("%s"), event_string.c_str() );
			}
			else
			{
				wxLogDebug( _T("%s"), event_string.c_str() );
			}
		}
		TORRENT_CATCH(std::exception& e)
		{
			wxLogError( _T( "%s\n" ), e.what() );
		}
	}
	//alerts.clear();
}

