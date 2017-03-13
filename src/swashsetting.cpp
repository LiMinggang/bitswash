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
// Class: SwashSetting
// Created by: lim k. c. <admin@bitswash.org>
// Created on: Thu May 17 21:45:53 MYT 2007
//


#include <wx/wx.h>
#include <wx/imaglist.h>

#include "compat.h"
#include "bitswash.h"
#include "swashsetting.h"
#include "bittorrentsession.h"

BEGIN_EVENT_TABLE(SwashSetting, wxDialog)
	EVT_BUTTON(wxID_OK, SwashSetting::OnOK)
	EVT_BUTTON(wxID_CANCEL, SwashSetting::OnCancel)
END_EVENT_TABLE()

SwashSetting::SwashSetting( wxWindow* parent, Configuration *pcfg, int id, wxString title, wxPoint pos, wxSize size, int style ) : wxDialog( parent, id, title, pos, size, style )
{
	//wxASSERT(pcfg == NULL);

	m_pcfg = pcfg;

	this->SetSizeHints( wxSize( -1,-1 ), wxDefaultSize );
	this->Centre( wxBOTH );
	
	wxFlexGridSizer* fgMain;
	fgMain = new wxFlexGridSizer( 2, 1, 0, 0 );

	
	m_panel_main = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxFlexGridSizer* fgInner;
	fgInner = new wxFlexGridSizer( 1, 1, 0, 0 );
	
#if wxCHECK_VERSION(2,8,0)
	m_listbookmenu = new wxTreebook( m_panel_main, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLB_DEFAULT );
#else
	m_listbookmenu = new wxListbook( m_panel_main, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLB_DEFAULT );
#endif

	m_listbookmenu->SetImageList(wxGetApp().GetSettingIconsList() );

	m_pane_generalsettings = new GeneralSettingPane(m_listbookmenu);

	m_pane_connectionsettings = new ConnectionSettingPane(m_listbookmenu);
	m_pane_torrentsettings = new TorrentSettingPane((wxWindow*)m_listbookmenu);
	m_pane_extensionsettings = new ExtensionSettingPane(m_listbookmenu);


	m_listbookmenu->AddPage(m_pane_generalsettings, _("General"), false, 0);
	m_listbookmenu->AddPage(m_pane_connectionsettings, _("Connections"), false, 1);
	m_listbookmenu->AddPage(m_pane_torrentsettings, _("Torrents"), false, 2);
	m_listbookmenu->AddPage(m_pane_extensionsettings, _("Extensions"), false, 3);
	
	fgInner->Add( m_listbookmenu, 1, wxEXPAND | wxALL, 5 );
	
	m_panel_main->SetSizer( fgInner );
	m_panel_main->Layout();
	fgInner->Fit( m_panel_main );
	fgMain->Add( m_panel_main, 1, wxEXPAND | wxALL, 5 );
	
	m_szokcancel = new wxStdDialogButtonSizer();
	m_szokcancel->AddButton( new wxButton( this, wxID_OK ) );
	m_szokcancel->AddButton( new wxButton( this, wxID_CANCEL ) );
	m_szokcancel->Realize();
	fgMain->Add( m_szokcancel, 0, wxALIGN_RIGHT| wxALL, 5 );
	
	this->SetSizer( fgMain );
	this->Layout();
	fgMain->Fit(this);
}

SwashSetting::~SwashSetting()
{
}

