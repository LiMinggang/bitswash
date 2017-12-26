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

#ifdef __WXMSW__
	#include <WinSock2.h>
	#define PREFIX _T(".")
	//#include "resource.h"
#endif

#include <wx/artprov.h>
#include <wx/aboutdlg.h>
#include <wx/dir.h>
#include <wx/toolbar.h>
#include <wx/image.h>
#include <wx/imaglist.h>
#include <wx/ffile.h>
#include <wx/filefn.h>
#include <wx/filename.h>
#include <wx/mimetype.h>
#include <wx/stdpaths.h>
#include <wx/stream.h>
#include <wx/wfstream.h>
#include <wx/txtstrm.h>
#include <wx/url.h>
#include <wx/tokenzr.h>
#include <wx/aui/auibar.h>
#include <wx/clipbrd.h>
#include <wx/textfile.h>
#include <wx/tokenzr.h>

#ifdef __UNIX_LIKE__
	#include <wx/utils.h>
#endif

#include <libtorrent/entry.hpp>
#include <libtorrent/bencode.hpp>
#include <libtorrent/session.hpp>
#include <libtorrent/fingerprint.hpp>
#include <libtorrent/session_settings.hpp>
#include <libtorrent/identify_client.hpp>
#include <libtorrent/alert_types.hpp>
#include <libtorrent/session_status.hpp>

//plugins
//#include <libtorrent/extensions/metadata_transfer.hpp>
#include <libtorrent/extensions/ut_pex.hpp>

#include "bitswash.h"
#include "mainframe.h"
#include "torrentproperty.h"
#include "functions.h"
#include "urldialog.h"
#include "swashlistctrl.h" //for itemlist_t
#include "magneturi.h"

//namespace lt = libtorrent;
wxDEFINE_EVENT( CHECK_METADATA, wxCommandEvent );

#ifndef __WXMSW__
#include "languages.h"
#else
struct swashlang
{
	wxString lang;
};

wxUniChar g_ConfigSeparator(0x1F);

const struct swashlang languages[] =
{
	{ _T( "en_US" )},
	{ _T( "de_DE" )},
	{ _T( "nb_NO" )},
	{ _T( "ru_RU" )},
	{ _T( "zh_CN" )},
	{ _T( "zh_TW" )},
};
#endif

const long MainFrame::ID_FILE_EXIT = wxID_EXIT;
const long MainFrame::ID_FILE_OPEN_URL = wxNewId();
const long MainFrame::ID_FILE_OPEN_TORRENT = wxNewId();
const long MainFrame::ID_FILE_CREATE_TORRENT = wxNewId();
const long MainFrame::ID_VIEW_STATUS = wxNewId();
const long MainFrame::ID_VIEW_TORRENT_INFO = wxNewId();
const long MainFrame::ID_VIEW_TOOLBAR = wxNewId();
const long MainFrame::ID_VIEW_FAVORITE = wxNewId();
const long MainFrame::ID_TORRENT_START = wxNewId();
const long MainFrame::ID_TORRENT_FORCE_START = wxNewId();
const long MainFrame::ID_TORRENT_PAUSE = wxNewId();
const long MainFrame::ID_TORRENT_STOP = wxNewId();
const long MainFrame::ID_TORRENT_OPENDIR = wxNewId();
const long MainFrame::ID_TORRENT_PROPERTIES = wxNewId();
const long MainFrame::ID_TORRENT_REMOVE = wxNewId();
const long MainFrame::ID_TORRENT_REMOVEDATA = wxNewId();
const long MainFrame::ID_TORRENT_MOVEUP = wxNewId();
const long MainFrame::ID_TORRENT_MOVEDOWN = wxNewId();
const long MainFrame::ID_TORRENT_REANNOUNCE = wxNewId();
const long MainFrame::ID_TORRENT_RECHECK = wxNewId();
const long MainFrame::ID_TORRENT_COPYMAGNETURI = wxNewId();
const long MainFrame::ID_OPTIONS_PREFERENCES = wxNewId();
const long MainFrame::ID_OPTIONS_LANGUAGE = wxNewId();
const long MainFrame::ID_HELP_BITSWASH_HELP = wxNewId();
const long MainFrame::ID_HELP_ABOUT = wxID_ABOUT;
const long MainFrame::ID_HELP_HOMEPAGE = wxNewId();
const long MainFrame::ID_TIMER_GUI_UPDATE = wxNewId();
const long MainFrame::ID_TIMER_BT_UPDATE = wxNewId();

MainFrame::wxCmdEvtHandlerMap_t MainFrame::m_menu_evt_map[]=
{
	{ ID_FILE_CREATE_TORRENT, &MainFrame::ToDo },
	{ ID_FILE_OPEN_TORRENT, &MainFrame::OnMenuOpenTorrent },
	{ ID_FILE_OPEN_URL, &MainFrame::OnMenuOpenTorrentUrl },
	{ ID_FILE_EXIT, &MainFrame::OnQuit },
	{ ID_VIEW_FAVORITE, &MainFrame::ToDo },
	{ ID_VIEW_STATUS, &MainFrame::OnMenuViewStatusBar },
	{ ID_VIEW_TORRENT_INFO, &MainFrame::OnMenuViewTorrentInfo },
	{ ID_VIEW_TOOLBAR, &MainFrame::OnMenuViewToolbar },
	{ ID_TORRENT_START, &MainFrame::OnMenuTorrentStart },
	{ ID_TORRENT_FORCE_START, &MainFrame::OnMenuTorrentStart },
	{ ID_TORRENT_PAUSE, &MainFrame::OnMenuTorrentPause },
	{ ID_TORRENT_STOP, &MainFrame::OnMenuTorrentStop },
	{ ID_TORRENT_PROPERTIES, &MainFrame::OnMenuTorrentProperties },
	{ ID_TORRENT_REMOVE, &MainFrame::OnMenuTorrentRemove },
	{ ID_TORRENT_REMOVEDATA, &MainFrame::OnMenuTorrentRemoveData },
	{ ID_TORRENT_MOVEUP, &MainFrame::OnMenuTorrentMoveUp },
	{ ID_TORRENT_MOVEDOWN, &MainFrame::OnMenuTorrentMoveDown },
	{ ID_TORRENT_REANNOUNCE, &MainFrame::OnMenuTorrentReannounce },
	{ ID_TORRENT_RECHECK, &MainFrame::OnMenuTorrentRecheck },
	{ ID_TORRENT_OPENDIR, &MainFrame::OnMenuTorrentOpenDir },
	{ ID_TORRENT_COPYMAGNETURI, &MainFrame::OnMenuTorrentCopyMagnetUri },
	{ ID_OPTIONS_PREFERENCES, &MainFrame::OnMenuPreferences },
	{ ID_HELP_BITSWASH_HELP, &MainFrame::OnMenuHelp },
	{ ID_HELP_HOMEPAGE, &MainFrame::OnHelpHomepage },
	{ ID_HELP_ABOUT, &MainFrame::OnAbout },
};

MainFrame::wxUIUpdateEvtHandlerMap_t MainFrame::m_menu_ui_updater_map[] =
{
	{ ID_VIEW_STATUS, &MainFrame::OnUpdateUI_MenuViewStatusBar },
	{ ID_VIEW_TORRENT_INFO, &MainFrame::OnUpdateUI_MenuViewTorrentInfo },
	{ ID_VIEW_TOOLBAR, &MainFrame::OnUpdateUI_MenuViewToolbar },

	{ ID_TORRENT_START, &MainFrame::OnUpdateUI_MenuTorrentStart },
	{ ID_TORRENT_FORCE_START, &MainFrame::OnUpdateUI_MenuTorrentStart },
	{ ID_TORRENT_PAUSE, &MainFrame::OnUpdateUI_MenuTorrentPause },
	{ ID_TORRENT_STOP, &MainFrame::OnUpdateUI_MenuTorrentStop },

	{ ID_TORRENT_MOVEUP,     &MainFrame::OnUpdateUI_MenuMoveup     },
	{ ID_TORRENT_MOVEDOWN,   &MainFrame::OnUpdateUI_MenuMovedown   },
	{ ID_TORRENT_OPENDIR,    &MainFrame::OnUpdateUI_MenuOpendir    },
	{ ID_TORRENT_PROPERTIES, &MainFrame::OnUpdateUI_MenuProperties },
	{ ID_TORRENT_REMOVE,     &MainFrame::OnUpdateUI_MenuRemove     },
	{ ID_TORRENT_REMOVEDATA, &MainFrame::OnUpdateUI_MenuRemovedata },

	{ ID_TORRENT_REANNOUNCE, &MainFrame::OnUpdateUI_MenuTorrentReannounce },
	{ ID_TORRENT_RECHECK, &MainFrame::OnUpdateUI_MenuTorrentRecheck },
	{ ID_TORRENT_COPYMAGNETURI, &MainFrame::OnUpdateUI_MenuCopyMagnetUri },
};

MainFrame * g_BitSwashMainFrame = nullptr;

void TorrentMetadataNotify( )
{
	if( g_BitSwashMainFrame )
	{
		g_BitSwashMainFrame->TorrentMetadataReceived();
	}
}

