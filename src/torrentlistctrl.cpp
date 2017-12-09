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
// Class: TorrentListCtrl
//
// Created by: lim k. c. <admin@bitswash.org>
// Created on: Mon Feb  5 11:09:21 2007
//
//
#include <wx/thread.h>
#include "torrentlistctrl.h"
#include "mainframe.h"
#include "functions.h"

//namespace lt = libtorrent;

// TorrentListCtrl

enum torrentlistcolumnid
{
	TORRENTLIST_COLUMN_INDEX = 0,
	TORRENTLIST_COLUMN_TORRENT,
	TORRENTLIST_COLUMN_STATUS,
	TORRENTLIST_COLUMN_SIZE,
	TORRENTLIST_COLUMN_SELECTEDSIZE,
	TORRENTLIST_COLUMN_PROGRESS,
	TORRENTLIST_COLUMN_DOWNSPEED,
	TORRENTLIST_COLUMN_UPSPEED,
	TORRENTLIST_COLUMN_PEER,
	TORRENTLIST_COLUMN_DOWNLOADED,
	TORRENTLIST_COLUMN_UPLOADED,
	TORRENTLIST_COLUMN_COPIES,
	TORRENTLIST_COLUMN_TIMELEFT,
	TORRENTLIST_COLUMN_RATIO
};

