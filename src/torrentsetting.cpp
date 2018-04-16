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
//(*InternalHeaders(TorrentSettingPane)
#include <wx/intl.h>
#include <wx/settings.h>
#include <wx/string.h>
//*)
#include <wx/sstream.h>
#include <wx/textctrl.h>
#include <wx/log.h>
#include "configuration.h"
#include "autosizeinput.h"
#include "mainframe.h"
#include "torrentsetting.h"
#include "functions.h"
#include "torrentsetting.h"

//(*IdInit(TorrentSettingPane)
//*)

TorrentSettingPane::TorrentSettingPane( wxWindow* parent, std::shared_ptr<torrent_t>& pTorrent, wxWindowID id, const wxPoint& pos, const wxSize& size, int style) :
		m_directory_change_func(nullptr)
{
	MainFrame * pMainFrame = dynamic_cast<MainFrame *>( wxGetApp().GetTopWindow() );
	wxASSERT(pMainFrame != nullptr);

	Configuration* pcfg = pMainFrame->GetConfig();
	wxASSERT(pcfg != nullptr);
	m_pTorrent = pTorrent;
	bool isTorrent = bool(m_pTorrent);

	//(*Initialize(TorrentSettingPane)
	wxBoxSizer* BoxSizer1;
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer4;
	wxFlexGridSizer* FlexGridSizer5;
	wxPanel* pane_info1;
	wxStaticLine* StaticLine1;
	wxStaticText* StaticText1;
	wxStaticText* StaticText2;
	wxStaticText* StaticText3;
	wxStaticText* StaticText4;
	wxStaticText* StaticText5;
	wxStaticText* StaticText6;
	wxStaticText* StaticText7;
	wxStaticText* StaticText8;
	wxStaticText* StaticText9;
	wxStaticText* pane_label_settings;

	Create(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 3, 3);
	pane_info1 = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	pane_info1->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	BoxSizer1 = new wxBoxSizer(wxVERTICAL);
	pane_label_settings = new wxStaticText(pane_info1, wxID_ANY, _("Torrent Settings"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	BoxSizer1->Add(pane_label_settings, 1, wxALL|wxEXPAND, 5);
	pane_info1->SetSizer(BoxSizer1);
	BoxSizer1->Fit(pane_info1);
	BoxSizer1->SetSizeHints(pane_info1);
	FlexGridSizer1->Add(pane_info1, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer2 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer3 = new wxFlexGridSizer(0, 3, 5, 5);
	FlexGridSizer3->AddGrowableCol(1);
	StaticText1 = new wxStaticText(this, wxID_ANY, _("Save:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer3->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	
	std::vector<wxString>& historypath = pcfg->GetSavePathHistory();
	
	wxString t_saveas = isTorrent?m_pTorrent->config->GetDownloadPath():pcfg->GetDownloadPath();
	m_combo_saveas = new wxComboBox( this, wxID_ANY, t_saveas, wxDefaultPosition, wxDefaultSize, 0, 0, wxTE_PROCESS_ENTER, wxDefaultValidator, _T("wxID_ANY"));
	m_combo_saveas->SetMinSize(wxSize(260,-1));
	//m_combo_saveas->Append( t_saveas );

	std::vector<wxString>::reverse_iterator path  = historypath.rbegin();

	for(; path != historypath.rend(); ++path)
	{
		if((t_saveas.Cmp(*path)) == 0)
			continue;
		m_combo_saveas->Append(*path);
	}
	m_combo_saveas->SetValue(t_saveas);
	FlexGridSizer3->Add(m_combo_saveas, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	m_button_saveasdir = new wxButton(this, wxID_ANY, wxT("..."), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("wxID_ANY"));
	FlexGridSizer3->Add(m_button_saveasdir, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	if(!isTorrent)
	{
		StaticText3 = new wxStaticText(this, wxID_ANY, _("Open:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
		FlexGridSizer3->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
		wxString t_open = pcfg->GetOpenTorrentPath();
		m_combo_open = new wxComboBox(this, wxID_ANY, t_open, wxDefaultPosition, wxDefaultSize, 0, 0, wxTE_PROCESS_ENTER, wxDefaultValidator, _T("wxID_ANY"));
		m_combo_open->SetMinSize(wxSize(260,-1));
		
		std::vector<wxString>& ohispath = pcfg->GetOpenPathHistory();
		std::vector<wxString>::reverse_iterator opath  = ohispath.rbegin();
		
		for(; opath != ohispath.rend(); ++opath)
		{
			if((t_open.Cmp(*opath)) == 0)
				continue;
			m_combo_open->Append(*opath);
		}

		FlexGridSizer3->Add(m_combo_open, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
		m_button_opendir = new wxButton(this, wxID_ANY, wxT("..."), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("wxID_ANY"));
		FlexGridSizer3->Add(m_button_opendir, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	}
	else
	{
		StaticText3 = nullptr;
		m_combo_open = nullptr;
		m_button_opendir = nullptr;
	}
	StaticText2 = new wxStaticText(this, wxID_ANY, _("Free:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer3->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);

	wxString downloadPath = m_combo_saveas->GetValue(), freeSpace = _("0 Byte(s)");
	if(downloadPath != wxEmptyString)
	{
		wxFileName fdir;
		fdir.AssignDir(downloadPath); // make sure we got path seperator at the end
		fdir.MakeAbsolute();
		size_t dcount = fdir.GetDirCount( );
		while(dcount-- > 0)
		{
			if(!fdir.DirExists()) fdir.RemoveLastDir();
			else break;
		}

		if(fdir.DirExists())
		{
			wxLongLong free(0);  
			if(wxGetDiskSpace( fdir.GetPathWithSep(), nullptr, &free))
			{
				freeSpace = HumanReadableByte(wxDouble(free.ToDouble()));
			}
		}
	}

	m_label_diskfreespace = new wxStaticText(this, wxID_ANY, freeSpace, wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer3->Add(m_label_diskfreespace, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2->Add(FlexGridSizer3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer4 = new wxFlexGridSizer(0, 2, 5, 5);
	m_check_start = new wxCheckBox(this, wxID_ANY, _("Start Torrent"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("wxID_ANY"));
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
	
	FlexGridSizer4->Add(m_check_start, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	m_check_usedefault = new wxCheckBox(this, wxID_ANY, _("Do not prompt for torrent settings"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("wxID_ANY"));

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

	FlexGridSizer4->Add(m_check_usedefault, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	m_check_preview_video_files = new wxCheckBox(this, wxID_ANY, _("Enable preview video file(s)"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("wxID_ANY"));
	m_check_preview_video_files->SetValue(isTorrent?m_pTorrent->config->GetEnableVideoPreview():pcfg->GetEnableVideoPreview());
	FlexGridSizer4->Add(m_check_preview_video_files, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	m_check_sequential_download = new wxCheckBox(this, wxID_ANY, _("Sequential Download"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("wxID_ANY"));
	m_check_sequential_download->SetValue(isTorrent?m_pTorrent->config->GetSequentialDownload():pcfg->GetSequentialDownload());
	FlexGridSizer4->Add(m_check_sequential_download, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2->Add(FlexGridSizer4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	StaticLine1 = new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxSize(10,-1), wxLI_HORIZONTAL, _T("wxID_ANY"));
	FlexGridSizer2->Add(StaticLine1, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer5 = new wxFlexGridSizer(0, 2, 0, 0);
	StaticText4 = new wxStaticText(this, wxID_ANY, _("Storage Allocation Mode:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer5->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	wxArrayString m_arrStorageMode;
	
	m_arrStorageMode.Add(_("Full"));
	m_arrStorageMode.Add(_("Sparse"));
	m_combo_storagemode = new wxComboBox(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, m_arrStorageMode, wxCB_READONLY);
	enum lt::storage_mode_t e_storagemode = isTorrent?m_pTorrent->config->GetTorrentStorageMode():pcfg->GetDefaultStorageMode();

	wxString strStorageMode;
	switch (e_storagemode)
	{
		case lt::storage_mode_allocate:
			strStorageMode = _("Full");
			break;
		case lt::storage_mode_sparse:
		default:
			strStorageMode = _("Sparse");
			break;
	}
	m_combo_storagemode->SetValue(strStorageMode);
	FlexGridSizer5->Add(m_combo_storagemode, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	StaticText5 = new wxStaticText(this, wxID_ANY, _("Download Rate Limit:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer5->Add(StaticText5, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	m_autoDownLimit = new AutoSizeInput(this, _T("Bps"));
	m_autoDownLimit->SetValue(isTorrent?m_pTorrent->config->GetTorrentDownloadLimit():pcfg->GetDefaultDownloadLimit());
	FlexGridSizer5->Add(m_autoDownLimit, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	StaticText6 = new wxStaticText(this, wxID_ANY, _("Upload Rate Limit:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer5->Add(StaticText6, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	m_autoUpLimit = new AutoSizeInput(this, _T("Bps") );
	m_autoUpLimit->SetValue(isTorrent?m_pTorrent->config->GetTorrentUploadLimit():pcfg->GetDefaultUploadLimit());
	FlexGridSizer5->Add(m_autoUpLimit, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	StaticText7 = new wxStaticText(this, wxID_ANY, _("Maximum Connections:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer5->Add(StaticText7, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	wxString t_maxconnect= wxString::Format(_T("%d"), isTorrent?m_pTorrent->config->GetTorrentMaxConnections():pcfg->GetMaxConnections());
	m_spin_maxconnect = new wxSpinCtrl(this, wxID_ANY, t_maxconnect, wxDefaultPosition, wxDefaultSize, 0, -1, 99999999, 0, _T("wxID_ANY"));
	//m_spin_maxconnect->SetValue(_T("0"));
	FlexGridSizer5->Add(m_spin_maxconnect, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	StaticText8 = new wxStaticText(this, wxID_ANY, _("Maximum Upload Slots:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer5->Add(StaticText8, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	wxString t_maxupload = wxString::Format(_T("%d"), isTorrent?m_pTorrent->config->GetTorrentMaxUploads():pcfg->GetMaxUploads());
	m_spin_maxupload = new wxSpinCtrl(this, wxID_ANY, t_maxupload, wxDefaultPosition, wxDefaultSize, 0, -1, 99999999, 0, _T("wxID_ANY"));
	//m_spin_maxupload->SetValue(_T("0"));
	FlexGridSizer5->Add(m_spin_maxupload, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	StaticText9 = new wxStaticText(this, wxID_ANY, _("Seed Ratio:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer5->Add(StaticText9, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	wxString t_ratio= wxString::Format(_T("%d"), isTorrent?m_pTorrent->config->GetTorrentRatio():pcfg->GetDefaultRatio());
	m_spin_ratio = new wxSpinCtrl(this, wxID_ANY, t_ratio, wxDefaultPosition, wxDefaultSize, 0, 0, 99999999, 0, _T("wxID_ANY"));
	//m_spin_ratio->SetValue(_T("0"));
	FlexGridSizer5->Add(m_spin_ratio, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2->Add(FlexGridSizer5, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	//Connect(wxID_ANY,wxEVT_COMMAND_TEXT_ENTER,(wxObjectEventFunction)&TorrentSettingPane::OnSaveDirectoryChanged);
	//Connect(wxID_ANY,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&TorrentSettingPane::OnButtonSaveDirClick);
	//Connect(wxID_ANY,wxEVT_COMMAND_TEXT_ENTER,(wxObjectEventFunction)&TorrentSettingPane::OnOpenDirectoryChanged);
	//Connect(wxID_ANY,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&TorrentSettingPane::OnButtonOpenDirClick);
	//Connect(wxID_ANY,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&TorrentSettingPane::OnPreviewVideoFilesClick);
	//Connect(wxID_ANY,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&TorrentSettingPane::OnSequentialDownloadClick);
	//*)

	Bind(wxEVT_BUTTON, &TorrentSettingPane::OnButtonSaveDirClick, this, m_button_saveasdir->GetId());
	Bind(wxEVT_TEXT_ENTER, &TorrentSettingPane::OnSaveDirectoryChanged, this, m_combo_saveas->GetId());
	if(!isTorrent)
	{
		Bind(wxEVT_BUTTON, &TorrentSettingPane::OnButtonOpenDirClick, this, m_button_opendir->GetId());
		Bind(wxEVT_TEXT_ENTER, &TorrentSettingPane::OnOpenDirectoryChanged, this, m_combo_open->GetId());
	}
	Bind(wxEVT_CHECKBOX, &TorrentSettingPane::OnPreviewVideoFilesClick, this, m_check_preview_video_files->GetId());
	Bind(wxEVT_CHECKBOX, &TorrentSettingPane::OnSequentialDownloadClick, this, m_check_sequential_download->GetId());
}

TorrentSettingPane::~TorrentSettingPane()
{
	//(*Destroy(TorrentSettingPane)
	//*)
}

wxString TorrentSettingPane::GetDownloadPath()
{
	wxFileName fn;
	fn.AssignDir(m_combo_saveas->GetValue());
	return fn.GetPathWithSep();
}

wxString TorrentSettingPane::GetOpenTorrentPath()
{
	wxFileName fn;
	fn.AssignDir(m_combo_open->GetValue());
	return fn.GetPathWithSep();
}

lt::storage_mode_t TorrentSettingPane::GetStorageMode()
{
	switch (m_combo_storagemode->GetCurrentSelection())
	{
	case 0:
		return lt::storage_mode_allocate;
		break;
	case 1:
	default:
		return lt::storage_mode_sparse;
	}
}

void TorrentSettingPane::OnButtonSaveDirClick(wxCommandEvent& event)
{
#if WXVER >= 280
	long dirstyle = wxDD_DEFAULT_STYLE| wxDD_DIR_MUST_EXIST |wxDD_CHANGE_DIR;
#else
	long dirstyle = wxDD_DEFAULT_STYLE| wxDD_CHANGE_DIR;
#endif

	wxDirDialog dir_dlg(this, _("Choose default download directory"), m_combo_saveas->GetValue(),dirstyle );

	if (dir_dlg.ShowModal() == wxID_OK)
	{
		wxString newpath = dir_dlg.GetPath();
		//wxLogDebug(_T("DirDlg return %s"), newpath.c_str());
		m_combo_saveas->SetValue(newpath);
	}
	event.Skip();
}

void TorrentSettingPane::OnSaveDirectoryChanged(wxCommandEvent& event)
{
	wxString downloadPath = m_combo_saveas->GetValue();
	wxFileName fdir;
	fdir.AssignDir(downloadPath); // make sure we got path seperator at the end
	fdir.MakeAbsolute();
	if(!fdir.DirExists())
	{
		wxMessageDialog dlg( this, fdir.GetFullPath() + _("does not exists! Do you want to create it?"), wxT( "Bitswash" ), wxYES_NO | wxICON_QUESTION );
		dlg.SetYesNoLabels( wxMessageDialog::ButtonLabel( _( "&Yes" ) ), wxMessageDialog::ButtonLabel( _( "&No" ) ) );

		if( dlg.ShowModal() == wxID_YES )
		{
			wxMkdir(fdir.GetFullPath());
		}
		else
			return;
	}
		
	size_t dcount = fdir.GetDirCount( );
	while(dcount-- > 0)
	{
		if(!fdir.DirExists()) fdir.RemoveLastDir();
		else break;
	}

	wxLongLong free(0);  
	if(fdir.DirExists())
	{
		wxGetDiskSpace( fdir.GetPathWithSep(), nullptr, &free);
	}
	m_label_diskfreespace->SetLabelText(HumanReadableByte(wxDouble(free.ToDouble())));
}

void TorrentSettingPane::OnPreviewVideoFilesClick(wxCommandEvent& event)
{
	m_check_sequential_download->SetValue(false);
}

void TorrentSettingPane::OnSequentialDownloadClick(wxCommandEvent& event)
{
	m_check_preview_video_files->SetValue(false);
}

void TorrentSettingPane::OnButtonOpenDirClick(wxCommandEvent& event)
{
	wxASSERT(m_button_opendir != nullptr);
#if WXVER >= 280
	long dirstyle = wxDD_DEFAULT_STYLE| wxDD_DIR_MUST_EXIST |wxDD_CHANGE_DIR;
#else
	long dirstyle = wxDD_DEFAULT_STYLE| wxDD_CHANGE_DIR;
#endif

	wxDirDialog dir_dlg(this, _("Choose default open torrent directory"), m_combo_saveas->GetValue(),dirstyle );

	if (dir_dlg.ShowModal() == wxID_OK)
	{
		wxString newpath = dir_dlg.GetPath();
		//wxLogDebug(_T("DirDlg return %s"), newpath.c_str());
		m_combo_open->SetValue(newpath);
	}
	event.Skip();
}

void TorrentSettingPane::OnOpenDirectoryChanged(wxCommandEvent& event)
{
	wxASSERT(m_combo_open != nullptr);
	wxString openTorrentPath = m_combo_open->GetValue();
	wxFileName fdir;
	fdir.AssignDir(openTorrentPath); // make sure we got path seperator at the end
	fdir.MakeAbsolute();
	if(!fdir.DirExists())
	{
		wxMessageDialog dlg( this, fdir.GetFullPath() + _("does not exists! Do you want to create it?"), wxT( "Bitswash" ), wxYES_NO | wxICON_QUESTION );
		dlg.SetYesNoLabels( wxMessageDialog::ButtonLabel( _( "&Yes" ) ), wxMessageDialog::ButtonLabel( _( "&No" ) ) );

		if( dlg.ShowModal() == wxID_YES )
		{
			wxMkdir(fdir.GetFullPath());
		}
	}
}