MainFrame::MainFrame( wxFrame *frame, const wxString& title )
	: wxFrame( frame, wxID_ANY, title ),
	  m_btsession( nullptr ), m_torrentmenu( nullptr ), m_languagemenu( nullptr ),
	  m_torrentpane( nullptr ), m_torrentinfo( nullptr ), m_torrentlistctrl( nullptr ),
	  m_peerlistctrl( nullptr ), m_filelistctrl( nullptr ), m_summarypane( nullptr ),
	  m_trackerlistctrl( nullptr ), m_config( nullptr ), m_torrentproperty( nullptr ),
	  m_swashsetting( nullptr ), m_swashstatbar( nullptr ), m_oldlog( nullptr ),
	  m_swashtrayicon( nullptr ),
	  m_upnp_started( false ),
	  m_natpmp_started( false ),
	  m_lsd_started( false ),
	  m_prevlocale( 0 ),
	  m_prevselecteditem( 0 ),
	  m_closed( false )
{
	int x, y;
	m_config = wxGetApp().GetConfig();
	SetClientSize( m_config->GetGui()->w, m_config->GetGui()->h );
	x = m_config->GetGui()->x;
	x = ( x < 0 ? 20 : x );
	y = m_config->GetGui()->y;
	y = ( y < 0 ? 20 : y );
	Move( x, y );
	int w, h;
	GetClientSize( &w, &h );
	m_trayicon.CopyFromBitmap( wxGetApp().GetAppIcon( BITSWASH_ICON_APP256 ).Rescale( 16, 16 ) );
	m_frameicon.CopyFromBitmap( wxGetApp().GetAppIcon( BITSWASH_ICON_APP256 ).Rescale( 64, 64 ) );
	//wxLogMessage(_T("orig %dx%d, traicon %dx%d, frameicon %dx%d"), wxGetApp().GetAppIcon(BITSWASH_ICON_APP256).GetHeight(), wxGetApp().GetAppIcon(BITSWASH_ICON_APP256).GetWidth(), m_trayicon.GetHeight(), m_trayicon.GetWidth(), m_frameicon.GetHeight(), m_frameicon.GetWidth());
	SetIcon( m_frameicon );
#if WXVER >= 280
	m_mgr.SetManagedWindow( this );
#else
	m_mgr.SetFrame( this );
#endif
	SetMenuBar( CreateMainMenuBar() );
#if wxUSE_STATUSBAR
	m_swashstatbar = new SwashStatBar( this );
	SetStatusBar( m_swashstatbar );
#endif // wxUSE_STATUSBAR
	m_btsession = wxGetApp().GetBitTorrentSession();
	CreateToolBar();
	//XXX category
	/*mgr.AddPane(CreateSideTreeCtrl(), AUIPANEINFO.
	              Name(_T("category")).Caption(_("Category")).
	              Left().Layer(1).Position(0).
	              MinimizeButton(true).CloseButton(false));*/

	/* layer 1 put side bar occupied whole vertical zone
	m_mgr.AddPane(CreateSideTreeCtrl(), AUIPANEINFO.
	              Name(_T("category")).Caption(_("Category")).
	              Left().Layer(0).Position(0).
	              MinimizeButton(true).CloseButton(false));
	              */
	CreateTorrentPanel();
	m_mgr.SetFlags( m_mgr.GetFlags() | wxAUI_MGR_ALLOW_ACTIVE_PANE );
//#ifndef _DEBUG //XXX asert failure fired in gtk debug library.
	m_oldlog = wxLog::SetActiveTarget( ( wxLog* )m_torrentinfo->GetLogger() );
//#endif

	if( !wxIsEmpty( m_config->GetAuiPerspective() ) )
	{ m_mgr.LoadPerspective( m_config->GetAuiPerspective() ); }

	m_mgr.Update();
	ShowSystray( m_config->GetUseSystray() );
	UpdateUI();
	//Refresh timer
	WXLOGDEBUG(( _T( "Refresh timer %d\n" ), m_config->GetRefreshTime() ));
	m_refreshtimer.SetOwner( this, ID_TIMER_GUI_UPDATE );
	m_refreshtimer.Start( m_config->GetRefreshTime() * 1000, wxTIMER_CONTINUOUS );

	for(size_t i = 0; i < sizeof(m_menu_evt_map)/sizeof(m_menu_evt_map[0]); ++i)
	{
		Bind( wxEVT_MENU, m_menu_evt_map[i].method, this, m_menu_evt_map[i].evtTag );
	}

	for(size_t i = 0; i < sizeof(m_menu_ui_updater_map)/sizeof(m_menu_ui_updater_map[0]); ++i)
	{
		Bind( wxEVT_UPDATE_UI, m_menu_ui_updater_map[i].method, this, m_menu_ui_updater_map[i].evtTag );
	}

	Bind( CHECK_METADATA, &MainFrame::OnTorrentMetadata, this );
	//Bind( wxEVT_MENU_OPEN, &MainFrame::OnMenuOpen, this );
	
	Bind( wxEVT_SIZE, &MainFrame::OnSize, this );
	Bind( wxEVT_MOVE, &MainFrame::OnMove, this );
	Bind( wxEVT_TIMER, &MainFrame::OnRefreshTimer, this, ID_TIMER_GUI_UPDATE );

	Bind(wxEVT_CLOSE_WINDOW, &MainFrame::OnClose, this );
	Bind( wxEVT_ICONIZE, &MainFrame::OnMinimize, this );
	//Refresh torrent list on timer
	//
}

MainFrame::~MainFrame()
{
	//free members
	if( m_config->GetUseSystray() )
	{ delete m_swashtrayicon; }

	delete m_config;

#ifndef __WXDEBUG__
	//wxLog::SetActiveTarget(m_oldlog);
#endif
}

void MainFrame::OnClose( wxCloseEvent& event )
{
	if( m_closed ) { return; }

	WXLOGDEBUG(( _T( "MainFrame Closing\n" ) ));
	//stop update timer
	m_refreshtimer.Stop();
	m_config->SetTorrentListCtrlSetting( m_torrentlistctrl->Settings() );
	m_config->SetPeerListCtrlSetting( m_peerlistctrl->Settings() );
	m_config->SetFileListCtrlSetting( m_filelistctrl->Settings() );
	m_config->SetTrackerListCtrlSetting( m_trackerlistctrl->Settings() );
	//saving config
	m_config->SetAuiPerspective( m_mgr.SavePerspective() );
	m_config->Save();
	m_mgr.UnInit();

	if( m_swashsetting ) { delete m_swashsetting; }

	Destroy();
	m_closed = true;
}

void MainFrame::OnMinimize( wxIconizeEvent& event )
{
	if( m_config->GetUseSystray() && m_config->GetHideTaskbar() )
	{
		if( event.IsIconized() )
		{
			Show( false );
		}
		else
		{
			Show( true );
		}
	}
}

void MainFrame::SaveFramePosition()
{
	//save window
	int x, y;
	GetPosition( &x, &y );
	m_config->SetGui( x, y, m_config->GetGui()->w, m_config->GetGui()->h );
	//m_config->Save();
}

void MainFrame::SaveFrameSize()
{
	//save window
	int w, h;
	GetClientSize( &w, &h );
	m_config->SetGui( m_config->GetGui()->x, m_config->GetGui()->y, w, h );
	//m_config->Save();
}

void MainFrame::OnSize( wxSizeEvent& event )
{
	SaveFrameSize();
}

void MainFrame::OnMove( wxMoveEvent& event )
{
	SaveFramePosition();
}

wxTreeCtrl* MainFrame::CreateSideTreeCtrl()
{
	wxTreeCtrl* tree = new wxTreeCtrl( this, wxID_ANY,
									   wxPoint( 0, 0 ), wxSize( 160, 250 ),
									   wxTR_DEFAULT_STYLE | wxNO_BORDER );
	wxImageList* imglist = new wxImageList( 16, 16, true, 2 );
	imglist->Add( wxArtProvider::GetBitmap( wxART_FOLDER, wxART_OTHER, wxSize( 16, 16 ) ) );
	imglist->Add( wxArtProvider::GetBitmap( wxART_NORMAL_FILE, wxART_OTHER, wxSize( 16, 16 ) ) );
	tree->AssignImageList( imglist );
	wxTreeItemId root = tree->AddRoot( _( "Categories" ), 0 );
	wxArrayTreeItemIds items;
	items.Add( tree->AppendItem( root, _( "All" ), 0 ) );
	items.Add( tree->AppendItem( root, _( "Downloads" ), 0 ) );
	items.Add( tree->AppendItem( root, _( "Seeds" ), 0 ) );
	tree->Expand( root );
	return tree;
}

void MainFrame::ToDo( wxCommandEvent& event )
{
	wxMessageBox( _T( "ToDo" ), _T( "Bitswash" ), wxICON_INFORMATION | wxOK, this );
}

void MainFrame::OnQuit( wxCommandEvent& event )
{
	Close();
}

void MainFrame::OnMenuTorrentOpenDir( wxCommandEvent& event )
{
	std::shared_ptr<torrent_t> pTorrent = GetSelectedTorrent();

	if( pTorrent )
	{
		//XXX windows might needs special treatment here!
#if  defined(__WXMSW__)
		wxExecute( _T( "Explorer " ) + pTorrent->config->GetDownloadPath() + wxConvUTF8.cMB2WC(pTorrent->info->name().c_str() ) + wxFileName::GetPathSeparator(), wxEXEC_ASYNC, nullptr );
#elif defined(__APPLE__)
		wxExecute( _T( "/usr/bin/open ") + pTorrent->config->GetDownloadPath() + wxConvUTF8.cMB2WC(pTorrent->info->name().c_str() ) + wxFileName::GetPathSeparator(), wxEXEC_ASYNC, nullptr );
#elif defined(__WXGTK__)
		wxString loc = _T( "file://" ) + pTorrent->config->GetDownloadPath() + wxConvUTF8.cMB2WC(pTorrent->info->name().c_str() ) + wxFileName::GetPathSeparator();
		//SystemOpenURL( loc );
		wxLaunchDefaultBrowser( loc );
#endif
	}
}

void MainFrame::OnAbout( wxCommandEvent& event )
{
	wxAboutDialogInfo info;
	info.SetName( _T( "Bitswash-Mod" ) );
	info.SetVersion( BITSWASH_VERSION );
	info.SetDescription( _( "Bittorrent Client" ) );
	info.SetCopyright( _T( "(c) 2017 Bitswash Developers" ) );
	info.SetWebSite( _T( "https://github.com/LiMinggang/bitswash" ), _( "Bitswash Homepage" ) );
	info.SetLicence( _T( " \n\n \
 This program is free software; you can redistribute it and/or modify\n \
 it under the terms of the GNU General Public License as published by\n \
 the Free Software Foundation; either version 3 of the License, or\n \
 (at your option) any later version.\n\n \
 This program is distributed in the hope that it will be useful,\n \
 but WITHOUT ANY WARRANTY; without even the implied warranty of\n \
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n \
 GNU Library General Public License for more details.\n\n \
 You should have received a copy of the GNU General Public License\n \
 along with this program; if not, write to the Free Software\n \
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.\n \
" ) );
	info.AddDeveloper( _T( "Minggang Li <minggang.li@gmail.com>" ) );
	info.AddArtist( _T( "elijah <elijah@riseup.net>" ) );
	info.AddDeveloper( _T( "\nMisc-----------errno0 <errno0@gmail.com>" ) );
	//XXX make this stuff autogenerate in credit.h or whatever.
	//we could parse .po file and grab author info from there.
	info.AddTranslator( _T( "de_DE--------Vinzenz Vietzke <vinz@v1nz.de>" ) );
	info.AddTranslator( _T( "\nnb_NO--------Andreas <core12@gmail.com>" ) );
	info.AddTranslator( _T( "\nru_RU--------Osipov Igor <oival@yandex.ru>" ) );
	info.AddTranslator( _T( "\nzh_CN--------Aarons Wang Shi <aarons.wang@gmail.com>" ) );
	info.AddTranslator( _T( "\nzh_TW--------Aarons Wang Shi <aarons.wang@gmail.com>" ) );
	wxAboutBox( info );
}

