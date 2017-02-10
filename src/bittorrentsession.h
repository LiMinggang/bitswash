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
// Class: BitTorrentSession
//
// Created by: lim k. c. <admin@bitswash.org>
// Created on: Sun Aug 12 13:49:33 MYT 2007
//

#ifndef _BITTORRENTSESSION_H_
#define _BITTORRENTSESSION_H_

#include <vector>
#include <wx/app.h>
#include <wx/thread.h>

#include <libtorrent/session.hpp>

#include "configuration.h"
#include "torrentconfig.h"

typedef struct torrent_handle_t {

	torrent_handle_t() :
		isvalid(false), config(0)
	{}

	wxString name;
	wxString hash;
	libtorrent::torrent_handle handle;
	boost::intrusive_ptr<libtorrent::torrent_info> info;
	TorrentConfig *config;
	bool isvalid;
} torrent_t ;

/* watch out the s after torrent */
typedef std::vector<torrent_t*> torrents_t;

class BitTorrentSession : public wxThread
{
	public:
		BitTorrentSession(wxApp* pParent, Configuration* config);
		~BitTorrentSession();

		virtual void *Entry();
		virtual void OnExit();

		void ConfigureSession();
		void SetLogSeverity();
		void SetConnection();
		void SetConnectionLimit();
		void SetDht();
		void SetEncryption();
		void StartExtensions();
		void StartUpnp();
		void StartNatpmp();
		void StartLsd();
		
		void AddTorrentSession(torrent_t* torrent);
		bool AddTorrent(torrent_t* torrent);
		void RemoveTorrent(torrent_t* torrent, bool deletedata);

		torrent_t* ParseTorrent(wxString filename);

		torrents_t* GetTorrentQueue() { return &m_torrent_queue; }

		void StartTorrent(torrent_t* torrent, bool force);
		void StopTorrent(torrent_t* torrent);
		void QueueTorrent(torrent_t* torrent);
		void PauseTorrent(torrent_t* torrent);
		void MoveTorrentUp(torrent_t* torrent);
		void MoveTorrentDown(torrent_t* torrent);
		void ReannounceTorrent(torrent_t* torrent);
		void ConfigureTorrent(torrent_t* torrent);
		void ConfigureTorrentFilesPriority(torrent_t* torrent);
		void ConfigureTorrentTrackers(torrent_t* torrent);
	
		libtorrent::session* GetLibTorrent() { return m_libbtsession;}

		void GetTorrentLog();

	private:

		void ScanTorrentsDirectory(wxString dirname);
		int find_torrent_from_hash(wxString hash);

		void SaveTorrentResumeData(torrent_t* torrent);
		void SaveAllTorrent();

		void DumpTorrents();
		void CheckQueueItem();

		bool m_upnp_started;
		bool m_natpmp_started;
		bool m_lsd_started;

		torrents_t m_torrent_queue;

		wxApp* m_pParent;
		Configuration* m_config;
		libtorrent::session* m_libbtsession;
		
};

#endif // _BITTORRENTSESSION_H_
