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
// Class: TrackerListCtrl
//
// Created by: lim k. c. <admin@bitswash.org>
// Created on: Wed Mar 19 23:57:27 EDT 2008
//

#include <wx/clipbrd.h>
#include "torrentinfo.h"
#include "trackerlistctrl.h"
#include "functions.h"

#include "mainframe.h"
//namespace lt = libtorrent;

enum
{
	TRACKERLISTCTRL_MENU_ADD= 10000,
	TRACKERLISTCTRL_MENU_EDIT ,
	TRACKERLISTCTRL_MENU_DELETE,
	TRACKERLISTCTRL_MENU_COPY,
	TRACKERLISTCTRL_MENU_REANNOUCE,
};

// TrackerListCtrl

enum torrentlistcolumnid {
	TRACKERLIST_COLUMN_URL = 0,
	TRACKERLIST_COLUMN_STATUS,
	TRACKERLIST_COLUMN_NEXT_ANNOUNCE,
};

#ifndef __WXMSW__
static SwashColumnItem trackerlistcols[] = 
{
	{ id: TRACKERLIST_COLUMN_URL, name: _T("URL"), title: _("URL"), tooltip:_("Torrent Tracker URL"), width: 450, show: true},
	{ id: TRACKERLIST_COLUMN_STATUS, name: _T("Status"), title: _("Status"), tooltip:_("Torrent Tracker Status"), width: 100, show: true},
	{ id: TRACKERLIST_COLUMN_NEXT_ANNOUNCE, name: _T("Next Announce Time"), title: _("Next Announce Time"), tooltip:_("Torrent Tracker Next Announce Time"), width: 100, show: true},
};
#else
static SwashColumnItem trackerlistcols[] = 
{
	{ TRACKERLIST_COLUMN_URL,  _T("URL"),  _("URL"), _("Torrent Tracker URL"),  450,  true},
	{ TRACKERLIST_COLUMN_STATUS,  _T("Status"), _("Status"), _("Torrent Tracker Status"), 100, true},
	{ TRACKERLIST_COLUMN_NEXT_ANNOUNCE, _T("Next Announce Time"), _("Next Announce Time"), _("Torrent Tracker Next Announce Time"), 100, true},
};
#endif
TrackerListCtrl::TrackerListCtrl(wxWindow *parent,
               const wxString settings,
               const wxWindowID id,
               const wxPoint& pos,
               const wxSize& size,
               long style)
			: SwashListCtrl(parent, SWASHLISTCOL_SIZE(trackerlistcols), trackerlistcols, settings, id, pos, size, style),
			m_pTorrent(nullptr)
{
	Bind( wxEVT_MENU, &TrackerListCtrl::OnMenuTracker, this, TRACKERLISTCTRL_MENU_ADD);
	Bind( wxEVT_MENU, &TrackerListCtrl::OnMenuTracker, this, TRACKERLISTCTRL_MENU_EDIT);
	Bind( wxEVT_MENU, &TrackerListCtrl::OnMenuTracker, this, TRACKERLISTCTRL_MENU_DELETE);
	Bind( wxEVT_MENU, &TrackerListCtrl::OnMenuTracker, this, TRACKERLISTCTRL_MENU_COPY);
	Bind( wxEVT_MENU, &TrackerListCtrl::OnMenuTracker, this, TRACKERLISTCTRL_MENU_REANNOUCE);
}

TrackerListCtrl::~TrackerListCtrl()
{
}

