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
//plugins
#include <libtorrent/extensions/metadata_transfer.hpp>
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

void *BitTorrentSession::Entry()
{
    session_settings btsetting;
    wxLogDebug( _T( "BitTorrentSession Thread Entry pid 0x%lx priority %u.\n" ), GetId(), GetPriority() );
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
    m_libbtsession->set_settings( btsetting );
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
		wxMutexLocker ml(m_torrent_queue_lock);

	    m_torrent_queue.empty();
	    m_torrent_queue.swap( m_torrent_queue );
	}
}

void BitTorrentSession::ConfigureSession()
{
    wxASSERT( m_libbtsession != NULL );
#ifndef TORRENT_DISABLE_GEO_IP
    m_libbtsession->load_asnum_db( "GeoIPASNum.dat" );
    m_libbtsession->load_country_db( "GeoIP.dat" );
#endif
    SetLogSeverity();
    //Network settings
    SetConnection();
    SetConnectionLimit();
    SetDht();
    SetEncryption();
    StartExtensions();
    StartUpnp();
    StartNatpmp();
    StartLsd();
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

    m_libbtsession->listen_on( std::make_pair( portmin, portmax ),
                               m_config->m_local_ip.IPAddress().ToAscii() );
}

void BitTorrentSession::SetConnectionLimit()
{
    wxASSERT( m_libbtsession != NULL );
    m_libbtsession->set_upload_rate_limit( m_config->GetGlobalUploadLimit() );
    m_libbtsession->set_download_rate_limit( m_config->GetGlobalDownloadLimit() );
    m_libbtsession->set_max_uploads( m_config->GetGlobalMaxUploads() );
    m_libbtsession->set_max_connections( m_config->GetGlobalMaxConnections() );
    m_libbtsession->set_max_half_open_connections( m_config->GetGlobalMaxHalfConnect() );
}

void BitTorrentSession::SetDht()
{
    wxASSERT( m_libbtsession != NULL );

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

        //dht_state.print(std::cout, 1);
        m_libbtsession->start_dht( dht_state );
        m_libbtsession->add_dht_router( std::make_pair( std::string( "dht.libtorrent.org" ), 25401 ) );
        m_libbtsession->add_dht_router( std::make_pair( std::string( "router.bittorrent.com" ), 6881 ) );
        m_libbtsession->add_dht_router( std::make_pair( std::string( "router.utorrent.com" ), 6881 ) );
        m_libbtsession->add_dht_router( std::make_pair( std::string( "dht.transmissionbt.com" ), 6881 ) );
        m_libbtsession->add_dht_router( std::make_pair( std::string( "dht.aelitis.com" ), 6881 ) ); // Vuze
        wxLogMessage( _T( "DHT service started, port %d\n" ), m_config->GetDHTPort() );
    }
    else
    {
        wxLogMessage( _T( "DHT is disabled\n" ) );
        m_libbtsession->stop_dht();
    }
}

void BitTorrentSession::SetEncryption()
{
    wxASSERT( m_libbtsession != NULL );
    struct pe_settings EncSettings;

    if( m_config->GetEncEnabled() )
    {
        EncSettings.out_enc_policy = ( pe_settings::enc_policy )( m_config->GetEncPolicy() );
        EncSettings.in_enc_policy = ( pe_settings::enc_policy )( m_config->GetEncPolicy() );
        EncSettings.allowed_enc_level = ( pe_settings::enc_level )( m_config->GetEncLevel() );
        EncSettings.prefer_rc4 = m_config->GetEncPreferRC4();
        wxLogMessage( _T( "Encryption enabled, policy %d\n" ), m_config->GetEncPolicy() );
    }
    else
    {
        EncSettings.out_enc_policy = pe_settings::disabled;
        EncSettings.in_enc_policy = pe_settings::disabled;
        wxLogMessage( _T( "Encryption is disabled\n" ) );
    }

    m_libbtsession->set_pe_settings( EncSettings );
}

void BitTorrentSession::StartExtensions()
{
    wxASSERT( m_libbtsession != NULL );

    /* XXX no way to remove extension currently
     * restart is needed
     */
    if( m_config->GetEnableMetadata() )
    { m_libbtsession->add_extension( &libtorrent::create_metadata_plugin ); }

    if( m_config->GetEnablePex() )
    { m_libbtsession->add_extension( &libtorrent::create_ut_pex_plugin ); }
}

