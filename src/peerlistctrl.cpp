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
// Class: PeerListCtrl
//
// Created by: lim k. c. <admin@bitswash.org>
// Created on: Mon Feb  5 11:57:07 2007
//


#include <wx/thread.h>

#include <libtorrent/address.hpp>
#include <libtorrent/peer_info.hpp>
#include "bitswash.h"
#include "torrentinfo.h"
#include "peerlistctrl.h"
#include "mainframe.h"
#include "functions.h"

BEGIN_EVENT_TABLE(PeerListCtrl, SwashListCtrl)
END_EVENT_TABLE()
// PeerListCtrl

enum peerlistcolumnid {
	PEERLIST_COLUMN_IP = 0,
	PEERLIST_COLUMN_COUNTRY,
	PEERLIST_COLUMN_TYPE,
	PEERLIST_COLUMN_STATUS,
	PEERLIST_COLUMN_CLIENT,
	PEERLIST_COLUMN_FLAGS,
	PEERLIST_COLUMN_DOWNLOADSPEED,
	PEERLIST_COLUMN_UPLOADSPEED,
	PEERLIST_COLUMN_DOWNLOADED,
	PEERLIST_COLUMN_UPLOADED,
	PEERLIST_COLUMN_PROGRESS
};

#ifndef __WXMSW__
static SwashColumnItem peerlistcols[] = 
{
	{ id: PEERLIST_COLUMN_IP, name: _T("ip"), title: _("IP"), tooltip:_("Client IP address"), width: 128, show: true},
	{ id: PEERLIST_COLUMN_COUNTRY, name: _T("country"), title: _("Country"), tooltip:_("Client country"), width: 0, show: false },
	{ id: PEERLIST_COLUMN_TYPE, name: _T("type"), title: _("Type"), tooltip:_("Peer type"), width: 76, show: true }, 
	{ id: PEERLIST_COLUMN_STATUS, name: _T("status"), title: _("Status"), tooltip:_("Peer status"), width: 100, show: true }, 
	{ id: PEERLIST_COLUMN_CLIENT, name: _T("client"), title: _("Client"), tooltip:_("Client name"), width: 106, show: true },
	{ id: PEERLIST_COLUMN_FLAGS, name: _T("flags"), title: _("Flags"), tooltip:_("Peer flags"), width: 70, show: true },
	{ id: PEERLIST_COLUMN_DOWNLOADSPEED, name: _T("downloadspeed"), title: _("Download Speed"), tooltip:_("Download Speed"), width: 120, show: true },
	{ id: PEERLIST_COLUMN_UPLOADSPEED, name: _T("uploadspeed"), title: _("Upload Speed"), tooltip:_("Upload Speed"), width: 120, show: true },
	{ id: PEERLIST_COLUMN_DOWNLOADED, name: _T("downloaded"), title: _("Downloaded"), tooltip:_("Total bytes downloaded"), width: 88, show: true },
	{ id: PEERLIST_COLUMN_UPLOADED, name: _T("uploaded"), title: _("Uploaded"), tooltip:_("Total bytes uploaded"), width: 88, show: true },
	{ id: PEERLIST_COLUMN_PROGRESS, name: _T("progress"), title: _("Progress"), tooltip:_("Progress"), width: 0, show: true },
};
#else
static SwashColumnItem peerlistcols[] = 
{
	{  PEERLIST_COLUMN_IP,  _T("ip"),  _("IP"), _("Client IP address"),  128,  true},
	{  PEERLIST_COLUMN_COUNTRY,  _T("country"),  _("Country"), _("Client country"),  0,  false },
	{  PEERLIST_COLUMN_TYPE,  _T("type"),  _("Type"), _("Peer type"),  76,  true }, 
	{  PEERLIST_COLUMN_STATUS,  _T("status"),  _("Status"), _("Peer status"),  100,  true }, 
	{  PEERLIST_COLUMN_CLIENT,  _T("client"),  _("Client"), _("Client name"),  106,  true },
	{  PEERLIST_COLUMN_FLAGS,  _T("flags"),  _("Flags"), _("Peer flags"),  70,  true },
	{  PEERLIST_COLUMN_DOWNLOADSPEED,  _T("downloadspeed"),  _("Download Speed"), _("Download Speed"),  120,  true },
	{  PEERLIST_COLUMN_UPLOADSPEED,  _T("uploadspeed"),  _("Upload Speed"), _("Upload Speed"),  120,  true },
	{  PEERLIST_COLUMN_DOWNLOADED,  _T("downloaded"),  _("Downloaded"), _("Total bytes downloaded"),  88,  true },
	{  PEERLIST_COLUMN_UPLOADED,  _T("uploaded"),  _("Uploaded"), _("Total bytes uploaded"),  88,  true },
	{  PEERLIST_COLUMN_PROGRESS,  _T("progress"),  _("Progress"), _("Progress"),  0,  true },
};
#endif
PeerListCtrl::PeerListCtrl(wxWindow *parent,
               const wxString settings,
               const wxWindowID id,
               const wxPoint& pos,
               const wxSize& size,
               long style)
        : SwashListCtrl(parent, SWASHLISTCOL_SIZE(peerlistcols), peerlistcols, settings, id, pos, size, style)
{

	SetImageList(wxGetApp().GetCountryFlagsList(), wxIMAGE_LIST_SMALL);

}

