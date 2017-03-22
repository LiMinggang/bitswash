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


#ifndef _URLDIALOG_H_
#define _URLDIALOG_H_

#include <wx/wx.h>
#include <wx/dialog.h>
#include <wx/stattext.h>
#include <wx/sizer.h>
#include <wx/panel.h>

class UrlDialog: public wxDialog 
{
public:
	UrlDialog( wxString* url, wxWindow* parent,  int id = wxID_ANY, wxString title = _T("Enter Torrent URL"), wxPoint pos = wxDefaultPosition, wxSize size = wxDefaultSize, int style = wxDEFAULT_DIALOG_STYLE );

protected:

private:
	void OnOk(wxCommandEvent& event);
	void OnCancel(wxCommandEvent& event);
	wxString* m_url;
	wxPanel* m_panelURL;
	wxStaticText* m_staticURL;
	wxTextCtrl* m_textURL;
	wxStdDialogButtonSizer* m_sdbOkCancel;
};

#endif
