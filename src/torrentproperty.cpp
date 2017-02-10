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
//
// Created by: lim k. c. <admin@bitswash.org>
// Created on: Sat Feb  3 17:02:38 2007
//

#include <wx/filename.h>
#include <wx/sstream.h>
#include <wx/textctrl.h>
#include <wx/log.h>

//#include <boost/date_time/posix_time/posix_time.hpp>

#include "mainframe.h"
#include "torrentsetting.h"

#include "functions.h"

BEGIN_EVENT_TABLE(TorrentProperty, wxDialog)
	EVT_BUTTON(wxID_OK, TorrentProperty::OnOK)
	EVT_BUTTON(wxID_CANCEL, TorrentProperty::OnCancel)
	EVT_SET_FOCUS(TorrentProperty::OnFocus)
END_EVENT_TABLE()

TorrentProperty::TorrentProperty(torrent_t* pTorrent, wxWindow *parent,
                const wxWindowID id,
		wxString title,
               const wxPoint& pos,
               const wxSize& size,
               long style) : wxDialog(parent, id, title, pos, size, style)
{

	wxASSERT( pTorrent != NULL);

	m_pMainFrame = (wxFrame*)parent;

	m_pTorrent = pTorrent;

	this->SetSizeHints(wxDefaultSize, wxDefaultSize);
	this->Centre (wxBOTH);

	SetMaxSize(wxSize(400,-1));
	int w,h;
	//SetScrollbars(20,20,5,5);

	GetClientSize(&w, &h);

	wxFlexGridSizer* m_torrentsettings_sizer;
	m_torrentsettings_sizer = new wxFlexGridSizer( 1, 1, 3, 3 );
	m_torrentsettings_sizer->SetFlexibleDirection( wxVERTICAL);
	
	m_pane_info = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxSize( -1,10 ), wxTAB_TRAVERSAL );
	m_pane_info->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_ACTIVECAPTION ) );
	m_pane_info->SetMaxSize( wxSize( -1,20 ) );
	
	wxBoxSizer* m_pane_boxsizer;
	m_pane_boxsizer = new wxBoxSizer( wxVERTICAL );
	
	m_pane_label_info = new wxStaticText( m_pane_info, wxID_ANY, _("Torrent Info"), wxDefaultPosition, wxDefaultSize, 0 );
	m_pane_boxsizer->Add( m_pane_label_info, 0, wxALL, 2 );
	
	m_pane_info->SetSizer( m_pane_boxsizer );
	m_pane_info->Layout();
	m_torrentsettings_sizer->Add( m_pane_info, 1, wxEXPAND | wxALL, 5 );
	
	wxFlexGridSizer* fgSizer11;
	fgSizer11 = new wxFlexGridSizer( 1, 2, 5, 5 );
	fgSizer11->AddGrowableCol( 1 );
	fgSizer11->SetFlexibleDirection( wxBOTH );
	
	m_label_name = new wxStaticText( this, wxID_ANY, _("Name:"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer11->Add( m_label_name, 0, wxALL, 5 );
	
	libtorrent::torrent_info const& torrent_info = *(m_pTorrent->info);
 	wxString t_name = wxString(wxConvUTF8.cMB2WC(torrent_info.name().c_str()));


	m_label_torrentname = new wxStaticText( this, wxID_ANY, t_name, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer11->Add( m_label_torrentname, 0, wxALL, 5 );
	
	m_label_comment = new wxStaticText( this, wxID_ANY, _("Comment:"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer11->Add( m_label_comment, 0, wxALL, 5 );
	
	wxString t_comment = wxString(wxConvUTF8.cMB2WC(torrent_info.comment().c_str()));

	int cw, ch;
	m_label_comment->GetClientSize(&cw, &ch);

	m_text_torrentcomment = new wxTextCtrl(this, wxID_ANY, t_comment, wxDefaultPosition, wxSize(w-50 - cw , 80) , wxTE_AUTO_URL | wxTE_BESTWRAP | wxTE_MULTILINE | wxTE_READONLY) ;
	//m_label_torrentcomment = new wxStaticText( this, wxID_ANY, t_comment, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer11->Add( m_text_torrentcomment, 0, wxALL, 5 );
	
	m_label_size = new wxStaticText( this, wxID_ANY, _("Size:"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer11->Add( m_label_size, 0, wxALL, 5 );
	
	wxString t_size = HumanReadableByte((wxDouble)torrent_info.total_size());
	m_label_torrentsize = new wxStaticText( this, wxID_ANY, t_size, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer11->Add( m_label_torrentsize, 0, wxALL, 5 );
	
	m_torrentsettings_sizer->Add( fgSizer11, 1, wxEXPAND, 5 );
	
	m_notebook_property = new wxNotebook( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
	m_notebook_property->SetMinSize( wxSize( 400,-1 ) );
	
	m_filespane= new FileListCtrl( m_notebook_property, wxEmptyString, wxID_ANY );

	m_filespane->SetStaticHandle(pTorrent); 
	m_filespane->SetItemCount(pTorrent->info->num_files()); 
	
	m_filespane->UpdateSwashList();
	m_notebook_property->AddPage( m_filespane, _("Files"), false );

	m_panel_settings = new TorrentSettingPane( m_notebook_property, pTorrent);

	m_notebook_property->AddPage( m_panel_settings, _("Settings"), false );
	
	m_notebook_property->SetSelection(0);
	m_torrentsettings_sizer->Add( m_notebook_property, 1, wxEXPAND | wxALL, 5 );
	
	m_sdbSizer = new wxStdDialogButtonSizer();
	m_sdbSizer->AddButton( new wxButton( this, wxID_OK ) );
	m_sdbSizer->AddButton( new wxButton( this, wxID_CANCEL ) );
	m_sdbSizer->Realize();

	m_torrentsettings_sizer->Add( m_sdbSizer, 0, wxALIGN_RIGHT|wxALL, 5 );

	this->SetSizer( m_torrentsettings_sizer );
	
	this->Layout();
	m_torrentsettings_sizer->Fit(this);


	/*
	std::vector<struct libtorrent::announce_entry>::const_iterator idx;
	std::vector<struct libtorrent::announce_entry> trackers = m_pTorrent->handle.trackers();

	for(idx = trackers.begin(); idx != trackers.end(); ++idx)
	{
		struct libtorrent::announce_entry an = *idx;
		wxLogMessage(_T("tracker [%d] - %s\n"), an.tier, wxString::FromAscii(an.url.c_str()).c_str());
	}
	*/
}


TorrentProperty::~TorrentProperty()
{
		/* //delete all pointer?

	delete m_status_info_sizer;
	delete m_status_chunk_sizer;
	delete m_statusbox;
	*/
}


void TorrentProperty::OnOK(wxCommandEvent& event)
{
	wxLogDebug(_T("OnOk\n"));

	TorrentConfig* pConfig = m_pTorrent->config;
	Configuration *pMainConfig = wxGetApp().GetConfig();

	pConfig->SetDownloadPath(m_panel_settings->GetDownloadPath());

	//pConfig->SetTorrentCompactAlloc(m_panel_settings->GetCompactAlloc());
	pConfig->SetTorrentStorageMode(m_panel_settings->GetStorageMode());
	

	pConfig->SetTorrentState(m_panel_settings->GetStartTorrent()?TORRENT_STATE_QUEUE:TORRENT_STATE_STOP);

	pMainConfig->SetUseDefault(m_panel_settings->GetUseDefault());

	pConfig->SetTorrentDownloadLimit(m_panel_settings->GetDownloadRate());
	pConfig->SetTorrentUploadLimit(m_panel_settings->GetUploadRate());
	pConfig->SetTorrentMaxConnections(m_panel_settings->GetMaxConnections());
	pConfig->SetTorrentMaxUploads(m_panel_settings->GetMaxUpload());

	pConfig->SetTorrentRatio(m_panel_settings->GetRatio());

	pConfig->Save();
	EndModal(wxID_OK);
	event.Skip();

}

void TorrentProperty::OnCancel(wxCommandEvent& event)
{
	wxLogDebug(_T("OnCancel\n"));

	EndModal (wxID_CANCEL);
	event.Skip();

}

//XXX onfocus not working
void TorrentProperty::OnFocus(wxFocusEvent& event)
{
		wxLogDebug(_T("TorrentProperty OnFocus %d\n"), m_notebook_property->GetSelection());
		if(m_notebook_property->GetSelection() == 0)
		{
			m_filespane->UpdateSwashList();
		}
}
