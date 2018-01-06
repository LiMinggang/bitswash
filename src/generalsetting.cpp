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
#include "configuration.h"
#include "generalsetting.h"
#include "functions.h"

//(*IdInit(GeneralSettingPane)
//*)
GeneralSettingPane::GeneralSettingPane(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size, int style)
{
	//(*Initialize(GeneralSettingPane)
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
	wxStaticText* StaticText4;
	wxStaticText* StaticText5;
	wxStaticText* StaticText6;
	wxStaticText* StaticText7;
	wxStaticText* StaticText8;
	wxStaticText* StaticText9;
	m_pMainFrame = dynamic_cast<MainFrame *>( wxGetApp().GetTopWindow() );
	wxASSERT(m_pMainFrame != nullptr);
	m_pcfg = m_pMainFrame->GetConfig();

	Create(parent, id, wxDefaultPosition, wxDefaultSize, style, _T("id"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	Panel1 = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	Panel1->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	BoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
	StaticText1 = new wxStaticText(Panel1, wxID_ANY, _("General"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	BoxSizer1->Add(StaticText1, 1, wxALL|wxEXPAND, 5);
	Panel1->SetSizer(BoxSizer1);
	BoxSizer1->Fit(Panel1);
	BoxSizer1->SetSizeHints(Panel1);
	FlexGridSizer1->Add(Panel1, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	StaticText2 = new wxStaticText(this, wxID_ANY, _("Maximum Running Job:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer2->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	m_spin_maxstart = new wxSpinCtrl(this, wxID_ANY, _T("1"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 100, 1, _T("wxID_ANY"));
	m_spin_maxstart->SetValue(_T("1"));
	FlexGridSizer2->Add(m_spin_maxstart, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	StaticText3 = new wxStaticText(this, wxID_ANY, _("Maximum Active Seeds:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer2->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	m_spin_max_active_seeds = new wxSpinCtrl(this, wxID_ANY, _T("-1"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -1, 100, -1, _T("wxID_ANY"));
	m_spin_max_active_seeds->SetValue(m_pcfg->GetMaxActiveSeeds());
	FlexGridSizer2->Add(m_spin_max_active_seeds, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	m_check_excludeseed = new wxCheckBox( this, wxID_ANY, _("Exclude seeding"), wxDefaultPosition, wxDefaultSize, 0 );
	m_check_excludeseed->SetValue(m_pcfg->GetExcludeSeed());
	FlexGridSizer2->Add(m_check_excludeseed, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	m_check_igore_slow_torrents = new wxCheckBox(this, wxID_ANY, _("Don\'t Count Slow Torrents"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("wxID_ANY"));
	m_check_igore_slow_torrents->SetValue(m_pcfg->GetIgnoreSlowTorrents());
	FlexGridSizer2->Add(m_check_igore_slow_torrents, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	StaticText4 = new wxStaticText(this, wxID_ANY, _("Save Fastresume every (s):"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer2->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	m_spin_fastresume_save_time = new wxSpinCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 86400, 0, _T("wxID_ANY"));
	m_spin_fastresume_save_time->SetValue(m_pcfg->GetFastResumeSaveTime());
	FlexGridSizer2->Add(m_spin_fastresume_save_time, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
#ifdef __WXMSW__
	m_check_associate_torrent = new wxCheckBox(this, wxID_ANY, _("Associate .torrent with Bitswash"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("wxID_ANY"));
	m_check_associate_torrent->SetValue(m_pcfg->GetAssociateTorrent());
	FlexGridSizer2->Add(m_check_associate_torrent, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	m_check_associate_magneturi = new wxCheckBox(this, wxID_ANY, _("Associate Magnet URI with Bitswash"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("wxID_ANY"));
	m_check_associate_magneturi->SetValue(m_pcfg->GetAssociateMagnetURI());
	FlexGridSizer2->Add(m_check_associate_magneturi, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	m_check_runatstartup = new wxCheckBox(this, wxID_ANY, _("Run at Windows startup"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("wxID_ANY"));
	m_check_runatstartup->SetValue(m_pcfg->GetRunAtStartup());
	FlexGridSizer2->Add(m_check_runatstartup, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
#endif
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 5);
	Panel2 = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	Panel2->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	BoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
	StaticText5 = new wxStaticText(Panel2, wxID_ANY, _("GUI"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	BoxSizer2->Add(StaticText5, 1, wxALL|wxALIGN_CENTER_VERTICAL, 5);
	Panel2->SetSizer(BoxSizer2);
	BoxSizer2->Fit(Panel2);
	BoxSizer2->SetSizeHints(Panel2);
	FlexGridSizer1->Add(Panel2, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
	StaticText6 = new wxStaticText(this, wxID_ANY, _("Refresh Timer (s):"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer3->Add(StaticText6, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	m_spin_refreshtimer = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 3600, 0, _T("wxID_ANY"));
	m_spin_refreshtimer->SetValue(m_pcfg->GetRefreshTime());
	FlexGridSizer3->Add(m_spin_refreshtimer, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	m_check_usesystray = new wxCheckBox(this, wxID_ANY, _("Enable Systray Icon"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("wxID_ANY"));
	m_check_usesystray->SetValue(m_pcfg->GetUseSystray());
	FlexGridSizer3->Add(m_check_usesystray, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	m_check_hidetaskbar = new wxCheckBox(this, wxID_ANY, _("Minimize to Systray"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("wxID_ANY"));
	m_check_hidetaskbar->SetValue(m_pcfg->GetHideTaskbar());
	m_check_hidetaskbar->Enable(m_pcfg->GetUseSystray());
	FlexGridSizer3->Add(m_check_hidetaskbar, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer3, 1, wxALL|wxEXPAND, 5);
	Panel3 = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(100,100), wxTAB_TRAVERSAL, _T("wxID_ANY"));
	Panel3->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	BoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
	StaticText7 = new wxStaticText(Panel3, wxID_ANY, _("Log"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	BoxSizer3->Add(StaticText7, 1, wxALL|wxALIGN_CENTER_VERTICAL, 5);
	Panel3->SetSizer(BoxSizer3);
	BoxSizer3->Fit(Panel3);
	BoxSizer3->SetSizeHints(Panel3);

	FlexGridSizer1->Add(Panel3, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer4 = new wxFlexGridSizer(0, 2, 0, 0);
	StaticText8 = new wxStaticText(this, wxID_ANY, _("Log Severity:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer4->Add(StaticText8, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	wxString m_choice_logseverityChoices[] = { _("Debug"), _("Info"), _("Warning"), _("Critical"), _("Fatal"), _("None") };
	int m_choice_logseverityNChoices = sizeof( m_choice_logseverityChoices ) / sizeof( wxString );
	m_choice_logseverity = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_choice_logseverityNChoices, m_choice_logseverityChoices, 0 );

	m_choice_logseverity->Select(m_pcfg->GetLogSeverity());

	FlexGridSizer4->Add(m_choice_logseverity, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	StaticText9 = new wxStaticText(this, wxID_ANY, _("Maximum Lines:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer4->Add(StaticText9, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	m_spin_loglinecount = new wxSpinCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 100, 10000, 0, _T("wxID_ANY"));
	m_spin_loglinecount->SetValue(m_pcfg->GetLogLineCount());
	FlexGridSizer4->Add(m_spin_loglinecount, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	m_check_logtofile = new wxCheckBox(this, wxID_ANY, _("Log to File"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("wxID_ANY"));
	m_check_logtofile->SetValue(m_pcfg->GetLogFile());
	FlexGridSizer4->Add(m_check_logtofile, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);
	//*)
	Bind(wxEVT_CHECKBOX, &GeneralSettingPane::OnUseSystray, this, m_check_usesystray->GetId() );
}

GeneralSettingPane::~GeneralSettingPane()
{
	//(*Destroy(GeneralSettingPane)
	//*)
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
	m_pMainFrame->ShowSystray(usesystray);

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

int GeneralSettingPane::GetMaxActiveSeeds()
{
	return m_spin_max_active_seeds->GetValue();
}

bool GeneralSettingPane::GetIgnoreSlowTorrents()
{
	return m_check_igore_slow_torrents->GetValue();
}

