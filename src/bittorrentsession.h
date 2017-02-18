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
#if __cplusplus <= 199711L
#include <boost/shared_ptr.hpp>
using boost::shared_ptr;
#else
#include <memory>
using std::shared_ptr;
#endif

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
	shared_ptr<TorrentConfig> config;
	bool isvalid;
} torrent_t ;

/* watch out the s after torrent */
typedef std::vector<shared_ptr<torrent_t> > torrents_t;
typedef std::map<wxString, int> torrents_map;

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
		
		void AddTorrentSession(shared_ptr<torrent_t>& torrent);
		bool AddTorrent(shared_ptr<torrent_t>& torrent);
		void RemoveTorrent(shared_ptr<torrent_t>& torrent, bool deletedata);
		shared_ptr<torrent_t> FindTorrent(const wxString &hash) const;

		shared_ptr<torrent_t> ParseTorrent(const wxString& filename);

		void GetTorrentQueue(torrents_t & queue_copy);

		void StartTorrent(shared_ptr<torrent_t>& torrent, bool force);
		void StopTorrent(shared_ptr<torrent_t>& torrent);
		void QueueTorrent(shared_ptr<torrent_t>& torrent);
		void PauseTorrent(shared_ptr<torrent_t>& torrent);
		void MoveTorrentUp(shared_ptr<torrent_t>& torrent);
		void MoveTorrentDown(shared_ptr<torrent_t>& torrent);
		void ReannounceTorrent(shared_ptr<torrent_t>& torrent);
		void ConfigureTorrent(shared_ptr<torrent_t>& torrent);
		void ConfigureTorrentFilesPriority(shared_ptr<torrent_t>& torrent);
		void ConfigureTorrentTrackers(shared_ptr<torrent_t>& torrent);
	
		libtorrent::session* GetLibTorrent() { return m_libbtsession;}

		void GetTorrentLog();

	private:

		void ScanTorrentsDirectory(const wxString& dirname);
		int find_torrent_from_hash(const wxString&  hash) const;

		void SaveTorrentResumeData(shared_ptr<torrent_t>& torrent);
		void SaveAllTorrent();

		void DumpTorrents();
		void CheckQueueItem();

		bool m_upnp_started;
		bool m_natpmp_started;
		bool m_lsd_started;

		wxMutex m_torrent_queue_lock;
		torrents_t m_torrent_queue;
		torrents_map m_torrent_map;

		wxApp* m_pParent;
		Configuration* m_config;
		libtorrent::session* m_libbtsession;
		
};

#endif // _BITTORRENTSESSION_H_
