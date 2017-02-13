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
// Class: GeneralSettingPane
//
// Created by: lim k. c. <admin@bitswash.org>
// Created on: Tue Jun 26 13:23:51 MYT 2007
//

#include <wx/sstream.h>
#include <wx/textctrl.h>
#include <wx/log.h>

#include "mainframe.h"
#include "generalsetting.h"
#include "functions.h"

enum 
{ 
	GENERAL_ID_USESYSTRAY  = wxID_HIGHEST,
	GENERAL_ID_LOGTOFILE  
};

BEGIN_EVENT_TABLE(GeneralSettingPane, wxPanel)
	EVT_CHECKBOX(GENERAL_ID_USESYSTRAY, GeneralSettingPane::OnUseSystray)
END_EVENT_TABLE()

GeneralSettingPane::GeneralSettingPane( wxWindow* parent,
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
	
	m_pane_infogeneral = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxSize( -1,20 ), wxTAB_TRAVERSAL );
	m_pane_infogeneral->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_ACTIVECAPTION ) );
	
	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer( wxVERTICAL );
	
	m_static_infogeneral = new wxStaticText( m_pane_infogeneral, wxID_ANY, _("General"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer1->Add( m_static_infogeneral, 0, wxALL, 2 );
	
	m_pane_infogeneral->SetSizer( bSizer1 );
	m_pane_infogeneral->Layout();
	fgSizerMain->Add( m_pane_infogeneral, 1, wxEXPAND | wxALL, 5 );
	
	wxFlexGridSizer* fgSizerGeneral;
	fgSizerGeneral = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizerGeneral->SetFlexibleDirection( wxBOTH );
	
	m_static_maxstart = new wxStaticText( this, wxID_ANY, _("Maximum Running Job:"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizerGeneral->Add( m_static_maxstart, 0, wxALL, 5 );
	
	m_spin_maxstart = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 100, 0);
	m_spin_maxstart->SetValue(m_pcfg->GetMaxStart());

	fgSizerGeneral->Add( m_spin_maxstart, 0, wxALL, 5 );
	
	m_check_excludeseed = new wxCheckBox( this, wxID_ANY, _("Exclude seeding"), wxDefaultPosition, wxDefaultSize, 0 );

	m_check_excludeseed->SetValue(m_pcfg->GetExcludeSeed());

	fgSizerGeneral->Add( m_check_excludeseed, 0, wxALL, 5 );

#ifdef __WXMSW__
	m_check_runatstartup = new wxCheckBox( this, wxID_ANY, _("Run at Windows startup"), wxDefaultPosition, wxDefaultSize, 0 );

	m_check_runatstartup->SetValue(m_pcfg->GetRunAtStartup());
	
	fgSizerGeneral->Add( m_check_runatstartup, 0, wxALL, 5 );
#endif

//	fgSizerGeneral->AddSpacer( 5 );

	m_static_fastresume_save_time= new wxStaticText( this, wxID_ANY, _("Save Fastresume every (s):"), wxDefaultPosition, wxDefaultSize, 0 );
	
	fgSizerGeneral->Add( m_static_fastresume_save_time, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	m_spin_fastresume_save_time = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 86400, 0);
	m_spin_fastresume_save_time->SetValue(m_pcfg->GetFastResumeSaveTime());

	fgSizerGeneral->Add( m_spin_fastresume_save_time, 0, wxALL, 5 );
	
	fgSizerGeneral->AddSpacer( 5 );
	fgSizerMain->Add( fgSizerGeneral, 1, wxEXPAND, 5 );
	
	m_pane_infogui = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxSize( -1,20 ), wxTAB_TRAVERSAL );
	m_pane_infogui->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_ACTIVECAPTION ) );
	
	wxBoxSizer* bSizer11;
	bSizer11 = new wxBoxSizer( wxVERTICAL );
	
	m_static_infogui = new wxStaticText( m_pane_infogui, wxID_ANY, _("GUI"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer11->Add( m_static_infogui, 0, wxALL, 2 );
	
	m_pane_infogui->SetSizer( bSizer11 );
	m_pane_infogui->Layout();
	fgSizerMain->Add( m_pane_infogui, 1, wxEXPAND | wxALL, 5 );
	
	wxFlexGridSizer* fgSizerGUI;
	fgSizerGUI = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizerGUI->SetFlexibleDirection( wxBOTH );
	
	m_static_refreshtimer = new wxStaticText( this, wxID_ANY, _("Refresh Timer (s):"), wxDefaultPosition, wxDefaultSize, 0 );
	
	fgSizerGUI->Add( m_static_refreshtimer, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	m_spin_refreshtimer = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 3600, 0);
	m_spin_refreshtimer->SetValue(m_pcfg->GetRefreshTime());

	fgSizerGUI->Add( m_spin_refreshtimer, 0, wxALL, 5 );
	
	m_check_usesystray = new wxCheckBox( this, GENERAL_ID_USESYSTRAY, _("Enable Systray Icon"), wxDefaultPosition, wxDefaultSize, 0 );
	m_check_usesystray->SetValue(m_pcfg->GetUseSystray());
	
	fgSizerGUI->Add( m_check_usesystray, 0, wxALL, 5 );
	
	m_check_hidetaskbar = new wxCheckBox( this, wxID_ANY, _("Minimize to Systray"), wxDefaultPosition, wxDefaultSize, 0 );

	m_check_hidetaskbar->SetValue(m_pcfg->GetHideTaskbar());
	
	m_check_hidetaskbar->Enable(m_pcfg->GetUseSystray());
	fgSizerGUI->Add( m_check_hidetaskbar, 0, wxALL, 5 );
	
	fgSizerMain->Add( fgSizerGUI, 1, wxEXPAND, 5 );


	m_pane_infolog = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxSize( -1,20 ), wxTAB_TRAVERSAL );
	m_pane_infolog->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_ACTIVECAPTION ) );
	
	wxBoxSizer* bSizer2;
	bSizer2 = new wxBoxSizer( wxVERTICAL );
	
	m_static_infolog = new wxStaticText( m_pane_infolog, wxID_ANY, _("Log"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer2->Add( m_static_infolog, 0, wxALL, 2 );
	
	m_pane_infolog->SetSizer( bSizer2 );
	m_pane_infolog->Layout();
	fgSizerMain->Add( m_pane_infolog, 1, wxEXPAND | wxALL, 5 );
	
	wxFlexGridSizer* fgSizerLog;
	fgSizerLog = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizerLog->SetFlexibleDirection( wxBOTH );
	
	m_static_logseverity = new wxStaticText( this, wxID_ANY, _("Log Severity:"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizerLog->Add( m_static_logseverity, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxString m_choice_logseverityChoices[] = { _("Debug"), _("Info"), _("Warning"), _("Critical"), _("Fatal"), _("None") };
	int m_choice_logseverityNChoices = sizeof( m_choice_logseverityChoices ) / sizeof( wxString );
	m_choice_logseverity = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_choice_logseverityNChoices, m_choice_logseverityChoices, 0 );

	m_choice_logseverity->Select(m_pcfg->GetLogSeverity());

	fgSizerLog->Add( m_choice_logseverity, 0, wxALL, 5 );
	

	m_static_loglinecount = new wxStaticText( this, wxID_ANY, _("Maximum Lines:"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizerLog->Add( m_static_loglinecount, 0, wxALL, 5 );
	
	m_spin_loglinecount = new wxSpinCtrl( this, wxID_ANY, _("line"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 100, 10000, 0);
	m_spin_loglinecount->SetValue(m_pcfg->GetLogLineCount());
	fgSizerLog->Add( m_spin_loglinecount, 0, wxALL, 5 );

	m_check_logtofile= new wxCheckBox( this, GENERAL_ID_LOGTOFILE, _("Log to File:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_check_logtofile->SetValue(m_pcfg->GetLogFile());

	fgSizerLog->Add( m_check_logtofile, 0, wxALL, 5 );
	fgSizerLog->AddSpacer( 5 );

	fgSizerMain->Add( fgSizerLog, 1, wxEXPAND, 5 );
	
	this->SetSizer( fgSizerMain );
	this->Layout();
}

int GeneralSettingPane::GetMaxStart() 
{
	return m_spin_maxstart->GetValue();

}

int GeneralSettingPane::GetRefreshTimer() 
{
	return m_spin_refreshtimer->GetValue();

}

int GeneralSettingPane::GetFastResumeSaveTime() 
{
	return m_spin_fastresume_save_time->GetValue();

}

bool GeneralSettingPane::GetUseSystray() 
{
	return m_check_usesystray->GetValue();
}

bool GeneralSettingPane::GetExcludeSeed() 
{
	return m_check_excludeseed->GetValue();
}


bool GeneralSettingPane::GetHideTaskbar() 
{
	return m_check_hidetaskbar->GetValue();

}


void GeneralSettingPane::OnUseSystray(wxCommandEvent& event)
{
	bool usesystray = m_check_usesystray->GetValue();
	((MainFrame*)m_pMainFrame)->ShowSystray(usesystray);

	m_pcfg->SetUseSystray(usesystray);
	
	m_check_hidetaskbar->Enable(usesystray);

}

int GeneralSettingPane::GetLogSeverity() 
{
	return m_choice_logseverity->GetSelection();
}

int GeneralSettingPane::GetLogLineCount() 
{
	return m_spin_loglinecount->GetValue();
}

bool GeneralSettingPane::GetLogToFile() 
{
	return m_check_logtofile->GetValue();
}

