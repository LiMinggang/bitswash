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
// Class: ConnectionSettingPane
//
// Created by: lim k. c. <admin@bitswash.org>
// Created on: Tue Jun 26 13:23:51 MYT 2007
//
#include <wx/intl.h>

#include <wx/sstream.h>
#include <wx/textctrl.h>


#include <wx/log.h>

#include "mainframe.h"

#include "connectionsetting.h"
#include "configuration.h"
#include "functions.h"

//CLASS ConnectionSettingPane
ConnectionSettingPane::ConnectionSettingPane( wxWindow* parent, wxWindowID id,const wxPoint& pos,const wxSize& size, int style)
{
	//(*Initialize(ConnectionSettingPane)
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
	wxStaticLine* StaticLine1;
	wxStaticLine* StaticLine2;
	wxStaticText* StaticText1;
	wxStaticText* StaticText2;
	wxStaticText* StaticText3;
	wxStaticText* StaticText4;
	wxStaticText* StaticText5;
	wxStaticText* StaticText6;
	wxStaticText* StaticText7;
	wxStaticText* StaticText8;
	wxStaticText* StaticText9;

	MainFrame* pMainFrame = dynamic_cast<MainFrame *>( wxGetApp().GetTopWindow() );
	wxASSERT(pMainFrame != nullptr);
	Configuration* pcfg = pMainFrame->GetConfig();
	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, style, _T("wxID_ANY"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	
	Panel1 = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	Panel1->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	
	BoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
	StaticText1 = new wxStaticText(Panel1, wxID_ANY, _("Ports Setting"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	BoxSizer1->Add(StaticText1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Panel1->SetSizer(BoxSizer1);
	BoxSizer1->Fit(Panel1);
	BoxSizer1->SetSizeHints(Panel1);
	FlexGridSizer1->Add(Panel1, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer2 = new wxFlexGridSizer(0, 4, 0, 0);
	StaticText2 = new wxStaticText(this, wxID_ANY, _("Port Range (TCP):"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer2->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	
	wxString t_portFrom  = wxString::Format(_T("%d"), pcfg->GetPortMin());
	m_spinPortFrom = new wxSpinCtrl(this, wxID_ANY, t_portFrom, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 65535, 0, _T("wxID_ANY"));
	//m_spinPortFrom->SetValue(_T("0"));
	FlexGridSizer2->Add(m_spinPortFrom, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	StaticText3 = new wxStaticText(this, wxID_ANY, _("to"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer2->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	
	wxString t_portTo = wxString::Format(_T("%d"), pcfg->GetPortMax());
	m_spinPortTo = new wxSpinCtrl(this, wxID_ANY, t_portTo, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 65535, 0, _T("wxID_ANY"));
	//m_spinPortTo->SetValue(_T("0"));
	FlexGridSizer2->Add(m_spinPortTo, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	m_checkDHT = new wxCheckBox(this, wxID_ANY, _("Enable DHT, Port (UDP):"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("wxID_ANY"));
	m_checkDHT->SetValue( pcfg->GetDHTEnabled());
	FlexGridSizer2->Add(m_checkDHT, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	
	wxString t_dhtport = wxString::Format(_T("%d"), pcfg->GetDHTPort());
	m_spinDHTPort = new wxSpinCtrl(this, wxID_ANY, t_dhtport, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 65535, 0, _T("wxID_ANY"));
	//m_spinDHTPort->SetValue(_T("0"));
	FlexGridSizer2->Add(m_spinDHTPort, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticLine1 = new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxSize(10,-1), wxLI_HORIZONTAL, _T("wxID_ANY"));
	FlexGridSizer1->Add(StaticLine1, 1, wxALL|wxEXPAND, 5);
	
	Panel2 = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	Panel2->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	BoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
	StaticText4 = new wxStaticText(Panel2, wxID_ANY, _("Limits"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	BoxSizer2->Add(StaticText4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Panel2->SetSizer(BoxSizer2);
	BoxSizer2->Fit(Panel2);
	BoxSizer2->SetSizeHints(Panel2);
	FlexGridSizer1->Add(Panel2, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
	StaticText5 = new wxStaticText(this, wxID_ANY, _("Download Rate Limit:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer3->Add(StaticText5, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	
	m_autoDownLimit = new AutoSizeInput( this, _T("Bps"));
	
	m_autoDownLimit->SetValue(pcfg->GetGlobalDownloadLimit());
	FlexGridSizer3->Add(m_autoDownLimit, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText6 = new wxStaticText(this, wxID_ANY, _("Upload Rate Limit:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer3->Add(StaticText6, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	
	m_autoUpLimit = new AutoSizeInput (this, _T("Bps"));
	m_autoUpLimit->SetValue(pcfg->GetGlobalUploadLimit());
	FlexGridSizer3->Add(m_autoUpLimit, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText7 = new wxStaticText(this, wxID_ANY, _("Maximum Connections:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer3->Add(StaticText7, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	m_spinMaxConnections = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 99999999, 0);
	m_spinMaxConnections->SetValue(pcfg->GetGlobalMaxConnections());
	FlexGridSizer3->Add(m_spinMaxConnections, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText8 = new wxStaticText(this, wxID_ANY, _("Maximum Upload Slots:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer3->Add(StaticText8, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	
	m_spinMaxUploads = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 99999999, 0);
	m_spinMaxUploads->SetValue(pcfg->GetGlobalMaxUploads());

	FlexGridSizer3->Add(m_spinMaxUploads, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	StaticLine2 = new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxSize(10,-1), wxLI_HORIZONTAL, _T("wxID_ANY"));
	FlexGridSizer1->Add(StaticLine2, 1, wxALL|wxEXPAND, 5);
	Panel3 = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	Panel3->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	BoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
	StaticText9 = new wxStaticText(Panel3, wxID_ANY, _("Miscellaneous"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	BoxSizer3->Add(StaticText9, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Panel3->SetSizer(BoxSizer3);
	BoxSizer3->Fit(Panel3);
	BoxSizer3->SetSizeHints(Panel3);
	FlexGridSizer1->Add(Panel3, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer4 = new wxFlexGridSizer(0, 2, 0, 0);
	m_check_anonymous_mode = new wxCheckBox(this, wxID_ANY, _("Anonymous mode"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("wxID_ANY"));
	m_check_anonymous_mode->SetValue(false);
	FlexGridSizer4->Add(m_check_anonymous_mode, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	m_check_os_cache = new wxCheckBox(this, wxID_ANY, _("Enable OS cache"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("wxID_ANY"));
	m_check_os_cache->SetValue(false);
	FlexGridSizer4->Add(m_check_os_cache, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer4, 1, wxALL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);
	//*)
}

ConnectionSettingPane::~ConnectionSettingPane()
{
	//(*Destroy(ConnectionSettingPane)
	//*)
}
int ConnectionSettingPane::GetPortFrom()
{
	return m_spinPortFrom->GetValue();
}

int ConnectionSettingPane::GetPortTo()
{
	return m_spinPortTo->GetValue();
}

int ConnectionSettingPane::GetDHTPort()
{
	return m_spinDHTPort->GetValue();
}

bool ConnectionSettingPane::GetDHTState()
{
	return m_checkDHT->IsChecked();
}

long ConnectionSettingPane::GetDownloadLimit()
{
	return m_autoDownLimit->GetValue();
}

long ConnectionSettingPane::GetUploadLimit()
{
	return m_autoUpLimit->GetValue();
}

int ConnectionSettingPane::GetMaxUploads()
{
	return m_spinMaxUploads->GetValue();
}

int ConnectionSettingPane::GetMaxConnections()
{
	return m_spinMaxConnections->GetValue();
}

bool ConnectionSettingPane::IsAnonymousMode()
{

	return m_check_anonymous_mode->GetValue();
}

bool ConnectionSettingPane::UseOSCache()
{

	return m_check_os_cache->GetValue();
}

