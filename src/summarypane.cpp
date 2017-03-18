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
// Class: SummaryPane
//
// Created by: lim k. c. <admin@bitswash.org>
// Created on: Sat Feb  3 17:02:38 2007
//


#include <wx/filename.h>
#include <wx/sstream.h>

#include <wx/stattext.h>
#include <wx/textctrl.h>

#include <wx/log.h>

#include <boost/date_time/posix_time/posix_time.hpp>

#ifdef __WXMSW__
#include <wx/msw/winundef.h>
#endif

#include "mainframe.h"
#include "torrentinfo.h"
#include "summarypane.h"

#include "functions.h"

namespace lt = libtorrent;

BEGIN_EVENT_TABLE(SummaryPane, wxScrolledWindow)
	//EVT_SET_FOCUS(SummaryPane::OnFocus)
END_EVENT_TABLE()

SummaryPane::SummaryPane(wxWindow *parent,
                const wxWindowID id,
               const wxPoint& pos,
               const wxSize& size,
               long style): wxScrolledWindow(parent, id, pos, size,  wxVSCROLL /* | wxFULL_REPAINT_ON_RESIZE */)
{

	m_pMainFrame = ((TorrentInfo*)parent)->GetMainFrame();

	int w,h;

	wxFlexGridSizer* m_summarypane_sizer;

	//SetMinSize(wxSize(500,100));
	GetClientSize(&w, &h);
	
	m_summarypane_sizer = new wxFlexGridSizer( 0, 1, 3, 0 );
	m_summarypane_sizer->SetFlexibleDirection( wxBOTH );
	m_summarypane_sizer->SetDimension(0, 0, w-50, -1);
	m_summarypane_sizer->SetMinSize(500, -1);

	
	m_pane_info = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	m_pane_info->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_ACTIVECAPTION) );
	//m_pane_info->SetSize(0, 0, w-50, 30, wxSIZE_AUTO);
	m_pane_info->SetMaxSize( wxSize( -1,20 ) );
	
	wxBoxSizer* m_pane_boxsizer;
	m_pane_boxsizer = new wxBoxSizer( wxVERTICAL );
	//m_pane_boxsizer->SetDimension(0,0,w-50, -1);
	
	m_pane_label_info = new wxStaticText( m_pane_info, wxID_ANY, _("Info"), wxDefaultPosition, wxDefaultSize, 0 );
	m_pane_boxsizer->Add( m_pane_label_info, 0, wxALL, 2 );
	
	m_pane_info->SetSizer( m_pane_boxsizer );
	m_pane_info->Layout();
	m_summarypane_sizer->Add( m_pane_info, 1, wxEXPAND | wxALL, 3 );

	
	m_label_saveas = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize,0 );
	//m_label_saveas->SetMinSize (wxSize(300, -1));
	//m_label_saveas->Wrap(w-50);
	
	m_summarypane_sizer->Add( m_label_saveas, 0, wxALL, 3 );
	
	wxFlexGridSizer* m_status_info_sizer;
	m_status_info_sizer = new wxFlexGridSizer( 0, 3, 3, 0 );
	m_status_info_sizer->AddGrowableCol( 0 );
	m_status_info_sizer->AddGrowableCol( 1 );
	m_status_info_sizer->AddGrowableCol( 2 );
	m_status_info_sizer->SetFlexibleDirection( wxBOTH);
	m_status_info_sizer->SetMinSize(wxSize(500, -1));
	
	m_label_size = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_status_info_sizer->Add( m_label_size, 0, wxALL, 3 );
	
	m_label_pieces = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_status_info_sizer->Add( m_label_pieces, 0, wxALL, 3 );
	
	m_status_info_sizer->Add( 0, 0, 1, wxEXPAND, 0 );
	
	m_label_peers = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_status_info_sizer->Add( m_label_peers, 0, wxALL, 3 );
	
	m_label_seeds = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_status_info_sizer->Add( m_label_seeds, 0, wxALL, 3 );
	
	m_status_info_sizer->Add( 0, 0, 1, wxEXPAND, 0 );
	
	m_label_downspeed = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_status_info_sizer->Add( m_label_downspeed, 0, wxALL, 3 );
	
	m_label_upspeed = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_status_info_sizer->Add( m_label_upspeed, 0, wxALL, 3 );
	
	m_status_info_sizer->Add( 0, 0, 1, wxEXPAND, 0 );
	
	m_label_downlimit = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_status_info_sizer->Add( m_label_downlimit, 0, wxALL, 3 );
	
	m_label_uplimit = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_status_info_sizer->Add( m_label_uplimit, 0, wxALL, 3 );
	
	m_status_info_sizer->Add( 0, 0, 1, wxEXPAND, 0 );
	
	m_label_downloaded = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_status_info_sizer->Add( m_label_downloaded, 0, wxALL, 3 );
	
	m_label_uploaded = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_status_info_sizer->Add( m_label_uploaded, 0, wxALL, 3 );
	
	m_label_ratio = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_status_info_sizer->Add( m_label_ratio, 0, wxALL, 3 );
	
	
	m_summarypane_sizer->Add( m_status_info_sizer, 1, wxEXPAND, 3 );

	m_label_hash = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_summarypane_sizer->Add( m_label_hash, 0, wxALL, 3 );
	
	m_label_comment_static = new wxStaticText( this, wxID_ANY, _("Comment:"), wxDefaultPosition, wxDefaultSize, 0 );

	m_summarypane_sizer->Add( m_label_comment_static, 0, wxALL, 3 );
	
	m_text_comment = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(w-50, 80), wxTE_AUTO_URL | wxTE_BESTWRAP | wxTE_MULTILINE | wxTE_READONLY);
	//m_label_comment = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	//m_label_comment->SetMinSize( wxSize(-1, 50));
	//m_label_comment->Wrap(w-50);

	m_summarypane_sizer->Add( m_text_comment, 0, wxALL, 3 );

	m_pane_tracker = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	m_pane_tracker->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_ACTIVECAPTION) );
	m_pane_tracker->SetMaxSize( wxSize( -1,20 ) );
	
	wxBoxSizer* m_pane_boxsizer1;
	m_pane_boxsizer1 = new wxBoxSizer( wxVERTICAL );
	
	m_pane_label_tracker = new wxStaticText( m_pane_tracker, wxID_ANY, _("Tracker"), wxDefaultPosition, wxDefaultSize, 0 );
	m_pane_boxsizer1->Add( m_pane_label_tracker, 0, wxALL, 2 );
	
	m_pane_tracker->SetSizer( m_pane_boxsizer1 );
	m_pane_tracker->Layout();
	m_summarypane_sizer->Add( m_pane_tracker, 1, wxEXPAND | wxALL, 3 );
	
	m_label_tracker_url = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );

	m_summarypane_sizer->Add( m_label_tracker_url, 0, wxALL, 3 );

	wxFlexGridSizer* m_status_tracker_sizer;
	m_status_tracker_sizer = new wxFlexGridSizer( 1, 3, 3, 0 );
	m_status_tracker_sizer->AddGrowableCol( 0 );
	m_status_tracker_sizer->AddGrowableCol( 1 );
	m_status_tracker_sizer->AddGrowableCol( 2 );
	m_status_tracker_sizer->SetFlexibleDirection( wxBOTH);
	
	
	m_label_trackerstatus = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_status_tracker_sizer->Add( m_label_trackerstatus, 0, wxALL, 3 );
	
	m_label_nextupdate = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_status_tracker_sizer->Add( m_label_nextupdate, 0, wxALL, 3 );
	
	m_status_tracker_sizer->Add( 0, 0, 1, wxEXPAND, 0 );
	
	
	m_summarypane_sizer->Add( m_status_tracker_sizer, 1, wxEXPAND, 3 );

	m_label_dht = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_summarypane_sizer->Add( m_label_dht, 0, wxALL, 3 );
	
	
	this->SetSizer( m_summarypane_sizer );
	this->Layout();

	m_summarypane_sizer->FitInside(this);

	ResetSummary();

	SetScrollbars(20,20,5,5);
}