#ifndef __WXMSW__
static SwashColumnItem torrentlistcols[] =
{
	{ id: TORRENTLIST_COLUMN_INDEX, name: _( "Index" ), title: _( "Index" ), tooltip: _( "Queue Index" ), width: 0, show: false},
	{ id: TORRENTLIST_COLUMN_ORRENT, name: _( "Torrent" ), title: _( "Torrent" ), tooltip: _( "Torrent name" ), width: 256, show: true },
	{ id: TORRENTLIST_COLUMN_STATUS, name: _( "Status" ), title: _( "Status" ), tooltip: _( "Torrent status" ), width: 60, show: true },
	{ id: TORRENTLIST_COLUMN_SIZE, name: _( "Size" ), title: _( "Size" ), tooltip: _( "Torrent size" ), width: 80, show: true },
	{ id: TORRENTLIST_COLUMN_SELECTEDSIZE, name: _( "Selected Size" ), title: _( "Selected Size" ), tooltip: _( "Selected file size" ), width: 80, show: true },
	{ id: TORRENTLIST_COLUMN_PROGRESS, name: _( "Progress" ), title: _( "Progress" ), tooltip: _( "Progress" ), width: 68, show: true },
	{ id: TORRENTLIST_COLUMN_DOWNSPEED, name: _( "Download Speed" ), title: _( "Download Speed" ), tooltip: _( "Download Speed" ), width: 80, show: true },
	{ id: TORRENTLIST_COLUMN_UPSPEED, name: _( "Upload Speed" ), title: _( "Upload Speed" ), tooltip: _( "Upload Speed" ), width: 80, show: true },
	{ id: TORRENTLIST_COLUMN_PEER, name: _( "Peers" ), title: _( "Peers" ), tooltip: _( "Number or peers" ), width: 50, show: true },
	{ id: TORRENTLIST_COLUMN_DOWNLOADED, name: _( "Downloaded" ), title: _( "Downloaded" ), tooltip: _( "Total bytes downloaded" ), width: 80, show: false },
	{ id: TORRENTLIST_COLUMN_UPLOADED, name: _( "Uploaded" ), title: _( "Uploaded" ), tooltip: _( "Total bytes uploaded" ), width: 80, show: false },
	{ id: TORRENTLIST_COLUMN_COPIES, name: _( "Copies" ), title: _( "Copies" ), tooltip: _( "Number of copies in swarm" ), width: 54, show: true },
	{ id: TORRENTLIST_COLUMN_IMELEFT, name: _( "Time Left" ), title: _( "Time Left" ), tooltip: _( "Estimated time left to finish" ), width: 70, show: true },
	{ id: TORRENTLIST_COLUMN_RATIO, name: _( "Ratio" ), title: _( "Share Ratio" ), tooltip: _( "Upload/Download ratio" ), width: 82, show: true },
};
#else
static SwashColumnItem torrentlistcols[] =
{
	{  TORRENTLIST_COLUMN_INDEX,  _( "Index" ),  _( "Index" ), _( "Queue Index" ),  0,  false},
	{  TORRENTLIST_COLUMN_TORRENT,  _( "Torrent" ),  _( "Torrent" ), _( "Torrent name" ),  256,  true },
	{  TORRENTLIST_COLUMN_STATUS,  _( "Status" ),  _( "Status" ), _( "Torrent status" ),  60,  true },
	{  TORRENTLIST_COLUMN_SIZE,  _( "Size" ),  _( "Size" ), _( "Torrent size" ),  80,  true },
	{  TORRENTLIST_COLUMN_SELECTEDSIZE, _( "Selected Size" ), _( "Selected Size" ), _( "Selected file size" ), 80, true },
	{  TORRENTLIST_COLUMN_PROGRESS,  _( "Progress" ),  _( "Progress" ), _( "Progress" ),  68,  true },
	{  TORRENTLIST_COLUMN_DOWNSPEED,  _( "Download Speed" ),  _( "Download Speed" ), _( "Download Speed" ),  128,  true },
	{  TORRENTLIST_COLUMN_UPSPEED,  _( "Upload Speed" ),  _( "Upload Speed" ), _( "Upload Speed" ),  128,  true },
	{  TORRENTLIST_COLUMN_PEER,  _( "Peers" ),  _( "Peers" ), _( "Number or peers" ),  50,  true },
	{  TORRENTLIST_COLUMN_DOWNLOADED,  _( "Downloaded" ),  _( "Downloaded" ), _( "Total bytes downloaded" ),  92,  true },
	{  TORRENTLIST_COLUMN_UPLOADED,  _( "Uploaded" ),  _( "Uploaded" ), _( "Total bytes uploaded" ),  92,  true },
	{  TORRENTLIST_COLUMN_COPIES,  _( "Copies" ),  _( "Copies" ), _( "Number of copies in swarm" ),  54,  true },
	{  TORRENTLIST_COLUMN_TIMELEFT,  _( "Time Left" ),  _( "Time Left" ), _( "Estimated time left to finish" ),  70,  true },
	{  TORRENTLIST_COLUMN_RATIO,  _( "Share Ratio" ),  _( "Share Ratio" ), _( "Upload/Download ratio" ),  82,  true },
};
#endif
TorrentListCtrl::TorrentListCtrl( wxWindow *parent,
								  const wxString settings,
								  const wxWindowID id,
								  const wxPoint& pos,
								  const wxSize& size,
								  long style )
	: SwashListCtrl( parent, SWASHLISTCOL_SIZE( torrentlistcols ), torrentlistcols, settings, id, pos, size, style ), m_torrentmenu( nullptr )
{
	m_pMainFrame = dynamic_cast<MainFrame *> ( parent );
	wxASSERT(m_pMainFrame != nullptr);
}

TorrentListCtrl::~TorrentListCtrl()
{
}