PeerListCtrl::~PeerListCtrl()
{

}

wxString PeerListCtrl::GetItemValue(long item, long columnid) const
{
	PeerListCtrl* pThis = const_cast<PeerListCtrl*>(this);

	MainFrame* pMainFrame = (MainFrame*)(wxGetApp().GetTopWindow());

	const PeerListCtrl::peer_list_t *peers_list= pMainFrame->GetPeersList();

	if ( (peers_list == NULL) || (peers_list->size() <= 0) || item >= peers_list->size())
	{
		return _T("");
	}

	libtorrent::peer_info peer_info = peers_list->at(item);

	int peertype_flag;
	static const wxChar* peer_source_flag[] = 
	{
		_("(T)"),
		_("(D)"),
		_("(P)"),
		_("(L)"),
		_("(R)")
	};

	//wxLogDebug(_T("PeerListCtrl::Showing %d items \n"), peers_list->size());

	libtorrent::address c_ip = peer_info.ip.address();

	wxString ret;
 	switch(columnid) 
	{
		case PEERLIST_COLUMN_IP:
			ret = wxString::FromAscii(c_ip.to_string().c_str());
			break;
		case PEERLIST_COLUMN_COUNTRY:
			ret = (wxString::FromAscii(peer_info.country));
			break;
		case PEERLIST_COLUMN_CLIENT:
			ret = wxString::FromAscii(peer_info.client.c_str());
			break;
		case PEERLIST_COLUMN_TYPE:
			peertype_flag = peer_info.source;
			if( (peertype_flag < 0 ) || (peertype_flag > 4)) peertype_flag = 4;
			
			ret = wxString::Format(_T("%s %s"), (peer_info.flags & libtorrent::peer_info::seed)?_("Seed"):_("Peer"), peer_source_flag[peertype_flag]);
			break;
		case PEERLIST_COLUMN_STATUS:
			if (peer_info.flags & libtorrent::peer_info::handshake) 
			{ 
				ret = _("Handshaking");
			} 
			else if ( peer_info.flags & libtorrent::peer_info::connecting) 
			{
				ret = _("Connecting");
			}
			else if ( peer_info.flags & libtorrent::peer_info::queued )
			{
				ret = _("Queued");
			}
			else if (peer_info.flags & libtorrent::peer_info::local_connection)
			{
				ret = _("Local");
			} else
				ret = _("Remote");
			break;
		case PEERLIST_COLUMN_FLAGS:
			ret = wxString::Format(_T("%c%c%c%c%c%c%c"), 
							(peer_info.flags & libtorrent::peer_info::interesting)? 'I':' ',
							(peer_info.flags & libtorrent::peer_info::choked)? 'C':' ',
							(peer_info.flags & libtorrent::peer_info::remote_interested)? 'i':' ',
							(peer_info.flags & libtorrent::peer_info::remote_choked)? 'c':' ',
							(peer_info.flags & libtorrent::peer_info::supports_extensions)? 'e':' ',
							(peer_info.flags & libtorrent::peer_info::rc4_encrypted)? 'X':' ',
							(peer_info.flags & libtorrent::peer_info::plaintext_encrypted)? 'x':' ');
			break;
		case PEERLIST_COLUMN_DOWNLOADSPEED:
			ret = HumanReadableByte( (wxDouble) peer_info.down_speed) + wxString(_T("ps"));
			break;
		case PEERLIST_COLUMN_UPLOADSPEED:
			ret = HumanReadableByte((wxDouble) peer_info.up_speed) + wxString(_T("ps"));
			break;
		case PEERLIST_COLUMN_DOWNLOADED:
			ret = HumanReadableByte((wxDouble)peer_info.total_download);
			break;
		case PEERLIST_COLUMN_UPLOADED:
			ret = HumanReadableByte((wxDouble) peer_info.total_upload);
			break;
		case PEERLIST_COLUMN_PROGRESS:
			ret = wxString::Format(_T("%d/%d"), peer_info.downloading_progress, peer_info.downloading_total);
			break;
		default:
			ret = _T("");
	}

	return ret;
	//ret = wxString::Format(_T("%d.%d"), item, columnid);

}

