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
// Class: TorrentInfo
//
// Created by: lim k. c. <admin@bitswash.org>
// Created on: Sat Feb  3 13:14:01 2007
//

#include "mainframe.h"
#include "torrentinfo.h"

//namespace lt = libtorrent;

enum bw_torrentinfo_page {
	BW_TORRENTINFO_SUMMARY=0,
	BW_TORRENTINFO_PEERLIST,
	BW_TORRENTINFO_FILELIST,
	BW_TORRENTINFO_TRACKERLIST,
	BW_TORRENTINFO_LOGGER,
	BW_TORRENTINFO_CHUNK,
};

const long TorrentInfo::TORRENT_INFO_ID = wxNewId();

TorrentInfo::TorrentInfo(wxWindow *parent,
               const wxWindowID id,
               const wxPoint& pos,
               const wxSize& size,
               long style) 
: wxAuiNotebook(parent, id,  pos, size, style)
{
	Configuration* config = wxGetApp().GetConfig();

	m_pMainFrame = (wxFrame*)parent;

	m_summarypane = new SummaryPane(this, wxID_ANY, wxDefaultPosition, wxSize(size.GetWidth(), -1));
    InsertPage(BW_TORRENTINFO_SUMMARY, m_summarypane, _("Status") );

	m_peerspane = new PeerListCtrl(this, config->GetPeerListCtrlSetting(), wxID_ANY);
	InsertPage(BW_TORRENTINFO_PEERLIST, m_peerspane , _("Peers") );
	m_filespane = new FileListCtrl(this, config->GetFileListCtrlSetting(), wxID_ANY);
	InsertPage(BW_TORRENTINFO_FILELIST, m_filespane, _("Files") );

	m_trackerspane = new TrackerListCtrl(this, config->GetTrackerListCtrlSetting(), wxID_ANY);
	InsertPage(BW_TORRENTINFO_TRACKERLIST, m_trackerspane, _("Trackers") );

	m_logger_panel = new LoggerCtrl (this, wxLog::GetActiveTarget(), wxID_ANY, wxDefaultPosition, wxSize(size.GetWidth(), -1) );

	InsertPage(BW_TORRENTINFO_LOGGER, m_logger_panel, _("Log") );
	Bind(wxEVT_AUINOTEBOOK_PAGE_CHANGED, &TorrentInfo::OnNotebookChanged, this, TORRENT_INFO_ID);
}

TorrentInfo::~TorrentInfo()
{
}

void TorrentInfo::UpdateTorrentInfo(bool updateall) 
{
	bool summary_updated, peers_updated, files_updated, trackers_updated;
	summary_updated = peers_updated = files_updated = trackers_updated = false;
	switch(GetSelection())
	{
		case BW_TORRENTINFO_SUMMARY:
			if(m_summarypane != nullptr)
			{
				m_summarypane->UpdateSummary();
				summary_updated = true;
			}
			break;
		case BW_TORRENTINFO_PEERLIST:
			if (m_peerspane != nullptr)
			{
				m_peerspane->UpdateSwashList();
				peers_updated = true;
			}
			break;
		case BW_TORRENTINFO_FILELIST:
			if (m_filespane != nullptr)
			{
				m_filespane->UpdateSwashList();
				files_updated = true;
			}
			break;
		case BW_TORRENTINFO_TRACKERLIST:
			if (m_trackerspane != nullptr)
			{
				m_trackerspane->UpdateSwashList();
				trackers_updated = true;
			}
			break;
		default:
			break;
	}

	if (updateall) 
	{
		if(!summary_updated && (m_summarypane != nullptr))
			m_summarypane->UpdateSummary();

		if(!peers_updated && (m_peerspane != nullptr))
			m_peerspane->UpdateSwashList();
		
		if(!files_updated && (m_filespane != nullptr))
			m_filespane->UpdateSwashList();

		if(!trackers_updated && (m_trackerspane != nullptr))
			m_trackerspane->UpdateSwashList();
	} 
}

void TorrentInfo::OnNotebookChanged(wxAuiNotebookEvent& event)
{
	UpdateTorrentInfo(false);
}
