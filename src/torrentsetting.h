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
#ifndef TORRENTSETTING_H
#define TORRENTSETTING_H

#include <memory>

//(*Headers(TorrentSettingPane)
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/combobox.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/statline.h>
#include <wx/stattext.h>
//*)
#include "bittorrentsession.h"

class AutoSizeInput;
typedef void ( *OnSaveDirectoryChangePtr )( const wxString &newdirectory );

class TorrentSettingPane: public wxPanel
{
	public:

		TorrentSettingPane( wxWindow* parent, std::shared_ptr<torrent_t>& pTorrent = std::shared_ptr<torrent_t>(), wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, int style = wxTAB_TRAVERSAL);
		virtual ~TorrentSettingPane();

		//(*Declarations(TorrentSettingPane)
		AutoSizeInput* m_autoDownLimit;
		AutoSizeInput* m_autoUpLimit;
		wxButton* m_button_showdir;
		wxCheckBox* m_check_preview_video_files;
		wxCheckBox* m_check_start;
		wxCheckBox* m_check_usedefault;
		wxComboBox* m_combo_saveas;
		wxComboBox* m_combo_storagemode;
		wxSpinCtrl* m_spin_maxconnect;
		wxSpinCtrl* m_spin_maxupload;
		wxSpinCtrl* m_spin_ratio;
		wxStaticText* m_label_diskfreespace;
		//*)

	protected:

		//(*Identifiers(TorrentSettingPane)
		//*)

	private:

		//(*Handlers(TorrentSettingPane)
		void OnBbuttonShowDirClick(wxCommandEvent& event);
		void OnSaveDirectoryChanged(wxCommandEvent& event);
		//*)
		
		std::shared_ptr<torrent_t> m_pTorrent;
		OnSaveDirectoryChangePtr m_directory_change_func;

	public:

		wxString GetDownloadPath();
		libtorrent::storage_mode_t GetStorageMode();

		bool GetStartTorrent()
		{
			return (m_check_start->IsChecked());
		}
		
		bool GetUseDefault()
		{
			return m_check_usedefault->IsChecked();
		}
		
		bool GetEnableVideoPreview()
		{
			return m_check_preview_video_files->IsChecked();
		}
		long GetDownloadRate()
		{
			return m_autoDownLimit->GetValue();
		}
		
		long GetUploadRate()
		{
			return m_autoUpLimit->GetValue();
		}
		
		int GetMaxConnections()
		{
			return m_spin_maxconnect->GetValue();
		}
		
		int GetMaxUpload()
		{
			return m_spin_maxupload->GetValue();
		}
		
		int GetRatio()
		{
			return m_spin_ratio->GetValue();
		}

		void SetTorrentHandle(std::shared_ptr<torrent_t>& pTorrent) { m_pTorrent= pTorrent; }
		void SetTorrentDirectoryChange(OnSaveDirectoryChangePtr pFunc) { m_directory_change_func= pFunc; }
};

#endif
