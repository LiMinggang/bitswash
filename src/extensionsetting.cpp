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

//(*InternalHeaders(ExtensionSettingPane)
#include <wx/sstream.h>
#include <wx/textctrl.h>
#include <wx/log.h>
//*)
#include "mainframe.h"
#include "configuration.h"
#include "extensionsetting.h"

//(*IdInit(ExtensionSettingPane)
//*)

ExtensionSettingPane::ExtensionSettingPane(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size, int style )
{
	//(*Initialize(ExtensionSettingPane)
	wxBoxSizer* BoxSizer1;
	wxBoxSizer* BoxSizer2;
	wxBoxSizer* BoxSizer3;
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer4;
	wxPanel* Panel1;
	wxPanel* Panel2;
	wxPanel* Panel3;
	wxStaticText* StaticText1;
	wxStaticText* StaticText2;
	wxStaticText* StaticText3;

	Create(parent, id, wxDefaultPosition, wxDefaultSize, style, _T("id"));

	MainFrame* pMainFrame = dynamic_cast< MainFrame* >(wxGetApp().GetTopWindow());
	wxASSERT(pMainFrame != nullptr);
	m_pcfg = pMainFrame->GetConfig();

	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	Panel1 = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	Panel1->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	BoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
	StaticText1 = new wxStaticText(Panel1, wxID_ANY, _("Extensions"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	BoxSizer1->Add(StaticText1, 1, wxALL|wxALIGN_CENTER_VERTICAL, 5);
	Panel1->SetSizer(BoxSizer1);
	BoxSizer1->Fit(Panel1);
	BoxSizer1->SetSizeHints(Panel1);
	FlexGridSizer1->Add(Panel1, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer2 = new wxFlexGridSizer(0, 1, 0, 0);
	m_check_metadata = new wxCheckBox(this, wxID_ANY, _("Enable Metadata"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("wxID_ANY"));
	m_check_metadata->SetValue( m_pcfg->GetEnableMetadata() );
	FlexGridSizer2->Add(m_check_metadata, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	m_check_pex = new wxCheckBox(this, wxID_ANY, _("Enable Peer Exchange"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("wxID_ANY"));
	m_check_pex->SetValue( m_pcfg->GetEnablePex() );
	FlexGridSizer2->Add(m_check_pex, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Panel2 = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	Panel2->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	BoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
	StaticText2 = new wxStaticText(Panel2, wxID_ANY, _("Protocol Encryption"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	BoxSizer2->Add(StaticText2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Panel2->SetSizer(BoxSizer2);
	BoxSizer2->Fit(Panel2);
	BoxSizer2->SetSizeHints(Panel2);
	FlexGridSizer1->Add(Panel2, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer3 = new wxFlexGridSizer(0, 1, 0, 0);
	m_check_pe = new wxCheckBox(this, wxID_ANY, _("Enable Encryption"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("wxID_ANY"));
	m_check_pe->SetValue( m_pcfg->GetEncEnabled() );
	FlexGridSizer3->Add(m_check_pe, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	m_check_pe_notforce = new wxCheckBox(this, wxID_ANY, _("Accept Unencrypted Peer"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("wxID_ANY"));
	m_check_pe_notforce->SetValue( ( m_pcfg->GetEncPolicy() > 0 ) );
	FlexGridSizer3->Add(m_check_pe_notforce, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Panel3 = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	Panel3->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	BoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
	StaticText3 = new wxStaticText(Panel3, wxID_ANY, _("NAT Service"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	BoxSizer3->Add(StaticText3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Panel3->SetSizer(BoxSizer3);
	BoxSizer3->Fit(Panel3);
	BoxSizer3->SetSizeHints(Panel3);
	FlexGridSizer1->Add(Panel3, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer4 = new wxFlexGridSizer(0, 1, 0, 0);
	m_check_upnp = new wxCheckBox(this, wxID_ANY, _("Enable UPnP"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("wxID_ANY"));
	m_check_upnp->SetValue( m_pcfg->GetEnableUpnp() );
	FlexGridSizer4->Add(m_check_upnp, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	m_check_natpmp = new wxCheckBox(this, wxID_ANY, _("Enable NAT Port Mapping"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("wxID_ANY"));
	m_check_natpmp->SetValue( m_pcfg->GetEnableNatpmp() );
	FlexGridSizer4->Add(m_check_natpmp, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	m_check_lsd = new wxCheckBox(this, wxID_ANY, _("Enable Local Service Discovery"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("wxID_ANY"));
	m_check_lsd->SetValue( m_pcfg->GetEnableLsd() );
	FlexGridSizer4->Add(m_check_lsd, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	//Connect(wxID_ANY,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&ExtensionSettingPane::OnEnablePe);
	//*)
	Bind( wxEVT_CHECKBOX, &ExtensionSettingPane::OnEnablePe, this, m_check_pe->GetId() );
}

ExtensionSettingPane::~ExtensionSettingPane()
{
	//(*Destroy(ExtensionSettingPane)
	//*)
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

void ExtensionSettingPane::OnEnablePe( wxCommandEvent& event )
{
	bool enablepe = m_check_pe->GetValue();
	m_check_pe_notforce->Enable( enablepe );
}

