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
// Class: MainFrame
//
// Created by: lim k. c. <admin@bitswash.org>
// Created on: Mon Feb  5 11:09:21 2007
//
#ifndef _MAINFRAME_H_
#define _MAINFRAME_H_

#include <set>
#include "compat.h"
#include <wx/hashmap.h>
#include <wx/menu.h>
#include <wx/treectrl.h>
#include <wx/timer.h>
#include <wx/thread.h>

#include <libtorrent/session.hpp>

#define ID_BWLIST_CTRL_START wxID_HIGHEST + 100
#define ID_FLIST_CTRL_START wxID_HIGHEST + 200
#define ID_PLIST_CTRL_START wxID_HIGHEST + 300
#define ID_TORRENTSETTING_START wxID_HIGHEST + 400
#define ID_SETTINGS_START wxID_HIGHEST + 500

#ifdef __WXMSW__
#include <wx/msw/winundef.h>
#endif

#include "bittorrentsession.h"
#include "torrentlistctrl.h"
#include "peerlistctrl.h"
#include "filelistctrl.h"
#include "trackerlistctrl.h"
#include "torrentinfo.h"
#include "summarypane.h"
#include "configuration.h"
//#include "torrentproperty.h"
#include "torrentconfig.h"
#include "swashsetting.h"
#include "swashstatbar.h"
#include "swashtrayicon.h"

class TorrentInfo;
class SummaryPane;
class TorrentProperty;

class MainFrame: public wxFrame 
{
    enum
    {
        ID_FILE_EXIT = wxID_EXIT,
        ID_HELP_ABOUT = wxID_ABOUT,
        ID_FILE_OPEN_URL = wxID_HIGHEST+1,
        ID_FILE_OPEN_TORRENT,
        ID_FILE_CREATE_TORRENT,

        ID_VIEW_STATUS,
        ID_VIEW_TOOLBAR,
        ID_VIEW_FAVORITE,
        ID_TORRENT_START,
        ID_TORRENT_FORCE_START,
        ID_TORRENT_PAUSE,
        ID_TORRENT_STOP,
        ID_TORRENT_OPENDIR,
        ID_TORRENT_PROPERTIES,
        ID_TORRENT_REMOVE,
        ID_TORRENT_REMOVEDATA,
        ID_TORRENT_MOVEUP,
        ID_TORRENT_MOVEDOWN,
        ID_TORRENT_REANNOUNCE,
        ID_TORRENT_RECHECK,
        ID_OPTIONS_PREFERENCES,
        ID_HELP_BITSWASH_HELP,
        ID_HELP_HOMEPAGE,
    	
    	/* TIMER */
    	ID_TIMER_GUI_UPDATE,

        ID_OPTIONS_LANGUAGE,
    	/* plenty of languages follows
    	 * next item should starts with +100 or so
    	 */
    };

public:
    MainFrame(wxFrame *frame, const wxString& title);
	~MainFrame();

	wxMenu* GetNewTorrentMenu();
	void OnMenuOpen(wxMenuEvent& event);
	void TorrentOperationMenu(wxMenu* torrentmenu, bool enabled);
	void UpdateUI();
		void UpdateSelectedTorrent();

	shared_ptr<torrent_t> GetSelectedTorrent() ;
	
	const std::vector<shared_ptr<torrent_t> >* GetTorrentList() { return &m_torrentlistitems;  }
	const PeerListCtrl::peer_list_t* GetPeersList() { return &m_peerlistitems;  }
	
	wxMutex& GetListLock() {return  m_listlock; }
	wxString GetStatusDownloadRate();
	wxString GetStatusUploadRate();
	wxString GetStatusPeers();
	wxString GetStatusDHT();
	wxString GetStatusIncoming();
	Configuration* GetConfig() { return m_config ; }

	void ShowSystray(bool show);

	void OpenTorrent();
	void OpenTorrentUrl();
	void ShowPreferences();

	void SetLogSeverity();

	void OnListItemClick(long item);

	bool TorrentListIsValid() { return m_torrentlistisvalid; }
	void TorrentListIsValid(bool valid) { m_torrentlistisvalid = valid; }
    static void ReceiveTorrent(wxString fileorurl);
private:

