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

#ifndef URLDIALOG_H
#define URLDIALOG_H

//(*Headers(UrlDialog)
#include <wx/dialog.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)

class UrlDialog: public wxDialog
{
	public:

		UrlDialog(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~UrlDialog();

		//(*Declarations(UrlDialog)
		wxTextCtrl* m_textURL;
		//*)

	protected:

		//(*Identifiers(UrlDialog)
		//*)

	private:

		//(*Handlers(UrlDialog)
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
