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

//(*Headers(ConnectionSettingPane)
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/statline.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)
class ConnectionSettingPane : public wxPanel
{
public:
	ConnectionSettingPane( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, int style = wxTAB_TRAVERSAL);
	virtual ~ConnectionSettingPane();

	int  GetPortFrom();
	int  GetPortTo();
	bool GetDHTState();
	int  GetDHTPort();
	long GetDownloadLimit();
	long GetUploadLimit();
	int  GetMaxUploads();
	int  GetMaxConnections();
	bool IsAnonymousMode();
	bool UseOSCache();
	bool GetSupperSeeding();
	int  GetEnabledProtocols();
	bool GetRateLimitIpOverhead();
	bool GetRateLimitUtp();
	bool GetRateLimitPeersOnLan();
	bool GetPreferUdpTrackers();
	bool GetAllowMultipleConnectionsPerIp();
	int  GetChokingAlgorithm();
	int  GetSeedChokingAlgorithm();
	wxString GetDhtBootstrapNodes();

private: 
		//(*Declarations(ConnectionSettingPane)
		AutoSizeInput* m_autoDownLimit;
		AutoSizeInput* m_autoUpLimit;
		wxCheckBox* m_checkDHT;
		wxCheckBox* m_check_allow_multiple_connections_per_ip;
		wxCheckBox* m_check_anonymous_mode;
		wxCheckBox* m_check_os_cache;
		wxCheckBox* m_check_prefer_udp_trackers;
		wxCheckBox* m_check_rate_limit_ip_overhead;
		wxCheckBox* m_check_rate_limit_peers_on_lan;
		wxCheckBox* m_check_rate_limit_utp_protocol;
		wxCheckBox* m_check_supper_seeding;
		wxChoice* m_choice_choking_algorithm;
		wxChoice* m_choice_enabled_protocols;
		wxChoice* m_choice_seed_choking_algorithm;
		wxSpinCtrl* m_spinDHTPort;
		wxSpinCtrl* m_spinMaxConnections;
		wxSpinCtrl* m_spinMaxUploads;
		wxSpinCtrl* m_spinPortFrom;
		wxSpinCtrl* m_spinPortTo;
		wxTextCtrl* m_dht_bootstrap_nodes;
		//*)
	protected:

		//(*Identifiers(ConnectionSettingPane)
		//*)

	private:

		//(*Handlers(ConnectionSettingPane)
		//*)

		//DECLARE_EVENT_TABLE()
};

#endif	//_CONNECTIONSETTING_H_