wxString TorrentListCtrl::GetItemValue( long item, long columnid ) const
{
	//wxLogDebug(_T("TorrentListCtrl::Showing %d items column %d"), torrentlistitems->size(), columnid);
	std::shared_ptr<torrent_t> torrent = m_pMainFrame->GetTorrent( item );
	if(!torrent)
	{
		wxLogError( _T( "Couldn't retrieve torrent %d\n" ), item );
		return _T( "" );
	}

	std::shared_ptr<TorrentConfig>& tconfig = torrent->config;
	static const wxChar *state_str[] =
	{
		_( "Queued" ),
		_( "Checking" ),
		_( "Downloading meta data" ),
		_( "Downloading" ),
		_( "Finished" ),
		_( "Seeding" ),
		_( "Allocating" ),
		_( "Checking resume data" )
	};
	static size_t state_size = ( sizeof( state_str ) / ( sizeof( wxChar * ) ) );

	switch( columnid )
	{
	case TORRENTLIST_COLUMN_INDEX:
		{
			int idx = tconfig->GetQIndex();
			if(idx >= 0) return wxString::Format( _T( "%d" ), idx + 1 );
			else return wxT("--");
		}

	case TORRENTLIST_COLUMN_TORRENT:
		return torrent->name;//wxString((torrentinfo.name().empty())?torrent->name:wxConvUTF8.cMB2WC( torrentinfo.name().c_str() ) );

	case TORRENTLIST_COLUMN_STATUS:
		{
			lt::torrent_handle &torrenthandle = torrent->handle;
			bool torrentstoped = !torrenthandle.is_valid();
			
			if( tconfig->GetTorrentState() == TORRENT_STATE_QUEUE )
			{
				return _( "Queueing" );
			}
			else if( ( tconfig->GetTorrentState() == TORRENT_STATE_STOP ) ||
					 ( torrentstoped ) )
			{
				return _( "Stopped" );
			}
			else
			{
				if( tconfig->GetTorrentState() == TORRENT_STATE_PAUSE )
				{
					return _( "Paused" );
				}
				else
				{
					if( torrenthandle.is_valid() )
					{
						lt::torrent_status torrentstatus;
						torrentstatus = torrenthandle.status(lt::torrent_handle::query_distributed_copies);
						if( torrentstatus.state < state_size )
						{ return wxString::Format( _T( "%s" ), _( state_str[torrentstatus.state] ) ); }
						else
						{ return _( "Error!" ); }
					}
					else
					{ return _( "Error!" ); }
				}
			}
		}

	case TORRENTLIST_COLUMN_SIZE:
		return HumanReadableByte( ( wxDouble ) tconfig->GetTotalSize() );
	case TORRENTLIST_COLUMN_SELECTEDSIZE:
		return HumanReadableByte( ( wxDouble ) tconfig->GetSelectedSize() );
	default:
		{
			stats_t& torrentstats = tconfig->GetTorrentStats();
			lt::torrent_handle &torrenthandle = torrent->handle;
			lt::torrent_status torrentstatus;
			bool torrentstoped = !torrenthandle.is_valid();

			if( torrenthandle.is_valid() )
			{
				torrentstatus = torrenthandle.status(lt::torrent_handle::query_distributed_copies);
				torrentstats.progress = torrentstatus.progress;
				torrentstats.total_download = torrentstatus.total_payload_download;
				torrentstats.total_upload = torrentstatus.total_payload_upload;
			}
			switch(columnid)
			{
			case TORRENTLIST_COLUMN_DOWNLOADED:
				return HumanReadableByte( ( wxDouble ) torrentstats.total_download );
			case TORRENTLIST_COLUMN_UPLOADED:
				return HumanReadableByte( ( wxDouble ) torrentstats.total_upload );

			case TORRENTLIST_COLUMN_PROGRESS:
				return wxString::Format( _T( "%.02f%%" ), ( torrentstats.progress * 100 ) ) ;

			case TORRENTLIST_COLUMN_DOWNSPEED:
				if( torrentstoped )
				{ return _T( "0 Bps" ); }
				else
				{ return HumanReadableByte( ( wxDouble ) torrentstatus.download_payload_rate ) + wxString( _T( "ps" ) ); }

			case TORRENTLIST_COLUMN_UPSPEED:
				if( torrentstoped )
				{ return _T( "0 Bps" ); }
				else
				{ return HumanReadableByte( ( wxDouble ) torrentstatus.upload_payload_rate ) + wxString( _T( "ps" ) ); }

			case TORRENTLIST_COLUMN_TIMELEFT:
				if( ( (torrenthandle.is_valid()) && ( torrentstatus.state == lt::torrent_status::seeding )) || ( torrentstatus.progress >= 1 ) )
				{
					return _T( "00:00:00" );
				}
				else if( torrentstatus.distributed_copies <= 0 )
				{
					return _T( "99:99:99" );
				}
				else
				{
					unsigned long byteleft = ( unsigned long )( tconfig->GetTotalSize() );
					unsigned long secleft = 999999;
					unsigned long downloadrate = torrentstatus.download_payload_rate ;

					if( downloadrate > 0 )
					{
						secleft =  byteleft / downloadrate;
					}

					//wxLogDebug(_T("XXX byteleft %ld, secleft %ld"), secleft);
					if( secleft > 360000 ) //approx 99:99:99 ??
					{
						return _T( "99:99:99" );
					}
					else
					{
						unsigned long hrleft = secleft / 3600;
						secleft %= 3600;
						unsigned long mnleft = secleft / 60 ;
						secleft %= 60;
						return wxString::Format( _T( "%2d:%02d:%02d" ), ( unsigned long )hrleft, ( unsigned long )mnleft, ( unsigned long )secleft );
					}
				}

			case TORRENTLIST_COLUMN_PEER:
				if( torrentstoped )
				{ return _T( "0/0" ); }
				else
				{ return wxString::Format( _T( "%d/%d" ), torrentstatus.num_peers, torrentstatus.num_seeds ); }

			case TORRENTLIST_COLUMN_COPIES:
				if( torrentstoped || torrentstatus.distributed_copies < 0.f)
				{ return _T( "--" ); }
				else
				{ return wxString::Format( _T( "%.02f" ), torrentstatus.distributed_copies ); }

			case TORRENTLIST_COLUMN_RATIO:
				if( torrentstats.total_download > 0 )
				{ return wxString::Format( _T( "%.02f" ), ( double )( torrentstats.total_upload ) / ( double )torrentstats.total_download ); }
				else
				{ return _( "inf" ); }

			default:
				return _T( "" );
			}
		}
	}

	return _T("");
}

