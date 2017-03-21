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
// Class: bwtorrentinfo
// Created by: lim k. c. <admin@bitswash.org>
// Created on: Sat Feb  3 13:14:01 2007
//

#ifndef _TORRENTINFO_H_
#define _TORRENTINFO_H_

#include <wx/wx.h>
#include "compat.h"
#include <wx/frame.h>
#include <wx/panel.h>
#include <wx/textctrl.h>

#include "summarypane.h"
#include "peerlistctrl.h"
#include "filelistctrl.h"
#include "loggerctrl.h"

#define TORRENT_INFO_ID wxID_HIGHEST

class TorrentInfo : public wxAuiNotebook
{
	public:
		TorrentInfo(wxWindow *parent,
               const wxWindowID id,
               const wxPoint& pos = wxDefaultPosition,
               const wxSize& size = wxDefaultSize,
               long style = wxNB_TOP | wxTAB_TRAVERSAL );
               
		 ~TorrentInfo();
	
		 void UpdateTorrentInfo(bool updateall);
		 wxFrame* GetMainFrame() { return m_pMainFrame; }
		 LoggerCtrl* GetLogger() { return m_logger_panel;}

		 PeerListCtrl* GetPeerListCtrl() { return m_peerspane ; }
		 FileListCtrl* GetFileListCtrl() { return m_filespane ; }
		 SummaryPane* GetSummaryPane() { return m_summarypane ;}
		 TrackerListCtrl* GetTrackerListCtrl() { return m_trackerspane;}
	
	private:
		SummaryPane* m_summarypane;
		PeerListCtrl* m_peerspane;
		FileListCtrl* m_filespane;
		TrackerListCtrl* m_trackerspane;
		void OnNotebookChanged(wxAuiNotebookEvent& event);
		LoggerCtrl* m_logger_panel;
	
		wxFrame* m_pMainFrame;

		DECLARE_EVENT_TABLE()
};


#endif	//_TORRENTINFO_H_

