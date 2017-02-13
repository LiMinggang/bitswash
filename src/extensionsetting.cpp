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
// Class: ExtensionSettingPane
//
// Created by: lim k. c. <admin@bitswash.org>
// Created on: Tue Jun 26 13:23:51 MYT 2007
//

#include <wx/sstream.h>
#include <wx/textctrl.h>
#include <wx/log.h>

#include "mainframe.h"
#include "extensionsetting.h"

enum {
	EXTENSION_ID_ENABLEPE  = wxID_HIGHEST,
	EXTENSION_ID_CHECK_UPNP,
	EXTENSION_ID_CHECK_NATPMP,
	EXTENSION_ID_CHECK_LSD
};

BEGIN_EVENT_TABLE(ExtensionSettingPane, wxPanel)
	EVT_CHECKBOX(EXTENSION_ID_ENABLEPE, ExtensionSettingPane::OnEnablePe)
#if 0
	EVT_CHECKBOX(EXTENSION_ID_CHECK_UPNP, ExtensionSettingPane::OnCheckUpnp)
	EVT_CHECKBOX(EXTENSION_ID_CHECK_NATPMP, ExtensionSettingPane::OnCheckNatpmp)
	EVT_CHECKBOX(EXTENSION_ID_CHECK_LSD, ExtensionSettingPane::OnCheckLsd)
#endif
END_EVENT_TABLE()

