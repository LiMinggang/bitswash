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
#include <wx/datetime.h>
#include <wx/filename.h>
#include <wx/txtstrm.h>
#include <wx/wfstream.h>
#include "mainframe.h"
#include "loggerctrl.h"

enum
{
	LOGGER_CTRL_CHOICE_SEVERITY  = wxID_HIGHEST,
	LOGGER_CTRL_CHECK_LOGFILE,
	LOGGER_CTRL_BUTTON_CLEARLOG,
	LOGGER_CTRL_BUTTON_SUSPEND,
	LOGGER_CTRL_TEXTAREA
};

BEGIN_EVENT_TABLE( LoggerCtrl, wxPanel )
	EVT_CHOICE( LOGGER_CTRL_CHOICE_SEVERITY, LoggerCtrl::OnSeverityChoice )
	EVT_CHECKBOX( LOGGER_CTRL_CHECK_LOGFILE, LoggerCtrl::OnLogFileCheck )
	EVT_BUTTON( LOGGER_CTRL_BUTTON_CLEARLOG, LoggerCtrl::OnClearLog )
	EVT_TOGGLEBUTTON( LOGGER_CTRL_BUTTON_SUSPEND, LoggerCtrl::OnSuspend )
	EVT_SIZE( LoggerCtrl::OnSize )
	EVT_TEXT( LOGGER_CTRL_TEXTAREA, LoggerCtrl::OnLogText )
END_EVENT_TABLE()