wxString TrackerListCtrl::GetItemValue(long item, long columnid) const
{
	static MainFrame* pMainFrame = nullptr;
	if(pMainFrame == nullptr)
	{
		pMainFrame = ( dynamic_cast< MainFrame* >(wxGetApp().GetTopWindow()));
		wxASSERT(pMainFrame != nullptr);
	}

	wxString ret(_T(""));

	//wxLogDebug(_T("TrackerListCtrl column %ld of item %ld"), columnid, item);
	std::shared_ptr<torrent_t> pTorrent;
	
	if ( m_pTorrent.use_count() > 1)
	{
		pTorrent = m_pTorrent;
	}
	else
	{
		pTorrent = pMainFrame->GetSelectedTorrent();
	}

	if (!pTorrent)
		return ret;

	std::vector<lt::announce_entry>& trackers = pTorrent->config->GetTrackersURL();
	if(pTorrent->handle.is_valid())
		trackers = pTorrent->handle.trackers();

	if (item >= trackers.size())
	{
		TrackerListCtrl* pThis = const_cast<TrackerListCtrl*>(this);
		pThis->SetItemCount(trackers.size());
		return ret;

	}

	lt::announce_entry tracker = trackers.at(item);

//	if (h.is_valid())

	switch(columnid)
	{
		case TRACKERLIST_COLUMN_URL:
		{
			ret = wxString(wxConvUTF8.cMB2WC(tracker.url.c_str()));
			break;
		}
		case TRACKERLIST_COLUMN_STATUS:
		{
			/*auto best_ae = std::min_element(tracker.endpoints.begin(), tracker.endpoints.end()
				, [](lt::announce_endpoint const& l, lt::announce_endpoint const& r) { return l.info_hashes[1].fails < r.info_hashes[1].fails; } );

			bool valid = (best_ae != tracker.endpoints.end());
			if(tracker.verified && valid)
			{
				if(best_ae->info_hashes[1].is_working()) ret = _("Working");
				else ret = wxString(best_ae->info_hashes[1].last_error.message());
			}
			else*/
				ret = _("Not connected");
			break;
		}
		case TRACKERLIST_COLUMN_NEXT_ANNOUNCE:
		{
			/*auto best_ae = std::min_element(tracker.endpoints.begin(), tracker.endpoints.end()
				, [](lt::announce_endpoint const& l, lt::announce_endpoint const& r) { return l.info_hashes[1].fails < r.info_hashes[1].fails; } );
			bool valid = (best_ae != tracker.endpoints.end());
 			if(tracker.verified && valid && best_ae->info_hashes[1].is_working())
			{
				lt::time_point const now = lt::clock_type::now();
				ret = HumanReadableTime(int(lt::total_seconds(best_ae->info_hashes[1].next_announce - now)));
			}
			else*/
				ret = _T("N/A");
			break;
		}
		default:
			;//ret = _T("");
	
	}	

	return ret;

}

void TrackerListCtrl::ShowContextMenu(const wxPoint& pos)
{
	static wxMenu menu( ( long )0 );
	static bool needInit = true;

	if( needInit )
	{
	    menu.Append(TRACKERLISTCTRL_MENU_REANNOUCE, _("Reannounce"));
		menu.AppendSeparator();
	    menu.Append(TRACKERLISTCTRL_MENU_ADD, _("Add"));
	    menu.Append(TRACKERLISTCTRL_MENU_COPY, _("Copy"));
	    menu.Append(TRACKERLISTCTRL_MENU_EDIT, _("Edit"));
	    menu.Append(TRACKERLISTCTRL_MENU_DELETE, _("Delete"));
		needInit = false;
	}

    PopupMenu(&menu, pos);
}


