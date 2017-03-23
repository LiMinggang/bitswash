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
// Created by: lim k. c. <admin@bitswash.org>
// Created on: Sat Feb  3 17:02:38 2007
//

#ifndef _SUMMARYPANE_H_
#define _SUMMARYPANE_H_

#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/scrolwin.h>

#include "mainframe.h"
#include "torrentinfo.h"

class SummaryPane :  public wxScrolledWindow
{
public:
	SummaryPane(wxWindow *parent,
            	const wxWindowID id,
           		const wxPoint& pos = wxDefaultPosition,
           		const wxSize& size = wxDefaultSize,
           		long style = wxTAB_TRAVERSAL);

	~SummaryPane();

	void UpdateSummary();

	// SummaryPane interface

protected:
	wxPanel* m_pane_info;
	wxStaticText* m_pane_label_info;
	wxStaticText* m_label_saveas;
	wxStaticText* m_label_size;
	wxStaticText* m_label_pieces;
	wxStaticText* m_label_peers;
	wxStaticText* m_label_seeds;
	wxStaticText* m_label_downspeed;
	wxStaticText* m_label_upspeed;
	wxStaticText* m_label_downlimit;
	wxStaticText* m_label_uplimit;
	wxStaticText* m_label_downloaded;
	wxStaticText* m_label_uploaded;
	wxStaticText* m_label_ratio;
	wxStaticText* m_label_hash;
	wxStaticText* m_label_comment_static;
	wxTextCtrl* m_text_comment;
	wxPanel* m_pane_tracker;
	wxStaticText* m_pane_label_tracker;
	wxStaticText* m_label_tracker_url;
	wxStaticText* m_label_trackerstatus;
	wxStaticText* m_label_nextupdate;
	wxStaticText* m_label_dht;

private:
	// SummaryPane variables
	
	void UpdateSaveAs(wxString s);	
	void UpdateSize(wxString s);	
	void UpdatePieces(wxString s);	
	void UpdateHash(wxString s);	
	void UpdatePeers(wxString s);	
	void UpdateSeeds(wxString s);	
	void UpdateDownSpeed(wxString s);	
	void UpdateUpSpeed(wxString s);	
	void UpdateDownLimit(wxString s);	
	void UpdateUpLimit(wxString s);	
	void UpdateDownloaded(wxString s);	
	void UpdateUploaded(wxString s);	
	void UpdateRatio(wxString s);	
	void UpdateComment(wxString s);	

	void UpdateTrackerUrl(wxString s); 
	void UpdateTrackerStatus(wxString s); 
	void UpdateNextUpdate(wxString s); 
	void UpdateDht(wxString s); 
	void ResetSummary();

	wxFrame* m_pMainFrame;


private:
};


#endif	//_SUMMARYPANE_H_