LoggerCtrl::LoggerCtrl( wxWindow *parent, wxLog* oldlog,
						const wxWindowID id,
						const wxPoint& pos,
						const wxSize& size,
						long style )
	: wxPanel( parent, id, pos, size, style ),
	  m_issuspend( 0 )
{
	int w, h;
	GetClientSize( &w, &h );
	m_oldlog = oldlog;
	m_pMainFrame = ( wxFrame* ) wxGetApp().GetTopWindow();
	m_pcfg = ( ( MainFrame* )m_pMainFrame )->GetConfig();
	m_logtofile = m_pcfg->GetLogFile();
	wxFlexGridSizer* fgSizerMain;
	fgSizerMain = new wxFlexGridSizer( 2, 1, 0, 0 );
	fgSizerMain->SetFlexibleDirection( wxBOTH );
	fgSizerMain->AddGrowableRow( 1 );
	m_panelCtrl = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxFlexGridSizer* fgSizerCtrl;
	fgSizerCtrl = new wxFlexGridSizer( 1, 5, 0, 0 );
	fgSizerCtrl->AddGrowableCol( 1 );
	fgSizerCtrl->AddGrowableCol( 2 );
	fgSizerCtrl->SetFlexibleDirection( wxBOTH );
	//fgSizerCtrl->SetMinSize(300,30);
	m_static_severity = new wxStaticText( m_panelCtrl, wxID_ANY, _( "Severity" ), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizerCtrl->Add( m_static_severity, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5 );
	wxString m_choice_logseverityChoices[] = { _( "Debug" ), _( "Info" ), _( "Warning" ), _( "Critical" ), _( "Fatal" ), _( "None" ) };
	int m_choice_logseverityNChoices = sizeof( m_choice_logseverityChoices ) / sizeof( wxString );
	m_choice_logseverity = new wxChoice( m_panelCtrl, LOGGER_CTRL_CHOICE_SEVERITY, wxDefaultPosition, wxDefaultSize, m_choice_logseverityNChoices, m_choice_logseverityChoices, 0 );
	m_choice_logseverity->Select( m_pcfg->GetLogSeverity() );
	fgSizerCtrl->Add( m_choice_logseverity, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5 );
	m_checkLogFile = new wxCheckBox( m_panelCtrl, LOGGER_CTRL_CHECK_LOGFILE, _( "Log to File" ), wxDefaultPosition, wxDefaultSize, 0 );
	m_checkLogFile->SetValue( m_logtofile );
	fgSizerCtrl->Add( m_checkLogFile, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5 );
	m_button_clearlog = new wxButton( m_panelCtrl, LOGGER_CTRL_BUTTON_CLEARLOG, _( "Clear Log" ), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizerCtrl->Add( m_button_clearlog, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5 );
	m_button_suspend = new wxToggleButton( m_panelCtrl, LOGGER_CTRL_BUTTON_SUSPEND, _( "Pause Log" ), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizerCtrl->Add( m_button_suspend, 0, wxALL | wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT, 5 );
	m_panelCtrl->SetSizer( fgSizerCtrl );
	m_panelCtrl->Layout();
	fgSizerCtrl->Fit( m_panelCtrl );
	fgSizerMain->Add( m_panelCtrl, 1, wxEXPAND | wxALL, 5 );
	m_panelLog = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxFlexGridSizer* fgSizerLog;
	fgSizerLog = new wxFlexGridSizer( 1, 3, 0, 0 );
	fgSizerLog->AddGrowableCol( 2 );
	fgSizerLog->SetFlexibleDirection( wxBOTH );
	m_log_text = new wxTextCtrl( m_panelLog, LOGGER_CTRL_TEXTAREA, wxEmptyString, wxDefaultPosition, wxSize( 600, 200 ), wxTE_MULTILINE | wxTE_READONLY | wxTE_DONTWRAP );
	fgSizerLog->Add( m_log_text, 0, wxALL, 5 );
	m_panelLog->SetSizer( fgSizerLog );
	m_panelLog->Layout();
	fgSizerCtrl->Fit( m_panelLog );
	fgSizerMain->Add( m_panelLog, 0, wxEXPAND | wxALL, 5 );
	this->SetSizer( fgSizerMain );
	this->Layout();
	fgSizerMain->Fit( this );
}

LoggerCtrl::~LoggerCtrl()
{
	wxASSERT( m_oldlog );
	wxLog::SetActiveTarget( m_oldlog );
}

void LoggerCtrl::OnSize( wxSizeEvent& event )
{
	int panelheight;
	int ctrlheight;
	int p_width = 0;
	int p_height = 0;
	m_panelCtrl->Layout();
	m_panelCtrl->GetSizer()->Fit( m_panelCtrl );
	panelheight = GetClientSize().GetHeight();
	ctrlheight = m_panelCtrl->GetSize().GetHeight();
	p_width = GetClientSize().GetWidth();
	p_height = GetClientSize().GetHeight();
	m_panelLog->SetSize( wxSize( p_width, p_height - ctrlheight ) );
	m_panelLog->Layout();
	m_panelLog->SetPosition( wxPoint( 0, ctrlheight ) );
	m_log_text->SetSize( wxSize( m_panelLog->GetSize().GetWidth() - 5, m_panelLog->GetSize().GetHeight() - 5 ) );
}

void LoggerCtrl::OnSeverityChoice( wxCommandEvent& event )
{
	wxLogDebug( _T( "Severity update %d\n" ), m_choice_logseverity->GetCurrentSelection() );
	m_pcfg->SetLogSeverity( m_choice_logseverity->GetCurrentSelection() );
	( ( MainFrame* )m_pMainFrame )->SetLogSeverity();
}

void LoggerCtrl::OnLogFileCheck( wxCommandEvent& event )
{
	wxLogDebug( _T( "OnLogFileCheck\n" ) );
	bool logtofile = m_checkLogFile->GetValue();
	m_pcfg->SetLogFile( logtofile );
	if( logtofile )
	{
		wxLogDebug( _T( "LogToFile\n" ) );
	}
	else
	{
		wxLogDebug( _T( "Not LogToFile\n" ) );
	}
	m_logtofile = logtofile;
}

void LoggerCtrl::OnClearLog( wxCommandEvent& event )
{
	wxLogDebug( _T( "Clear Log\n" ) );
	m_log_text->Clear();
}

void LoggerCtrl::OnSuspend( wxCommandEvent& event )
{
	wxLogDebug( _T( "Suspend Log\n" ) );
	if( m_button_suspend->GetValue() )
		m_issuspend = true;
	else
	{
		m_issuspend = false;
		PopSuspendedLog();
	}
}

void LoggerCtrl::OnLogText( wxCommandEvent& event )
{
}

#if WXWIN_COMPATIBILITY_2_8
void LoggerCtrl::DoLog( wxLogLevel level, const wxChar *szString, time_t t )
{
#if 0
	wxString st( szString );
	std::cout << t << ":" << __func__
			  << " level " << level
			  << ": str " << st.c_str() << std::endl;
#endif
	if( m_issuspend )
	{
		wxString msg;
		TimeStamp( &msg );
		msg += szString;
		struct log_data logdata;
		logdata.level = level;
		logdata.msg = msg;
		m_suspended_logdata.push_back( logdata );
	}
	else
	{
		if( level <= wxLOG_Error )
			m_log_text->SetDefaultStyle( wxTextAttr( *wxRED ) );
		else
			if( level <= wxLOG_Warning )
				m_log_text->SetDefaultStyle( wxTextAttr( *wxBLUE ) );
			else
				m_log_text->SetDefaultStyle( wxTextAttr( *wxBLACK ) );
		RotateLog( 1 );
		wxLog::DoLogTextAtLevel( level, szString/*, t*/ );
	}
}

void LoggerCtrl::DoLogString( const wxChar *szString, time_t WXUNUSED( t ) )
{
#if 0
	wxString st( szString );
	std::cout << __func__
			  << ": str " << st.c_str() << std::endl;
#endif
	wxString msg;
	TimeStamp( &msg );
	msg += szString;
	//Use WriteText will prevent scrolling
	m_log_text->AppendText( ( msg ) );
	if( m_logtofile )
	{
		LogToFile( msg );
	}
}
#else
void LoggerCtrl::DoLogTextAtLevel( wxLogLevel level, const wxString& msg )
{
	if( m_issuspend )
	{
		struct log_data logdata;
		logdata.level = level;
		logdata.msg = msg;
		m_suspended_logdata.push_back( logdata );
	}
	else
	{
		if( level <= wxLOG_Error )
			m_log_text->SetDefaultStyle( wxTextAttr( *wxRED ) );
		else
			if( level <= wxLOG_Warning )
				m_log_text->SetDefaultStyle( wxTextAttr( *wxBLUE ) );
			else
				m_log_text->SetDefaultStyle( wxTextAttr( *wxBLACK ) );
		RotateLog( 1 );
		wxLog::DoLogTextAtLevel( level, msg/*, t*/ );
	}
}

void LoggerCtrl::DoLogText( const wxString& msg )
{
	//Use WriteText will prevent scrolling
	m_log_text->AppendText(msg);
	if( m_logtofile )
	{
		LogToFile(msg);
	}
}
#endif
void LoggerCtrl::RotateLog( int line )
{
	wxASSERT( ( size_t )line < m_pcfg->GetLogLineCount() );
	size_t n = m_log_text->GetNumberOfLines() + ( size_t )line;
	if( n > m_pcfg->GetLogLineCount() )
	{
		int l = m_log_text->XYToPosition( 0, line );
		m_log_text->Remove( 0, l );
	}
}

void LoggerCtrl::PopSuspendedLog()
{
	std::vector<struct log_data>::const_iterator logdata_i;
	if( m_issuspend || m_suspended_logdata.size() <= 0 )
		return;
	RotateLog( m_suspended_logdata.size() );
	for( logdata_i = m_suspended_logdata.begin();
			logdata_i != m_suspended_logdata.end();
			++logdata_i )
	{
		wxLogLevel level = logdata_i->level;
		if( level <= wxLOG_Error )
		{
			m_log_text->SetDefaultStyle( wxTextAttr( *wxRED ) );
		}
		else
			if( level <= wxLOG_Warning )
			{
				m_log_text->SetDefaultStyle( wxTextAttr( *wxBLUE ) );
			}
			else
			{
				m_log_text->SetDefaultStyle( wxTextAttr( *wxBLACK ) );
			}
		m_log_text->AppendText( logdata_i->msg );
		if( m_logtofile )
		{
			LogToFile( logdata_i->msg );
		}
	}
	m_suspended_logdata.clear();
	m_suspended_logdata.swap( m_suspended_logdata );
}


void LoggerCtrl::LogToFile( wxString msg )
{
	wxString logpath = wxGetApp().LogPath();
	wxString now = wxDateTime::Now().Format( _T( "%Y%m%d" ) );
	wxChar dirsep = wxFileName::GetPathSeparator( wxPATH_NATIVE );
	wxString log_filename = logpath + dirsep + now + _T( ".log" );
	wxFFileOutputStream log_file( log_filename, _T( "a+" ) );
	if( log_file.IsOk() )
	{
		//log_file.SeekO(0, wxFromEnd);
		wxTextOutputStream out_file( log_file );
		out_file << msg;
	}
	else
	{
		std::cerr << "Unable to open log file " << log_filename.ToAscii() << std::endl;
	}
}

void LoggerCtrl::Update()
{
	m_checkLogFile->SetValue( m_pcfg->GetLogFile() );
	m_choice_logseverity->Select( m_pcfg->GetLogSeverity() );
}

