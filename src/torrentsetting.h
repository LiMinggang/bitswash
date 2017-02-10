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
// Class:TorrentSettingPane
//
// Created by: lim k. c. <admin@bitswash.org>
// Created on: Tue Jun 26 13:23:51 MYT 2007
//

#ifndef _TORRENTSETTING_H_
#define _TORRENTSETTING_H_

#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/scrolwin.h>
#include <wx/statline.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
#include <wx/combobox.h>

#include "configuration.h"
#include "autosizeinput.h"

#ifndef ID_TORRENTSETTING_START
#define ID_TORRENTSETTING_START wxID_HIGHEST
#endif

class TorrentSettingPane : public wxPanel
{
	public:
		enum {
			ID_TORRENTSETTING_FILEBUTTON = ID_TORRENTSETTING_START
		};
		TorrentSettingPane( wxWindow* parent, torrent_t* pTorrent = NULL, int id = wxID_ANY, wxPoint pos = wxDefaultPosition, wxSize size = wxDefaultSize, int style = wxTAB_TRAVERSAL);

		wxString GetDownloadPath();
		/* libtorrent r1749 */
		//bool GetCompactAlloc();
		libtorrent::storage_mode_t GetStorageMode();

		bool GetStartTorrent();
		bool GetUseDefault();
		long GetDownloadRate();
		long GetUploadRate();
		int GetMaxConnections();
		int GetMaxUpload();
		int GetRatio();

		void SetTorrentHandle(torrent_t* pTorrent) { m_pTorrent= pTorrent; }
	private:
		torrent_t* m_pTorrent;
		void OnFileButton(wxCommandEvent& event);
		/* torrent settings */
		wxPanel* m_pane_info1;
		wxStaticText* m_pane_label_settings;
				
		wxStaticText* m_staticStorageMode;
		wxStaticText* m_staticSaveAs;
		wxComboBox* m_combo_saveas;
		wxButton* m_button_showdir;
		/* libtorrent r1749 */
		//wxCheckBox* m_check_compact;
		wxComboBox* m_combo_storagemode;

		wxCheckBox* m_check_start;
		wxCheckBox* m_check_usedefault;
		wxStaticLine* m_staticHline;
		wxStaticText* m_staticDownLimit;
		wxStaticText* m_staticUpLimit;
		wxStaticText* m_staticMaxConnect;
		wxSpinCtrl* m_spin_maxconnect;
		wxStaticText* m_staticMaxUpload;
		wxSpinCtrl* m_spin_maxupload;
		wxStaticText* m_staticRatio;
		wxSpinCtrl* m_spin_ratio;

		AutoSizeInput* m_autoDownLimit;
		AutoSizeInput* m_autoUpLimit;
		
	DECLARE_EVENT_TABLE()
};


#endif	//_TORRENTSETTING_H_

