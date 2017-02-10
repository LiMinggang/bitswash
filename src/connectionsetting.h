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
// Class:ConnectionSettingPane
//
// Created by: lim k. c. <admin@bitswash.org>
// Created on: Tue Jun 26 13:23:51 MYT 2007
//

#ifndef _CONNECTIONSETTING_H_
#define _CONNECTIONSETTING_H_

#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/scrolwin.h>
#include <wx/statline.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>

#include "configuration.h"

#ifndef ID_CONNECTIONSETTING_START
#define ID_CONNECTIONSETTING_START wxID_HIGHEST
#endif

class ConnectionSettingPane : public wxPanel
{
	public:
		ConnectionSettingPane( wxWindow* parent, int id = wxID_ANY, wxPoint pos = wxDefaultPosition, wxSize size = wxDefaultSize, int style = wxTAB_TRAVERSAL);

		int GetPortFrom();
		int GetPortTo();
		bool GetDHTState();
		int GetDHTPort();
		long GetDownloadLimit();
		long GetUploadLimit();
		int GetMaxUploads();
		int GetMaxConnections();
		int GetMaxHalfOpen();

		
	private: 
		/* connection settings */
		wxPanel* m_pane_infoport;
		wxStaticText* m_static_infoport;
		wxStaticText* m_staticPortRange;
		wxSpinCtrl* m_spinPortFrom;
		wxStaticText* m_staticPortRangeTo;
		wxSpinCtrl* m_spinPortTo;
		wxCheckBox* m_checkDHT;
		wxSpinCtrl* m_spinDHTPort;
		wxStaticLine* m_staticline1;
		wxPanel* m_pane_infolimit;
		wxStaticText* m_static_infoport1;
		wxStaticText* m_staticDownLimit;
		wxStaticText* m_staticUpLimit;
		wxStaticText* m_staticMaxUploads;
		wxSpinCtrl* m_spinMaxUploads;
		wxStaticText* m_staticMaxConnections;
		wxSpinCtrl* m_spinMaxConnections;
		wxStaticText* m_staticMaxHalfOpen;
		wxSpinCtrl* m_spinMaxHalfOpen;

		AutoSizeInput* m_autoDownLimit;
		AutoSizeInput* m_autoUpLimit;
		


	DECLARE_EVENT_TABLE()
};

#endif	//_CONNECTIONSETTING_H_