void SwashSetting::OnOK(wxCommandEvent& event)
{
	m_pcfg->SetMaxStart(m_pane_generalsettings->GetMaxStart());
	m_pcfg->SetExcludeSeed(m_pane_generalsettings->GetExcludeSeed());

	m_pcfg->SetAssociateMagnetURI(m_pane_generalsettings->GetAssociateMagnetURI());
#ifdef __WXMSW__
	m_pcfg->SetRunAtStartup(m_pane_generalsettings->GetRunAtStartup());
	m_pcfg->SetAssociateTorrent(m_pane_generalsettings->GetAssociateTorrent());
#endif

	m_pcfg->SetFastResumeSaveTime(m_pane_generalsettings->GetFastResumeSaveTime());
	m_pcfg->SetRefreshTime(m_pane_generalsettings->GetRefreshTimer());
	m_pcfg->SetUseSystray(m_pane_generalsettings->GetUseSystray());
	m_pcfg->SetHideTaskbar(m_pane_generalsettings->GetHideTaskbar());
	m_pcfg->SetLogSeverity(m_pane_generalsettings->GetLogSeverity());
	m_pcfg->SetLogFile(m_pane_generalsettings->GetLogToFile());
	m_pcfg->SetLogLineCount(m_pane_generalsettings->GetLogLineCount());

	m_pcfg->SetPortMin(m_pane_connectionsettings->GetPortFrom());
	m_pcfg->SetPortMax(m_pane_connectionsettings->GetPortTo());

	m_pcfg->SetDHTEnabled(m_pane_connectionsettings->GetDHTState());
	m_pcfg->SetDHTPort(m_pane_connectionsettings->GetDHTPort());

	m_pcfg->SetGlobalDownloadLimit(m_pane_connectionsettings->GetDownloadLimit());
	m_pcfg->SetGlobalUploadLimit(m_pane_connectionsettings->GetUploadLimit());

	m_pcfg->SetGlobalMaxConnections(m_pane_connectionsettings->GetMaxConnections());
	m_pcfg->SetGlobalMaxUploads(m_pane_connectionsettings->GetMaxUploads());

	m_pcfg->SetGlobalMaxHalfConnect(m_pane_connectionsettings->GetMaxHalfOpen());
	
	//m_pcfg->SetCompactAlloc(m_pane_torrentsettings->GetCompactAlloc());
	m_pcfg->SetDefaultStorageMode(m_pane_torrentsettings->GetStorageMode());

	m_pcfg->SetDefaultState((m_pane_torrentsettings->GetStartTorrent())?TORRENT_STATE_QUEUE:TORRENT_STATE_STOP);
	m_pcfg->SetUseDefault(m_pane_torrentsettings->GetUseDefault());

	m_pcfg->SetDownloadPath(m_pane_torrentsettings->GetDownloadPath());
	m_pcfg->SetDefaultDownloadLimit(m_pane_torrentsettings->GetDownloadRate());
	m_pcfg->SetDefaultUploadLimit(m_pane_torrentsettings->GetUploadRate());
	m_pcfg->SetMaxConnections(m_pane_torrentsettings->GetMaxConnections());
	m_pcfg->SetMaxUploads(m_pane_torrentsettings->GetMaxUpload());
	m_pcfg->SetDefaultRatio(m_pane_torrentsettings->GetRatio());

	m_pcfg->SetEnableMetadata(m_pane_extensionsettings->GetEnableMetadata());
	m_pcfg->SetEnablePex(m_pane_extensionsettings->GetEnablePex());

	if (m_pane_extensionsettings->GetEnableEncryption())
	{
		m_pcfg->SetEncEnabled(true);
		m_pcfg->SetEncPolicy(m_pane_extensionsettings->GetForceEncryption()?0:1);
	}
	else
	{
		m_pcfg->SetEncEnabled(false);
		m_pcfg->SetEncPolicy(2);
	}

	m_pcfg->SetEnableUpnp(m_pane_extensionsettings->GetEnableUpnp());
	m_pcfg->SetEnableNatpmp(m_pane_extensionsettings->GetEnableNatpmp());
	m_pcfg->SetEnableLsd(m_pane_extensionsettings->GetEnableLsd());

	m_pcfg->Save();

	EndModal(wxID_OK);
	event.Skip();
}

void SwashSetting::OnCancel(wxCommandEvent& event)
{
	EndModal (wxID_CANCEL);
	event.Skip();
}

