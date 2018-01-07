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
// Class:GeneralSettingPane
//
// Created by: lim k. c. <admin@bitswash.org>
// Created on: Tue Jun 26 13:23:51 MYT 2007
//

#ifndef _GENERALSETTING_H_
#define _GENERALSETTING_H_

//(*Headers(GeneralSettingPane)
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/statline.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
//*)

class Configuration;
class MainFrame;
class GeneralSettingPane :  public wxPanel
{
public:

	GeneralSettingPane(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize, int style = wxTAB_TRAVERSAL);
	virtual ~GeneralSettingPane();
	int GetMaxStart();
	int GetLogSeverity();
	int GetRefreshTimer();
	int GetFastResumeSaveTime();
	bool GetUseSystray();
	bool GetExcludeSeed();
	bool GetHideTaskbar();
	int GetLogLineCount();
	bool GetLogToFile(); 
	int GetMaxActiveSeeds();
	bool GetIgnoreSlowTorrents();
#ifdef __WXMSW__
	void SetRunAtStartup(bool start) { m_check_runatstartup->SetValue(start); }
	bool GetRunAtStartup() { return m_check_runatstartup->GetValue(); }
	void SetAssociateTorrent(bool associate) { m_check_associate_torrent->SetValue(associate); }
	bool GetAssociateTorrent() { return m_check_associate_torrent->GetValue(); }
	void SetAssociateMagnetURI(bool associate) { m_check_associate_magneturi->SetValue(associate); }
	bool GetAssociateMagnetURI() { return m_check_associate_magneturi->GetValue(); }
    static wxString m_startup_regkey;
#endif
private:
	MainFrame* m_pMainFrame;
	Configuration *m_pcfg;
	//(*Declarations(GeneralSettingPane)
#ifdef __WXMSW__
	wxCheckBox* m_check_associate_magneturi;
	wxCheckBox* m_check_associate_torrent;
		wxCheckBox* m_check_runatstartup;
#endif
	wxCheckBox* m_check_excludeseed;
	wxCheckBox* m_check_hidetaskbar;
	wxCheckBox* m_check_igore_slow_torrents;
	wxCheckBox* m_check_logtofile;
	wxCheckBox* m_check_usesystray;
	wxChoice* m_choice_logseverity;
	wxSpinCtrl* m_spin_fastresume_save_time;
	wxSpinCtrl* m_spin_loglinecount;
	wxSpinCtrl* m_spin_max_active_seeds;
	wxSpinCtrl* m_spin_maxstart;
	wxSpinCtrl* m_spin_refreshtimer;
	//*)

protected:

	//(*Identifiers(GeneralSettingPane)
	//*)

private:

	//(*Handlers(GeneralSettingPane)
	void OnUseSystray(wxCommandEvent& event);
	//*)

};
#endif	//_GENERALSETTING_H_

