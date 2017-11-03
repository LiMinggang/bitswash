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


#include <wx/sstream.h>
#include <wx/textctrl.h>


#include <wx/log.h>

#include "mainframe.h"

#include "connectionsetting.h"
#include "functions.h"

//CLASS ConnectionSettingPane
ConnectionSettingPane::ConnectionSettingPane( wxWindow* parent, int id, wxPoint pos, wxSize size, int style) : wxPanel(parent, id, pos, size, style)
{
	MainFrame* pMainFrame = dynamic_cast<MainFrame *>( wxGetApp().GetTopWindow() );
	wxASSERT(pMainFrame != 0);
	Configuration* pcfg = pMainFrame->GetConfig();

	wxFlexGridSizer* fgConnectionMain;
	fgConnectionMain = new wxFlexGridSizer( 0, 1, 0, 0 );
	fgConnectionMain->SetFlexibleDirection( wxBOTH );
	
	m_pane_infoport = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	m_pane_infoport->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_ACTIVECAPTION ) );
	
	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer( wxVERTICAL );
	
	m_static_infoport = new wxStaticText( m_pane_infoport, wxID_ANY, _("Ports Setting"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer1->Add( m_static_infoport, 0, wxALL, 2 );
	
	m_pane_infoport->SetSizer( bSizer1 );
	m_pane_infoport->Layout();
	fgConnectionMain->Add( m_pane_infoport, 1, wxEXPAND | wxALL, 5 );
	
	wxFlexGridSizer* fgPortSettings;
	fgPortSettings = new wxFlexGridSizer( 1, 4, 0, 0 );
	fgPortSettings->AddGrowableCol( 0 );
	fgPortSettings->SetFlexibleDirection( wxBOTH );
	
	m_staticPortRange = new wxStaticText( this, wxID_ANY, _("Port Range (TCP):"), wxDefaultPosition, wxDefaultSize, 0 );
	fgPortSettings->Add( m_staticPortRange, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxString t_portFrom  = wxString::Format(_T("%d"), pcfg->GetPortMin());
	m_spinPortFrom = new wxSpinCtrl( this, wxID_ANY, t_portFrom, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 65535, 0);
	fgPortSettings->Add( m_spinPortFrom, 0, wxALL, 5 );
	
	m_staticPortRangeTo = new wxStaticText( this, wxID_ANY, _("to"), wxDefaultPosition, wxDefaultSize, 0 );
	fgPortSettings->Add( m_staticPortRangeTo, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxString t_portTo = wxString::Format(_T("%d"), pcfg->GetPortMax());
	m_spinPortTo = new wxSpinCtrl( this, wxID_ANY, t_portTo, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 65535, 0);
	fgPortSettings->Add( m_spinPortTo, 0, wxALL, 5 );
	
	fgConnectionMain->Add( fgPortSettings, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgDHT;
	fgDHT = new wxFlexGridSizer( 2, 2, 0, 0 );
	fgDHT->AddGrowableCol( 1 );
	fgDHT->SetFlexibleDirection( wxBOTH );
	
	m_checkDHT = new wxCheckBox( this, wxID_ANY, _("Enable DHT, Port (UDP):"), wxDefaultPosition, wxDefaultSize );

	m_checkDHT->SetValue( pcfg->GetDHTEnabled());
	
	fgDHT->Add( m_checkDHT, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxString t_dhtport = wxString::Format(_T("%d"), pcfg->GetDHTPort());
	m_spinDHTPort = new wxSpinCtrl( this, wxID_ANY, t_dhtport, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 65535, 0);
	fgDHT->Add( m_spinDHTPort, 0, wxALL, 5 );
	
	fgConnectionMain->Add( fgDHT, 1, wxEXPAND, 5 );
	
	m_staticline1 = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	fgConnectionMain->Add( m_staticline1, 0, wxALL, 5 );
	
	m_pane_infolimit = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	m_pane_infolimit->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_ACTIVECAPTION ) );
	
	wxBoxSizer* bSizer11;
	bSizer11 = new wxBoxSizer( wxVERTICAL );
	
	m_static_infoport1 = new wxStaticText( m_pane_infolimit, wxID_ANY, _("Limits"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer11->Add( m_static_infoport1, 0, wxALL, 2 );
	
	m_pane_infolimit->SetSizer( bSizer11 );
	m_pane_infolimit->Layout();
	fgConnectionMain->Add( m_pane_infolimit, 1, wxEXPAND | wxALL, 5 );
	
	wxFlexGridSizer* fgLimit;
	fgLimit = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgLimit->AddGrowableCol( 0 );
	fgLimit->SetFlexibleDirection( wxBOTH );
	
	m_staticDownLimit = new wxStaticText( this, wxID_ANY, _("Download Rate Limit:"), wxDefaultPosition, wxDefaultSize, 0 );
	fgLimit->Add( m_staticDownLimit, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_autoDownLimit = new AutoSizeInput( this, _T("Bps"));
	
	m_autoDownLimit->SetValue(pcfg->GetGlobalDownloadLimit());
	fgLimit->Add( m_autoDownLimit, 0, wxALL, 5 );
	
	m_staticUpLimit = new wxStaticText( this, wxID_ANY, _("Upload Rate Limit:"), wxDefaultPosition, wxDefaultSize, 0 );
	fgLimit->Add( m_staticUpLimit, 0, wxALL, 5 );
	
	m_autoUpLimit = new AutoSizeInput (this, _T("Bps"));
	m_autoUpLimit->SetValue(pcfg->GetGlobalUploadLimit());
	fgLimit->Add( m_autoUpLimit, 0, wxALL, 5 );
	
	m_staticMaxConnections = new wxStaticText( this, wxID_ANY, _("Maximum Connections:"), wxDefaultPosition, wxDefaultSize, 0 );
	fgLimit->Add( m_staticMaxConnections, 0, wxALL, 5 );
	
	m_spinMaxConnections = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 99999999, 0);
	m_spinMaxConnections->SetValue(pcfg->GetGlobalMaxConnections());
	fgLimit->Add( m_spinMaxConnections, 0, wxALL, 5 );
	
	m_staticMaxUploads = new wxStaticText( this, wxID_ANY, _("Maximum Upload Slots:"), wxDefaultPosition, wxDefaultSize, 0 );
	fgLimit->Add( m_staticMaxUploads, 0, wxALL, 5 );
	
	m_spinMaxUploads = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 99999999, 0);
	m_spinMaxUploads->SetValue(pcfg->GetGlobalMaxUploads());

	fgLimit->Add( m_spinMaxUploads, 0, wxALL, 5 );
	
	m_staticMaxHalfOpen = new wxStaticText( this, wxID_ANY, _("Maximum Half Open Connections:"), wxDefaultPosition, wxDefaultSize, 0 );
	fgLimit->Add( m_staticMaxHalfOpen, 0, wxALL, 5 );
	
	m_spinMaxHalfOpen = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 99999999, 0);
	m_spinMaxHalfOpen->SetValue(pcfg->GetGlobalMaxHalfConnect());
	fgLimit->Add( m_spinMaxHalfOpen, 0, wxALL, 5 );
	
	fgConnectionMain->Add( fgLimit, 1, wxEXPAND, 5 );
	
	this->SetSizer( fgConnectionMain );
	this->Layout();
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

int ConnectionSettingPane::GetMaxHalfOpen()
{
	return m_spinMaxHalfOpen->GetValue();
}

