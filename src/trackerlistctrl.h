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
// Created by: lim k. c. <admin@bitswash.org>
// Created on: Wed Mar 19 23:57:27 EDT 2008
//

#ifndef _TRACKERLISTCTRL_H_
#define _TRACKERLISTCTRL_H_

#include <vector>

#include <wx/wx.h>
#include <wx/listctrl.h>
#include <wx/frame.h>


#include <libtorrent/torrent_handle.hpp>
#include <libtorrent/announce_entry.hpp>



#define ID_FILELIST_CTRL_START wxID_HIGHEST + 1

#include "swashlistctrl.h"

#include "bittorrentsession.h"

class TrackerListCtrl : public SwashListCtrl
{

public:
	// IDs for the menu commands
		TrackerListCtrl(wxWindow *parent,
			const wxString settings,
			const wxWindowID id,
			const wxPoint& pos = wxDefaultPosition,
			const wxSize& size = wxDefaultSize,
			long style = wxLC_VIRTUAL | wxLC_REPORT);
               
		~TrackerListCtrl();
	
		void SetStaticHandle(shared_ptr<torrent_t>& torrent) { m_pTorrent = torrent; }
		void DeleteStaticHandle() { m_pTorrent.reset(); }
	protected:
		// TrackerListCtrl variables
	
	private:
		wxString GetItemValue(long item, long columnid) const;
		void ShowContextMenu(const wxPoint& pos);
		void OnMenuTracker(wxCommandEvent &event);

		shared_ptr<torrent_t> m_pTorrent;

    	DECLARE_NO_COPY_CLASS(TrackerListCtrl)
};	
	
	
#endif	//_TRACKERLISTCTRL_H_

