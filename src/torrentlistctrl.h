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
#ifndef _TORRENTLISTCTRL_H_
#define _TORRENTLISTCTRL_H_

#include <vector>

#include <wx/wx.h>
#include <wx/listctrl.h>
#include <libtorrent/torrent_handle.hpp>
#include <libtorrent/torrent_status.hpp>

#include "swashlistctrl.h"
#include "bitswash.h"

#define ID_TORRENTLIST_CTRL wxID_HIGHEST + 1


class TorrentListCtrl: public SwashListCtrl
{
		
public:
	TorrentListCtrl(wxWindow* parent,
					const wxString settings,
					const wxWindowID id = wxID_ANY,
             		const wxPoint& pos = wxDefaultPosition,
               		const wxSize& size = wxDefaultSize,
               		long style = wxLC_VIRTUAL | wxLC_REPORT | wxSUNKEN_BORDER);
	virtual ~TorrentListCtrl();
												 
private:
	
	//wxString OnGetItemText(long item, long column) const;
	wxString GetItemValue(long item, long columnid) const;
	void ShowContextMenu(const wxPoint& pos);
	wxMenu* m_torrentmenu;

    DECLARE_NO_COPY_CLASS(TorrentListCtrl)
};



#endif // _TORRENTLISTCTRL_H_