void BitTorrentSession::StartUpnp()
{
    wxASSERT( m_libbtsession != NULL );

    if( m_config->GetEnableUpnp() )
    {
        // XXX workaround upnp crashed when started twice
        // libtorrent bugs
        if( ! m_upnp_started )
        {
#if LIBTORRENT_VERSION_NUM < 10100
            m_libbtsession->start_upnp();
#else
            settings_pack settingsPack = m_libbtsession->get_settings();
            settingsPack.set_bool( settings_pack::enable_upnp, true );
            m_libbtsession->apply_settings( settingsPack );
#endif
        }

        m_upnp_started = true;
    }
    else
    {
#if LIBTORRENT_VERSION_NUM < 10100
        m_libbtsession->stop_upnp();
#else
        settings_pack settingsPack = m_libbtsession->get_settings();
        settingsPack.set_bool( settings_pack::enable_upnp, false );
        m_libbtsession->apply_settings( settingsPack );
#endif
        m_upnp_started = true;
    }
}

void BitTorrentSession::StartNatpmp()
{
    wxASSERT( m_libbtsession != NULL );

    if( m_config->GetEnableNatpmp() )
    {
        if( !m_natpmp_started )
        {
#if LIBTORRENT_VERSION_NUM < 10100
            m_libbtsession->start_natpmp();
#else
            settings_pack settingsPack = m_libbtsession->get_settings();
            settingsPack.set_bool( settings_pack::enable_natpmp, true );
            m_libbtsession->apply_settings( settingsPack );
#endif
        }

        m_natpmp_started = true;
    }
    else
    {
#if LIBTORRENT_VERSION_NUM < 10100
        m_libbtsession->stop_natpmp();
#else
        settings_pack settingsPack = m_libbtsession->get_settings();
        settingsPack.set_bool( settings_pack::enable_natpmp, false );
        m_libbtsession->apply_settings( settingsPack );
        m_natpmp_started = false;
#endif
    }
}

void BitTorrentSession::StartLsd()
{
    wxASSERT( m_libbtsession != NULL );

    if( m_config->GetEnableLsd() )
    {
        if( !m_lsd_started ) { m_libbtsession->start_lsd(); }

        m_lsd_started = true;
    }
    else
    { m_libbtsession->stop_lsd(); }
}

void BitTorrentSession::AddTorrentSession( shared_ptr<torrent_t>& torrent )
{
    torrent_handle &handle = torrent->handle;
    boost::intrusive_ptr<torrent_info> t = torrent->info;
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

        if( torrent->config->GetTorrentCompactAlloc() )
        {
            wxLogInfo( _T( "%s: Compact allocation mode\n" ), torrent->name.c_str() );
            p.storage_mode = libtorrent::storage_mode_compact;
            handle = m_libbtsession->add_torrent( p );
        }
        else
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

            case libtorrent::storage_mode_compact:
                strStorageMode = _( "Compact" );
                break;

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

		{
			wxMutexLocker ml(m_torrent_queue_lock);
	        m_torrent_queue.push_back( torrent );
			m_torrent_map.insert(std::pair<wxString, int>(torrent->hash, m_torrent_queue.size()-1));
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
    libtorrent::torrent_handle& h = torrent->handle;
    long idx = -1;
    idx = find_torrent_from_hash( torrent->hash );

    if( idx < 0 )
    {
        wxLogError( _T( "RemoveTorrent %s: Torrent not found in queue\n" ), torrent->name.c_str() );
        return ;
    }

    wxLogInfo( _T( "%s: Removing Torrent\n" ), torrent->name.c_str() );

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
        else
            if( wxFileExists( download_data ) )
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
		wxMutexLocker ml(m_torrent_queue_lock);
		m_torrent_map.erase((*torrent_it)->hash);
	    m_torrent_queue.erase( torrent_it );
	}
}

shared_ptr<torrent_t> BitTorrentSession::FindTorrent(const wxString &hash) const
{
	shared_ptr<torrent_t> torrent;
	int idx = find_torrent_from_hash( hash );
	if(idx > 0)	torrent = m_torrent_queue.at(idx);

	return torrent;
}

