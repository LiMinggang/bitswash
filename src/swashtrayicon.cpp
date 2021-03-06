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
#include <wx/msgdlg.h>

#include "swashtrayicon.h"
#include "mainframe.h"

const long SwashTrayIcon::TRAY_RESTORE = wxNewId();
const long SwashTrayIcon::TRAY_EXIT = wxNewId();
const long SwashTrayIcon::TRAY_HIDETASKBAR = wxNewId();
const long SwashTrayIcon::TRAY_OPENTORRENT = wxNewId();
const long SwashTrayIcon::TRAY_OPENTORRENTURL = wxNewId();
const long SwashTrayIcon::TRAY_PREFERENCE = wxNewId();

SwashTrayIcon::SwashTrayIcon(wxFrame* parent) 
{ 
	m_pMainFrame = dynamic_cast<MainFrame*>(parent);
	wxASSERT(m_pMainFrame != nullptr);

	m_hidetaskbar = m_pMainFrame->GetConfig()->GetHideTaskbar();
	
	Bind( wxEVT_MENU, &SwashTrayIcon::OnMenuRestore, this, TRAY_RESTORE );
	Bind( wxEVT_MENU, &SwashTrayIcon::OnMenuExit, this, TRAY_EXIT );
	Bind( wxEVT_MENU,&SwashTrayIcon::OnMenuHideTaskbar, this, TRAY_HIDETASKBAR );
	Bind( wxEVT_UPDATE_UI,&SwashTrayIcon::OnMenuUICheckmark, this, TRAY_HIDETASKBAR );
	Bind( wxEVT_TASKBAR_LEFT_UP, &SwashTrayIcon::OnLeftButton, this );
	Bind( wxEVT_MENU, &SwashTrayIcon::OnMenuOpenTorrent, this, TRAY_OPENTORRENT );
	Bind( wxEVT_MENU, &SwashTrayIcon::OnMenuOpenTorrentUrl, this, TRAY_OPENTORRENTURL );
	Bind( wxEVT_MENU, &SwashTrayIcon::OnMenuPreference, this, TRAY_PREFERENCE );
}

void SwashTrayIcon::OnMenuRestore(wxCommandEvent& )
{
	ShowHideMainFrame();
}

void SwashTrayIcon::OnMenuExit(wxCommandEvent& )
{
	m_pMainFrame->Close(true);
}

void SwashTrayIcon::OnMenuOpenTorrent(wxCommandEvent& )
{
	m_pMainFrame->OpenTorrent();
}

void SwashTrayIcon::OnMenuOpenTorrentUrl(wxCommandEvent& )
{
	m_pMainFrame->OpenTorrentUrl();
}

void SwashTrayIcon::OnMenuPreference(wxCommandEvent& )
{
	wxMenuEvent * event = new wxMenuEvent(wxEVT_MENU, MainFrame::ID_OPTIONS_PREFERENCES);
	
	event->SetEventObject( m_pMainFrame );
	wxQueueEvent( m_pMainFrame, event );
}

void SwashTrayIcon::OnMenuHideTaskbar(wxCommandEvent& )
{
	m_hidetaskbar =!m_hidetaskbar;
	m_pMainFrame->GetConfig()->SetHideTaskbar(m_hidetaskbar);
}

void SwashTrayIcon::OnMenuUICheckmark(wxUpdateUIEvent &event)
{
	event.Check( m_hidetaskbar);
}

// Overridables
wxMenu *SwashTrayIcon::CreatePopupMenu()
{
	// Try creating menus different ways
	// TODO: Probably try calling SetBitmap with some XPMs here
	wxMenu *menu = new wxMenu;
	menu->Append(TRAY_RESTORE, _("&Restore"));
	menu->Append(TRAY_HIDETASKBAR, _("&Hide Taskbar Icon"),wxT(""), wxITEM_CHECK);
	menu->AppendSeparator();
	menu->Append(TRAY_OPENTORRENT, _("&Open Torrent\tCtrl+O"),_("Open Torrent"));
	menu->Append(TRAY_OPENTORRENTURL, _("Open &URL/Magnet URI\tCtrl+U"), _("Open URL/Magnet URI"));
	menu->AppendSeparator();
	menu->Append(TRAY_PREFERENCE, _("Prefere&nce\tCtrl+F"), _("Preference"));

#ifndef __WXMAC_OSX__ /*Mac has built-in quit menu*/
	menu->AppendSeparator();
	menu->Append(TRAY_EXIT, _("E&xit"));
#endif
	return menu;
}

void SwashTrayIcon::OnLeftButton(wxTaskBarIconEvent&)
{
	ShowHideMainFrame();
}

void SwashTrayIcon::ShowHideMainFrame()
{
	bool hidetaskbar = m_pMainFrame->GetConfig()->GetHideTaskbar();
	
	if (hidetaskbar) 
	{
		if (m_pMainFrame->IsShown())
		{
			m_pMainFrame->Show(false);
		} 
		else 
		{
			m_pMainFrame->UpdateUI(true);
			m_pMainFrame->Show(true);
			m_pMainFrame->Iconize(false);
			m_pMainFrame->Raise();
		}
	} 
	else
	{
		if (m_pMainFrame->IsIconized())
		{
			m_pMainFrame->UpdateUI(true);
			m_pMainFrame->Iconize(false);
		} 
		else 
		{
			m_pMainFrame->Iconize(true);
		}
	}
}


