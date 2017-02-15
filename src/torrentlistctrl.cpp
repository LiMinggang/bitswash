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

BEGIN_EVENT_TABLE(TorrentListCtrl, SwashListCtrl)
END_EVENT_TABLE()
// TorrentListCtrl

enum torrentlistcolumnid {
	TORRENTLIST_COLUMN_INDEX=0,
	TORRENTLIST_COLUMN_TORRENT,
	TORRENTLIST_COLUMN_STATUS,
	TORRENTLIST_COLUMN_SIZE,
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
	{ id: TORRENTLIST_COLUMN_INDEX, name: _T("index"), title: _("Index"), tooltip:_("Queue Index"), width: 0, show: false},
	{ id: TORRENTLIST_COLUMN_TORRENT, name: _T("torrent"), title: _("Torrent"), tooltip:_("Torrent name"), width: 256, show: true },
	{ id: TORRENTLIST_COLUMN_STATUS, name: _T("status"), title: _("Status"), tooltip:_("Torrent status"), width: 60, show: true }, 
	{ id: TORRENTLIST_COLUMN_SIZE, name: _T("size"), title: _("Size"), tooltip:_("Torrent size"), width: 80, show: true },
	{ id: TORRENTLIST_COLUMN_PROGRESS, name: _T("progress"), title: _("Progress"), tooltip:_("Progress"), width: 68, show: true },
	{ id: TORRENTLIST_COLUMN_DOWNSPEED, name: _T("downspeed"), title: _("Download Speed"), tooltip:_("Download Speed"), width: 80, show: true },
	{ id: TORRENTLIST_COLUMN_UPSPEED, name: _T("upspeed"), title: _("Upload Speed"), tooltip:_("Upload Speed"), width: 80, show: true },
	{ id: TORRENTLIST_COLUMN_PEER, name: _T("peer"), title: _("Peers"), tooltip:_("Number or peers"), width: 50, show: true },
	{ id: TORRENTLIST_COLUMN_DOWNLOADED, name: _T("downloaded"), title: _("Downloaded"), tooltip:_("Total bytes downloaded"), width: 80, show: false },
	{ id: TORRENTLIST_COLUMN_UPLOADED, name: _T("uploaded"), title: _("Uploaded"), tooltip:_("Total bytes uploaded"), width: 80, show: false },
	{ id: TORRENTLIST_COLUMN_COPIES, name: _T("copies"), title: _("Copies"), tooltip:_("Number of copies in swarm"), width: 54, show: true },
	{ id: TORRENTLIST_COLUMN_TIMELEFT, name: _T("timeleft"), title: _("Time Left"), tooltip:_("Estimated time left to finish"), width: 70, show: true },
	{ id: TORRENTLIST_COLUMN_RATIO, name: _T("ratio"), title: _("Share Ratio"), tooltip:_("Upload/Download ratio"), width: 82, show: true },
};
#else
static SwashColumnItem torrentlistcols[] = 
{
	{  TORRENTLIST_COLUMN_INDEX,  _T("index"),  _("Index"), _("Queue Index"),  0,  false},
	{  TORRENTLIST_COLUMN_TORRENT,  _T("torrent"),  _("Torrent"), _("Torrent name"),  256,  true },
	{  TORRENTLIST_COLUMN_STATUS,  _T("status"),  _("Status"), _("Torrent status"),  60,  true }, 
	{  TORRENTLIST_COLUMN_SIZE,  _T("size"),  _("Size"), _("Torrent size"),  80,  true },
	{  TORRENTLIST_COLUMN_PROGRESS,  _T("progress"),  _("Progress"), _("Progress"),  68,  true },
	{  TORRENTLIST_COLUMN_DOWNSPEED,  _T("downspeed"),  _("Download Speed"), _("Download Speed"),  128,  true },
	{  TORRENTLIST_COLUMN_UPSPEED,  _T("upspeed"),  _("Upload Speed"), _("Upload Speed"),  128,  true },
	{  TORRENTLIST_COLUMN_PEER,  _T("peer"),  _("Peers"), _("Number or peers"),  50,  true },
	{  TORRENTLIST_COLUMN_DOWNLOADED,  _T("downloaded"),  _("Downloaded"), _("Total bytes downloaded"),  92,  true },
	{  TORRENTLIST_COLUMN_UPLOADED,  _T("uploaded"),  _("Uploaded"), _("Total bytes uploaded"),  92,  true },
	{  TORRENTLIST_COLUMN_COPIES,  _T("copies"),  _("Copies"), _("Number of copies in swarm"),  54,  true },
	{  TORRENTLIST_COLUMN_TIMELEFT,  _T("timeleft"),  _("Time Left"), _("Estimated time left to finish"),  70,  true },
	{  TORRENTLIST_COLUMN_RATIO,  _T("ratio"),  _("Share Ratio"), _("Upload/Download ratio"),  82,  true },
};
#endif
TorrentListCtrl::TorrentListCtrl(wxWindow *parent,
				const wxString settings,
               const wxWindowID id,
               const wxPoint& pos,
               const wxSize& size,
               long style)
        : SwashListCtrl(parent, SWASHLISTCOL_SIZE(torrentlistcols), torrentlistcols, settings, id, pos, size, style), m_torrentmenu(0)
{

}