ExtensionSettingPane::ExtensionSettingPane( wxWindow* parent,
	       int id, 
	       wxPoint pos, 
	       wxSize size, 
	       int style): wxPanel(parent, id, pos, size, style)
{

	m_pMainFrame = (wxFrame*)wxGetApp().GetTopWindow();
	m_pcfg = ((MainFrame*)m_pMainFrame)->GetConfig();

	wxFlexGridSizer* fgSizerMain;
	fgSizerMain = new wxFlexGridSizer( 0, 1, 0, 0 );
	fgSizerMain->SetFlexibleDirection( wxBOTH );
	
	m_pane_infoextension = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxSize( -1,20 ), wxTAB_TRAVERSAL );
	m_pane_infoextension->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_ACTIVECAPTION ) );
	
	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer( wxVERTICAL );
	
	m_static_infoextension = new wxStaticText( m_pane_infoextension, wxID_ANY, _("Extensions"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer1->Add( m_static_infoextension, 0, wxALL, 2 );
	
	m_pane_infoextension->SetSizer( bSizer1 );
	m_pane_infoextension->Layout();
	fgSizerMain->Add( m_pane_infoextension, 1, wxEXPAND | wxALL, 5 );
	
	wxFlexGridSizer* fgSizerExtensions;
	fgSizerExtensions = new wxFlexGridSizer( 0, 1, 0, 0 );
	fgSizerExtensions->SetFlexibleDirection( wxBOTH );
	
	m_check_metadata = new wxCheckBox( this, wxID_ANY, _("Enable Metadata"), wxDefaultPosition, wxDefaultSize, 0 );
	m_check_metadata->SetValue(m_pcfg->GetEnableMetadata());
	
	fgSizerExtensions->Add( m_check_metadata, 0, wxALL, 5 );
	
	m_check_pex = new wxCheckBox( this, wxID_ANY, _("Enable Peer Exchange"), wxDefaultPosition, wxDefaultSize, 0 );
	m_check_pex->SetValue(m_pcfg->GetEnablePex());
	
	fgSizerExtensions->Add( m_check_pex, 0, wxALL, 5 );
	
	fgSizerMain->Add( fgSizerExtensions, 1, wxEXPAND, 5 );
	
	m_pane_infoencryption = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxSize( -1,20 ), wxTAB_TRAVERSAL );
	m_pane_infoencryption->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_ACTIVECAPTION ) );
	
	wxBoxSizer* bSizer11;
	bSizer11 = new wxBoxSizer( wxVERTICAL );
	
	m_static_infoencryption = new wxStaticText( m_pane_infoencryption, wxID_ANY, _("Protocol Encryption"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer11->Add( m_static_infoencryption, 0, wxALL, 2 );
	
	m_pane_infoencryption->SetSizer( bSizer11 );
	m_pane_infoencryption->Layout();
	fgSizerMain->Add( m_pane_infoencryption, 1, wxEXPAND | wxALL, 5 );
	
	wxFlexGridSizer* fgSizerEncryption;
	fgSizerEncryption = new wxFlexGridSizer( 0, 1, 0, 0 );
	fgSizerEncryption->SetFlexibleDirection( wxBOTH );
	
	m_check_pe = new wxCheckBox( this, EXTENSION_ID_ENABLEPE, _("Enable Encryption"), wxDefaultPosition, wxDefaultSize, 0 );
	m_check_pe->SetValue(m_pcfg->GetEncEnabled());
	
	fgSizerEncryption->Add( m_check_pe, 0, wxALL, 5 );
	
	m_check_pe_notforce = new wxCheckBox( this, wxID_ANY, _("Accept Unencrypted Peer"), wxDefaultPosition, wxDefaultSize, 0 );
	m_check_pe_notforce->SetValue((m_pcfg->GetEncPolicy() > 0));
	m_check_pe_notforce->Enable(m_pcfg->GetEncEnabled());
	
	fgSizerEncryption->Add( m_check_pe_notforce, 0, wxALL, 5 );
	
	fgSizerMain->Add( fgSizerEncryption, 1, wxEXPAND, 5 );
	
	m_pane_infonetwork = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxSize( -1,20 ), wxTAB_TRAVERSAL );
	m_pane_infonetwork->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_ACTIVECAPTION ) );
	
	wxBoxSizer* bSizer111;
	bSizer111 = new wxBoxSizer( wxVERTICAL );
	
	m_static_infonat = new wxStaticText( m_pane_infonetwork, wxID_ANY, _("NAT Service"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer111->Add( m_static_infonat, 0, wxALL, 2 );
	
	m_pane_infonetwork->SetSizer( bSizer111 );
	m_pane_infonetwork->Layout();
	fgSizerMain->Add( m_pane_infonetwork, 1, wxEXPAND | wxALL, 5 );
	
	wxFlexGridSizer* fgSizerNetwork;
	fgSizerNetwork = new wxFlexGridSizer( 0, 1, 0, 0 );
	fgSizerNetwork->SetFlexibleDirection( wxBOTH );
	
	m_check_upnp = new wxCheckBox( this, EXTENSION_ID_CHECK_UPNP, _("Enable UPnP"), wxDefaultPosition, wxDefaultSize, 0 );
	m_check_upnp->SetValue(m_pcfg->GetEnableUpnp());
	
	fgSizerNetwork->Add( m_check_upnp, 0, wxALL, 5 );
	
	m_check_natpmp = new wxCheckBox( this, EXTENSION_ID_CHECK_NATPMP, _("Enable NAT Port Mapping"), wxDefaultPosition, wxDefaultSize, 0 );
	m_check_natpmp->SetValue(m_pcfg->GetEnableNatpmp());
	
	fgSizerNetwork->Add( m_check_natpmp, 0, wxALL, 5 );
	
	m_check_lsd = new wxCheckBox( this, EXTENSION_ID_CHECK_LSD, _("Enable Local Service Discovery"), wxDefaultPosition, wxDefaultSize, 0 );
	m_check_lsd->SetValue(m_pcfg->GetEnableLsd());
	
	fgSizerNetwork->Add( m_check_lsd, 0, wxALL, 5 );
	
	fgSizerMain->Add( fgSizerNetwork, 1, wxEXPAND, 5 );
	
	this->SetSizer( fgSizerMain );
	this->Layout();
	fgSizerMain->Fit( this );


}

bool ExtensionSettingPane::GetEnableMetadata()
{
		return m_check_metadata->GetValue();
}

bool ExtensionSettingPane::GetEnablePex()
{
		return m_check_pex->GetValue();
}

bool ExtensionSettingPane::GetEnableEncryption()
{
		return m_check_pe->GetValue();
}

bool ExtensionSettingPane::GetForceEncryption()
{
		return !m_check_pe_notforce->GetValue();
}

bool ExtensionSettingPane::GetEnableUpnp()
{
		return m_check_upnp->GetValue();
}

bool ExtensionSettingPane::GetEnableNatpmp()
{
		return m_check_natpmp->GetValue();
}

bool ExtensionSettingPane::GetEnableLsd()
{
		return m_check_lsd->GetValue();
}


void ExtensionSettingPane::OnEnablePe(wxCommandEvent& event)
{
	bool enablepe = m_check_pe->GetValue();

	m_check_pe_notforce->Enable(enablepe);

}

void ExtensionSettingPane::OnCheckUpnp(wxCommandEvent& event)
{
	bool enableupnp = m_check_upnp->GetValue();

	//((MainFrame*)m_pMainFrame)->StartUpnp(enableupnp);
}

void ExtensionSettingPane::OnCheckNatpmp(wxCommandEvent& event)
{
	bool enablenatpmp = m_check_natpmp->GetValue();

	//((MainFrame*)m_pMainFrame)->StartNatpmp(enablenatpmp);

}

void ExtensionSettingPane::OnCheckLsd(wxCommandEvent& event)
{
	bool enablelsd = m_check_lsd->GetValue();

	//((MainFrame*)m_pMainFrame)->StartLsd(enablelsd);

}