void BitTorrentSession::MergeTorrent(shared_ptr<torrent_t>& dst_torrent, shared_ptr<torrent_t>& src_torrent)
{
	libtorrent::torrent_handle &torrent_handle = dst_torrent->handle;
	std::vector<libtorrent::announce_entry> const& trackers = src_torrent->info->trackers();
	for(size_t i = 0; i < trackers.size(); ++i)
	{
		torrent_handle.add_tracker(trackers.at(i));
	}
	std::vector<libtorrent::web_seed_entry> const& web_seeds = src_torrent->info->web_seeds();
	for (std::vector<libtorrent::web_seed_entry>::const_iterator i = web_seeds.begin(); i != web_seeds.end(); ++i)
	{
		if (i->type == libtorrent::web_seed_entry::url_seed)
			torrent_handle.add_url_seed(i->url);
		else if (i->type == libtorrent::web_seed_entry::http_seed)
			torrent_handle.add_http_seed(i->url);
	}
}

void BitTorrentSession::MergeTorrent(shared_ptr<torrent_t>& dst_torrent, MagnetUri& src_magneturi)
{
	libtorrent::torrent_handle &torrent_handle = dst_torrent->handle;
	std::vector<std::string> const& trackers = src_magneturi.trackers();
	for(size_t i = 0; i < trackers.size(); ++i)
	{
		torrent_handle.add_tracker(libtorrent::announce_entry(trackers.at(i)));
	}
	std::vector<std::string> const& url_seeds = src_magneturi.urlSeeds();
	for (std::vector<std::string>::const_iterator i = url_seeds.begin(); i != url_seeds.end(); ++i)
	{
		torrent_handle.add_url_seed(*i);
	}
}
void BitTorrentSession::ScanTorrentsDirectory( const wxString& dirname )
{
    wxASSERT( m_libbtsession != NULL );
    wxDir torrents_dir( dirname );
    wxString filename;

    if( !torrents_dir.IsOpened() )
    {
        wxLogWarning( _T( "Error opening directory %s for processing\n" ), wxGetApp().SaveTorrentsPath().c_str() );
        return ;
    }

    bool cont = torrents_dir.GetFirst( &filename, _T( "*.torrent" ), wxDIR_FILES );

    while( cont )
    {
        wxString fullpath = dirname + wxGetApp().PathSeparator() + filename;
        wxLogDebug( _T( "Saved Torrent: %s\n" ), fullpath.c_str() );
		shared_ptr<torrent_t> torrent(ParseTorrent( fullpath ));

        if( torrent->isvalid )
        { AddTorrent( torrent ); }

        cont = torrents_dir.GetNext( &filename );
    }

	{
		wxMutexLocker ml(m_torrent_queue_lock);
	    std::sort( m_torrent_queue.begin(), m_torrent_queue.end(),
	               BTQueueSortAsc() );
		m_torrent_map.clear();
		for(int i = 0; i < m_torrent_queue.size(); ++i)
		{
			m_torrent_map[m_torrent_queue.at(i)->hash] = i;
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
	torrents_map::const_iterator it = m_torrent_map.find(hash);
	if(it != m_torrent_map.end()) j = it->second;

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

        for( std::deque<alert*>::iterator i = alerts.begin()
                                              , end( alerts.end() ); i != end; ++i )
        {
            // make sure to delete each alert
            std::auto_ptr<alert> a( *i );
            torrent_paused_alert const* tp = alert_cast<torrent_paused_alert>( *i );

            if( tp )
            {
                ++num_paused;
                wxLogDebug( _T( "\rleft: %d failed: %d pause: %d " )
                            , num_outstanding_resume_data, num_failed, num_paused );
                continue;
            }

            if( alert_cast<save_resume_data_failed_alert>( *i ) )
            {
                ++num_failed;
                --num_outstanding_resume_data;
                wxLogDebug( _T( "\rleft: %d failed: %d pause: %d " )
                            , num_outstanding_resume_data, num_failed, num_paused );
                continue;
            }

            save_resume_data_alert const* rd = alert_cast<save_resume_data_alert>( *i );

            if( !rd ) { continue; }

            --num_outstanding_resume_data;
            wxLogDebug( _T( "\rleft: %d failed: %d pause: %d " )
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
}

void BitTorrentSession::SaveTorrentResumeData( shared_ptr<torrent_t>& torrent )
{
    wxLogDebug( _T( "%s: SaveTorrentResumeData\n" ), torrent->name.c_str() );
    wxString fastresumefile = wxGetApp().SaveTorrentsPath() + wxGetApp().PathSeparator() + torrent->hash + _T( ".fastresume" );
    //new api, write to disk in save_resume_data_alert
    torrent->handle.save_resume_data();
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
    shared_ptr<torrent_t> torrent( new torrent_t());
    wxLogDebug( _T( "Parse Torrent: %s\n" ), filename.c_str() );

    try
    {
        std::ifstream in( ( const char* )filename.mb_str( wxConvFile ), std::ios_base::binary );
        in.unsetf( std::ios_base::skipws );
        entry e = bdecode( std::istream_iterator<char>( in ), std::istream_iterator<char>() );
        boost::intrusive_ptr<libtorrent::torrent_info> t = new torrent_info( e );
        torrent->info = t;
        torrent->name = wxString( wxConvUTF8.cMB2WC( t->name().c_str() ) );
        std::stringstream hash_stream;
        hash_stream << t->info_hash();
        torrent->hash = wxString::FromAscii( hash_stream.str().c_str() );
        torrent->config.reset( new TorrentConfig( torrent->hash ));

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
	shared_ptr<torrent_t> torrent( new torrent_t());

	try
	{
		libtorrent::add_torrent_params& p = magneturi.addTorrentParams();
		libtorrent::error_code ec;

		// Limits
		//p.max_connections = maxConnectionsPerTorrent();
		//p.max_uploads = maxUploadsPerTorrent();
		shared_ptr<torrent_t> torrent( new torrent_t());
		
        boost::intrusive_ptr<libtorrent::torrent_info> t = new torrent_info( sha1_hash(magneturi.hash()) );
        torrent->info = t;
		torrent->name = wxString( wxConvUTF8.cMB2WC( t->name().c_str() ) );
        torrent->hash = magneturi.hash();
        torrent->config.reset( new TorrentConfig( torrent->hash ));

		p.save_path = ( const char* )torrent->config->GetDownloadPath().mb_str( wxConvUTF8 );
		// Flags
        if( torrent->config->GetTorrentCompactAlloc() )
        {
            wxLogInfo( _T( "%s: Compact allocation mode\n" ), torrent->name.c_str() );
            p.storage_mode = libtorrent::storage_mode_compact;
        }
        else
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

            case libtorrent::storage_mode_compact:
                strStorageMode = _( "Compact" );
                break;

            default:
                eStorageMode = libtorrent::storage_mode_sparse;
                strStorageMode = _( "Sparse" );
                break;
            }

            p.storage_mode = eStorageMode;
            wxLogInfo( _T( "%s: %s allocation mode\n" ), torrent->name.c_str(), strStorageMode.c_str() );
        }

        p.duplicate_is_error = true;
        p.auto_managed = true;

		// Forced start
		p.flags &= ~libtorrent::add_torrent_params::flag_paused;
		p.flags &= ~libtorrent::add_torrent_params::flag_auto_managed;
		// Solution to avoid accidental file writes
		p.flags |= libtorrent::add_torrent_params::flag_upload_mode;
		
		// Adding torrent to BitTorrent session
		torrent->handle = m_libbtsession->add_torrent(p, ec);
        torrent->handle.resolve_countries( true );
		if (!ec) torrent->isvalid = true;
	}
	catch( std::exception &e )
	{
		wxLogError( wxString::FromAscii( e.what() ) + _T( "\n" ) );
	}

	return torrent;
}

bool BitTorrentSession::SaveTorrent(shared_ptr<torrent_t>& torrent, const wxString& filename)
{
	libtorrent::create_torrent ct_torrent(*(torrent->info));

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

void BitTorrentSession::GetTorrentQueue(torrents_t & queue_copy)
{
	size_t t = 0;
	queue_copy.clear();

	while( t < m_torrent_queue.size() )
	{
		queue_copy.push_back( m_torrent_queue.at( t ) );
		t++;
	}
}
void BitTorrentSession::StartTorrent( shared_ptr<torrent_t>& t_torrent, bool force )
{
    long idx = 0;
    idx = find_torrent_from_hash( t_torrent->hash );

    if( idx < 0 )
    {
        wxLogError( _T( "StartTorrent %s: Torrent not found in queue\n" ), t_torrent->name.c_str() );
        return ;
    }

    // XXX pointer ???
	shared_ptr<torrent_t>torrent = m_torrent_queue[idx];
    wxLogInfo( _T( "%s: Start %s\n" ), torrent->name.c_str(), force ? _T( "force" ) : _T( "" ) );
    torrent_handle& handle = torrent->handle;

    if( !handle.is_valid() )
    {
        AddTorrentSession( torrent );
    }

    if( handle.is_paused() )
    { handle.resume(); }

    torrent->config->SetTorrentState( force ? TORRENT_STATE_FORCE_START : TORRENT_STATE_START );
    torrent->config->Save();
    ConfigureTorrent( torrent );
}

void BitTorrentSession::StopTorrent( shared_ptr<torrent_t>& t_torrent )
{
    long idx = 0;
    idx = find_torrent_from_hash( t_torrent->hash );

    if( idx < 0 )
    {
        wxLogError( _T( "Stop %s: Torrent not found in queue\n" ), t_torrent->name.c_str() );
        return ;
    }

	shared_ptr<torrent_t> torrent = m_torrent_queue[idx];
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

void BitTorrentSession::QueueTorrent( shared_ptr<torrent_t>& t_torrent )
{
    long idx = 0;
    idx = find_torrent_from_hash( t_torrent->hash );

    if( idx < 0 )
    {
        wxLogError( _T( "QueueTorrent %s: Torrent not found in queue\n" ), t_torrent->name.c_str() );
        return ;
    }

	shared_ptr<torrent_t> torrent = m_torrent_queue[idx];
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

void BitTorrentSession::PauseTorrent( shared_ptr<torrent_t>& t_torrent )
{
    long idx = 0;
    idx = find_torrent_from_hash( t_torrent->hash );

    if( idx < 0 )
    {
        wxLogError( _T( "PauseTorrent %s: Torrent not found in queue\n" ), t_torrent->name.c_str() );
        return ;
    }

	shared_ptr<torrent_t> torrent = m_torrent_queue[idx];
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
    long idx = -1;
    idx = find_torrent_from_hash( torrent->hash );

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
	shared_ptr<torrent_t> prev_torrent(*( torrent_it ));
    wxLogDebug( _T( "Prev %d now %d\n" ), prev_torrent->config->GetQIndex(), torrent->config->GetQIndex() );
    long prev_qindex = prev_torrent->config->GetQIndex();
    long qindex = torrent->config->GetQIndex();
    torrent->config->SetQIndex( prev_qindex );
    torrent->config->Save();
    prev_torrent->config->SetQIndex( qindex );
    prev_torrent->config->Save();
	
	{
		wxMutexLocker ml(m_torrent_queue_lock);
		m_torrent_queue[idx-1] = torrent;
		m_torrent_queue[idx] = prev_torrent;
		m_torrent_map[torrent->hash] = idx-1;
		m_torrent_map[prev_torrent->hash] = idx;
	}
	//m_torrent_queue.erase( torrent_it );
    //torrent_it = m_torrent_queue.begin() + idx;
    //m_torrent_queue.insert( torrent_it, prev_torrent );
}

void BitTorrentSession::MoveTorrentDown( shared_ptr<torrent_t>& torrent )
{
    long idx = -1;
    idx = find_torrent_from_hash( torrent->hash );

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
	shared_ptr<torrent_t> next_torrent(*( torrent_it + 1 ));
    wxLogDebug( _T( "Next %d now %d\n" ), next_torrent->config->GetQIndex(), torrent->config->GetQIndex() );
    long next_qindex = next_torrent->config->GetQIndex();
    long qindex = torrent->config->GetQIndex();
    torrent->config->SetQIndex( next_qindex );
    torrent->config->Save();
    next_torrent->config->SetQIndex( qindex );
    next_torrent->config->Save();
	{
		wxMutexLocker ml(m_torrent_queue_lock);
		m_torrent_queue[idx] = next_torrent;
		m_torrent_queue[idx+1] = torrent;
		m_torrent_map[torrent->hash] = idx+1;
		m_torrent_map[next_torrent->hash] = idx;
	}
    //m_torrent_queue.erase( torrent_it );
    //torrent_it = m_torrent_queue.begin() + idx + 1 ;
    //m_torrent_queue.insert( torrent_it, torrent );
}

void BitTorrentSession::ReannounceTorrent( shared_ptr<torrent_t>& t_torrent )
{
    long idx = -1;
    idx = find_torrent_from_hash( t_torrent->hash );

    if( idx < 0 )
    {
        wxLogError( _T( "ReannounceTorrent %s: Torrent not found in queue\n" ), t_torrent->name.c_str() );
        return ;
    }

    shared_ptr<torrent_t>& torrent = m_torrent_queue[idx];
    wxLogInfo( _T( "%s: Reannounce\n" ), torrent->name.c_str() );
    torrent_handle &h = torrent->handle;

    if( h.is_valid() )
    {
        h.force_reannounce();
    }
}

void BitTorrentSession::ConfigureTorrentFilesPriority( shared_ptr<torrent_t>& torrent )
{
    long idx = -1;
#if 0
    idx = find_torrent_from_hash( torrent->hash );

    if( idx < 0 )
    {
        wxLogError( _T( "ConfigureTorrentFilesPriority %s: Torrent not found in queue\n" ), torrent->name.c_str() );
        return ;
    }

    shared_ptr<torrent_t>& torrent = torrent;
#endif
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
    long idx = -1;
    libtorrent::torrent_handle& t_handle = torrent->handle;
#if 0
    idx = find_torrent_from_hash( torrent->hash );

    if( idx < 0 )
    {
        wxLogError( _T( "%s: Torrent not found in queue\n" ), torrent->name.c_str() );
        return ;
    }

    shared_ptr<torrent_t>& torrent = torrent;
#endif

    if( t_handle.is_valid() )
    { t_handle.replace_trackers( torrent->config->GetTrackersURL() ); }
}

void BitTorrentSession::ConfigureTorrent( shared_ptr<torrent_t>& torrent )
{
    long idx = -1;
#if 0
    idx = find_torrent_from_hash( torrent->hash );

    if( idx < 0 )
    {
        wxLogError( _T( "%s: Torrent not found in queue\n" ), torrent->name.c_str() );
        return ;
    }

    shared_ptr<torrent_t>& torrent = torrent;
#endif
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
            copysuccess = (CopyDirectory( oldpath, newpath ) != 0);

            if( copysuccess )
            {
                RemoveDirectory( oldpath );
            }
        }
        else
            if( wxFileExists( oldpath ) )
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
                copysuccess = 0;
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

	wxMutexLocker ml(m_torrent_queue_lock);

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
    else
        if( ( start_count < maxstart ) && ( queue_torrents.size() > 0 ) )
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
    std::auto_ptr<libtorrent::alert> a;
    a = m_libbtsession->pop_alert();
    wxString event_string;

    while( a.get() )
    {
        event_string.Empty();

        if( torrent_finished_alert* p = dynamic_cast<torrent_finished_alert*>( a.get() ) )
        {
            event_string << p->handle.get_torrent_info().name() << _T( ": " )
                         << a->message();
        }
        else
            if( peer_ban_alert* p = dynamic_cast<peer_ban_alert*>( a.get() ) )
            {
                event_string << _T( "peer: (" ) << p->ip.address().to_string() << _T( ")" ) << a->message();
            }
            else
                if( peer_error_alert* p = dynamic_cast<peer_error_alert*>( a.get() ) )
                {
                    event_string << _T( "peer: " ) << identify_client( p->pid ) << _T( ": " ) << a->message();
                }
                else
                    if( peer_blocked_alert* p = dynamic_cast<peer_blocked_alert*>( a.get() ) )
                    {
                        event_string << _T( "peer: (" ) << p->ip.to_string() << _T( ")" ) << a->message();
                    }
                    else
                        if( invalid_request_alert* p = dynamic_cast<invalid_request_alert*>( a.get() ) )
                        {
                            event_string << identify_client( p->pid ) << _T( ": " ) << a->message();
                        }
                        else
                            if( tracker_warning_alert* p = dynamic_cast<tracker_warning_alert*>( a.get() ) )
                            {
                                event_string << _T( "tracker: " ) << p->message();
                            }
                            else
                                if( tracker_reply_alert* p = dynamic_cast<tracker_reply_alert*>( a.get() ) )
                                {
                                    event_string << _T( "tracker:" ) << p->message() << _T( " (" ) << p->num_peers << _T( ")" );
                                }
                                else
                                    if( url_seed_alert* p = dynamic_cast<url_seed_alert*>( a.get() ) )
                                    {
                                        event_string << _T( "web seed '" ) << p->url << _T( "': " ) << p->message();
                                    }
                                    else
                                        if( peer_blocked_alert* p = dynamic_cast<peer_blocked_alert*>( a.get() ) )
                                        {
                                            event_string << _T( "(" ) << p->ip.to_string() << _T( ") " ) << p->message();
                                        }
                                        else
                                            if( listen_failed_alert* p = dynamic_cast<listen_failed_alert*>( a.get() ) )
                                            {
                                                event_string << _T( "listen failed: " ) << p->message();
                                            }
                                            else
                                                if( portmap_error_alert* p = dynamic_cast<portmap_error_alert*>( a.get() ) )
                                                {
                                                    event_string << _T( "portmap error: " ) << p->message();
                                                }
                                                else
                                                    if( portmap_alert* p = dynamic_cast<portmap_alert*>( a.get() ) )
                                                    {
                                                        event_string << _T( "portmap: " ) << p->message();
                                                    }
                                                    else
                                                        if( tracker_announce_alert* p = dynamic_cast<tracker_announce_alert*>( a.get() ) )
                                                        {
                                                            event_string << _T( "tracker: " ) << p->message();
                                                        }
                                                        else
                                                            if( file_error_alert* p = dynamic_cast<file_error_alert*>( a.get() ) )
                                                            {
                                                                event_string << _T( "file error: " ) << p->message();
                                                            }
                                                            else
                                                                if( tracker_reply_alert* p = dynamic_cast<tracker_reply_alert*>( a.get() ) )
                                                                {
                                                                    event_string << p->message() << _T( " number of peers " ) << p->num_peers;
                                                                }
                                                                else
                                                                    if( tracker_warning_alert* p = dynamic_cast<tracker_warning_alert*>( a.get() ) )
                                                                    {
                                                                        event_string << p->message();
                                                                    }
                                                                    else
                                                                        if( url_seed_alert* p = dynamic_cast<url_seed_alert*>( a.get() ) )
                                                                        {
                                                                            event_string << _T( "url-seed: " ) << p->message();
                                                                        }
                                                                        else
                                                                            if( hash_failed_alert* p = dynamic_cast<hash_failed_alert*>( a.get() ) )
                                                                            {
                                                                                event_string << _T( "hash: " ) << p->message();
                                                                            }
                                                                            else
                                                                                if( metadata_failed_alert* p = dynamic_cast<metadata_failed_alert*>( a.get() ) )
                                                                                {
                                                                                    event_string << _T( "metadata: " ) << p->message();
                                                                                }
                                                                                else
                                                                                    if( metadata_received_alert* p = dynamic_cast<metadata_received_alert*>( a.get() ) )
                                                                                    {
                                                                                        event_string << _T( "metadata: " ) << p->message();
                                                                                    }
                                                                                    else
                                                                                        if( fastresume_rejected_alert* p = dynamic_cast<fastresume_rejected_alert*>( a.get() ) )
                                                                                        {
                                                                                            event_string << _T( "fastresume: " ) << p->message();
                                                                                        }
                                                                                        else
                                                                                            if( fastresume_rejected_alert* p = dynamic_cast<fastresume_rejected_alert*>( a.get() ) )
                                                                                            {
                                                                                                event_string << _T( "fastresume: " ) << p->message();
                                                                                            }
                                                                                            else
                                                                                                if( save_resume_data_alert *p = dynamic_cast<save_resume_data_alert*>( a.get() ) )
                                                                                                {
                                                                                                    torrent_handle h = p->handle;

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
                                                                                                else
                                                                                                {
                                                                                                    event_string << a->message();
                                                                                                }

        //XXX include more alert from latest libtorrent
        //
        event_string << _T( '\n' ) << _T( '\0' );

        if( a->severity() == alert::fatal )
        {
            wxLogError( event_string );
        }
        else
            if( a->severity() == alert::critical )
            {
                wxLogError( event_string );
            }
            else
                if( a->severity() == alert::warning )
                {
                    wxLogWarning( event_string );
                }
                else
                    if( a->severity() == alert::info )
                    {
                        wxLogInfo( event_string );
                    }
                    else
                        if( a->severity() == alert::debug )
                        {
                            wxLogDebug( event_string );
                        }
                        else
                        {
                            wxLogDebug( event_string );
                        }

        a = m_libbtsession->pop_alert();
    }
}