void MainFrame::OnMenuHelp( wxCommandEvent& event )
{
	SystemOpenURL( wxString(BITSWASH_HELP_URL ));
}

void MainFrame::OnHelpHomepage( wxCommandEvent& event )
{
	SystemOpenURL(wxString(BITSWASH_HOMEPAGE ));
}

wxMenuBar* MainFrame::CreateMainMenuBar()
{
	// create a menu bar
	wxMenuBar* mbar = new wxMenuBar();
	wxMenu* fileMenu = new wxMenu( wxT( "" ) );
	wxMenuItem* menuitem;
	menuitem = fileMenu->Append( ID_FILE_OPEN_TORRENT, _( "&Open Torrent\tCtrl+O" ), _( "Open Torrent" ) );
	menuitem->SetBitmap( wxBitmap( wxGetApp().GetAppIcon( BITSWASH_ICON_TORRENT_OPEN ) ) );
	menuitem = fileMenu->Append( ID_FILE_OPEN_URL, _( "Open &URL\tCtrl+U" ), _( "Open URL" ) );
	menuitem->SetBitmap( wxBitmap( wxGetApp().GetAppIcon( BITSWASH_ICON_TORRENT_OPENURL ) ) );
	//menuitem = fileMenu->Append(ID_FILE_OPEN_URL, _("&Create Torrent\tCtrl+C"), _("Create Torrent"));
	//menuitem->SetBitmap(wxBitmap(wxGetApp().GetAppIcon(BITSWASH_ICON_TORRENT_CREATE)));
	//
	fileMenu->AppendSeparator();
	menuitem = fileMenu->Append( ID_FILE_EXIT, _( "E&xit\tAlt-F4" ), _( "Exit Bitswash" ) );
	menuitem->SetBitmap( wxArtProvider::GetBitmap( wxART_QUIT, wxART_OTHER, wxSize( 16, 16 ) ) );
	mbar->Append( fileMenu, _( "&File" ) );
	wxMenu* viewMenu = new wxMenu( wxT( "" ) );
	menuitem = viewMenu->Append( ID_VIEW_TOOLBAR, _( "View &Toolbar" ), _( "View Toolbar" ), wxITEM_CHECK );
	menuitem = viewMenu->Append( ID_VIEW_STATUS, _( "View &Status" ), _( "View Status" ) , wxITEM_CHECK);
	menuitem = viewMenu->Append( ID_VIEW_TORRENT_INFO, _( "View Torrent &Information" ), _( "View Torrent Information" ) , wxITEM_CHECK);
	//menuitem = viewMenu->Append(ID_VIEW_FAVORITE, _("View &Categories"), _("View Categories"));
	mbar->Append( viewMenu, _( "&View" ) );
	m_torrentmenu = GetNewTorrentMenu();
	mbar->Append( m_torrentmenu, _( "&Torrent" ) );
	wxMenu* optionsMenu = new wxMenu( wxEmptyString );
	menuitem = optionsMenu->Append( ID_OPTIONS_PREFERENCES, _( "Prefere&nce\tCtrl+F" ), _( "Preference" ) );
	menuitem->SetBitmap( wxBitmap( wxGetApp().GetAppIcon( BITSWASH_ICON_OPTION_PREFERENCE ) ) );
	m_languagemenu = new wxMenu( wxEmptyString );
	const struct swashlang* availlang = languages;
	int i = 0;
	wxMenuItem* tmpitem1;
	tmpitem1 = m_languagemenu->AppendRadioItem( ID_OPTIONS_LANGUAGE, _( "System Language" ) );

	if( m_config->GetLanguage() == _T( "" ) )
	{
		tmpitem1->Check( true );
		m_prevlocale = wxLANGUAGE_DEFAULT;
	}

	m_languagemenu->AppendSeparator();

	while( i < ( sizeof( languages ) / sizeof( struct swashlang ) ) )
	{
		wxString strlang =  availlang[i].lang;

		if( strlang != _T( "" ) )
		{
			const wxLanguageInfo * const langinfo = wxLocale::FindLanguageInfo( strlang );

			if( langinfo == nullptr )
			{
				wxLogError( _T( "Language %s is not available\n" ), strlang.c_str() );
				i++;
				continue;
			}

			int langmenuid = ID_OPTIONS_LANGUAGE + langinfo->Language;
			tmpitem1 = m_languagemenu->AppendRadioItem( langmenuid, langinfo->Description );

			if( m_config->GetLanguage() == strlang )
			{
				tmpitem1->Check( true );
				m_prevlocale = langinfo->Language;
			}
		}

		i++;
	}

	Bind(wxEVT_COMMAND_MENU_SELECTED, &MainFrame::OnMenuOptionLanguage, this, ID_OPTIONS_LANGUAGE, ID_OPTIONS_LANGUAGE + wxLANGUAGE_USER_DEFINED);

	optionsMenu->Append( wxID_ANY, _( "&Language" ), m_languagemenu,  _( "Language" ) );
	mbar->Append( optionsMenu, _( "&Options" ) );
	wxMenu* helpMenu = new wxMenu( wxEmptyString );
	helpMenu->Append( ID_HELP_BITSWASH_HELP, _( "&Help\tF1" ), _( "Help" ) );
	helpMenu->AppendSeparator();
	helpMenu->Append( ID_HELP_HOMEPAGE, _( "Home&page" ), _( "Homepage" ) );
	helpMenu->Append( ID_HELP_ABOUT, _( "&About" ), _( "About" ) );
	mbar->Append( helpMenu, _( "&Help" ) );
	return mbar;
}

