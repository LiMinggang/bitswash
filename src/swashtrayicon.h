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
// Class: SwashTrayIcon
// Created by: lim k. c. <admin@bitswash.org>
// Created on: Thu Jun 14 11:16:06 MYT 2007
//

#ifndef _SWASHTRAYICON_H_
#define _SWASHTRAYICON_H_

#include <wx/defs.h>	// Needed before any other wx/*.h
#include <wx/taskbar.h>
#include <wx/menu.h>
#include <wx/icon.h>
#include <wx/frame.h>

class SwashTrayIcon : public wxTaskBarIcon 
{
public:
	SwashTrayIcon(wxFrame* parent) ;

private:
	wxFrame* m_pMainFrame;
	bool m_hidetaskbar;
	void OnLeftButton(wxTaskBarIconEvent&);
	void OnMenuRestore(wxCommandEvent&);
	void OnMenuExit(wxCommandEvent&);
	void OnMenuHideTaskbar(wxCommandEvent&);
	void OnMenuUICheckmark(wxUpdateUIEvent &event);
	void ShowHideMainFrame();
	void OnMenuOpenTorrent(wxCommandEvent&);
	void OnMenuOpenTorrentUrl(wxCommandEvent&);
	void OnMenuPreference(wxCommandEvent&);
	virtual wxMenu *CreatePopupMenu();


	DECLARE_EVENT_TABLE()
};


#endif //_SWASHTRAYICON_H_
