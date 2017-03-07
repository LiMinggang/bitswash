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
// Class: UrlDialog
// Created by: lim k. c. <admin@bitswash.org>
// Created on: Tue Jun  5 11:49:36 MYT 2007
//
//

// TODO: More responsive download status

#include "urldialog.h"

BEGIN_EVENT_TABLE(UrlDialog, wxDialog)
	EVT_BUTTON(wxID_OK, UrlDialog::OnOk)
	EVT_BUTTON(wxID_CANCEL, UrlDialog::OnCancel)
END_EVENT_TABLE()

UrlDialog::UrlDialog( wxString* url, wxWindow* parent, int id,  wxString title, wxPoint pos, wxSize size, int style ) : wxDialog( parent, id, title, pos, size, style )
{
	m_url = url;

	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxFlexGridSizer* fgSizerURLMain;
	fgSizerURLMain = new wxFlexGridSizer( 2, 2, 0, 0 );
	
	m_panelURL = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxFlexGridSizer* fgSizerURL;
	fgSizerURL = new wxFlexGridSizer( 2, 2, 0, 0 );
	
	m_staticURL = new wxStaticText( m_panelURL, wxID_ANY, _("URL:"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizerURL->Add( m_staticURL, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	m_textURL = new wxTextCtrl( m_panelURL, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_textURL->SetMinSize( wxSize( 380,-1 ) );
	
	fgSizerURL->Add( m_textURL, 0, wxALL, 5 );
	
	fgSizerURL->Add( 0, 0, 1, wxEXPAND, 0 );
	
	m_sdbOkCancel= new wxStdDialogButtonSizer();
	wxButton *ButtonOK = new wxButton( m_panelURL, wxID_OK );
	m_sdbOkCancel->AddButton( ButtonOK );
	m_sdbOkCancel->AddButton( new wxButton( m_panelURL, wxID_CANCEL ) );
	ButtonOK->SetDefault();
	m_sdbOkCancel->Realize();
	fgSizerURL->Add( m_sdbOkCancel, 0, wxALL|wxALIGN_RIGHT, 5 );
	
	m_panelURL->SetSizer( fgSizerURL );
	m_panelURL->Layout();
	fgSizerURL->Fit( m_panelURL );
	fgSizerURLMain->Add( m_panelURL, 1, wxEXPAND | wxALL, 5 );
	
	this->SetSizer( fgSizerURLMain );
	this->Layout();
	fgSizerURLMain->Fit( this );
}

void UrlDialog::OnOk(wxCommandEvent& event)
{
	*m_url = m_textURL->GetValue();

	EndModal(wxID_OK);
	event.Skip();
}

void UrlDialog::OnCancel(wxCommandEvent& event)
{
	EndModal(wxID_CANCEL);
	event.Skip();
}