SummaryPane::~SummaryPane()
{
		/* //delete all pointer?

	delete m_status_info_sizer;
	delete m_status_chunk_sizer;
	delete m_statusbox;
	*/
}

void SummaryPane::UpdateSummary()
{
	wxLogDebug(_T("UpdateSummary\n"));
	shared_ptr<torrent_t> pTorrent = ((MainFrame*)m_pMainFrame)->GetSelectedTorrent();

	if ( !pTorrent )
	{
		ResetSummary();
		//wxLogInfo(_T("No torrent selected\n"));
		return;
	}

	lt::torrent_handle h = pTorrent->handle;

	shared_ptr<const lt::torrent_info> t = pTorrent->info;
	lt::torrent_status s;

	if(h.is_valid()) 
		s = h.status();

	if(t)
	{
		wxString t_name = wxString(wxConvUTF8.cMB2WC(t->name().c_str()));
		wxLogDebug(_T("UpdateSummary - Update torrent %s\n"), t_name.c_str());

		//if(h.is_valid()) UpdateSaveAs(wxString::FromAscii( pTorrent->handle.save_path().string().c_str()));
		UpdateSaveAs(pTorrent->config->GetDownloadPath() + t_name);

		UpdateSize(HumanReadableByte((wxDouble) t->total_size()));
		if(!s.has_metadata)
			UpdatePieces(wxString::Format(_T("%d x %s"), 0, HumanReadableByte(0).c_str()));
		else
			UpdatePieces(wxString::Format(_T("%d x %s"), t->num_pieces(), HumanReadableByte(t->piece_length()).c_str()));

		UpdateHash(wxString(InfoHash(t->info_hash())));
	}
	//
	
	UpdatePeers(h.is_valid()?wxString::Format(_T("%d"), s.num_peers):_T("0"));
	UpdateSeeds(h.is_valid()?wxString::Format(_T("%d"), s.num_seeds):_T("0"));

	UpdateDownSpeed(h.is_valid()?HumanReadableByte(s.download_rate) + wxString(_T("ps")):_T("0bps"));
	UpdateUpSpeed(h.is_valid()?HumanReadableByte(s.upload_rate) + wxString(_T("ps")):_T("0bps"));

	UpdateDownLimit(h.is_valid()?HumanReadableByte( h.download_limit()) + _T("ps"):_T("0bps"));
	UpdateUpLimit(h.is_valid()?HumanReadableByte(h.upload_limit()) + _T("ps"):_T("0bps"));

	UpdateDownloaded(h.is_valid()?HumanReadableByte(s.total_payload_download):_T("0b"));
	UpdateUploaded(h.is_valid()?HumanReadableByte( s.total_payload_upload):_T("0b"));
	//TODO: Calculate ratio

	if (h.is_valid())
	{
		if(s.total_payload_download >0)
			UpdateRatio(wxString::Format(_T("%.02f"), (wxDouble) s.total_payload_upload/ (wxDouble)s.total_payload_download));
		else
			UpdateRatio(_T("inf"));
	}

	if(t)
		UpdateComment(wxString(wxConvUTF8.cMB2WC(t->comment().c_str())));

	UpdateTrackerUrl(h.is_valid()?wxString::FromUTF8(s.current_tracker.c_str()):_T(""));
	//
	//TODO: findout tracker status
	//UpdateTrackerStatus();
	//
	UpdateNextUpdate(h.is_valid()?wxString::Format(_T("%s"), wxLongLong(s.next_announce.count() ).ToString().c_str()):_T(""));

	// TODO: DHT status?
	//UpdateDht();

	wxFlexGridSizer *m_summarypane_sizer = (wxFlexGridSizer *)GetSizer();
	//this->SetClientSize(m_summarypane_sizer->GetSize());
	//
	m_summarypane_sizer->FitInside(this);

	//SetScrollbars(20,20,5,5);

}

