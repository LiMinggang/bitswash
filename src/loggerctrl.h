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
// Class: LoggerCtrl 
//
// Created by: lim k. c. <admin@bitswash.org>
// Created on: Mon Jul  2 14:08:15 MYT 2007
//
//
#ifndef _LOGGERCTRL_H_
#define _LOGGERCTRL_H_

#include <vector>

#include <wx/defs.h>
#include <wx/wx.h>

#include <wx/panel.h>
#include <wx/button.h>
#include <wx/tglbtn.h>
#include <wx/log.h>
#include <wx/textctrl.h>
#include <wx/sizer.h>
#include <wx/choice.h>
#include <wx/wfstream.h>


class LoggerCtrl: public wxPanel, wxLog
{
		
	public:
	
		LoggerCtrl(wxWindow *parent, wxLog* oldlog,
				const wxWindowID id = wxID_ANY,
				const wxPoint& pos = wxDefaultPosition,
				const wxSize& size = wxDefaultSize,
				long style = wxTAB_TRAVERSAL);

		~LoggerCtrl();

		wxTextCtrl* GetLoggerTextCtrl() { return m_log_text; }
		void ClearLog() {m_log_text->Clear();}
        int GetNumberOfLogLines() {return m_log_text->GetNumberOfLines();}
		void Update();

	private:
		wxFrame* m_pMainFrame;
		Configuration* m_pcfg;
		wxLog* m_oldlog;

		wxPanel* m_panelCtrl;
		wxStaticText* m_static_severity;
		wxChoice* m_choice_logseverity;
		wxCheckBox* m_checkLogFile;
		wxButton* m_button_clearlog;
		wxToggleButton* m_button_suspend;
		wxPanel* m_panelLog;
		wxTextCtrl* m_log_text;

		bool m_logtofile;

		struct log_data {
			wxLogLevel level;
			wxString msg;
		};

		bool m_issuspend;
		std::vector<struct log_data> m_suspended_logdata;
#if WXWIN_COMPATIBILITY_2_8
		virtual void DoLog(wxLogLevel level, const wxChar *szString, time_t t);

		virtual void DoLogString( const wxChar *szString, time_t WXUNUSED(t));
#else
		virtual void DoLogTextAtLevel(wxLogLevel level, const wxString& msg);
		
		virtual void DoLogText(const wxString& msg);
#endif
		void OnSize(wxSizeEvent& event);
		void OnSeverityChoice(wxCommandEvent& event);
		void OnLogFileCheck(wxCommandEvent& event);
		void OnClearLog(wxCommandEvent& event);
		void OnLogText(wxCommandEvent& event);
		void OnSuspend(wxCommandEvent& event);

		void RotateLog(int line);
		void PopSuspendedLog();
		void LogToFile(wxString msg);


	DECLARE_EVENT_TABLE()
};



#endif // _LOGGERCTRL_H_
