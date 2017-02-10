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
// Class:ExtensionSettingPane
//
// Created by: lim k. c. <admin@bitswash.org>
// Created on: Tue Jun 26 13:23:51 MYT 2007
//

#ifndef _EXTENSIONSETTING_H_
#define _EXTENSIONSETTING_H_

#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/scrolwin.h>
#include <wx/statline.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>

#include "configuration.h"

#ifndef ID_EXTENSIONSETTING_START
#define ID_EXTENSIONSETTING_START wxID_HIGHEST
#endif

class ExtensionSettingPane :  public wxPanel
{
	public:
		ExtensionSettingPane( wxWindow* parent, int id = wxID_ANY, wxPoint pos = wxDefaultPosition, wxSize size = wxDefaultSize, int style = wxTAB_TRAVERSAL);

		bool GetEnableMetadata();
		bool GetEnablePex();
		bool GetEnableEncryption();
		bool GetForceEncryption();
		bool GetEnableUpnp();
		bool GetEnableNatpmp();
		bool GetEnableLsd();

	private:
		wxFrame* m_pMainFrame;
		Configuration *m_pcfg;

		wxPanel* m_pane_infoextension;
		wxStaticText* m_static_infoextension;
		wxCheckBox* m_check_metadata;
		wxCheckBox* m_check_pex;
		wxPanel* m_pane_infoencryption;
		wxStaticText* m_static_infoencryption;
		wxCheckBox* m_check_pe;
		wxCheckBox* m_check_pe_notforce;
		wxPanel* m_pane_infonetwork;
		wxStaticText* m_static_infonat;
		wxCheckBox* m_check_upnp;
		wxCheckBox* m_check_natpmp;
		wxCheckBox* m_check_lsd;

		void OnEnablePe(wxCommandEvent& event);
		void OnCheckUpnp(wxCommandEvent& event);
		void OnCheckNatpmp(wxCommandEvent& event);
		void OnCheckLsd(wxCommandEvent& event);
	private:
		DECLARE_EVENT_TABLE()
	
};


#endif	//_EXTENSIONSETTING_H_

