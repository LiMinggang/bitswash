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

enum
{
	TRACKERLISTCTRL_MENU_ADD= 10000,
	TRACKERLISTCTRL_MENU_EDIT ,
	TRACKERLISTCTRL_MENU_DELETE,
	TRACKERLISTCTRL_MENU_COPY,
};


BEGIN_EVENT_TABLE(TrackerListCtrl, SwashListCtrl)
	EVT_MENU(TRACKERLISTCTRL_MENU_ADD, TrackerListCtrl::OnMenuEdit)
	EVT_MENU(TRACKERLISTCTRL_MENU_EDIT, TrackerListCtrl::OnMenuEdit)
	EVT_MENU(TRACKERLISTCTRL_MENU_DELETE, TrackerListCtrl::OnMenuEdit)
	EVT_MENU(TRACKERLISTCTRL_MENU_COPY, TrackerListCtrl::OnMenuEdit)
END_EVENT_TABLE()
// TrackerListCtrl


enum torrentlistcolumnid {
	TRACKERLIST_COLUMN_URL = 0,
};

#ifndef __WXMSW__
static SwashColumnItem trackerlistcols[] = 
{
	{ id: TRACKERLIST_COLUMN_URL, name: _T("URL"), title: _("URL"), tooltip:_("Torrent Tracker URL"), width: 450, show: true},
};
#else
static SwashColumnItem trackerlistcols[] = 
{
	{  TRACKERLIST_COLUMN_URL,  _T("URL"),  _("URL"), _("Torrent Tracker URL"),  450,  true},
};
#endif
TrackerListCtrl::TrackerListCtrl(wxWindow *parent,
               const wxString settings,
               const wxWindowID id,
               const wxPoint& pos,
               const wxSize& size,
               long style)
			: SwashListCtrl(parent, SWASHLISTCOL_SIZE(trackerlistcols), trackerlistcols, settings, id, pos, size, style),
			m_pTorrent(NULL)
{

}


TrackerListCtrl::~TrackerListCtrl()
{

}

wxString TrackerListCtrl::GetItemValue(long item, long columnid) const
{
	TrackerListCtrl* pThis = const_cast<TrackerListCtrl*>(this);

	MainFrame* pMainFrame = (MainFrame*)(wxGetApp().GetTopWindow());

	wxString ret;

	wxLogDebug(_T("TrackerListCtrl column %ld of item %ld\n"), columnid, item);
	shared_ptr<torrent_t> pTorrent;
	
	if (m_pTorrent)
	{
		pTorrent = m_pTorrent;
	}
	else
	{
		pTorrent = pMainFrame->GetSelectedTorrent();
	}

	if (!pTorrent)
		return _T("");

 	libtorrent::torrent_info const& t_info= *(pTorrent->info);

	wxString t_name = wxEmptyString;

	const std::vector<libtorrent::announce_entry>& trackers = pTorrent->config->GetTrackersURL();

	if (item >= trackers.size())
	{
		pThis->SetItemCount(trackers.size());
		return _T("");

	}
	libtorrent::announce_entry tracker= trackers.at(item);

//	if (h.is_valid())

	switch(columnid)
	{
		case TRACKERLIST_COLUMN_URL:
			t_name = wxString(wxConvUTF8.cMB2WC(tracker.url.c_str()));
			ret = t_name;
			break;
		default:
			ret = _T("");
	
	}	

	return ret;

}

void TrackerListCtrl::ShowContextMenu(const wxPoint& pos)
{
    wxMenu menu;

	/* XXX openfile */
    menu.Append(TRACKERLISTCTRL_MENU_ADD, _("Add"));
    menu.Append(TRACKERLISTCTRL_MENU_COPY, _("Copy"));
    menu.Append(TRACKERLISTCTRL_MENU_EDIT, _("Edit"));
    menu.Append(TRACKERLISTCTRL_MENU_DELETE, _("Delete"));

    PopupMenu(&menu, pos);
}


/* update files priority to torrent config */
void TrackerListCtrl::OnMenuEdit(wxCommandEvent& event)
{

	TrackerListCtrl* pThis = const_cast<TrackerListCtrl*>(this);
	MainFrame* pMainFrame = (MainFrame*)wxGetApp().GetTopWindow();

	//int cmd = event.GetId() - TRACKERLISTCTRL_MENU_ADD;
	int cmd = event.GetId(); 
	bool refreshtrk = false;
	shared_ptr<torrent_t> pTorrent;

	if (m_pTorrent)
		pTorrent = m_pTorrent;
	else
		pTorrent = pMainFrame->GetSelectedTorrent();

	if (!pTorrent)
		return;

	int item = GetFirstSelected();

	if(( (cmd == TRACKERLISTCTRL_MENU_EDIT) || (cmd == TRACKERLISTCTRL_MENU_EDIT ) )  )
	{
		if (item < 0)
			return;
		
	}

 	libtorrent::torrent_info const& t_info = *(pTorrent->info);

	wxLogDebug(_T("edit tracker no %d, cmd %d\n"), item, cmd);

	std::vector<libtorrent::announce_entry>& trackers = pTorrent->config->GetTrackersURL();


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
				std::string newtrackerurl_cstr( newtrackerurl.mb_str(wxConvUTF8));
				libtorrent::announce_entry e(newtrackerurl_cstr);
				e.tier = (trackers.size() * 10);
				trackers.push_back(e);
				wxLogDebug(_T("Add new tracker %s tier %d, trackers.size %d\n"), newtrackerurl.c_str(), e.tier , trackers.size()) ;
				
				//pTorrent->config->SetTrackersURL(trackers);
				pTorrent->config->Save();

				pThis->SetItemCount(trackers.size());
				refreshtrk = true;
#if 0
				h.replace_trackers(trackers);
#endif
			}

			break;
		}
		case TRACKERLISTCTRL_MENU_EDIT:
		{
			if ((item < 0) || (item >= trackers.size()))
				return;

			std::vector<libtorrent::announce_entry>::iterator tracker_it = trackers.begin() + item;
			wxString trackerurl = wxString::FromAscii(tracker_it->url.c_str());
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
					std::string tmpurl(returl.mb_str(wxConvUTF8));
					tracker_it->url = tmpurl;

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

			std::vector<libtorrent::announce_entry>::iterator tracker_it = trackers.begin() + item;

			wxMessageDialog dialogConfirm(NULL, 
							_("Confirm remove tracker ") + wxString::FromAscii(tracker_it->url.c_str()) + _T("?"), 
							_("Confirm remove tracker"), 
							wxNO_DEFAULT | wxYES_NO| wxICON_QUESTION);

			if (dialogConfirm.ShowModal() == wxID_YES)
			{
				trackers.erase(tracker_it);

				//pTorrent->config->SetTrackersURL(trackers);
				pTorrent->config->Save();

				refreshtrk = true;
			}
			break;
		}
		case TRACKERLISTCTRL_MENU_COPY:
		{
			wxString trackerurls;
			if(wxTheClipboard)
			{
				do {
					if ((item < 0) || (item >= trackers.size()))
						return;

					std::vector<libtorrent::announce_entry>::iterator tracker_it = trackers.begin() + item;
					libtorrent::announce_entry tracker= trackers.at(item);
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
		default:
			break;
	}


	if(refreshtrk) wxGetApp().GetBitTorrentSession()->ConfigureTorrentTrackers(pTorrent);

}

