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
// Class: TorrentProperty
// Created by: lim k. c. <admin@bitswash.org>
// Created on: Fri May  4 01:02:45 MYT 2007
//

#ifndef _TORRENTPROPERTY_H_
#define _TORRENTPROPERTY_H_

#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/scrolwin.h>
#include <wx/statline.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>

#include "bittorrentsession.h"
#include "mainframe.h"
#include "torrentconfig.h"
#include "torrentsetting.h"
#include "filelistctrl.h"

#include "autosizeinput.h"

#ifndef ID_TORRENTPROPERTY_START
#define ID_TORRENTPROPERTY_START wxID_HIGHEST
#endif

class TorrentProperty :  public wxDialog
{
	public:
		enum 
		{
			ID_TORRENTPROPERTY_FILEBUTTON = ID_TORRENTPROPERTY_START
		};

		TorrentProperty(torrent_t* pTorrent, wxWindow *parent,
       	        	const wxWindowID id,
			wxString title = _T("Torrent Setting"),
       	        	const wxPoint& pos = wxDefaultPosition,
               		const wxSize& size = wxDefaultSize, /*wxSize(420, 570),*/
               		long style = wxDEFAULT_DIALOG_STYLE);
		 ~TorrentProperty();
	
		// TorrentProperty interface
	
	protected:
		wxPanel* m_pane_info;
		wxStaticText* m_pane_label_info;
		wxStaticText* m_label_name;
		wxStaticText* m_label_torrentname;
		wxStaticText* m_label_comment;
		wxTextCtrl* m_text_torrentcomment;
		wxStaticText* m_label_size;
		wxStaticText* m_label_torrentsize;
		wxPanel* m_pane_info1;
		wxStaticText* m_pane_label_settings;
		wxNotebook* m_notebook_property;
		TorrentSettingPane* m_panel_settings;
		FileListCtrl* m_filespane;
		wxStdDialogButtonSizer* m_sdbSizer;
	private:
		// TorrentProperty variables
		void OnFocus(wxFocusEvent& event);
		void OnOK(wxCommandEvent& event);
		void OnCancel(wxCommandEvent& event);
		void OnFileButton(wxCommandEvent& event);
		
		wxFrame* m_pMainFrame;
		torrent_t *m_pTorrent;



	private:
		DECLARE_EVENT_TABLE()
	
};


#endif	//_TORRENTPROPERTY_H_

