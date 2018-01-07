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
//(*Headers(ExtensionSettingPane)
#include <wx/checkbox.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
//*)

class Configuration;
class ExtensionSettingPane :  public wxPanel
{
public:
	ExtensionSettingPane(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize, int style = wxTAB_TRAVERSAL);
	virtual ~ExtensionSettingPane();
	bool GetEnableMetadata();
	bool GetEnablePex();
	bool GetEnableEncryption();
	bool GetForceEncryption();
	bool GetEnableUpnp();
	bool GetEnableNatpmp();
	bool GetEnableLsd();

private:
	Configuration *m_pcfg;
	//(*Declarations(ExtensionSettingPane)
	wxCheckBox* m_check_lsd;
	wxCheckBox* m_check_metadata;
	wxCheckBox* m_check_natpmp;
	wxCheckBox* m_check_pe;
	wxCheckBox* m_check_pe_notforce;
	wxCheckBox* m_check_pex;
	wxCheckBox* m_check_upnp;
	//*)

protected:

	//(*Identifiers(ExtensionSettingPane)
	//*)
	private:

	//(*Handlers(ExtensionSettingPane)
	void OnEnablePe(wxCommandEvent& event);
	//*)
};


#endif	//_EXTENSIONSETTING_H_

