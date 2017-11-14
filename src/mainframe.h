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

class wxURL;
class TorrentInfo;
class SummaryPane;
class TorrentProperty;
class MagnetUriHanlder;

wxDECLARE_EVENT( SHOW_HIDE_TRAYICON, wxCommandEvent );

class MainFrame: public wxFrame
{
public:
	MainFrame( wxFrame *frame, const wxString& title );
	~MainFrame();

	wxMenu* GetNewTorrentMenu();
	void OnMenuOpen( wxMenuEvent& event );
	void TorrentOperationMenu( wxMenu* torrentmenu );
	void UpdateUI(bool force = false);
	void UpdateSelectedTorrent();

	std::shared_ptr<torrent_t> GetSelectedTorrent() ;
	std::shared_ptr<torrent_t> GetTorrent(int idx) { return m_btsession->GetTorrent(idx);}
	const PeerListCtrl::peer_list_t* GetPeersList() { return &m_peerlistitems;  }

	wxMutex& GetListLock() {return  m_listlock; }
	wxString GetStatusDownloadRate();
	wxString GetStatusUploadRate();
	wxString GetStatusPeers();
	wxString GetStatusDHT();
	wxString GetStatusIncoming();
	Configuration* GetConfig() { return m_config ; }

	void ShowSystray( bool show );

	void OpenTorrent();
	void OpenTorrentUrl();
	void OpenMagnetURI(const wxString & magneturi);
	void DownloadTorrent(wxURL & url);
	void ShowPreferences();

	void SetLogSeverity();

	void OnListItemClick( long item );

	static void ReceiveTorrent( wxString fileorurl );
	static const long ID_OPTIONS_PREFERENCES; // Share with Tray Icon;

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

	void AddTorrent( wxString filename, bool usedefault = false );
	void RemoveTorrent( bool deletedata );
	void OnMenuOpenTorrent( wxCommandEvent& event );
	void OnMenuOpenTorrentUrl( wxCommandEvent& event );
	void ToDo( wxCommandEvent& event );
	void OnQuit( wxCommandEvent& event );
	void OnMenuHelp( wxCommandEvent& event );
	void OnHelpHomepage( wxCommandEvent& event );
	void OnAbout( wxCommandEvent& event );
	void OnMenuViewStatusBar( wxCommandEvent& event );
	void OnMenuViewToolbar( wxCommandEvent& event );
	void OnMenuTorrentStart( wxCommandEvent& event );
	void OnMenuTorrentPause( wxCommandEvent& event );
	void OnMenuTorrentStop( wxCommandEvent& event );
	void OnMenuTorrentProperties( wxCommandEvent& event );
	void OnMenuTorrentRemove( wxCommandEvent& event );
	void OnMenuTorrentRemoveData( wxCommandEvent& event );
	void OnMenuTorrentMoveUp( wxCommandEvent& event );
	void OnMenuTorrentMoveDown( wxCommandEvent& event );
	void OnMenuTorrentReannounce( wxCommandEvent& event );
	void OnMenuTorrentRecheck( wxCommandEvent& event );
	void OnMenuTorrentOpenDir( wxCommandEvent& event );
	void OnMenuPreferences( wxCommandEvent& event );
	void OnMenuOptionLanguage( wxCommandEvent& event );
	void OnUpdateOptionLanguage( wxUpdateUIEvent& event );
	void OnClose( wxCloseEvent& event );
	void OnMinimize( wxIconizeEvent& event );
	void OnMove( wxMoveEvent& event );
	void OnSize( wxSizeEvent& event );

	void OnUpdateUI_MenuTorrent( wxUpdateUIEvent& event );

	void LoadIcons();
	//void SaveTorrentResumeData(std::shared_ptr<ptorrent_t>& torrent);
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

	void OnRefreshTimer( wxTimerEvent& WXUNUSED( event ) );

	wxTimer m_refreshtimer;

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
	bool m_closed;
	MagnetUriHanlder* m_magneturi_handler;
	typedef struct 
	{
		const long evtTag;
		void (MainFrame::*method)( wxCommandEvent &);
	} wxCmdEvtHandlerMap_t;
    static wxCmdEvtHandlerMap_t m_menu_evt_map[];
	typedef struct 
	{
		const long evtTag;
		void (MainFrame::*method)( wxUpdateUIEvent &);
	} wxUIUpdateEvtHandlerMap_t;
	static wxUIUpdateEvtHandlerMap_t m_menu_ui_updater_map[];

	static const long ID_FILE_EXIT;
	static const long ID_HELP_ABOUT;

	static const long ID_FILE_OPEN_URL;
	static const long ID_FILE_OPEN_TORRENT;
	static const long ID_FILE_CREATE_TORRENT;

	static const long ID_VIEW_STATUS;
	static const long ID_VIEW_TOOLBAR;
	static const long ID_VIEW_FAVORITE;
	static const long ID_TORRENT_START;
	static const long ID_TORRENT_FORCE_START;
	static const long ID_TORRENT_PAUSE;
	static const long ID_TORRENT_STOP;
	static const long ID_TORRENT_OPENDIR;
	static const long ID_TORRENT_PROPERTIES;
	static const long ID_TORRENT_REMOVE;
	static const long ID_TORRENT_REMOVEDATA;
	static const long ID_TORRENT_MOVEUP;
	static const long ID_TORRENT_MOVEDOWN;
	static const long ID_TORRENT_REANNOUNCE;
	static const long ID_TORRENT_RECHECK;
	static const long ID_HELP_BITSWASH_HELP;
	static const long ID_HELP_HOMEPAGE;

	/* TIMER */
	static const long ID_TIMER_GUI_UPDATE;
	static const long ID_TIMER_BT_UPDATE;

	static const long ID_OPTIONS_LANGUAGE;
};

#endif // _MAINFRAME_H_