void SummaryPane::OnFocus(wxFocusEvent& event)
{
	wxLogDebug(_T("OnFocus\n"));
	//UpdateSummary();
	//this->Layout();
	//event.Skip();

}

void SummaryPane::UpdateSaveAs(wxString s) 
{
	m_label_saveas->SetLabel(wxString::Format(_("Save As: %s"), s.c_str()));

}

void SummaryPane::UpdateSize(wxString s) 
{
	m_label_size->SetLabel(wxString::Format(_("Size: %s"), s.c_str()));

}

void SummaryPane::UpdatePieces(wxString s) 
{
	m_label_pieces->SetLabel(wxString::Format(_("Pieces: %s"), s.c_str()));

}

void SummaryPane::UpdateHash(wxString s) 
{
	m_label_hash->SetLabel(wxString::Format(_("Hash: %s"), s.c_str()));

}

void SummaryPane::UpdatePeers(wxString s) 
{
	m_label_peers->SetLabel(wxString::Format(_("Peers: %s"), s.c_str()));

}

void SummaryPane::UpdateSeeds(wxString s) 
{
	m_label_seeds->SetLabel(wxString::Format(_("Seeds: %s"), s.c_str()));

}

void SummaryPane::UpdateDownSpeed(wxString s) 
{
	m_label_downspeed->SetLabel(wxString::Format(_("Download Speed: %s"), s.c_str()));

}

