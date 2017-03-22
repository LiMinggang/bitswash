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
// Class: Setting
// Created by: lim k. c. <admin@bitswash.org>
// Created on: Thu May 17 21:45:53 MYT 2007
//

#ifndef _AUTOSIZEINPUT_H
#define _AUTOSIZEINPUT_H

#include <wx/wx.h>
#include <wx/spinctrl.h>
#include <wx/combobox.h>


/*custom class auto adjust value and convert units*/
class AutoSizeInput : public wxPanel
{
public:
	AutoSizeInput(wxWindow* parent, wxString unitbase = _T("B"), int usize = 3, int id = wxID_ANY, wxPoint pos = wxDefaultPosition, wxSize size = wxDefaultSize, int style = wxTAB_TRAVERSAL);
	void SetValue(long val);
	long GetValue();

protected:
	wxSpinCtrl* m_spinValue;
	wxComboBox* m_comboUnit;
	int m_comboidx; /* overcome limitation of combo index */
	
private:
	void OnSpinUpdate(wxSpinEvent &event);
	void OnComboUpdate(wxCommandEvent &event);
	long m_value;
	int m_unit_size;
	wxString m_unit_base;
	static const long AUTOSIZE_ID_SPIN;
	static const long AUTOSIZE_ID_COMBO;
};

#endif //__AUTOSIZEINPUT_H__
