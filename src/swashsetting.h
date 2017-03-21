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
// Class: SwashSetting
// Created by: lim k. c. <admin@bitswash.org>
// Created on: Thu May 17 21:45:53 MYT 2007
//

#ifndef _SWASHSETTING_H_
#define _SWASHSETTING_H_

#include <wx/wx.h>
#include <wx/listbook.h>
#include <wx/treebook.h>
#include <wx/listctrl.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/scrolwin.h>
#include <wx/statline.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>

#include "configuration.h"
#include "autosizeinput.h"
#include "connectionsetting.h"
#include "torrentsetting.h"
#include "generalsetting.h"
#include "extensionsetting.h"

#ifndef ID_SWASHSETTING_START
#define ID_SWASHSETTING_START wxID_HIGHEST
#endif

class AutoSizeInput;

class SwashSetting : public wxDialog 
{

	public:
		SwashSetting( wxWindow* parent, Configuration* pcfg, int id = wxID_ANY, wxString title = _("Preference"), wxPoint pos = wxDefaultPosition, wxSize size = wxDefaultSize, int style = wxDEFAULT_DIALOG_STYLE );
		~SwashSetting();
	protected:

	private:
		void OnOK(wxCommandEvent& event);
		void OnCancel(wxCommandEvent& event);
		wxPanel* m_panel_main;
		wxTreebook* m_listbookmenu;
		wxStdDialogButtonSizer* m_szokcancel;
		
		Configuration *m_pcfg;

		void Todo() { wxLogMessage(_T("Todo")) ; }	

		TorrentSettingPane* m_pane_torrentsettings;
		ConnectionSettingPane* m_pane_connectionsettings;
		GeneralSettingPane* m_pane_generalsettings;
		ExtensionSettingPane* m_pane_extensionsettings;
	
	private:
	DECLARE_EVENT_TABLE()
		
};


#endif //_SWASHSETTING_H_