TorrentListCtrl::~TorrentListCtrl()
{

}

wxString TorrentListCtrl::GetItemValue(long item, long columnid) const
{
	TorrentListCtrl* pThis = const_cast<TorrentListCtrl*>(this);

	MainFrame* pMainFrame = (MainFrame*)(wxGetApp().GetTopWindow());
	const std::vector<torrent_t*> *torrentlistitems = pMainFrame->GetTorrentList();

	if (torrentlistitems == NULL)
	{
		wxLogError(_T("TorrentListCtrl: Couldn't retrieve torrent queue\n"));
		return _T("");
	}

	//wxLogDebug(_T("TorrentListCtrl::Showing %d items column %d\n"), torrentlistitems->size(), columnid);
	torrent_t &torrent= *(torrentlistitems->at(item));
	stats_t& torrentstats = torrent.config->GetTorrentStats();

	libtorrent::torrent_handle &torrenthandle = torrent.handle;

	libtorrent::torrent_info const& torrentinfo = *torrent.info;

	libtorrent::torrent_status torrentstatus;

	int torrentstoped = !torrenthandle.is_valid();
	
	if (!torrentstoped)
	{
		torrentstatus = torrenthandle.status();

		torrentstats.progress = torrentstatus.progress;
		torrentstats.total_download = torrentstatus.total_payload_download;
		torrentstats.total_upload = torrentstatus.total_payload_upload;

	}

	static const wxChar *state_str[] = {
			_("queued"),
			_("checking"),
			_("downloading meta data"),
			_("downloading"),
			_("finished"),
			_("seeding"),
			_("allocating"),
			_("checking resume data")
	};

	wxString ret;
 	switch(columnid) 
	{
		case TORRENTLIST_COLUMN_INDEX:
			ret = wxString::Format(_T("%d"), torrent.config->GetQIndex());
			break;
		case TORRENTLIST_COLUMN_TORRENT:
			ret = wxString(wxConvUTF8.cMB2WC(torrentinfo.name().c_str()));
			break;
		case TORRENTLIST_COLUMN_STATUS:
			
			if (torrent.config->GetTorrentState() == TORRENT_STATE_QUEUE)
			{
				ret = _("Queueing");
			} 
			else if ( (torrent.config->GetTorrentState() == TORRENT_STATE_STOP) ||
					(torrentstoped) )
			{
				ret = _("Stopped");
			}
			else
			{
				if((torrent.config->GetTorrentState() == TORRENT_STATE_PAUSE ) || 
								(torrentstatus.paused))
				{
					ret = _("Paused");
				}
				else
				{
					if(torrentstatus.state < (sizeof(state_str)/(sizeof(wxChar *))))
						ret = wxString::Format(_T("%s"),state_str[torrentstatus.state]);
					else
						ret = _("Error!");
				}
			}
			break;
		case TORRENTLIST_COLUMN_SIZE:
			ret = HumanReadableByte((wxDouble) torrentinfo.total_size());
			break;
		case TORRENTLIST_COLUMN_DOWNLOADED:
			ret = HumanReadableByte((wxDouble) torrentstats.total_download);
			break;
		case TORRENTLIST_COLUMN_UPLOADED:
			ret = HumanReadableByte((wxDouble) torrentstats.total_upload);
			break;
		case TORRENTLIST_COLUMN_PROGRESS:
			ret =  wxString::Format(_T("%.02f%%"), (torrentstats.progress * 100)) ;
			break;
		case TORRENTLIST_COLUMN_DOWNSPEED:
			if(torrentstoped)
				ret = _T("0 Bps");
			else
				ret = HumanReadableByte((wxDouble) torrentstatus.download_payload_rate) + wxString(_T("ps"));
			break;
		case TORRENTLIST_COLUMN_UPSPEED:
			if(torrentstoped)
				ret = _T("0 Bps");
			else
				ret = HumanReadableByte((wxDouble) torrentstatus.upload_payload_rate) + wxString(_T("ps"));
			break;
		case TORRENTLIST_COLUMN_TIMELEFT:
			if((torrentstatus.state == libtorrent::torrent_status::seeding )|| (torrentstatus.progress >= 1))
			{
				ret = _T("00:00:00");
			}
			else if (torrentstatus.distributed_copies <= 0 )
			{
				ret = _T("99:99:99");
			} 
			else
			{
				unsigned long byteleft =(unsigned long)( (double)torrentinfo.total_size() * (1 - torrentstats.progress));
				unsigned long secleft = 999999;
				unsigned long downloadrate = torrentstatus.download_payload_rate ;
				if ( downloadrate > 0)
				{
					secleft =  byteleft / downloadrate;
				}

				//wxLogDebug(_T("XXX byteleft %ld, secleft %ld\n"), secleft);
				if (secleft > 360000 ) //approx 99:99:99 ?? 
				{
					ret = _T("99:99:99");
				}
				else
				{
					unsigned long hrleft = secleft / 3600;
					secleft %= 3600;
					unsigned long mnleft = secleft / 60 ;
					secleft %= 60;

					ret = wxString::Format(_T("%2d:%02d:%02d"), (unsigned long)hrleft, (unsigned long)mnleft,(unsigned long)secleft);
				}
			}
			break;
		case TORRENTLIST_COLUMN_PEER:
			if(torrentstoped)
				ret = _T("0/0");
			else
				ret = wxString::Format(_T("%d/%d"), torrentstatus.num_peers, torrentstatus.num_seeds);
			break;
		case TORRENTLIST_COLUMN_COPIES:
			if(torrentstoped)
				ret = _T("0");
			else
				ret = wxString::Format(_T("%.02f"), torrentstatus.distributed_copies);
			break;
		case TORRENTLIST_COLUMN_RATIO:
			if(torrentstats.total_download > 0) 
				ret = wxString::Format(_T("%.02f"), (double) (torrentstats.total_upload) / (double)torrentstats.total_download);
			else
				ret = _("inf");
			break;
		default:
			ret = _T("");
	}

	
	return ret;
	
	
}

#if USE_CONTEXT_MENU
void TorrentListCtrl::ShowContextMenu(const wxPoint& pos)
{
	MainFrame* pMainFrame = (MainFrame*) wxGetApp().GetTopWindow();
	if(!m_torrentmenu)
	{
		m_torrentmenu = pMainFrame->GetNewTorrentMenu();
	}

	pMainFrame->TorrentOperationMenu(m_torrentmenu, ( GetSelectedItemCount() > 0 ));

	PopupMenu(m_torrentmenu, pos);
}
#endif