void TorrentListCtrl::OnLeftDClick(wxMouseEvent& event)
{
	auto pTorrent = m_pMainFrame->GetSelectedTorrent();

	if( pTorrent )
	{
		bool nopriority = false;
		lt::torrent_info const& torrentinfo = *(pTorrent->info);
		std::vector<lt::download_priority_t> & filespriority = pTorrent->config->GetFilesPriorities();
		
		if( filespriority.size() != torrentinfo.num_files() )
		{
			nopriority = true;
		}

		for(int i  = 0; i < torrentinfo.num_files(); ++i)
		{
			if(nopriority || filespriority.at(i) != TorrentConfig::file_none)
			{
				lt::file_storage const& allfiles = torrentinfo.files();
				lt::file_index_t idx(i);
				wxString fname(pTorrent->config->GetDownloadPath() + wxString::FromUTF8((allfiles.file_path(idx)).c_str()));
				wxFileName filename (fname);
				filename.MakeAbsolute();
				//wxLogDebug(_T("File path %s\n"), filename.GetFullPath().c_str());
				if(wxFileName::FileExists(filename.GetFullPath()))
				{
					wxLaunchDefaultApplication(filename.GetFullPath()); 
				}
			}
		}
	}
}


#if USE_CONTEXT_MENU
void TorrentListCtrl::ShowContextMenu( const wxPoint& pos )
{
	if( GetSelectedItemCount() > 0 )
	{
		if( !m_torrentmenu )
		{
			m_torrentmenu = m_pMainFrame->GetNewTorrentMenu();
		}

		//m_pMainFrame->TorrentOperationMenu( m_torrentmenu );
		PopupMenu( m_torrentmenu, pos );
	}
}
#endif

