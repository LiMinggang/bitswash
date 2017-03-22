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

namespace lt = libtorrent;

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
	: SwashListCtrl( parent, SWASHLISTCOL_SIZE( torrentlistcols ), torrentlistcols, settings, id, pos, size, style ), m_torrentmenu( 0 )
{
}

TorrentListCtrl::~TorrentListCtrl()
{
}

wxString TorrentListCtrl::GetItemValue( long item, long columnid ) const
{
	MainFrame* pMainFrame = ( MainFrame* )( wxGetApp().GetTopWindow() );

	//wxLogDebug(_T("TorrentListCtrl::Showing %d items column %d"), torrentlistitems->size(), columnid);
	shared_ptr<torrent_t> torrent = pMainFrame->GetTorrent( item );
	if(!torrent)
	{
		wxLogError( _T( "TorrentListCtrl: Couldn't retrieve torrent queue\n" ) );
		return _T( "" );
	}

	stats_t& torrentstats = torrent->config->GetTorrentStats();
	lt::torrent_handle &torrenthandle = torrent->handle;
	shared_ptr<const lt::torrent_info> torrentinfo = torrent->info;
	lt::torrent_status torrentstatus;
	int torrentstoped = !torrenthandle.is_valid();

	if( !torrentstoped )
	{
		torrentstatus = torrenthandle.status(lt::torrent_handle::query_distributed_copies);
		torrentstats.progress = torrentstatus.progress;
		torrentstats.total_download = torrentstatus.total_payload_download;
		torrentstats.total_upload = torrentstatus.total_payload_upload;
	}

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
	wxString ret;

	switch( columnid )
	{
	case TORRENTLIST_COLUMN_INDEX:
		{
			int idx = torrent->config->GetQIndex();
			if(idx >= 0)
			{
				ret = wxString::Format( _T( "%d" ), torrent->config->GetQIndex() );
			}
			else
			{
				ret = wxT("--");
			}
		}
		break;

	case TORRENTLIST_COLUMN_TORRENT:
		ret = torrent->name;//wxString((torrentinfo.name().empty())?torrent->name:wxConvUTF8.cMB2WC( torrentinfo.name().c_str() ) );
		break;

	case TORRENTLIST_COLUMN_STATUS:
		if( torrent->config->GetTorrentState() == TORRENT_STATE_QUEUE )
		{
			ret = _( "Queueing" );
		}
		else if( ( torrent->config->GetTorrentState() == TORRENT_STATE_STOP ) ||
				 ( torrentstoped ) )
		{
			ret = _( "Stopped" );
		}
		else
		{
			if( ( torrent->config->GetTorrentState() == TORRENT_STATE_PAUSE ) ||
					( torrentstatus.paused ) )
			{
				ret = _( "Paused" );
			}
			else
			{
				if( torrentstatus.state < ( sizeof( state_str ) / ( sizeof( wxChar * ) ) ) )
				{ ret = wxString::Format( _T( "%s" ), state_str[torrentstatus.state] ); }
				else
				{ ret = _( "Error!" ); }
			}
		}

		break;

	case TORRENTLIST_COLUMN_SIZE:
		ret = HumanReadableByte( ( wxDouble ) ((torrentinfo) ? torrentinfo->total_size() : 0) );
		break;
	case TORRENTLIST_COLUMN_SELECTEDSIZE:
		ret = HumanReadableByte( ( wxDouble ) torrent->config->GetSelectedSize() );
		break;
	case TORRENTLIST_COLUMN_DOWNLOADED:
		ret = HumanReadableByte( ( wxDouble ) torrentstats.total_download );
		break;
	case TORRENTLIST_COLUMN_UPLOADED:
		ret = HumanReadableByte( ( wxDouble ) torrentstats.total_upload );
		break;

	case TORRENTLIST_COLUMN_PROGRESS:
		ret =  wxString::Format( _T( "%.02f%%" ), ( torrentstats.progress * 100 ) ) ;
		break;

	case TORRENTLIST_COLUMN_DOWNSPEED:
		if( torrentstoped )
		{ ret = _T( "0 Bps" ); }
		else
		{ ret = HumanReadableByte( ( wxDouble ) torrentstatus.download_payload_rate ) + wxString( _T( "ps" ) ); }

		break;

	case TORRENTLIST_COLUMN_UPSPEED:
		if( torrentstoped )
		{ ret = _T( "0 Bps" ); }
		else
		{ ret = HumanReadableByte( ( wxDouble ) torrentstatus.upload_payload_rate ) + wxString( _T( "ps" ) ); }

		break;

	case TORRENTLIST_COLUMN_TIMELEFT:
		if( ( torrentstatus.state == lt::torrent_status::seeding ) || ( torrentstatus.progress >= 1 ) )
		{
			ret = _T( "00:00:00" );
		}
		else if( torrentstatus.distributed_copies <= 0 )
		{
			ret = _T( "99:99:99" );
		}
		else
		{
			unsigned long byteleft = ( unsigned long )( ((torrentinfo) ? (( double )torrentinfo->total_size() * ( 1 - torrentstats.progress )) : 0) );
			unsigned long secleft = 999999;
			unsigned long downloadrate = torrentstatus.download_payload_rate ;

			if( downloadrate > 0 )
			{
				secleft =  byteleft / downloadrate;
			}

			//wxLogDebug(_T("XXX byteleft %ld, secleft %ld"), secleft);
			if( secleft > 360000 ) //approx 99:99:99 ??
			{
				ret = _T( "99:99:99" );
			}
			else
			{
				unsigned long hrleft = secleft / 3600;
				secleft %= 3600;
				unsigned long mnleft = secleft / 60 ;
				secleft %= 60;
				ret = wxString::Format( _T( "%2d:%02d:%02d" ), ( unsigned long )hrleft, ( unsigned long )mnleft, ( unsigned long )secleft );
			}
		}

		break;

	case TORRENTLIST_COLUMN_PEER:
		if( torrentstoped )
		{ ret = _T( "0/0" ); }
		else
		{ ret = wxString::Format( _T( "%d/%d" ), torrentstatus.num_peers, torrentstatus.num_seeds ); }

		break;

	case TORRENTLIST_COLUMN_COPIES:
		if( torrentstoped || torrentstatus.distributed_copies < 0.f)
		{ ret = _T( "--" ); }
		else
		{ ret = wxString::Format( _T( "%.02f" ), torrentstatus.distributed_copies ); }

		break;

	case TORRENTLIST_COLUMN_RATIO:
		if( torrentstats.total_download > 0 )
		{ ret = wxString::Format( _T( "%.02f" ), ( double )( torrentstats.total_upload ) / ( double )torrentstats.total_download ); }
		else
		{ ret = _( "inf" ); }

		break;

	default:
		ret = _T( "" );
	}

	return ret;
}

#if USE_CONTEXT_MENU
void TorrentListCtrl::ShowContextMenu( const wxPoint& pos )
{
	if( GetSelectedItemCount() > 0 )
	{
		MainFrame* pMainFrame = ( MainFrame* ) wxGetApp().GetTopWindow();

		if( !m_torrentmenu )
		{
			m_torrentmenu = pMainFrame->GetNewTorrentMenu();
		}

		pMainFrame->TorrentOperationMenu( m_torrentmenu );
		PopupMenu( m_torrentmenu, pos );
	}
}
#endif

