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
// Class: SwashStatBar
//
// Created by: lim k. c. <admin@bitswash.org>
// Created on: Thu Jun  7 13:10:21 MYT 2007
//
//
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <wx/wx.h>

#include "functions.h"
#include "swashstatbar.h"
#include "mainframe.h"

SwashStatBar::SwashStatBar(wxWindow* parent): wxStatusBar(parent, wxID_ANY), SwashBarField(BAR_FIELD_MAX)
{
	static const int widths[BAR_FIELD_MAX] = { -1, 100, 100, 160, 50 };

	SetFieldsCount(BAR_FIELD_MAX);
	SetStatusWidths(BAR_FIELD_MAX, widths);

	m_pMainFrame = (wxFrame*)parent;

}

SwashStatBar::~SwashStatBar()
{

}

void SwashStatBar::UpdateStatBar()
{
	MainFrame *pMainFrame = (MainFrame*)m_pMainFrame;

	SetStatusText(pMainFrame->GetStatusDownloadRate(), BAR_FIELD_DOWNLOADRATE );
	SetStatusText(pMainFrame->GetStatusUploadRate(), BAR_FIELD_UPLOADRATE );

	wxString peersdht = wxString::Format(_("Peers: %s DHT: %s"), pMainFrame->GetStatusPeers().c_str(), pMainFrame->GetStatusDHT().c_str());
	SetStatusText(peersdht, BAR_FIELD_PEERS );
	SetStatusText(pMainFrame->GetStatusIncoming(), BAR_FIELD_INCOMING);
}