/* update files priority to torrent config */
void TrackerListCtrl::OnMenuTracker(wxCommandEvent& event)
{
	//int cmd = event.GetId() - TRACKERLISTCTRL_MENU_ADD;
	int cmd = event.GetId(); 
	bool refreshtrk = false;
	std::shared_ptr<torrent_t> pTorrent;

	if ( m_pTorrent.use_count() > 1)
		pTorrent = m_pTorrent;
	else
	{
		MainFrame* pMainFrame = dynamic_cast< MainFrame* >( wxGetApp().GetTopWindow() );
		wxASSERT(pMainFrame != nullptr);
		pTorrent = pMainFrame->GetSelectedTorrent();
		
		if(m_pTorrent.use_count() == 1)
			m_pTorrent.reset();
	}

	if (!pTorrent)
		return;

	int item = GetFirstSelected();

	if( (cmd == TRACKERLISTCTRL_MENU_EDIT) || (cmd == TRACKERLISTCTRL_MENU_COPY ) || (cmd == TRACKERLISTCTRL_MENU_DELETE ) )
	{
		if (item < 0)
			return;
		
	}

	//wxLogDebug(_T("edit tracker no %d, cmd %d"), item, cmd);

	std::vector<lt::announce_entry>& trackers = pTorrent->config->GetTrackersURL();

	switch(cmd)
	{
		case TRACKERLISTCTRL_MENU_ADD:
		{
			wxTextEntryDialog dialog(this,
				_("Add tracker\nPlease enter a new tracker address"),
				_("Enter a new tracker address"),
				_T(""),
				wxOK | wxCANCEL);
				
			if (dialog.ShowModal() == wxID_OK)
			{
				wxString newtrackerurl = dialog.GetValue();
				std::string newtrackerurl_cstr( (const char *)((newtrackerurl.ToUTF8()).data()));
				lt::announce_entry e(newtrackerurl_cstr);
				e.tier = (trackers.size() * 10);
				if(pTorrent->handle.is_valid())
				{
					pTorrent->handle.add_tracker(e);                     
					wxString torrent_backup = wxGetApp().SaveTorrentsPath() + wxString(pTorrent->hash) + _T( ".torrent" );
					BitTorrentSession::SaveTorrent(pTorrent, torrent_backup );
				}
				trackers.push_back(e);
				//wxLogDebug(_T("Add new tracker %s tier %d, trackers.size %d"), newtrackerurl.c_str(), e.tier , trackers.size()) ;
				
				//pTorrent->config->SetTrackersURL(trackers);
				pTorrent->config->Save();

				TrackerListCtrl* pThis = const_cast<TrackerListCtrl*>(this);
				pThis->SetItemCount(trackers.size());
				refreshtrk = true;
			}

			break;
		}
		case TRACKERLISTCTRL_MENU_EDIT:
		{
			if ((item < 0) || (item >= trackers.size()))
				return;

			auto tracker_it = trackers.begin() + item;
			wxString trackerurl = wxString::FromUTF8(tracker_it->url.c_str());
			int tier = tracker_it->tier;
			wxTextEntryDialog dialog(this,
				_("Edit tracker\n"),
				_("Edit tracker address"),
				trackerurl,
				wxOK | wxCANCEL);
				
			if (dialog.ShowModal() == wxID_OK)
			{
				wxString returl = dialog.GetValue();
				if (returl.Len() > 0)
				{
					std::string tmpurl((const char *)((returl.ToUTF8()).data()));
					tracker_it->url = tmpurl;
					if(pTorrent->handle.is_valid())
					{               
						pTorrent->handle.replace_trackers(trackers);
						wxString torrent_backup = wxGetApp().SaveTorrentsPath() + wxString(pTorrent->hash) + _T( ".torrent" );
						BitTorrentSession::SaveTorrent(pTorrent, torrent_backup );
					}

					//pTorrent->config->SetTrackersURL(trackers);
					pTorrent->config->Save();

					refreshtrk = true;
				}
			}
			break;
		}
		case TRACKERLISTCTRL_MENU_DELETE:
		{
			if ((item < 0) || (item >= trackers.size()))
				return;

			auto tracker_it = trackers.begin() + item;

			wxMessageDialog dialogConfirm(nullptr, 
							_("Remove tracker ") + wxString::FromUTF8(tracker_it->url.c_str()) + _T("?"), 
							_("Confirm remove tracker"), 
							wxNO_DEFAULT | wxYES_NO | wxICON_QUESTION);

			if (dialogConfirm.ShowModal() == wxID_YES)
			{
				trackers.erase(tracker_it);
                
				if(pTorrent->handle.is_valid())
				{
					pTorrent->handle.replace_trackers(trackers);
					wxString torrent_backup = wxGetApp().SaveTorrentsPath() + wxString(pTorrent->hash) + _T( ".torrent" );
					BitTorrentSession::SaveTorrent(pTorrent, torrent_backup );
				}
				//pTorrent->config->SetTrackersURL(trackers);
				pTorrent->config->Save();

				refreshtrk = true;
			}
			break;
		}
		case TRACKERLISTCTRL_MENU_COPY:
		{
			if(wxTheClipboard)
			{
				wxString trackerurls;
				do {
					if ((item < 0) || (item >= trackers.size()))
						return;

					auto tracker_it = trackers.begin() + item;
					lt::announce_entry tracker= trackers.at(item);
					trackerurls += wxString(wxConvUTF8.cMB2WC(tracker.url.c_str())) + _T("\n");

				} while((item = GetNextSelected(item)) != -1);

				if( wxTheClipboard->Open() )
				{
					wxTheClipboard->SetData( new wxTextDataObject( trackerurls ) );
					wxTheClipboard->Flush();
					wxTheClipboard->Close();
				}
			}
			break;
		}
		case TRACKERLISTCTRL_MENU_REANNOUCE:
		{
			auto & handle = pTorrent->handle;
			if(handle.is_valid())
			{
				handle.force_reannounce(0, item);
			}
			break;
		}
		default:
			break;
	}

	if(refreshtrk) wxGetApp().GetBitTorrentSession()->ConfigureTorrentTrackers(pTorrent);
}

