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


#ifndef _SWASHSTATBAR_H_
#define _SWASHSTATBAR_H_

#include <wx/wx.h>
#include <wx/statusbr.h>


class SwashStatBar: public wxStatusBar
{
public:
	explicit SwashStatBar(wxWindow* parent);
	virtual ~SwashStatBar();
	void UpdateStatBar();

private:
	enum
	{
		BAR_FIELD_MESSAGE = 0,
		BAR_FIELD_DOWNLOADRATE,	
		BAR_FIELD_UPLOADRATE,	
		BAR_FIELD_PEERS,
		BAR_FIELD_INCOMING,
		BAR_FIELD_MAX,
	} SwashBarField;

	wxFrame* m_pMainFrame;
};

#endif // BITSWASH_H