    wxMenuBar* CreateMainMenuBar();
    void CreateToolBar();
    wxToolBar* CreateMainToolBar();
    void SaveFramePosition();
	void SaveFrameSize();
    wxTreeCtrl* CreateSideTreeCtrl();
    void DoSize();
    void CreateTorrentPanel();
    void UpdateTorrentPanel();
    void CreateMainPanel();
    void CreateTorrentList();
    void CreateTorrentInfo();
#if wxUSE_STATUSBAR
	void UpdateStatusBar();
#endif
	void UpdateTrayInfo();

	void AddTorrent(wxString filename, bool usedefault = false);
	void RemoveTorrent(bool deletedata);
	void OnMenuOpenTorrent(wxCommandEvent& event);
	void OnMenuOpenTorrentUrl(wxCommandEvent& event);
	void ToDo(wxCommandEvent& event);
	void OnQuit(wxCommandEvent& event);
	void OnMenuHelp(wxCommandEvent& event);
	void OnHelpHomepage(wxCommandEvent& event);
	void OnAbout(wxCommandEvent& event);
	void OnMenuViewStatusBar(wxCommandEvent& event);
	void OnMenuViewToolbar(wxCommandEvent& event);
	void OnMenuTorrentStart(wxCommandEvent& event);
	void OnMenuTorrentPause(wxCommandEvent& event);
	void OnMenuTorrentStop(wxCommandEvent& event);
	void OnMenuTorrentProperties(wxCommandEvent& event);
	void OnMenuTorrentRemove(wxCommandEvent& event);
	void OnMenuTorrentRemoveData(wxCommandEvent& event);
	void OnMenuTorrentMoveUp(wxCommandEvent& event);
	void OnMenuTorrentMoveDown(wxCommandEvent& event);
	void OnMenuTorrentReannounce(wxCommandEvent& event);
	void OnMenuTorrentRecheck(wxCommandEvent& event);
	void OnMenuTorrentOpenDir(wxCommandEvent& event);
	void OnMenuPreferences(wxCommandEvent& event);
	void OnMenuOptionLanguage(wxCommandEvent& event);
	void OnUpdateOptionLanguage(wxUpdateUIEvent& event);
	void OnClose(wxCloseEvent& event);
	void OnMinimize(wxIconizeEvent& event);
	void OnMove(wxMoveEvent& event);
	void OnSize(wxSizeEvent& event);

	void OnUpdateUI_MenuTorrent(wxUpdateUIEvent& event);

	void LoadIcons();
	//void SaveTorrentResumeData(shared_ptr<ptorrent_t>& torrent);
	//int find_torrent_from_hash(wxString hash) ;

private:
	BitTorrentSession* m_btsession;
	wxMenu* m_torrentmenu;
	wxMenu* m_languagemenu;

#if WXVER >= 280
	wxAuiManager m_mgr;
#else
	wxFrameManager m_mgr;
#endif
	wxPanel* m_torrentpane;
	TorrentInfo* m_torrentinfo;

	TorrentListCtrl* m_torrentlistctrl;
	PeerListCtrl* m_peerlistctrl;
	FileListCtrl* m_filelistctrl;
	SummaryPane* m_summarypane;
	TrackerListCtrl* m_trackerlistctrl;

	Configuration *m_config;
	TorrentProperty *m_torrentproperty;
	SwashSetting *m_swashsetting;
	SwashStatBar *m_swashstatbar;

	wxLog* m_oldlog;

	void OnRefreshTimer(wxTimerEvent& WXUNUSED(event));
	void GetTorrentLog();

	wxTimer m_refreshtimer;

	torrents_t m_torrentlistitems;
	PeerListCtrl::peer_list_t m_peerlistitems;

	int m_lastqindex;
	wxMutex m_listlock;

	SwashTrayIcon* m_swashtrayicon;
	wxIcon m_trayicon;
	wxIcon m_frameicon;

	bool m_upnp_started;
	bool m_natpmp_started;
	bool m_lsd_started;

	int m_prevlocale;
	long m_prevselecteditem;
	bool m_torrentlistisvalid;
    bool m_closed;
	DECLARE_EVENT_TABLE()
};

#endif // _MAINFRAME_H_