//#define BITSWASH_TOOLBAR_FLAGS (wxNO_BORDER | wxTB_HORIZONTAL | wxTB_3DBUTTONS | wxTB_FLAT)
#define BITSWASH_TOOLBAR_FLAGS (wxAUI_TB_HORIZONTAL | wxAUI_TB_HORZ_LAYOUT | wxAUI_TB_PLAIN_BACKGROUND | wxAUI_TB_GRIPPER)
void MainFrame::CreateToolBar()
{
	wxAuiToolBar* mainToolBar = new wxAuiToolBar( this, wxID_ANY, wxDefaultPosition, wxSize( 392, 29 ), BITSWASH_TOOLBAR_FLAGS );
	mainToolBar->SetToolBitmapSize( wxSize( 16, 16 ) );
	mainToolBar->AddTool( ID_FILE_OPEN_TORRENT, _( "Open Torrent" ),
						  wxBitmap( wxGetApp().GetAppIcon( BITSWASH_ICON_TORRENT_OPEN ) ), wxNullBitmap, wxITEM_NORMAL,
						  _( "Open torrent file" ), _( "Open torrent file" ), 0 );
	mainToolBar->AddTool( ID_FILE_OPEN_URL, _( "Open URL" ),
						  wxBitmap( wxGetApp().GetAppIcon( BITSWASH_ICON_TORRENT_OPENURL ) ), wxNullBitmap, wxITEM_NORMAL,
						  _( "Open torrent from url" ), _( "Open torrent from URL" ), 0 );
	//mainToolBar->AddSeparator();
#if 0 //TODO Create Torrent
	mainToolBar->AddTool( ID_FILE_CREATE_TORRENT, _( "Create Torrent" ),
						  wxBitmap( wxGetApp().GetAppIcon( BITSWASH_ICON_TORRENT_CREATE ), wxNullBitmap, wxITEM_NORMAL,
									_( "Create torrent file" ), _( "Create torrent file to publish" ) );
#endif
	mainToolBar->Realize();
	wxAuiToolBar* torrentToolBar = new wxAuiToolBar( this, wxID_ANY, wxDefaultPosition, wxSize( 392, 29 ), BITSWASH_TOOLBAR_FLAGS );
	torrentToolBar->SetToolBitmapSize( wxSize( 16, 16 ) );
	torrentToolBar->AddTool( ID_TORRENT_START, _( "Queue" ),
		  wxBitmap( wxGetApp().GetAppIcon( BITSWASH_ICON_TORRENT_START ) ), wxNullBitmap, wxITEM_NORMAL,
		  _( "Queue torrent" ), _( "Queue torrent" ), 0 );
	torrentToolBar->AddTool( ID_TORRENT_PAUSE, _( "Pause" ),
		  wxBitmap( wxGetApp().GetAppIcon( BITSWASH_ICON_TORRENT_PAUSE ) ), wxNullBitmap, wxITEM_NORMAL,
		  _( "Pause download" ), _( "Pause downlaoding" ), 0 );
	torrentToolBar->AddTool( ID_TORRENT_STOP, _( "stop" ),
		  wxBitmap( wxGetApp().GetAppIcon( BITSWASH_ICON_TORRENT_STOP ) ), wxNullBitmap, wxITEM_NORMAL,
		  _( "Stop download" ), _( "Stop downlaoding" ), 0 );
	torrentToolBar->AddSeparator();
	torrentToolBar->AddTool( ID_TORRENT_REMOVE, _( "Delete" ),
		  wxBitmap( wxGetApp().GetAppIcon( BITSWASH_ICON_TORRENT_REMOVE ) ), wxNullBitmap, wxITEM_NORMAL,
		  _( "Delete torrent file" ), _( "Delete torrent file" ), 0 );
	torrentToolBar->AddTool( ID_TORRENT_REMOVEDATA, _( "Delete Data" ),
		  wxBitmap( wxGetApp().GetAppIcon( BITSWASH_ICON_TORRENT_REMOVEDATA ) ), wxNullBitmap, wxITEM_NORMAL,
		  _( "Delete torrent file and downloaded data" ), _( "Delete torrent file and downloaded data" ), 0 );
	torrentToolBar->AddSeparator();
	torrentToolBar->AddTool( ID_TORRENT_MOVEUP, _( "Move Up" ),
		  wxArtProvider::GetBitmap( wxART_GO_UP, wxART_OTHER, wxSize( 16, 16 ) ),
		  wxNullBitmap, wxITEM_NORMAL,
		  _( "Move torrent up" ), _( "Move torrent up in the queue position" ), 0 );
	torrentToolBar->AddTool( ID_TORRENT_MOVEDOWN, _( "Move Down" ),
		  wxArtProvider::GetBitmap( wxART_GO_DOWN, wxART_OTHER, wxSize( 16, 16 ) ),
		  wxNullBitmap, wxITEM_NORMAL,
		  _( "Move torrent down" ), _( "Move torrent down in the queue position" ), 0 );
	torrentToolBar->AddSeparator();
	torrentToolBar->AddTool( ID_TORRENT_OPENDIR, _( "Open" ),
		  wxBitmap( wxGetApp().GetAppIcon( BITSWASH_ICON_TORRENT_OPENDIR ) ), wxNullBitmap, wxITEM_NORMAL,
		  _( "Open destination directory" ), _( "Open destination downloaded directory" ), 0 );
	torrentToolBar->Realize();
	wxAuiToolBar* miscToolBar = new wxAuiToolBar( this, wxID_ANY, wxDefaultPosition, wxSize( 392, 29 ), BITSWASH_TOOLBAR_FLAGS );
	miscToolBar->SetToolBitmapSize( wxSize( 16, 16 ) );
	miscToolBar->AddTool( ID_OPTIONS_PREFERENCES, _( "Preference" ),
		  wxBitmap( wxGetApp().GetAppIcon( BITSWASH_ICON_OPTION_PREFERENCE ) ), wxNullBitmap, wxITEM_NORMAL,
		  _( "Program settings" ), _( "Set user preference" ), 0 );
	miscToolBar->AddTool( ID_HELP_HOMEPAGE, _( "Homepage" ),
		  wxBitmap( wxGetApp().GetAppIcon( BITSWASH_ICON_APP ) ), wxNullBitmap, wxITEM_NORMAL,
		  _( "Bitswash homepage" ), _( "Visit Bitswash homepage" ), 0 );
	miscToolBar->AddSeparator();
	miscToolBar->AddTool( ID_FILE_EXIT, _( "Exit" ),
		  wxArtProvider::GetBitmap( wxART_QUIT, wxART_OTHER, wxSize( 16, 16 ) ), wxNullBitmap, wxITEM_NORMAL,
		  _( "Exit Bitswash" ), _( "Exit Bitswash" ), 0 );
	miscToolBar->Realize();
	// add the toolbars to the manager
	m_mgr.AddPane( mainToolBar, AUIPANEINFO.
				 Name( wxT( "mainToolBar" ) ).Caption( wxT( "Main ToolBar" ) ).
				 ToolbarPane().Top().
				 LeftDockable( false ).RightDockable( false ) );
	m_mgr.AddPane( torrentToolBar, AUIPANEINFO.
				 Name( wxT( "torrentToolBar" ) ).Caption( wxT( "Torrent ToolBar" ) ).
				 ToolbarPane().Top().Row( 0 ).Position( 1 ).
				 LeftDockable( false ).RightDockable( false ) );
	m_mgr.AddPane( miscToolBar, AUIPANEINFO.
				 Name( wxT( "miscToolBar" ) ).Caption( wxT( "Miscellanous" ) ).
				 ToolbarPane().Top().Row( 0 ).Position( 2 ).
				 LeftDockable( false ).RightDockable( false ) );
	m_mgr.Update();
}

void MainFrame::CreateTorrentPanel()
{
	wxSize client_size = GetClientSize();
	CreateTorrentList();
	m_mgr.AddPane( m_torrentlistctrl, AUIPANEINFO.Name( wxT( "Torrent List" ) ).
				   CenterPane().PaneBorder( false ).Show() );
	CreateTorrentInfo();
	m_mgr.AddPane( m_torrentinfo, AUIPANEINFO.Name( wxT( "Torrent Information" ) ).
				   Bottom().BestSize( wxSize( client_size.GetWidth(), 250 ) ).PaneBorder( false ).Floatable( false ).Gripper( false ).
				   CloseButton( false ).MinimizeButton( true ).CaptionVisible( false ) );
	m_peerlistctrl = m_torrentinfo->GetPeerListCtrl();
	m_filelistctrl = m_torrentinfo->GetFileListCtrl();
	m_summarypane = m_torrentinfo->GetSummaryPane();
	m_trackerlistctrl = m_torrentinfo->GetTrackerListCtrl();
}

/* bottom part, tabs of torrent information pane; */
void MainFrame::CreateTorrentInfo()
{
	wxSize client_size = GetClientSize();
	m_torrentinfo = new TorrentInfo( this, TorrentInfo::TORRENT_INFO_ID,
									 wxDefaultPosition,
									 wxSize( client_size.GetWidth(), -1 ),
									 wxAUI_NB_TOP|wxNO_BORDER|wxTAB_TRAVERSAL
								   );
}

/* torrents queue display area (main area) */
void MainFrame::CreateTorrentList()
{
	long flags = wxLC_REPORT | wxLC_VIRTUAL | wxSUNKEN_BORDER | wxVSCROLL;
	m_torrentlistctrl = new TorrentListCtrl( this, m_config->GetTorrentListCtrlSetting(), wxID_ANY,
			wxDefaultPosition, wxDefaultSize,
			flags );
}

void MainFrame::ReceiveTorrent( wxString fileorurl )
{
	if( g_BitSwashMainFrame )
	{
		wxStringTokenizer tkz( fileorurl, g_ConfigSeparator );
		wxString filename;

		while( tkz.HasMoreTokens() )
		{
			filename = tkz.GetNextToken();
			MagnetUri mag(filename);
			if(mag.isValid())
				g_BitSwashMainFrame->OpenMagnetURI(filename);
			else
				g_BitSwashMainFrame->AddTorrent( filename, true );
		}
	}
}

void MainFrame::TorrentMetadataReceived( )
{
	wxCommandEvent event( CHECK_METADATA );
	event.SetEventObject( this );
	AddPendingEvent( event );
}

/* parse torrent file and add to session */
void MainFrame::AddTorrent( wxString filename, bool usedefault )
{
	WXLOGDEBUG(( _T( "MainFrame: Add Torrent: %s\n" ), filename.c_str() ));
	std::shared_ptr<torrent_t> torrent = m_btsession->ParseTorrent( filename );

	if( torrent && torrent->isvalid )
	{
		int idx = m_btsession->FindTorrent( wxString(torrent->hash) );

		if( idx < 0 )
		{
			if( !m_config->GetUseDefault() )
			{
				TorrentProperty torrent_property_dlg( torrent, this, wxID_ANY );

				if( torrent_property_dlg.ShowModal() != wxID_OK )
				{
					wxLogMessage( _T( "Canceled by user" ) );
					wxString configfile(wxGetApp().SaveTorrentsPath() + wxString(torrent->hash) + _T( ".conf" ));
					if( ( wxFileExists( configfile ) ) &&
							( !wxRemoveFile( configfile ) ) )
					{
						wxLogError( _T( "Error removing file %s" ), configfile.c_str() );
					}
					return;
				}
			}

			if(torrent->info)
			{
				const lt::file_storage &allfiles = torrent->info->files();
				std::vector<lt::download_priority_t>& filespriority = torrent->config->GetFilesPriorities();
				int nfiles = torrent->info->num_files();
				wxASSERT(nfiles > 0);
				if (filespriority.size() != nfiles)
				{
					std::vector<lt::download_priority_t> deffilespriority( nfiles, TorrentConfig::file_normal );
					filespriority.swap(deffilespriority);
				}

				wxString fname;
				bool downloaded = false;
				for(int i = 0; i < nfiles; ++i)
				{
					lt::file_index_t idx(i);
					fname = (torrent->config->GetDownloadPath() + wxString::FromUTF8((allfiles.file_path(idx)).c_str()));
					if(filespriority[i] != TorrentConfig::file_none && ( wxFileExists( fname ) ))
					{
						downloaded = true;
						break;
					}
				}

				if (downloaded && (wxNO == ::wxMessageBox(_("It seemed that the torrent had been downloaded before. Are you going to download it again?"), _("Downloading"), wxYES_NO | wxICON_QUESTION)))
				{
					wxLogMessage( _T( "Canceled by user" ) );
					
					wxString configfile(wxGetApp().SaveTorrentsPath() + wxString(torrent->hash) + _T( ".conf" ));
					if( ( wxFileExists( configfile ) ) &&
							( !wxRemoveFile( configfile ) ) )
					{
						wxLogError( _T( "Error removing file %s" ), configfile.c_str() );
					}
					return;
				}
			}

			if( m_btsession->AddTorrent( torrent ) )
			{
				wxString torrent_backup = wxGetApp().SaveTorrentsPath() + wxString(torrent->hash) + _T( ".torrent" );

				/* save a copy of torrent file */
				if( !wxCopyFile( filename, torrent_backup, true ) )
				{
					wxLogError( _T( "Error copying backup file %s\n" ), filename.c_str() );
				}

				UpdateUI();
			}
		}
		else
		{
			m_torrentlistctrl->Select( idx, true );
			int answer = wxMessageBox( _("Torrent Exists. Do you want to update trackers/seeds from the torrent?"), _("Confirm"), wxYES_NO, this );

			if( answer == wxYES )
			{
				std::shared_ptr<torrent_t> tmp_torrent = m_btsession->GetTorrent(idx);
				wxASSERT(tmp_torrent);
				m_btsession->MergeTorrent( tmp_torrent, torrent );
			}
		}
	}
}

/* display file open dialog and choose torrent files to be added
 * multiple selection is allow
 */
void MainFrame::OpenTorrent()
{
	wxString wildcards =
#ifdef __WXMOTIF__
		_T( "Torrent files (*.torrent)|*.torrent|All files (*)|*" );
#else
		wxString::Format(
			_T( "Torrent files (*.torrent)|*.torrent|All files (%s)|%s" ),
			wxFileSelectorDefaultWildcardStr,
			wxFileSelectorDefaultWildcardStr
		);
#endif
	wxFileDialog file_dialog( this, _( "Open Torrent(s)..." ),
							  wxEmptyString, wxEmptyString, wildcards,
							  wxFD_OPEN | wxFD_MULTIPLE | wxFD_CHANGE_DIR );

	if( file_dialog.ShowModal() == wxID_OK )
	{
		wxArrayString paths, filenames;
		file_dialog.GetPaths( paths );
		file_dialog.GetFilenames( filenames );
		size_t count = paths.GetCount();

		for( size_t n = 0; n < count; n++ )
		{
			//add each torrent here
			WXLOGDEBUG(( wxT( "File %d: %s (%s)\n" ),
						( int )n, paths[n].c_str(), filenames[n].c_str() ));
			AddTorrent( paths[n], false );
		}
		if(count) m_btsession->PostQueueUpdateEvent();
	}
}

void MainFrame::OnTorrentMetadata( wxCommandEvent& WXUNUSED( event ) )
{
	metadata_t mdq;
	m_btsession->GetPendingMetadata(mdq);
	if(!mdq.empty())
	{
		std::shared_ptr<torrent_t> torrent;
		for(auto& hash : mdq)
		{
			torrent = m_btsession->GetTorrent( hash );
			if(torrent && torrent->isvalid)
			{
				m_btsession->UpdateTorrentFileSize(torrent);
				
				if(torrent->config->GetTrackersURL().size() <= 0 )
				{
					std::vector<lt::announce_entry> trackers = torrent->handle.trackers();
					torrent->config->SetTrackersURL( trackers );
				}

				if( torrent->handle.status().has_metadata )
				{
					wxString torrent_backup = wxGetApp().SaveTorrentsPath() + wxString(torrent->hash) + _T( ".torrent" );
					m_btsession->SaveTorrent( torrent, torrent_backup );
				}

				if( !m_config->GetUseDefault() )
				{
					TorrentProperty torrent_property_dlg( torrent, this, wxID_ANY );
				
					if( torrent_property_dlg.ShowModal() != wxID_OK )
					{
						wxLogMessage( _T( "Canceled by user" ) );

						wxString configfile(wxGetApp().SaveTorrentsPath() + wxString(torrent->hash) + _T( ".conf" ));
						if( ( wxFileExists( configfile ) ) &&
								( !wxRemoveFile( configfile ) ) )
						{
							wxLogError( _T( "Error removing file %s" ), configfile.c_str() );
						}
						return;
					}
					
				}

				torrent->handle.unset_flags(lt::torrent_flags::upload_mode);
				//m_btsession->StartTorrent(torrent, false);
				m_btsession->PostQueueUpdateEvent();
				UpdateUI();
			}
		}
	}
}

//
//
//File Menu
void MainFrame::OnMenuOpenTorrent( wxCommandEvent& WXUNUSED( event ) )
{
	OpenTorrent();
}

void MainFrame::OpenMagnetURI(const wxString & magneturi)
{
	MagnetUri magnetUri( magneturi );

	if( magnetUri.isValid() )
	{
		int idx = m_btsession->FindTorrent( magnetUri.hash() );

		if( idx < 0 )
		{
			std::shared_ptr<torrent_t> torrent = m_btsession->LoadMagnetUri( magnetUri );

			if( torrent && torrent->isvalid )
			{
				if( torrent->handle.status().has_metadata )
				{
					wxString torrent_backup = wxGetApp().SaveTorrentsPath() + wxString(torrent->hash) + _T( ".torrent" );
					m_btsession->SaveTorrent( torrent, torrent_backup );
				}

				m_btsession->PostQueueUpdateEvent();
				UpdateUI();
			}
			else
			{
				wxLogMessage( _T( "Load Magnet URI error" ) );
			}
		}
		else
		{
			m_torrentlistctrl->Select( idx, true );
			int answer = wxMessageBox( _("Torrent Exists. Do you want to update trackers/seeds from the torrent?"), _("Confirm"), wxYES_NO, this );

			if( answer == wxYES )
			{
				std::shared_ptr<torrent_t> torrent = m_btsession->GetTorrent(idx);
				wxASSERT(torrent);
				m_btsession->MergeTorrent( torrent, magnetUri );
			}
		}
	}
	else
	{
		wxLogMessage( _T( "Not Supported\n" ) );
	}
}

void MainFrame::DownloadTorrent(wxURL & url)
{
	if(url.GetError() == wxURL_NOERR )
	{
		wxInputStream *in_stream = url.GetInputStream();
	
		if( in_stream && in_stream->IsOk() )
		{
			wxString contenttype = url.GetProtocol().GetContentType();
			size_t s_size = in_stream->GetSize();
			WXLOGDEBUG(( _T( "openurl: type %s, size %d\n" ), contenttype.c_str(), s_size ));
			wxString tmpfile = wxFileName::CreateTempFileName( _T( "bitswash" ) );
			WXLOGDEBUG(( _T( "openurl: Tmp file %s\n" ), tmpfile.c_str() ));
			{
				/* write downloaded stream in to temporary file */
				wxFileOutputStream fos( tmpfile );
	
				if( !fos.Ok() )
				{
					wxLogError( _T( "Error creating temporary file %s\n" ), tmpfile.c_str() );
					return;
				}
	
				static char buffer[4096];
				memset( buffer, 0, 4096 );
				int counter = 0;
	
				while( !in_stream->Eof() && in_stream->CanRead() )
				{
					in_stream->Read( buffer, 4096 );
					size_t size = in_stream->LastRead();
	
					if( size == 0 )
					{ break; }
	
					fos.Write( buffer, size );
					counter += size;
				}
			}
			/* everythings seems fine
			 * add torrent to session
			 */
			AddTorrent( tmpfile, false );
			m_btsession->PostQueueUpdateEvent();
			
		}
	}
}

/* open torrent from web url
 * XXX more verbose downloading (progress bar, cancel button etc)
 */
void MainFrame::OpenTorrentUrl()
{
	UrlDialog urldialog( this );

	if( urldialog.ShowModal() == wxID_OK )
	{
		wxString urls = urldialog.m_textURL->GetValue(), url, strDelimiters = _T( " \t\r\n" );
        wxStringTokenizer tkz( urls, strDelimiters );

        while( tkz.HasMoreTokens() )
	    {
    		url = tkz.GetNextToken();
    		wxURL torrenturl( url );
			WXLOGDEBUG((_T("openurl: Fetch URL %s\n"), url.c_str()));

    		if( isUrl( torrenturl.GetScheme() ) )
    		{
    			DownloadTorrent(torrenturl);
    		}
    		else
    		{
    			OpenMagnetURI(url);
    		}
        }
	}
	else
	{
		wxLogMessage( _( "Download cancelled" ) );
	}
}

void MainFrame::OnMenuOpenTorrentUrl( wxCommandEvent& WXUNUSED( event ) )
{
	OpenTorrentUrl();
}

void MainFrame::OnRefreshTimer( wxTimerEvent& WXUNUSED( event ) )
{
	//GetTorrentLog();
	m_btsession->PostStatusUpdate();

	UpdateUI();
}

void MainFrame::OnListItemClick( long item )
{
	if( m_prevselecteditem == item )
	{ return; }

	m_prevselecteditem = item;
	UpdateUI(true);
}

void MainFrame::UpdateUI(bool force/* = false*/)
{
	wxASSERT( m_btsession != nullptr );
	WXLOGDEBUG(( _T( "UpdateUI\n" )));
	static std::shared_ptr<torrent_t> invalid_torrent;

	if(force || (this->IsShown() && !this->IsIconized()))
	{
		size_t torrent_queue_size = m_btsession->GetTorrentQueueSize();
		//int t = 0;
		//torrents_t *torrentqueue = m_btsession->GetTorrentQueue();
		///torrents_t::const_iterator torrent_it = torrentqueue->begin();
		/* XXX more filtering/sorting */
		WXLOGDEBUG(( _T( "Refreshing torrent list size %s\n" ), ( wxLongLong( torrent_queue_size ).ToString() ).c_str() ));
		m_torrentlistctrl->SetItemCount( torrent_queue_size );

		if(m_mgr.GetPane( m_torrentinfo ).IsShown())
		{
			if( torrent_queue_size > 0 )
			{
				SwashListCtrl::itemlist_t selecteditems;
				m_torrentlistctrl->GetSelectedItems(selecteditems);

				/* selected more than 1 item in the torrent list */
				if( selecteditems.size() > 0 )
				{
					std::shared_ptr<torrent_t> torrent = m_btsession->GetTorrent( selecteditems[0] );
					if(!torrent || !torrent->isvalid)
					{
						m_filelistctrl->SetStaticHandle(invalid_torrent);
						m_filelistctrl->SetItemCount( 0 );
						m_trackerlistctrl->SetStaticHandle(invalid_torrent);
						m_trackerlistctrl->SetItemCount( 0 );
						m_peerlistctrl->SetItemCount( 0 );
						
						m_torrentinfo->UpdateTorrentInfo( false );
						m_torrentlistctrl->Refresh(false);
						UpdateStatusBar();
						return;
					}

					lt::torrent_handle &torrent_handle = torrent->handle;
					WXLOGDEBUG(( _T( "MainFrame: list size %s selected items %s\n" ), ( wxLongLong( torrent_queue_size ).ToString() ).c_str(), ( wxLongLong( selecteditems.size() ).ToString() ).c_str() ));

					/* if torrent is started in libtorrent */
					if( torrent_handle.is_valid() )
					{
						/* get peer list */
						{
							torrent_handle.get_peer_info( m_peerlistitems );
							m_peerlistctrl->SetItemCount( m_peerlistitems.size() );
						}
					}
					else
					{
						m_peerlistctrl->SetItemCount( 0 );
					}

					/* get files list */
					WXLOGDEBUG(( _T( "get files list\n" )));
					{
						m_filelistctrl->SetStaticHandle(torrent);
						m_filelistctrl->SetItemCount( (torrent->info) ? torrent->info->num_files() : 0 );
					}
					{
						m_trackerlistctrl->SetStaticHandle( torrent );

						if( torrent_handle.is_valid() )
						{
							m_trackerlistctrl->SetItemCount( torrent_handle.trackers().size() );
						}
						else
						{
							m_trackerlistctrl->SetItemCount( torrent->config->GetTrackersURL().size() );
						}
					}
				}
				else
				{
					m_torrentlistctrl->Select( 0, true );
				}

				WXLOGDEBUG(( _T( "UpdateSwashList\n" )));
				m_torrentlistctrl->UpdateSwashList();
			}
			else
			{
				m_filelistctrl->SetStaticHandle( invalid_torrent );
				m_filelistctrl->SetItemCount( 0 );
				m_trackerlistctrl->SetStaticHandle( invalid_torrent );
				m_trackerlistctrl->SetItemCount( 0 );
				m_peerlistctrl->SetItemCount( 0 );
			}

			WXLOGDEBUG(( _T( "UpdateTorrentInfo\n" )));
			m_torrentinfo->UpdateTorrentInfo( false );
			m_torrentlistctrl->Refresh(false);
			UpdateStatusBar();
		}
	}

	if( m_config->GetUseSystray() )
	{
		UpdateTrayInfo();
	}

	WXLOGDEBUG(( _T( "MainFrame::UpdateUI done\n" ) ));
}

void MainFrame::UpdateSelectedTorrent()
{
	m_torrentinfo->UpdateTorrentInfo( true );
}

void MainFrame::OnUpdateUI_MenuTorrent( wxUpdateUIEvent& event )
{
	bool enable = false;

	if( m_btsession && m_btsession->GetTorrentQueueSize() > 0 )
	{
		SwashListCtrl::itemlist_t selecteditems;
		m_torrentlistctrl->GetSelectedItems(selecteditems);
		/* selected more than 1 item in the torrent list */
		size_t total = selecteditems.size();

		if( total > 0 )
		{
			int torrent_state;
			int menuId = event.GetId();

			if( ( ID_TORRENT_MOVEUP == menuId ) || ( ID_TORRENT_MOVEDOWN  == menuId )
					|| ( ID_TORRENT_REANNOUNCE  == menuId )  || ( ID_TORRENT_RECHECK  == menuId ) )
			{
				enable = true;
			}
			else
			{
				if( ID_TORRENT_OPENDIR == menuId )
				{
					enable = ( total == 1 );
				}
				else
				{
					for( size_t i = 0; i < total; ++i )
					{
						std::shared_ptr<torrent_t> torrent = m_btsession->GetTorrent( selecteditems[i] );
						if(!torrent || !torrent->isvalid)
						{
							break;
						}
						torrent_state = torrent->config->GetTorrentState();
						if( ID_TORRENT_START == menuId )
						{
							if( torrent_state != TORRENT_STATE_START && torrent_state != TORRENT_STATE_FORCE_START )
							{
								enable = true;
								break;
							}
							else
							{
								lt::torrent_handle &torrenthandle = torrent->handle;
								if((torrenthandle.is_valid() /*&& torrenthandle.status().paused*/ ))
								{
									enable = true;
									break;
								}
							}
						}
						else
							if( ID_TORRENT_STOP == menuId )
							{
								if( torrent_state != TORRENT_STATE_STOP )
								{
									enable = true;
									break;
								}
							}
							else
								if( ID_TORRENT_PAUSE == menuId )
								{
									if( torrent_state == TORRENT_STATE_START || torrent_state == TORRENT_STATE_FORCE_START)
									{
										enable = true;
										break;
									}
								}
					}
				}
			}
		}
	}

	event.Enable( enable );
}

void MainFrame::OnUpdateUI_MenuTorrentStart( wxUpdateUIEvent& event )
{
	bool enable = false;

	if( m_btsession && m_btsession->GetTorrentQueueSize() > 0 )
	{
		SwashListCtrl::itemlist_t selecteditems;
		m_torrentlistctrl->GetSelectedItems(selecteditems);
		/* selected more than 1 item in the torrent list */
		size_t total = selecteditems.size();

		if( total > 0 )
		{
			for( size_t i = 0; i < total; ++i )
			{
				std::shared_ptr<torrent_t> torrent = m_btsession->GetTorrent( selecteditems[i] );
				if(torrent &&& torrent->isvalid)
				{
					int torrent_state = torrent->config->GetTorrentState();
					if( torrent_state != TORRENT_STATE_START && torrent_state != TORRENT_STATE_FORCE_START )
					{
						enable = true;
						break;
					}
				}
			}
		}
	}

	event.Enable(enable);
}

void MainFrame::OnUpdateUI_MenuTorrentPause( wxUpdateUIEvent& event )
{
	bool enable = false;

	if( m_btsession && m_btsession->GetTorrentQueueSize() > 0 )
	{
		SwashListCtrl::itemlist_t selecteditems;
		m_torrentlistctrl->GetSelectedItems(selecteditems);
		/* selected more than 1 item in the torrent list */
		size_t total = selecteditems.size();

		if( total > 0 )
		{
			int torrent_state;
			for( size_t i = 0; i < total; ++i )
			{
				std::shared_ptr<torrent_t> torrent = m_btsession->GetTorrent( selecteditems[i] );
				if(!torrent || !torrent->isvalid)
				{
					break;
				}
				torrent_state = torrent->config->GetTorrentState();
				if( torrent_state == TORRENT_STATE_START || torrent_state == TORRENT_STATE_FORCE_START )
				{
					enable = true;
					break;
				}
			}
		}
	}

	event.Enable(enable);
}


void MainFrame::OnUpdateUI_MenuTorrentStop( wxUpdateUIEvent& event )
{
	bool enable = false;

	if( m_btsession && m_btsession->GetTorrentQueueSize() > 0 )
	{
		SwashListCtrl::itemlist_t selecteditems;
		m_torrentlistctrl->GetSelectedItems(selecteditems);
		/* selected more than 1 item in the torrent list */
		size_t total = selecteditems.size();

		if( total > 0 )
		{
			int torrent_state;
			for( size_t i = 0; i < total; ++i )
			{
				std::shared_ptr<torrent_t> torrent = m_btsession->GetTorrent( selecteditems[i] );
				if(!torrent || !torrent->isvalid)
				{
					break;
				}
				torrent_state = torrent->config->GetTorrentState();

				if( torrent_state != TORRENT_STATE_STOP )
				{
					enable = true;
					break;
				}
			}
		}
	}

	event.Enable(enable);
}


void MainFrame::OnUpdateUI_MenuViewStatusBar( wxUpdateUIEvent& event )
{
	event.Check( m_swashstatbar->IsShown());
}

void MainFrame::OnUpdateUI_MenuViewTorrentInfo( wxUpdateUIEvent& event )
{
	event.Check( m_mgr.GetPane( m_torrentinfo ).IsShown() );
}

void MainFrame::OnUpdateUI_MenuViewToolbar( wxUpdateUIEvent& event )
{
	event.Check( m_mgr.GetPane( _T( "mainToolBar" ) ).IsShown() );
}

void MainFrame::OnUpdateUI_MenuMoveup( wxUpdateUIEvent& event )
{
	bool enable = false;

	if(m_torrentlistctrl)
	{
		long item = m_torrentlistctrl->GetFirstSelected();

		enable = ( item > 0 );
	}

	event.Enable(enable);
}

void MainFrame::OnUpdateUI_MenuMovedown( wxUpdateUIEvent& event )
{
	bool enable = false;

	if(m_torrentlistctrl)
	{
		SwashListCtrl::itemlist_t selecteditems;
		m_torrentlistctrl->GetSelectedItems(selecteditems);

		size_t last = selecteditems.size();
		if( last > 0 )
		{
			long idx = selecteditems[last -1];
			if( idx < (m_torrentlistctrl->GetItemCount() - 1 ) ) 
			{
				std::shared_ptr<torrent_t> torrent = m_btsession->GetTorrent( idx );
				if(torrent && torrent->isvalid)
				{
					long qindex = torrent->config->GetQIndex();
					if(qindex > 0)
					{
						std::shared_ptr<torrent_t> ntorrent = m_btsession->GetTorrent( idx + 1 );
						if(torrent->config->GetQIndex() > 0)
							enable = true;
					}
				}
			}
		}
	}
	event.Enable(enable);
}

void MainFrame::OnUpdateUI_MenuOpendir( wxUpdateUIEvent& event )
{
	int selected = m_torrentlistctrl->GetSelectedItemCount();
	bool enable = (( selected == 1 ) &&( GetSelectedTorrent() ));

	event.Enable(enable);
}

void MainFrame::OnUpdateUI_MenuProperties( wxUpdateUIEvent& event )
{
	int selected = m_torrentlistctrl->GetSelectedItemCount();
	bool enable = (( selected == 1 ) &&( GetSelectedTorrent() ));

	event.Enable(enable);
}

void MainFrame::OnUpdateUI_MenuCopyMagnetUri( wxUpdateUIEvent& event )
{
	event.Enable( m_torrentlistctrl->GetSelectedItemCount() );
}

void MainFrame::OnUpdateUI_MenuRemove( wxUpdateUIEvent& event )
{
	bool enable = (m_torrentlistctrl->GetSelectedItemCount() > 0);

	event.Enable(enable);
}

void MainFrame::OnUpdateUI_MenuRemovedata( wxUpdateUIEvent& event )
{
	bool enable = (m_torrentlistctrl->GetSelectedItemCount() > 0);

	event.Enable(enable);
}

void MainFrame::OnUpdateUI_MenuTorrentReannounce( wxUpdateUIEvent& event )
{
	bool enable = false;

	if( m_btsession && m_btsession->GetTorrentQueueSize() > 0 )
	{
		SwashListCtrl::itemlist_t selecteditems;
		m_torrentlistctrl->GetSelectedItems(selecteditems);
		/* selected more than 1 item in the torrent list */
		size_t total = selecteditems.size();

		if( total > 0 )
		{
			for( size_t i = 0; i < total; ++i )
			{
				std::shared_ptr<torrent_t> torrent = m_btsession->GetTorrent( selecteditems[i] );
				if(!torrent || !torrent->isvalid)
				{
					continue;
				}

				if(torrent->handle.is_valid())
				{
					enable = true;
					break;
				}
			}
		}
	}

	event.Enable(enable);
}

void MainFrame::OnUpdateUI_MenuTorrentRecheck( wxUpdateUIEvent& event )
{
	bool enable = false;

	if( m_btsession && m_btsession->GetTorrentQueueSize() > 0 )
	{
		SwashListCtrl::itemlist_t selecteditems;
		m_torrentlistctrl->GetSelectedItems(selecteditems);
		/* selected more than 1 item in the torrent list */
		size_t total = selecteditems.size();

		if( total > 0 )
		{
			for( size_t i = 0; i < total; ++i )
			{
				std::shared_ptr<torrent_t> torrent = m_btsession->GetTorrent( selecteditems[i] );
				if(!torrent || !torrent->isvalid)
				{
					enable = true;
					break;
				}
			}
		}
	}

	event.Enable(enable);
}

void MainFrame::OnMenuViewStatusBar( wxCommandEvent& event )
{
	if( m_swashstatbar->IsShown() )
	{ m_swashstatbar->Hide(); }
	else
	{ m_swashstatbar->Show(); }
}

void MainFrame::OnMenuViewTorrentInfo( wxCommandEvent& event )
{
	if( m_mgr.GetPane( m_torrentinfo ).IsShown() )
	{
		m_mgr.GetPane( m_torrentinfo ).Hide();
	}
	else
	{
		m_mgr.GetPane( m_torrentinfo ).Show();
	}
	m_mgr.Update();
}

void MainFrame::OnMenuViewToolbar( wxCommandEvent& event )
{
	if( m_mgr.GetPane( _T( "mainToolBar" ) ).IsShown() )
	{
		m_mgr.GetPane( _T( "mainToolBar" ) ).Hide();
	}
	else
	{
		m_mgr.GetPane( _T( "mainToolBar" ) ).Show();
	}

	if( m_mgr.GetPane( _T( "torrentToolBar" ) ).IsShown() )
	{
		m_mgr.GetPane( _T( "torrentToolBar" ) ).Hide();
	}
	else
	{
		m_mgr.GetPane( _T( "torrentToolBar" ) ).Show();
	}

	if( m_mgr.GetPane( _T( "miscToolBar" ) ).IsShown() )
	{
		m_mgr.GetPane( _T( "miscToolBar" ) ).Hide();
	}
	else
	{
		m_mgr.GetPane( _T( "miscToolBar" ) ).Show();
	}

	m_mgr.Update();
}

void MainFrame::OnMenuTorrentStart( wxCommandEvent& event )
{
	WXLOGDEBUG(( _T( "MainFrame: OnMenuTorrentStart\n" ) ));
	SwashListCtrl::itemlist_t selecteditems;
	m_torrentlistctrl->GetSelectedItems(selecteditems);

	if( selecteditems.size() <= 0 )
	{
		wxLogWarning( _T( "MainFrame: No torrent is selected\n" ) );
		return;
	}

	/* start selected torrent(s) */
	{
		/* start data selected in ui list */
		for( int i = 0; i < selecteditems.size(); i++ )
		{
			if( event.GetId() == ID_TORRENT_FORCE_START )
			{
				m_btsession->StartTorrent( selecteditems[i], true );
			}
			else
			{
				m_btsession->QueueTorrent( selecteditems[i] );
			}
		}
	}
}

void MainFrame::OnMenuTorrentPause( wxCommandEvent& event )
{
	int i = 0;

	WXLOGDEBUG(( _T( "MainFrame: OnMenuTorrentPause\n" ) ));
	SwashListCtrl::itemlist_t selecteditems;
	m_torrentlistctrl->GetSelectedItems(selecteditems);

	if( selecteditems.size() <= 0 )
	{
		wxLogWarning( _T( "MainFrame: No torrent is selected\n" ) );
		return;
	}

	/* pause selected torrent(s) */
	{
		/* pause data selected in ui list */
		for( i = 0; i < selecteditems.size(); i++ )
		{
			m_btsession->PauseTorrent( selecteditems[i] );
		}

		/* mark for list refresh */
	}
}

void MainFrame::OnMenuTorrentStop( wxCommandEvent& event )
{
	WXLOGDEBUG(( _T( "MainFrame: OnMenuTorrentStop\n" ) ));
	SwashListCtrl::itemlist_t selecteditems;
	m_torrentlistctrl->GetSelectedItems(selecteditems);

	if( selecteditems.size() > 0 )
	{
		for(auto const & i : selecteditems)
		{
			m_btsession->StopTorrent( i );
		}
		UpdateUI();
	}
}

void MainFrame::OnMenuTorrentCopyMagnetUri( wxCommandEvent& event )
{
	SwashListCtrl::itemlist_t selecteditems;
	m_torrentlistctrl->GetSelectedItems(selecteditems);

	if( selecteditems.size() > 0 )
	{
		wxString magneturis, magneturi;
		const wxChar* eol = wxTextFile::GetEOL();
		for(auto const & i : selecteditems)
		{
			if(m_btsession->GetTorrentMagnetUri( i, magneturi))
				magneturis += magneturi + *eol;
		}

		
		if( !magneturis.IsEmpty() && wxTheClipboard->Open() )
		{
			wxTheClipboard->SetData( new wxTextDataObject( magneturis ) );
			wxTheClipboard->Flush();
			wxTheClipboard->Close();
		}
	}
}

//Show torrent properties
void MainFrame::OnMenuTorrentProperties( wxCommandEvent& event )
{
	WXLOGDEBUG(( _T( "MainFrame: OnMenuTorrentProperties\n" ) ));
	SwashListCtrl::itemlist_t selecteditems;
	m_torrentlistctrl->GetSelectedItems(selecteditems);

	if( selecteditems.size() <= 0 )
	{
		wxLogWarning( _T( "MainFrame: No torrent is selected\n" ) );
		return;
	}

	/* shows selected torrent(s) property*/
	{
		for( int i = 0; i < selecteditems.size(); i++ )
		{
			std::shared_ptr<torrent_t> torrent = m_btsession->GetTorrent( selecteditems[i] );
			if(!torrent || !torrent->isvalid)
			{
				return;
			}
			TorrentProperty torrent_property_dlg( torrent, this, wxID_ANY );

			if( torrent_property_dlg.ShowModal() == wxID_OK )
			{
				//set selected torrent properties`
				m_btsession->ConfigureTorrent( selecteditems[i] );
			}
		}
	}
}

void MainFrame::RemoveTorrent( bool deletedata )
{
	int i = 0;
	SwashListCtrl::itemlist_t selecteditems;
	m_torrentlistctrl->GetSelectedItems(selecteditems);

	if( selecteditems.size() <= 0 )
	{
		wxLogWarning( _T( "MainFrame: No torrent is selected\n" ) );
		return;
	}

	wxString torrentwarning(_( "torrent" ));

	if( selecteditems.size() > 1 )
	{ torrentwarning = wxString( _( "torrents" ) ); }

	if( deletedata )
	{ torrentwarning += _( " and data" ); }

	wxMessageDialog dialog( nullptr, _( "Remove " ) + torrentwarning + _T( "?" ),
							_( "Confirm Remove" ), wxNO_DEFAULT | wxYES_NO | wxICON_QUESTION );

	if( dialog.ShowModal() == wxID_YES )
	{
		/* deselect items in list */
		for( i = 0; i < selecteditems.size(); i++ )
		{
			m_torrentlistctrl->Select( selecteditems[i], false );
		}

		/* set new item count, prevent assert fire */
		m_torrentlistctrl->SetItemCount( m_torrentlistctrl->GetItemCount() - selecteditems.size() );
		/* pointer to vector items will get shifted if erased
		 * offset is still needed to keep track shift items
		 */
        bool force = false;
		/* remove data in ui list, and bt list */
		for( i = selecteditems.size() - 1; i >= 0; i-- )
		{
			//std::shared_ptr<torrent_t> torrent = m_torrentlistitems[selecteditems[i] - (offset++)];
			m_btsession->RemoveTorrent( selecteditems[i], deletedata );
            force = true;
		}

		/* mark for list refresh */
		UpdateUI(force);
	}
	else
	{ wxLogInfo( _T( "Torrent removal cancelled\n" ) ); }
}

void MainFrame::OnMenuTorrentRemove( wxCommandEvent& event )
{
	WXLOGDEBUG(( _T( "MainFrame: OnMenuTorrentRemove\n" ) ));
	RemoveTorrent( false );
}

void MainFrame::OnMenuTorrentRemoveData( wxCommandEvent& event )
{
	WXLOGDEBUG(( _T( "MainFrame: OnMenuTorrentRemoveData\n" ) ));
	RemoveTorrent( true );
}

void MainFrame::OnMenuTorrentMoveUp( wxCommandEvent& event )
{
	int i = 0;

	WXLOGDEBUG(( _T( "MainFrame: OnMenuTorrentMoveUp\n" ) ));
	SwashListCtrl::itemlist_t selecteditems;
	m_torrentlistctrl->GetSelectedItems(selecteditems);

	if( selecteditems.size() <= 0 )
	{
		wxLogWarning( _T( "MainFrame: No torrent is selected\n" ) );
		return;
	}

	/* move selected torrent(s) */
	{
		for( i = 0; i < selecteditems.size(); i++ )
		{
			if( selecteditems[i] > 0 )
			{
				m_torrentlistctrl->Select( selecteditems[i], false );
				m_torrentlistctrl->Select( selecteditems[i] - 1, true );
			}

			m_btsession->MoveTorrentUp( selecteditems[i] );
		}

		UpdateUI();
	}
}

void MainFrame::OnMenuTorrentMoveDown( wxCommandEvent& event )
{
	int i = 0;

	WXLOGDEBUG(( _T( "MainFrame: OnMenuTorrentMoveDown\n" ) ));
	SwashListCtrl::itemlist_t selecteditems;
	m_torrentlistctrl->GetSelectedItems(selecteditems);

	if( selecteditems.size() <= 0 )
	{
		wxLogWarning( _T( "MainFrame: No torrent is selected\n" ) );
		return;
	}

	/* move selected torrent(s) */
	{
		for( i = ( selecteditems.size() - 1 ); i >= 0; i-- )
		{
			if( selecteditems[i] < ( m_torrentlistctrl->GetItemCount() - 1 ) )
			{
				m_torrentlistctrl->Select( selecteditems[i], false );
				m_torrentlistctrl->Select( selecteditems[i] + 1, true );
			}

			m_btsession->MoveTorrentDown( selecteditems[i] );
		}

		UpdateUI();
	}
}

void MainFrame::OnMenuTorrentReannounce( wxCommandEvent& event )
{
	int i = 0;
	WXLOGDEBUG(( _T( "MainFrame: OnMenuTorrentReannounce\n" ) ));
	SwashListCtrl::itemlist_t selecteditems;
	m_torrentlistctrl->GetSelectedItems(selecteditems);

	if( selecteditems.size() <= 0 )
	{
		wxLogWarning( _T( "MainFrame: No torrent is selected\n" ) );
		return;
	}

	/* reannounce selected torrent(s) */
	{
		for( i = 0; i < selecteditems.size(); i++ )
		{
			m_btsession->ReannounceTorrent( selecteditems[i] );
		}
	}
}

void MainFrame::OnMenuTorrentRecheck( wxCommandEvent& event )
{
	int i = 0;

	WXLOGDEBUG(( _T( "MainFrame: OnMenuTorrentRecheck\n" ) ));
	SwashListCtrl::itemlist_t selecteditems;
	m_torrentlistctrl->GetSelectedItems(selecteditems);

	if( selecteditems.size() <= 0 )
	{
		wxLogWarning( _T( "MainFrame: No torrent is selected\n" ) );
		return;
	}

	/* reannounce selected torrent(s) */
	{
		for( i = selecteditems.size() - 1; i >= 0; --i )
		{
			//m_btsession->ReannounceTorrent((m_torrentlistitems[selecteditems[i]]));
			//XXX Workaround for libtorrent lack of recheck redo if supported in core
			m_btsession->RecheckTorrent(selecteditems[i]);
		}
	}
}

std::shared_ptr<torrent_t> MainFrame::GetSelectedTorrent()
{
	std::shared_ptr<torrent_t> torrent;

	SwashListCtrl::itemlist_t selecteditems;
	m_torrentlistctrl->GetSelectedItems(selecteditems);

	if( selecteditems.size() > 0 )
	{
		torrent = m_btsession->GetTorrent( selecteditems[0] );
	}

	return torrent;
}

void MainFrame::ShowPreferences()
{
	if( m_swashsetting == nullptr )
	{ m_swashsetting = new SwashSetting( this, m_config ); }

	bool magneturi = m_config->GetAssociateMagnetURI();
	if( m_swashsetting->ShowModal() == wxID_OK )
	{
		m_btsession->PostQueueUpdateEvent();
	}
}

void MainFrame::OnMenuPreferences( wxCommandEvent& event )
{
	ShowPreferences();
}

void MainFrame::OnMenuOptionLanguage( wxCommandEvent& event )
{
	wxString strlang = wxEmptyString;
	const wxLanguageInfo* langinfo ;
	langinfo = wxLocale::GetLanguageInfo( event.GetId() - ID_OPTIONS_LANGUAGE );
	WXLOGDEBUG(( _T( "Command  %d, %s, item %p\n" ), event.GetId(), langinfo->CanonicalName.c_str(), event.GetEventObject() ));

	if( ( event.GetId() - ID_OPTIONS_LANGUAGE ) == 0 )
	{
		strlang = _T( "" );
	}
	else
	{
		strlang = langinfo->CanonicalName.c_str();
	}

	if( wxGetApp().SetLocale( strlang ) )
	{
		m_config->SetLanguage( strlang );
		m_config->Save();
	}
	else
	{
		wxMenuItem* tmpitem = m_languagemenu->FindItem( ID_OPTIONS_LANGUAGE + m_prevlocale );

		if( tmpitem )
		{
			tmpitem->Check( true );
		}
	}
}

void MainFrame::UpdateStatusBar()
{
	m_swashstatbar->UpdateStatBar();
}

wxString MainFrame::GetStatusDownloadRate()
{
	wxString d_rate = HumanReadableByte( m_btsession->GetDownloadRate() ) + _T( "/s" );
	return d_rate;
}

wxString MainFrame::GetStatusUploadRate()
{
	wxString u_rate = HumanReadableByte( m_btsession->GetUploadRate() ) + _T( "/s" );
	return u_rate;
}

wxString MainFrame::GetStatusPeers()
{
	wxString numpeers = wxString::Format( _T( "%s" ), (wxLongLong( m_btsession->GetNumPeers() ).ToString()).c_str() );
	return numpeers;
}

wxString MainFrame::GetStatusDHT()
{
	wxString numpeers = wxString::Format( _T( "%s" ),  (wxLongLong( m_btsession->GetDhtNodes() ).ToString()).c_str() );
	return numpeers;
}

wxString MainFrame::GetStatusIncoming()
{
	wxString incomingok =  \
						( m_btsession->HasInComingConns() ? \
						_("Ok") : \
						_( "Error" ));
	return incomingok;
}

void MainFrame::UpdateTrayInfo()
{
	if (m_swashtrayicon != nullptr)
	{
		wxString tooltips = wxString::Format(_T("%s %s\n%s : %s\n%s : %s\n%s : %s\n%s : %s\n%s : %s"),
			APPNAME, BITSWASH_VERSION,
			_("DLR"), GetStatusDownloadRate().c_str(),
			_("ULR"), GetStatusUploadRate().c_str(),
			_("PRS"), GetStatusPeers().c_str(),
			_("DHT"), GetStatusDHT().c_str(),
			_("INC"), GetStatusIncoming().c_str());
		m_swashtrayicon->SetIcon(m_trayicon, tooltips);
	}
}

void MainFrame::ShowSystray( bool show )
{
	if( show )
	{
		m_swashtrayicon = new SwashTrayIcon( this );

		//if( !m_swashtrayicon->SetIcon( m_trayicon, wxT( "Bitswash" ) ) )
		//{ wxMessageBox( _( "Could not set icon." ) ); }
		m_swashtrayicon->SetIcon( m_trayicon, wxT( "Bitswash" ) );

		SetIcon( m_frameicon );
	}
	else
	{
		if( m_swashtrayicon != nullptr )
		{
			if(m_swashtrayicon->IsIconInstalled())
				m_swashtrayicon->RemoveIcon();
			delete m_swashtrayicon;
			m_swashtrayicon = nullptr;
		}
	}
}

void MainFrame::SetLogSeverity()
{
	int wx_loglevel = wxLOG_Debug - m_config->GetLogSeverity();
	wxLog::SetLogLevel( wx_loglevel );

	if( wx_loglevel >= wxLOG_Info )
	{ wxLog::SetVerbose( true ); }
	else
	{ wxLog::SetVerbose( false ); }
}

void MainFrame::OnUpdateOptionLanguage( wxUpdateUIEvent& event )
{
	wxLogMessage( _T( "UI Command %s, item %p" ), event.GetString().c_str(), event.GetEventObject() );
}

wxMenu* MainFrame::GetNewTorrentMenu()
{
	wxMenuItem* menuitem = 0;
	wxMenu* torrentMenu = new wxMenu( wxT( "" ) );
	menuitem = torrentMenu->Append( ID_TORRENT_START, _( "&Queue Torrent\tCtrl+S" ), _( "Queue Download" ) );
	menuitem->SetBitmap( wxBitmap( wxGetApp().GetAppIcon( BITSWASH_ICON_TORRENT_START ) ) );
	menuitem = torrentMenu->Append( ID_TORRENT_FORCE_START, _( "&Force Start Torrent\tCtrl+Alt+S" ), _( "Force Torrent Download" ) );
	menuitem->SetBitmap( wxBitmap( wxGetApp().GetAppIcon( BITSWASH_ICON_TORRENT_START ) ) );
	menuitem = torrentMenu->Append( ID_TORRENT_PAUSE, _( "&Pause Torrent\tCtrl+P" ), _( "Pause Download" ) );
	menuitem->SetBitmap( wxBitmap( wxGetApp().GetAppIcon( BITSWASH_ICON_TORRENT_PAUSE ) ) );
	menuitem = torrentMenu->Append( ID_TORRENT_STOP, _( "St&op Torrent\tCtrl+T" ), _( "Stop Download" ) );
	menuitem->SetBitmap( wxBitmap( wxGetApp().GetAppIcon( BITSWASH_ICON_TORRENT_STOP ) ) );
	torrentMenu->AppendSeparator();
	menuitem = torrentMenu->Append( ID_TORRENT_REMOVE, _( "&Remove Torrent" ), _( "Delete Torrent" ) );
	menuitem->SetBitmap( wxBitmap( wxGetApp().GetAppIcon( BITSWASH_ICON_TORRENT_REMOVE ) ) );
	menuitem = torrentMenu->Append( ID_TORRENT_REMOVEDATA, _( "Remove Torrent and &Data" ), _( "Delete Torrent and Data" ) );
	menuitem->SetBitmap( wxBitmap( wxGetApp().GetAppIcon( BITSWASH_ICON_TORRENT_REMOVEDATA ) ) );
	torrentMenu->AppendSeparator();
	menuitem = torrentMenu->Append( ID_TORRENT_MOVEUP, _( "Move &Up\tAlt+UP" ), _( "Move torrent up" ) );
	//menuitem->SetBitmap(wxBitmap(wxGetApp().GetAppIcon(BITSWASH_ICON_TORRENT_MOVEUP)));
	menuitem->SetBitmap( wxArtProvider::GetBitmap( wxART_GO_UP, wxART_MENU, wxSize( 16, 16 ) ) );
	menuitem = torrentMenu->Append( ID_TORRENT_MOVEDOWN, _( "Move Do&wn\tAlt+DOWN" ), _( "Move torrent down" ) );
	//menuitem->SetBitmap(wxBitmap(wxGetApp().GetAppIcon(BITSWASH_ICON_TORRENT_MOVEDOWN)));
	menuitem->SetBitmap( wxArtProvider::GetBitmap( wxART_GO_DOWN, wxART_MENU, wxSize( 16, 16 ) ) );
	torrentMenu->AppendSeparator();
	menuitem = torrentMenu->Append( ID_TORRENT_REANNOUNCE, _( "Re&announce Torrent" ), _( "Force Reannounce Torrent" ) );
	//menuitem->SetBitmap(wxBitmap(wxGetApp().GetAppIcon(BITSWASH_ICON_TORRENT_REANNOUNCE)));
	menuitem = torrentMenu->Append( ID_TORRENT_RECHECK, _( "Force Re&check Torrent" ), _( "Force Recheck Torrent" ) );
	menuitem = torrentMenu->Append( ID_TORRENT_OPENDIR, _( "Open &Directory" ), _( "Open downloaded directory" ) );
	menuitem->SetBitmap( wxBitmap( wxGetApp().GetAppIcon( BITSWASH_ICON_TORRENT_OPENDIR ) ) );
	menuitem = torrentMenu->Append( ID_TORRENT_PROPERTIES, _( "Propert&ies" ), _( "Torrent Properties" ) );
	menuitem->SetBitmap( wxBitmap( wxGetApp().GetAppIcon( BITSWASH_ICON_TORRENT_PROPERTIES ) ) );
	torrentMenu->AppendSeparator();
	menuitem = torrentMenu->Append( ID_TORRENT_COPYMAGNETURI, _( "Copy &Magnet URI\tCtrl+C" ), _( "Copy Magnet URI" ) );
	return torrentMenu;
}
