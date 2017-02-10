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

#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/scrolwin.h>
#include <wx/statline.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>

#include "configuration.h"

#ifndef ID_GENERALSETTING_START
#define ID_GENERALSETTING_START wxID_HIGHEST
#endif

class GeneralSettingPane :  public wxPanel
{
	public:
		GeneralSettingPane( wxWindow* parent, int id = wxID_ANY, wxPoint pos = wxDefaultPosition, wxSize size = wxDefaultSize, int style = wxTAB_TRAVERSAL);
		int GetMaxStart();
		int GetLogSeverity();
		int GetRefreshTimer();
		int GetFastResumeSaveTime();
		bool GetUseSystray();
		bool GetExcludeSeed();
		bool GetHideTaskbar();
		int GetLogLineCount();
		bool GetLogToFile(); 
#ifdef __WXMSW__
		void SetStartWithSystem(bool start) { m_check_startwithsystem->SetValue(start); }
#endif
#ifdef __WXMSW__
        static wxString m_startup_regkey;
#endif
	private:
		wxFrame* m_pMainFrame;
		Configuration *m_pcfg;

		wxPanel* m_pane_infogeneral;
		wxStaticText* m_static_infogeneral;
		wxStaticText* m_static_maxstart;
		wxSpinCtrl* m_spin_maxstart;
		wxCheckBox* m_check_excludeseed;
#ifdef __WXMSW__
		wxCheckBox* m_check_startwithsystem;
#endif

		wxPanel* m_pane_infogui;
		wxStaticText* m_static_infogui;
		wxStaticText* m_static_refreshtimer;
		wxStaticText* m_static_fastresume_save_time;
		wxSpinCtrl* m_spin_refreshtimer;
		wxSpinCtrl* m_spin_fastresume_save_time;
		wxCheckBox* m_check_usesystray;
		wxCheckBox* m_check_hidetaskbar;

		wxPanel* m_pane_infolog;
		wxStaticText* m_static_infolog;
		wxStaticText* m_static_logseverity;
		wxChoice* m_choice_logseverity;
		wxCheckBox* m_check_logtofile;
		wxStaticText* m_static_loglinecount;
		wxSpinCtrl* m_spin_loglinecount;

		void OnUseSystray(wxCommandEvent& event);
	private:
		DECLARE_EVENT_TABLE()
	
};


#endif	//_GENERALSETTING_H_