#if WXVER >= 280
int PeerListCtrl::GetItemColumnImage(long item, long columnid) const
{
	PeerListCtrl* pThis = const_cast<PeerListCtrl*>(this);

	MainFrame* pMainFrame = (MainFrame*)(wxGetApp().GetTopWindow());
	const PeerListCtrl::peer_list_t *peers_list= pMainFrame->GetPeersList();

	if ( (peers_list == NULL) || (peers_list->size() <= 0))
	{
		return -1;
	}

	libtorrent::peer_info peer_info = peers_list->at(item);

	wxLogDebug(_T("PeerListCtrl GetItemColumnImage Column %ld of item %ld"), columnid, item);

	switch(columnid)
	{
		case PEERLIST_COLUMN_IP:
			wxString ctry = wxString::FromAscii(peer_info.country);
			if ((!ctry.IsEmpty())&&(('A' <= ctry[0]) &&
					(ctry[0] <= 'Z')) &&
				 (('A' <= ctry[1]) &&
					(ctry[1] <= 'Z')))
   			{
					ctry[2] = '\0';
				wxLogDebug(_T("Image idx %s %d\n"),ctry.c_str(), wxGetApp().GetCountryFlag(ctry.c_str()));
				return wxGetApp().GetCountryFlag(ctry.c_str());

			}
			else
				return wxGetApp().GetCountryFlag(wxEmptyString);

			break;
	}
    return -1;
}
#else
int PeerListCtrl::OnGetItemImage(long item) const
{

	PeerListCtrl* pThis = const_cast<PeerListCtrl*>(this);

	MainFrame* pMainFrame = (MainFrame*)(wxGetApp().GetTopWindow());
	const PeerListCtrl::peer_list_t *peers_list= pMainFrame->GetPeersList();

	if ( (peers_list == NULL) || (peers_list->size() <= 0))
	{
		return -1;
	}

	libtorrent::peer_info peer_info = peers_list->at(item);

	//wxLogDebug(_T("PeerListCtrl GetItemImage Column %ld of item %ld"), 0, item);

	wxString ctry = wxString::FromAscii(peer_info.country);
	//wxLogDebug(_T("Image idx %s %d\n"),ctry.c_str(), wxGetApp().GetCountryFlag(ctry.c_str()));
	return wxGetApp().GetCountryFlag(ctry.c_str());
}
#endif

