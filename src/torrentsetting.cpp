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
// Class: TorrentSettingPane
//
// Created by: lim k. c. <admin@bitswash.org>
// Created on: Tue Jun 26 13:23:51 MYT 2007
//

#include <wx/sstream.h>
#include <wx/textctrl.h>
#include <wx/log.h>

#include "mainframe.h"
#include "torrentsetting.h"
#include "functions.h"

//CLASS TorrentSettingPane
//
BEGIN_EVENT_TABLE(TorrentSettingPane, wxPanel)
	EVT_BUTTON(10000, TorrentSettingPane::OnFileButton)
END_EVENT_TABLE()

TorrentSettingPane::TorrentSettingPane( wxWindow* parent, torrent_t* pTorrent, int id, wxPoint pos, wxSize size, int style) : 
		wxPanel(parent, id, pos, size, style)
{

	Configuration* pcfg = ((MainFrame*)(wxGetApp().GetTopWindow()))->GetConfig();
	m_pTorrent = pTorrent;

	wxFlexGridSizer* m_torrentsettings_sizer;
	m_torrentsettings_sizer = new wxFlexGridSizer( 0, 1, 3, 3 );
	m_torrentsettings_sizer->SetFlexibleDirection( wxVERTICAL);
	
	wxFlexGridSizer* fgTorrentSettings;
	fgTorrentSettings = new wxFlexGridSizer( 0, 1, 0, 0 );
	fgTorrentSettings->SetFlexibleDirection( wxBOTH );
	
	m_pane_info1 = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	m_pane_info1->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_ACTIVECAPTION ) );
	m_pane_info1->SetMaxSize( wxSize( -1,20 ) );
	
	wxBoxSizer* m_pane_boxsizer1;
	m_pane_boxsizer1 = new wxBoxSizer( wxVERTICAL );
	
	m_pane_label_settings = new wxStaticText( m_pane_info1, wxID_ANY, _("Torrent Settings"), wxDefaultPosition, wxDefaultSize, 0 );
	m_pane_boxsizer1->Add( m_pane_label_settings, 0, wxALL, 2 );
	
	m_pane_info1->SetSizer( m_pane_boxsizer1 );
	m_pane_info1->Layout();
	m_torrentsettings_sizer->Add( m_pane_info1, 1, wxEXPAND | wxALL, 5 );

	wxFlexGridSizer* fgSaveAs;
	fgSaveAs = new wxFlexGridSizer( 1, 3, 5, 5 );
	fgSaveAs->AddGrowableCol( 1 );
	fgSaveAs->SetFlexibleDirection( wxBOTH );
	
	m_staticSaveAs = new wxStaticText( this, wxID_ANY, _("Save:"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSaveAs->Add( m_staticSaveAs, 0, wxALL, 5 );
	
	std::vector<wxString>& historypath = pcfg->GetSavePathHistory();
	
	wxString t_saveas = m_pTorrent?m_pTorrent->config->GetDownloadPath():pcfg->GetDownloadPath();
	m_combo_saveas = new wxComboBox( this, wxID_ANY, t_saveas, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );
	
	m_combo_saveas->SetMinSize(wxSize(260,-1));
	m_combo_saveas->Append( t_saveas );

	std::vector<wxString>::reverse_iterator path;

	for(path = historypath.rbegin(); path != historypath.rend(); path++)
	{
		if((t_saveas.Cmp(*path)) == 0)
			continue;
		m_combo_saveas->Append(*path);
	}
	m_combo_saveas->SetValue(t_saveas);
	fgSaveAs->Add( m_combo_saveas, 0, wxALL, 5 );
	
	m_button_showdir = new wxButton( this, 10000, wxT("..."), wxDefaultPosition, wxSize( 30,-1 ), 0 );
	fgSaveAs->Add( m_button_showdir, 0, wxALL, 5 );
	
	fgTorrentSettings->Add( fgSaveAs, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgCompact;
	fgCompact = new wxFlexGridSizer( 1, 2, 5, 5 );
	fgCompact->SetFlexibleDirection( wxBOTH );
	
	
	m_check_start = new wxCheckBox( this, wxID_ANY, _("Start Torrent"), wxDefaultPosition, wxDefaultSize, 0 );
	if (m_pTorrent)
	{
		if ( (m_pTorrent->config->GetTorrentState() == TORRENT_STATE_START) ||
			(m_pTorrent->config->GetTorrentState() == TORRENT_STATE_QUEUE) ||
			 (m_pTorrent->config->GetTorrentState() == TORRENT_STATE_PAUSE) )
				m_check_start->SetValue(true) ;
		else
				m_check_start->SetValue(false);
	}
	else
	{
		if ((pcfg->GetDefaultState() == TORRENT_STATE_START)  || 
				(pcfg->GetDefaultState() == TORRENT_STATE_QUEUE))
		{	
			m_check_start->SetValue(true) ;
		} 
		else 
		{ 
			m_check_start->SetValue(false); 
		}
	}
	
	fgCompact->Add( m_check_start, 0, wxALL, 5 );

	m_check_usedefault = new wxCheckBox( this, wxID_ANY, _("Do not prompt for torrent settings."), wxDefaultPosition, wxDefaultSize, 0 );

	{
		if (pcfg->GetUseDefault())  
		{	
			m_check_usedefault->SetValue(true) ;
		} 
		else 
		{ 
			m_check_usedefault->SetValue(false); 
		}
	}

	fgCompact->Add( m_check_usedefault, 0, wxALL, 5 );
	
	
	fgTorrentSettings->Add( fgCompact, 1, wxEXPAND, 5 );
	
	m_staticHline = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	fgTorrentSettings->Add( m_staticHline, 0, wxALL, 5 );
	
	wxFlexGridSizer* fgTorrentSettingsInner;
	fgTorrentSettingsInner = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgTorrentSettingsInner->AddGrowableCol( 0 );
	fgTorrentSettingsInner->SetFlexibleDirection( wxBOTH );
	

	m_staticStorageMode= new wxStaticText( this, wxID_ANY, _("Storage Allocation Mode:"), wxDefaultPosition, wxDefaultSize, 0 );
	fgTorrentSettingsInner->Add( m_staticStorageMode, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxArrayString m_arrStorageMode;
	
	m_arrStorageMode.Add(_("Full"));
	m_arrStorageMode.Add(_("Sparse"));
	m_arrStorageMode.Add(_("Compact"));

	m_combo_storagemode = new wxComboBox( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(80,-1), m_arrStorageMode, wxCB_READONLY );
	
	/* legacy */
	bool bCompactMode = false;
	enum libtorrent::storage_mode_t e_storagemode = libtorrent::storage_mode_sparse;

	bCompactMode = m_pTorrent?m_pTorrent->config->GetTorrentCompactAlloc():pcfg->GetCompactAlloc();

	if(bCompactMode)
	{
		e_storagemode = libtorrent::storage_mode_compact;
	}
	else
	{
		e_storagemode = m_pTorrent?m_pTorrent->config->GetTorrentStorageMode():pcfg->GetDefaultStorageMode();
	}

	wxString strStorageMode;
	switch (e_storagemode)
	{
		case libtorrent::storage_mode_allocate:
			strStorageMode = _("Full");
			break;
		case libtorrent::storage_mode_sparse:
			strStorageMode = _("Sparse");
			break;
		case libtorrent::storage_mode_compact:
			strStorageMode = _("Compact");
			break;
		default:
			strStorageMode = _("Sparse");
			break;
	}
	m_combo_storagemode->SetValue(strStorageMode);

	fgTorrentSettingsInner->Add( m_combo_storagemode, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	m_staticDownLimit = new wxStaticText( this, wxID_ANY, _("Download Rate Limit:"), wxDefaultPosition, wxDefaultSize, 0 );
	fgTorrentSettingsInner->Add( m_staticDownLimit, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_autoDownLimit = new AutoSizeInput(this, _T("Bps"));

	m_autoDownLimit->SetValue(m_pTorrent?m_pTorrent->config->GetTorrentDownloadLimit():pcfg->GetDefaultDownloadLimit());
	fgTorrentSettingsInner->Add( m_autoDownLimit, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	m_staticUpLimit = new wxStaticText( this, wxID_ANY, _("Upload Rate Limit:"), wxDefaultPosition, wxDefaultSize, 0 );
	fgTorrentSettingsInner->Add( m_staticUpLimit, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	m_autoUpLimit = new AutoSizeInput(this, _T("Bps") );
	m_autoUpLimit->SetValue(m_pTorrent?m_pTorrent->config->GetTorrentUploadLimit():pcfg->GetDefaultUploadLimit());
	fgTorrentSettingsInner->Add( m_autoUpLimit, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	m_staticMaxConnect= new wxStaticText( this, wxID_ANY, _("Maximum Connections:"), wxDefaultPosition, wxDefaultSize, 0 );
	fgTorrentSettingsInner->Add( m_staticMaxConnect,1, wxALIGN_CENTER_VERTICAL| wxALL, 5 );
	
	wxString t_maxconnect= wxString::Format(_T("%d"), m_pTorrent?m_pTorrent->config->GetTorrentMaxConnections():pcfg->GetMaxConnections());
	m_spin_maxconnect = new wxSpinCtrl( this, wxID_ANY, t_maxconnect, wxDefaultPosition, wxSize( 60,-1 ), wxSP_ARROW_KEYS, -1, 99999999, 0);
	fgTorrentSettingsInner->Add( m_spin_maxconnect, 0, wxALL, 5 );
	
	m_staticMaxUpload= new wxStaticText( this, wxID_ANY, _("Maximum Upload Slots:"), wxDefaultPosition, wxDefaultSize, 0 );
	fgTorrentSettingsInner->Add( m_staticMaxUpload, 1, wxALIGN_CENTER_VERTICAL |wxALL, 5 );
	
	wxString t_maxupload = wxString::Format(_T("%d"), m_pTorrent?m_pTorrent->config->GetTorrentMaxUploads():pcfg->GetMaxUploads());
	m_spin_maxupload = new wxSpinCtrl( this, wxID_ANY, t_maxupload, wxDefaultPosition, wxSize( 60,-1 ), wxSP_ARROW_KEYS, -1, 99999999, 0);
	fgTorrentSettingsInner->Add( m_spin_maxupload, 0, wxALL, 5 );
	
	m_staticRatio= new wxStaticText( this, wxID_ANY, _("Seed Ratio"), wxDefaultPosition, wxDefaultSize, 0 );
	fgTorrentSettingsInner->Add( m_staticRatio, 1, wxALIGN_CENTER_VERTICAL | wxALL, 5 );
	
	wxString t_ratio= wxString::Format(_T("%d"), m_pTorrent?m_pTorrent->config->GetTorrentRatio():pcfg->GetDefaultRatio());
	m_spin_ratio = new wxSpinCtrl( this, wxID_ANY, t_ratio, wxDefaultPosition, wxSize( 60,-1 ), wxSP_ARROW_KEYS, 0, 99999999, 0);
	fgTorrentSettingsInner->Add( m_spin_ratio, 0, wxALL, 5 );

	fgTorrentSettings->Add( fgTorrentSettingsInner, 1, wxEXPAND, 5 );

	m_torrentsettings_sizer->Add(fgTorrentSettings, 1, wxEXPAND, 5);
	this->SetSizer( m_torrentsettings_sizer );
	this->Layout();
	m_torrentsettings_sizer->Fit( this );

	//parent->SetSizer( m_torrentsettings_sizer );
	

	//parent->Layout();
	//m_torrentsettings_sizer->Fit(this);

}

wxString TorrentSettingPane::GetDownloadPath()
{
	return m_combo_saveas->GetValue();
}

/*
bool TorrentSettingPane::GetCompactAlloc()
{
	return m_check_compact->IsChecked();
}
*/

libtorrent::storage_mode_t TorrentSettingPane::GetStorageMode()
{
	//return (libtorrent::storage_mode_t)m_combo_storagemode->GetValue();
	return libtorrent::storage_mode_sparse;
}

bool TorrentSettingPane::GetStartTorrent()
{
	return (m_check_start->IsChecked());
}

bool TorrentSettingPane::GetUseDefault()
{
	return m_check_usedefault->IsChecked();
}


long TorrentSettingPane::GetDownloadRate()
{
	return m_autoDownLimit->GetValue();
}

long TorrentSettingPane::GetUploadRate()
{
	return m_autoUpLimit->GetValue();
}

int TorrentSettingPane::GetMaxConnections()
{
	return m_spin_maxconnect->GetValue();
}

int TorrentSettingPane::GetMaxUpload()
{
	return m_spin_maxupload->GetValue();
}

int TorrentSettingPane::GetRatio()
{
	return m_spin_ratio->GetValue();
}

void TorrentSettingPane::OnFileButton(wxCommandEvent& event)
{
	wxLogDebug(_T("OnFileButton\n"));

#if WXVER >= 280
	long dirstyle = wxDD_DEFAULT_STYLE| wxDD_DIR_MUST_EXIST |wxDD_CHANGE_DIR;
#else
	long dirstyle = wxDD_DEFAULT_STYLE| wxDD_CHANGE_DIR;
#endif

	wxDirDialog dir_dlg(this, _("Choose default download directory"), m_combo_saveas->GetValue(),dirstyle );

	if (dir_dlg.ShowModal() == wxID_OK) 
	{
		wxString newpath = dir_dlg.GetPath();
		wxLogDebug(_T("DirDlg return %s\n"), newpath.c_str());
		m_combo_saveas->SetValue(newpath);	
	}
	event.Skip();

}


