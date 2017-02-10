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
// Class: PeerListCtrl
// Created by: lim k. c. <admin@bitswash.org>
// Created on: Mon Feb  5 11:57:07 2007
//

#ifndef _PEERLISTCTRL_H_
#define _PEERLISTCTRL_H_

#include <vector>

#include <wx/wx.h>
#include <wx/listctrl.h>
#include <wx/frame.h>
#include <wx/thread.h>
#include <libtorrent/torrent_handle.hpp>
#include <libtorrent/peer_info.hpp>

#include "swashlistctrl.h"

#define ID_PEERLIST_CTRL_START wxID_HIGHEST + 1

class PeerListCtrl : public SwashListCtrl
{

public:
	
		PeerListCtrl(wxWindow *parent,
			const wxString settings,
			const wxWindowID id,
			const wxPoint& pos = wxDefaultPosition,
			const wxSize& size = wxDefaultSize,
			long style = wxLC_VIRTUAL | wxLC_REPORT | wxSUNKEN_BORDER);

		 ~PeerListCtrl();

		typedef std::vector<libtorrent::peer_info> peer_list_t;
	
	protected:
		// PeerListCtrl variables
	
	private:

		wxString GetItemValue(long item, long columnid) const;
#if WXVER >= 280
		int GetItemColumnImage(long item, long columnid) const;
#else
		int OnGetItemImage(long item) const;
#endif

		DECLARE_NO_COPY_CLASS(PeerListCtrl)
		DECLARE_EVENT_TABLE()
};	
#endif	//_PEERLISTCTRL_H_

