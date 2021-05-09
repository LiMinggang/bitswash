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

#include "autosizeinput.h"

/* CLASS AutoSizeInput */
#define MAX_UNITS 5
static wxChar units[MAX_UNITS][2] = { _T(""), _T("K"), _T("M"), _T("G"), _T("T") };

AutoSizeInput::AutoSizeInput( wxWindow* parent, wxString unitbase, int usize, int id, wxPoint pos , wxSize size, int style) : wxPanel (parent, id, pos, size, style) 
{
	wxFlexGridSizer* fgSizer;
	fgSizer = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer->SetFlexibleDirection( wxBOTH );
	
	m_unit_size = usize;

	m_spinValue = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(100,-1), wxSP_ARROW_KEYS, -1, 99999999, -1);
	
	fgSizer->Add( m_spinValue, 0, wxALL, 2 );
	
	m_comboUnit = new wxComboBox( this, wxID_ANY, unitbase, wxDefaultPosition, wxSize(80,-1), 0, nullptr, wxCB_READONLY );

	m_unit_base = unitbase;
	
	for (int i = 0 ; i < usize; i++)
	{
		m_comboUnit->Append( units[i] +  unitbase );
	}
	fgSizer->Add( m_comboUnit, 0, wxALL, 2 );
	
	SetSizerAndFit(fgSizer);

	fgSizer->Fit(this);
	Bind( wxEVT_SPINCTRL, &AutoSizeInput::OnSpinUpdate, this, m_spinValue->GetId() );
	Bind( wxEVT_COMBOBOX, &AutoSizeInput::OnComboUpdate, this, m_comboUnit->GetId() );
}
		
void AutoSizeInput::SetValue(long val) 
{
	m_value = val;
	double vl = 0;
	double ivl = 1024 ;
	int i=0;

	vl = val;
	for(i = 0; i < m_unit_size; i++)
	{
		if(val < ivl) break;
		vl = val / ivl;
		ivl *=ivl;
	}

    wxASSERT(i<MAX_UNITS);
	m_spinValue->SetValue((long)vl);
	m_comboidx = i;
	m_comboUnit->SetValue(units[i] + m_unit_base);
}

void AutoSizeInput::OnSpinUpdate(wxSpinEvent& WXUNUSED(event))
{
	long mply =((long)1) << (m_comboidx * 10);

	if (m_spinValue->GetValue() > 0)
		m_value = m_spinValue->GetValue() * mply;
	else
		m_value = m_spinValue->GetValue();
}

void AutoSizeInput::OnComboUpdate(wxCommandEvent& WXUNUSED(event))
{
	m_comboidx = m_comboUnit->GetCurrentSelection();
	
	long mply =((long)1) << (m_comboidx * 10);

	if (m_spinValue->GetValue() > 0)
		m_value = m_spinValue->GetValue() * mply;
	else
		m_value = m_spinValue->GetValue();
}