void SummaryPane::UpdateUpSpeed(wxString s) 
{
	m_label_upspeed->SetLabel(wxString::Format(_("Upload Speed: %s"), s.c_str()));

}

void SummaryPane::UpdateDownLimit(wxString s) 
{
	m_label_downlimit->SetLabel(wxString::Format(_("Download Rate Limit: %s"), s.c_str()));

}

void SummaryPane::UpdateUpLimit(wxString s) 
{
	m_label_uplimit->SetLabel(wxString::Format(_("Upload Rate Limit: %s"), s.c_str()));

}

void SummaryPane::UpdateDownloaded(wxString s) 
{
	m_label_downloaded->SetLabel(wxString::Format(_("Downloaded: %s"), s.c_str()));

}

void SummaryPane::UpdateUploaded(wxString s) 
{
	m_label_uploaded->SetLabel(wxString::Format(_("Uploaded: %s"), s.c_str()));

}

void SummaryPane::UpdateRatio(wxString s) 
{
	m_label_ratio->SetLabel(wxString::Format(_("Share Ratio: %s"), s.c_str()));

}

void SummaryPane::UpdateComment(wxString s) 
{
	//TODO: convert html tags
	//m_label_comment->SetLabel(wxString::Format(_T("%s"), s.c_str()));
//	m_text_comment->SetLabel(wxString::Format(_T("%s"), s.c_str()));
	m_text_comment->SetValue(wxString::Format(_("%s"), s.c_str()));

}

void SummaryPane::UpdateTrackerUrl(wxString s) 
{
	m_label_tracker_url->SetLabel(wxString::Format(_("Tracker URL: %s"), s.c_str()));

}

void SummaryPane::UpdateTrackerStatus(wxString s) 
{
	m_label_trackerstatus->SetLabel(wxString::Format(_("Status: %s"), s.c_str()));

}

void SummaryPane::UpdateNextUpdate(wxString s) 
{
	m_label_nextupdate->SetLabel(wxString::Format(_("Next Update: %s"), s.c_str()));

}

void SummaryPane::UpdateDht(wxString s) 
{
	m_label_dht->SetLabel(wxString::Format(_("DHT Status: %s"), s.c_str()));

}

void SummaryPane::ResetSummary()
{

	UpdateSaveAs(wxEmptyString);

	UpdateSize(wxEmptyString);

	UpdatePieces(wxEmptyString);

	UpdateHash(wxEmptyString);
	
	UpdatePeers(wxEmptyString);
	UpdateSeeds(wxEmptyString);

	UpdateDownSpeed(wxEmptyString);
	UpdateUpSpeed(wxEmptyString);

	UpdateDownLimit(wxEmptyString);
	UpdateUpLimit(wxEmptyString);

	UpdateDownloaded(wxEmptyString);
	UpdateUploaded(wxEmptyString);

	UpdateRatio(wxEmptyString);

	UpdateComment(wxEmptyString);

	UpdateTrackerUrl(wxEmptyString);
	//
	UpdateTrackerStatus(wxEmptyString);
	//
	UpdateNextUpdate(wxEmptyString);

	UpdateDht(wxEmptyString);
}
