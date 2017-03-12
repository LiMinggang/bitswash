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
#include <libtorrent/extensions/metadata_transfer.hpp>
#include <libtorrent/extensions/ut_pex.hpp>

#include "bitswash.h"
#include "mainframe.h"
#include "torrentproperty.h"
#include "functions.h"
#include "urldialog.h"
#include "swashlistctrl.h" //for itemlist_t
#include "magneturi.h"

namespace lt = libtorrent;

#ifndef __WXMSW__
#include "languages.h"
#else
struct swashlang
{
	wxString lang;
};

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

BEGIN_EVENT_TABLE( MainFrame, wxFrame )
	EVT_MENU( ID_FILE_CREATE_TORRENT, MainFrame::ToDo )
	EVT_MENU( ID_FILE_OPEN_TORRENT, MainFrame::OnMenuOpenTorrent )
	EVT_MENU( ID_FILE_OPEN_URL, MainFrame::OnMenuOpenTorrentUrl )
	EVT_MENU( ID_FILE_EXIT, MainFrame::OnQuit )
	EVT_MENU( ID_VIEW_FAVORITE, MainFrame::ToDo )
	EVT_MENU( ID_VIEW_STATUS, MainFrame::OnMenuViewStatusBar )
	EVT_MENU( ID_VIEW_TOOLBAR, MainFrame::OnMenuViewToolbar )
	EVT_MENU( ID_TORRENT_START, MainFrame::OnMenuTorrentStart )
	EVT_MENU( ID_TORRENT_FORCE_START, MainFrame::OnMenuTorrentStart )
	EVT_MENU( ID_TORRENT_PAUSE, MainFrame::OnMenuTorrentPause )
	EVT_MENU( ID_TORRENT_STOP, MainFrame::OnMenuTorrentStop )
	EVT_MENU( ID_TORRENT_PROPERTIES, MainFrame::OnMenuTorrentProperties )
	EVT_MENU( ID_TORRENT_REMOVE, MainFrame::OnMenuTorrentRemove )
	EVT_MENU( ID_TORRENT_REMOVEDATA, MainFrame::OnMenuTorrentRemoveData )
	EVT_MENU( ID_TORRENT_MOVEUP, MainFrame::OnMenuTorrentMoveUp )
	EVT_MENU( ID_TORRENT_MOVEDOWN, MainFrame::OnMenuTorrentMoveDown )
	EVT_MENU( ID_TORRENT_REANNOUNCE, MainFrame::OnMenuTorrentReannounce )
	EVT_MENU( ID_TORRENT_RECHECK, MainFrame::OnMenuTorrentRecheck )
	EVT_MENU( ID_TORRENT_OPENDIR, MainFrame::OnMenuTorrentOpenDir )
	EVT_MENU( ID_OPTIONS_PREFERENCES, MainFrame::OnMenuPreferences )
	EVT_MENU( ID_HELP_BITSWASH_HELP, MainFrame::OnMenuHelp )
	EVT_MENU( ID_HELP_HOMEPAGE, MainFrame::OnHelpHomepage )
	EVT_MENU( ID_HELP_ABOUT, MainFrame::OnAbout )
	EVT_MENU_OPEN( MainFrame::OnMenuOpen )

	EVT_UPDATE_UI( ID_TORRENT_START, MainFrame::OnUpdateUI_MenuTorrent )
	EVT_UPDATE_UI( ID_TORRENT_FORCE_START, MainFrame::OnUpdateUI_MenuTorrent )
	EVT_UPDATE_UI( ID_TORRENT_PAUSE, MainFrame::OnUpdateUI_MenuTorrent )
	EVT_UPDATE_UI( ID_TORRENT_STOP, MainFrame::OnUpdateUI_MenuTorrent )

	EVT_UPDATE_UI( ID_TORRENT_MOVEUP, MainFrame::OnUpdateUI_MenuTorrent )
	EVT_UPDATE_UI( ID_TORRENT_MOVEDOWN, MainFrame::OnUpdateUI_MenuTorrent )
	EVT_UPDATE_UI( ID_TORRENT_REANNOUNCE, MainFrame::OnUpdateUI_MenuTorrent )
	EVT_UPDATE_UI( ID_TORRENT_RECHECK, MainFrame::OnUpdateUI_MenuTorrent )
	EVT_UPDATE_UI( ID_TORRENT_OPENDIR, MainFrame::OnUpdateUI_MenuTorrent )

	EVT_SIZE( MainFrame::OnSize )
	EVT_MOVE( MainFrame::OnMove )
	EVT_TIMER( ID_TIMER_GUI_UPDATE, MainFrame::OnRefreshTimer )

	EVT_CLOSE( MainFrame::OnClose )
	EVT_ICONIZE( MainFrame::OnMinimize )

END_EVENT_TABLE()

MainFrame * g_BitSwashMainFrame = 0;

MainFrame::MainFrame( wxFrame *frame, const wxString& title )
	: wxFrame( frame, -1, title ),
	  m_btsession( 0 ), m_torrentmenu( 0 ), m_languagemenu( 0 ),
	  m_torrentpane( 0 ), m_torrentinfo( 0 ), m_torrentlistctrl( 0 ),
	  m_peerlistctrl( 0 ), m_filelistctrl( 0 ), m_summarypane( 0 ),
	  m_trackerlistctrl( 0 ), m_config( 0 ), m_torrentproperty( 0 ),
	  m_swashsetting( 0 ), m_swashstatbar( 0 ), m_oldlog( 0 ),
	  m_swashtrayicon( NULL ),
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
	/* layer 1 put side bar occupied whole vertical zone
	m_mgr.AddPane(CreateSideTreeCtrl(), AUIPANEINFO.
	              Name(_T("category")).Caption(_("Category")).
	              Left().Layer(1).Position(0).
	              MinimizeButton(true).CloseButton(false));

	m_mgr.AddPane(CreateSideTreeCtrl(), AUIPANEINFO.
	              Name(_T("category")).Caption(_("Category")).
	              Left().Layer(0).Position(0).
	              MinimizeButton(true).CloseButton(false));
	              */
	CreateTorrentPanel();
	m_mgr.SetFlags( m_mgr.GetFlags() | wxAUI_MGR_ALLOW_ACTIVE_PANE );
#ifdef NDEBUG //XXX asert failure fired in gtk debug library.
	m_oldlog = wxLog::SetActiveTarget( ( wxLog* )m_torrentinfo->GetLogger() );
#endif

	if( !wxIsEmpty( m_config->GetAuiPerspective() ) )
	{ m_mgr.LoadPerspective( m_config->GetAuiPerspective() ); }

	m_mgr.Update();
	ShowSystray( m_config->GetUseSystray() );
	UpdateUI();
	//Refresh timer
	wxLogDebug( _T( "Refresh timer %d" ), m_config->GetRefreshTime() );
	m_refreshtimer.SetOwner( this, ID_TIMER_GUI_UPDATE );
	m_refreshtimer.Start( m_config->GetRefreshTime() * 1000, wxTIMER_CONTINUOUS );
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

	wxLogDebug( _T( "MainFrame Closing" ) );
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
	shared_ptr<torrent_t> pTorrent = GetSelectedTorrent();

	if( pTorrent )
	{
		//XXX windows might needs special treatment here!
#if  defined(__WXMSW__)
		wxExecute( _T( "Explorer " ) + pTorrent->config->GetDownloadPath(), wxEXEC_ASYNC, NULL );
#elif defined(__APPLE__)
		wxExecute( _T( "/usr/bin/open " + pTorrent->config->GetDownloadPath(), wxEXEC_ASYNC, NULL );
#elif defined(__WXGTK__)
		wxString loc = _T( "file://" ) + pTorrent->config->GetDownloadPath();
		//SystemOpenURL( loc );
		wxLaunchDefaultBrowser( _T( "file://" ) + filepath );
#endif
	}
}

void MainFrame::OnAbout( wxCommandEvent& event )
{
	wxAboutDialogInfo info;
	info.SetName( _T( "Bitswash-Mod" ) );
	info.SetVersion( BITSWASH_VERSION );
	info.SetDescription( _T( "Bittorrent Client" ) );
	info.SetCopyright( _T( "(c) 2017 Bitswash Developers" ) );
	info.SetWebSite( _T( "https://github.com/LiMinggang/bitswash" ), _T( "Bitswash Homepage" ) );
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
	SystemOpenURL( BITSWASH_HELP_URL );
}

void MainFrame::OnHelpHomepage( wxCommandEvent& event )
{
	SystemOpenURL( BITSWASH_HOMEPAGE );
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
	menuitem = viewMenu->Append( ID_VIEW_TOOLBAR, _( "View &Toolbar" ), _( "View Toolbar" ) );
	menuitem = viewMenu->Append( ID_VIEW_STATUS, _( "View &Status" ), _( "View Status" ) );
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

			if( langinfo == NULL )
			{
				wxLogError( _T( "Language %s is not available" ), strlang.c_str() );
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

	Connect( ID_OPTIONS_LANGUAGE, ID_OPTIONS_LANGUAGE + wxLANGUAGE_USER_DEFINED, wxEVT_COMMAND_MENU_SELECTED,
			 ( wxObjectEventFunction )
			 ( wxEventFunction )
			 ( wxCommandEventFunction )&MainFrame::OnMenuOptionLanguage );
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
	m_torrentinfo = new TorrentInfo( this, TORRENT_INFO_ID,
									 wxDefaultPosition,
									 wxSize( client_size.GetWidth(), -1 ),
#if WXVER >= 280
									 wxAUI_NB_DEFAULT_STYLE | wxAUI_NB_TAB_EXTERNAL_MOVE | wxNO_BORDER
#else
									 wxNO_BORDER
#endif
								   );
}

/* torrents queue display area (main area) */
void MainFrame::CreateTorrentList()
{
	long flags = wxLC_REPORT | wxLC_VIRTUAL | wxSUNKEN_BORDER;
	m_torrentlistctrl = new TorrentListCtrl( this, m_config->GetTorrentListCtrlSetting(), wxID_ANY,
			wxDefaultPosition, wxDefaultSize,
			flags );
}


void MainFrame::ReceiveTorrent( wxString fileorurl )
{
	if( g_BitSwashMainFrame )
	{
		wxStringTokenizer tkz( fileorurl, ' ' );
		wxString filename;

		while( tkz.HasMoreTokens() )
		{
			filename = tkz.GetNextToken();
			g_BitSwashMainFrame->AddTorrent( filename, true );
		}
	}
}

/* parse torrent file and add to session */
void MainFrame::AddTorrent( wxString filename, bool usedefault )
{
	wxLogDebug( _T( "MainFrame: Add Torrent: %s" ), filename.c_str() );
	shared_ptr<torrent_t> torrent = m_btsession->ParseTorrent( filename );

	if( torrent->isvalid )
	{
		shared_ptr<torrent_t> tmp_torrent = m_btsession->FindTorrent( torrent->hash );

		if( !tmp_torrent )
		{
			if( !m_config->GetUseDefault() )
			{
				TorrentProperty torrent_property_dlg( torrent, this, wxID_ANY );

				if( torrent_property_dlg.ShowModal() != wxID_OK )
				{
					wxLogMessage( _T( "Canceled by user" ) );
					return;
				}
			}

			if( m_btsession->AddTorrent( torrent ) )
			{
				wxString torrent_backup = wxGetApp().SaveTorrentsPath() + wxGetApp().PathSeparator() + torrent->hash + _T( ".torrent" );

				/* save a copy of torrent file */
				if( !wxCopyFile( filename, torrent_backup, true ) )
				{
					wxLogError( _T( "Error copying backup file %s" ), filename.c_str() );
				}

				UpdateUI();
			}
		}
		else
		{
			int answer = wxMessageBox( _("Torrent Exists. Do you want to update trackers/seeds from the torrent?"), _("Confirm"), wxYES_NO, this );

			if( answer == wxYES )
			{
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
		_T( "Torrent files (*.torrent)|*.torrent|All files (%s)|%s" );
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
			wxLogDebug( wxT( "File %d: %s (%s)" ),
						( int )n, paths[n].c_str(), filenames[n].c_str() );
			AddTorrent( paths[n], false );
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

/* open torrent from web url
 * XXX more verbose downloading (progress bar, cancel button etc)
 */
void MainFrame::OpenTorrentUrl()
{
	wxString url;
	UrlDialog urldialog( &url, this );

	if( urldialog.ShowModal() == wxID_OK )
	{
		wxLogDebug( _T( "openurl: Fetch URL %s" ), url.c_str() );
		wxURL *torrenturl = new wxURL( url );

		if( isUrl( torrenturl->GetScheme() ) )
		{
			if( torrenturl->GetError() == wxURL_NOERR )
			{
				wxInputStream *in_stream = torrenturl->GetInputStream();

				if( in_stream && in_stream->IsOk() )
				{
					wxString contenttype =  torrenturl->GetProtocol().GetContentType();
					size_t s_size = in_stream->GetSize();
					wxLogDebug( _T( "openurl: type %s, size %d" ), contenttype.c_str(), s_size );
					wxString tmpfile = wxFileName::CreateTempFileName( _T( "bitswash" ) );
					wxLogDebug( _T( "openurl: Tmp file %s" ), tmpfile.c_str() );
					{
						/* write downloaded stream in to temporary file */
						wxFileOutputStream fos( tmpfile );

						if( !fos.Ok() )
						{
							wxLogError( _T( "Error creating temporary file %s" ), tmpfile.c_str() );
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
				}
			}
		}
		else
		{
			MagnetUri magnetUri( url );

			if( magnetUri.isValid() )
			{
				shared_ptr<torrent_t> torrent = m_btsession->FindTorrent( magnetUri.hash() );

				if( !torrent )
				{
					torrent = m_btsession->LoadMagnetUri( magnetUri );

					if( torrent && torrent->isvalid )
					{
						if( torrent->handle.status().has_metadata )
						{
							wxString torrent_backup = wxGetApp().SaveTorrentsPath() + wxGetApp().PathSeparator() + torrent->hash + _T( ".torrent" );
							m_btsession->SaveTorrent( torrent, torrent_backup );
						}
						
						UpdateUI();
					}
					else
					{
						wxLogMessage( _T( "Load Magnet URI error" ) );
					}
				}
				else
				{
					int answer = wxMessageBox( _("Torrent Exists. Do you want to update trackers/seeds from the torrent?"), _("Confirm"), wxYES_NO, this );

					if( answer == wxYES )
					{
						m_btsession->MergeTorrent( torrent, magnetUri );
					}
				}
			}
			else
			{
				wxLogMessage( _T( "Not Supported" ) );
			}
		}
	}
	else
	{
		wxLogMessage( _T( "Download cancelled" ) );
	}
}

void MainFrame::OnMenuOpenTorrentUrl( wxCommandEvent& WXUNUSED( event ) )
{
	OpenTorrentUrl();
}

void MainFrame::OnRefreshTimer( wxTimerEvent& WXUNUSED( event ) )
{
	//GetTorrentLog();
	UpdateUI();
}

void MainFrame::OnListItemClick( long item )
{
	if( m_prevselecteditem == item )
	{ return; }

	m_prevselecteditem = item;
	UpdateUI();
}

void MainFrame::UpdateUI(bool force/* = false*/)
{
	wxASSERT( m_btsession != 0 );
	bool need_refresh = false;
	wxLogDebug( _T( "UpdateUI" ));

	m_btsession->PostStatusUpdate();

	if(force || (this->IsShown() && !this->IsIconized()))
	{
		size_t torrent_queue_size = m_btsession->GetTorrentQueueSize();
		//int t = 0;
		//torrents_t *torrentqueue = m_btsession->GetTorrentQueue();
		///torrents_t::const_iterator torrent_it = torrentqueue->begin();
		/* XXX more filtering/sorting */
		//m_btsession->GetTorrentQueue( m_torrentlistitems );
		wxLogDebug( _T( "Refreshing torrent list size %s" ), ( wxLongLong( torrent_queue_size ).ToString() ).c_str() );
		m_torrentlistctrl->SetItemCount( torrent_queue_size );

		if( torrent_queue_size > 0 )
		{
			const SwashListCtrl::itemlist_t selecteditems = m_torrentlistctrl->GetSelectedItems();

			/* selected more than 1 item in the torrent list */
			if( selecteditems.size() > 0 )
			{
				shared_ptr<torrent_t> torrent = m_btsession->GetTorrent( selecteditems[0] );
				if(!torrent || !torrent->isvalid)
				{
					return;
				}
				lt::torrent_handle &torrent_handle = torrent->handle;
				wxLogDebug( _T( "MainFrame: list size %s selected items %s" ), ( wxLongLong( torrent_queue_size ).ToString() ).c_str(), ( wxLongLong( selecteditems.size() ).ToString() ).c_str() );

				/* if torrent is started in libtorrent */
				if( torrent_handle.is_valid() )
				{
					/* get peer list */
					{
						torrent_handle.get_peer_info( m_peerlistitems );
						m_peerlistctrl->SetItemCount( m_peerlistitems.size() );

						if( m_peerlistitems.size() > 0 )
						{
							m_peerlistctrl->UpdateSwashList();
						}
					}
				}

				/* get files list */
				
				wxLogDebug( _T( "get files list" ));
				{
					//m_filelistctrl->SetStaticHandle(m_torrentlistitems[selecteditems[0]]);
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
				wxLogDebug( _T( "UpdateSummary" ));
				m_summarypane->UpdateSummary();
			}
			else
			{
				m_torrentlistctrl->Select( 0, true );
			}

			wxLogDebug( _T( "UpdateSwashList" ));
			m_torrentlistctrl->UpdateSwashList();
		}

		wxLogDebug( _T( "UpdateTorrentInfo" ));
		m_torrentinfo->UpdateTorrentInfo( false );
		m_torrentlistctrl->Refresh(false);
		UpdateStatusBar();
	}

	if( m_config->GetUseSystray() )
	{
		UpdateTrayInfo();
	}

	LoggerCtrl* logger = m_torrentinfo->GetLogger();
	if(logger)
	{
		if(MAX_LOG_LINES <= logger->GetNumberOfLogLines()) logger->ClearLog();
	}

	wxLogDebug( _T( "MainFrame::UpdateUI done" ) );
}

void MainFrame::UpdateSelectedTorrent()
{
	m_torrentinfo->UpdateTorrentInfo( true );
}

void MainFrame::TorrentOperationMenu( wxMenu* torrentmenu )
{
	//wxLogMessage(_T("TorrentOperationMenu enabled %d"), enabled);
	/* TODO: Check state to enable certain menu only */
	/* TODO: enable/disable toolbar too */
	static int menuids[] =
	{
		ID_TORRENT_START,
		ID_TORRENT_FORCE_START,
		ID_TORRENT_PAUSE,
		ID_TORRENT_STOP,

		ID_TORRENT_MOVEUP,
		ID_TORRENT_MOVEDOWN,
		ID_TORRENT_OPENDIR,
		ID_TORRENT_PROPERTIES,
		ID_TORRENT_REMOVE,
		ID_TORRENT_REMOVEDATA,
	};
	bool menu_status[] =
	{
		true,
		true,
		true,
		true,

		true,
		true,
		true,
		true,
		true,
		true,
	};
	wxASSERT( ( sizeof( menuids ) / sizeof( menuids[0] ) ) == ( sizeof( menu_status ) / sizeof( menu_status[0] ) ) );
	int selected = m_torrentlistctrl->GetSelectedItemCount();

	if( selected > 0 )
	{
		if( selected == 1 )
		{
			shared_ptr<torrent_t> pTorrent = GetSelectedTorrent();

			if( pTorrent )
			{
				int state = pTorrent->config->GetTorrentState();
				switch( state )
				{
				case TORRENT_STATE_STOP:
					{
						menu_status[2] = false;
						menu_status[3] = false;
						break;
					}

				case TORRENT_STATE_START:
				case TORRENT_STATE_FORCE_START:
					{
						if(!( pTorrent->handle.status().paused ))
						{
							menu_status[0] = false;
							menu_status[1] = false;
						}
						break;
					}

				case TORRENT_STATE_QUEUE:
					{
						menu_status[0] = false;
						break;
					}

				case TORRENT_STATE_PAUSE:
					menu_status[2] = false;
					break;

				default:
					break;
				}
			}
			else
			{
				/*
				ID_TORRENT_OPENDIR,
				ID_TORRENT_PROPERTIES,
				*/
				menu_status[7] = false;
				menu_status[8] = false;
			}
		}

		for( int i = 0; i < ( sizeof( menuids ) / sizeof( menuids[0] ) ); ++i )
		{
			torrentmenu->Enable( menuids[i], menu_status[i] );
		}
	}
	else
	{
		for( int i = 0; i < ( sizeof( menuids ) / sizeof( menuids[0] ) ); ++i )
		{
			torrentmenu->Enable( menuids[i], false );
		}
	}
}

void MainFrame::OnUpdateUI_MenuTorrent( wxUpdateUIEvent& event )
{
	bool enable = false;

	if( m_btsession && m_btsession->GetTorrentQueueSize() > 0 )
	{
		const SwashListCtrl::itemlist_t selecteditems = m_torrentlistctrl->GetSelectedItems();
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
						shared_ptr<torrent_t> torrent = m_btsession->GetTorrent( selecteditems[i] );
						if(!torrent || !torrent->isvalid)
						{
							return;
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
								if(( torrenthandle.status().paused ))
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
									if( torrent_state == TORRENT_STATE_START )
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

// XXX probly not needed, we do this in torrentlist
void MainFrame::OnMenuOpen( wxMenuEvent& event )
{
	wxMenu* openmenu = event.GetMenu();

	if( openmenu == m_torrentmenu )
	{
		TorrentOperationMenu( m_torrentmenu );
	}
}

void MainFrame::OnMenuViewStatusBar( wxCommandEvent& event )
{
	if( m_swashstatbar->IsShown() )
	{ m_swashstatbar->Hide(); }
	else
	{ m_swashstatbar->Show(); }
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
	int i = 0;
	wxLogDebug( _T( "MainFrame: OnMenuTorrentStart" ) );
	const SwashListCtrl::itemlist_t selecteditems = m_torrentlistctrl->GetSelectedItems();

	if( selecteditems.size() <= 0 )
	{
		wxLogWarning( _T( "MainFrame: No torrent is selected" ) );
		return;
	}

	/* start selected torrent(s) */
	{
		/* start data selected in ui list */
		for( i = 0; i < selecteditems.size(); i++ )
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

	wxLogDebug( _T( "MainFrame: OnMenuTorrentPause" ) );
	const SwashListCtrl::itemlist_t selecteditems = m_torrentlistctrl->GetSelectedItems();

	if( selecteditems.size() <= 0 )
	{
		wxLogWarning( _T( "MainFrame: No torrent is selected" ) );
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
	int i = 0;

	wxLogDebug( _T( "MainFrame: OnMenuTorrentStop" ) );
	const SwashListCtrl::itemlist_t selecteditems = m_torrentlistctrl->GetSelectedItems();

	if( selecteditems.size() <= 0 )
	{
		wxLogWarning( _T( "MainFrame: No torrent is selected" ) );
		return;
	}

	/* stop selected torrent(s) */
	{
		/* stop data selected in ui list */
		for( i = 0; i < selecteditems.size(); i++ )
		{
			m_btsession->StopTorrent( selecteditems[i] );
		}
	}
	UpdateUI();
}

//Show torrent properties
void MainFrame::OnMenuTorrentProperties( wxCommandEvent& event )
{
	int i = 0;

	wxLogDebug( _T( "MainFrame: OnMenuTorrentProperties" ) );
	const SwashListCtrl::itemlist_t selecteditems = m_torrentlistctrl->GetSelectedItems();

	if( selecteditems.size() <= 0 )
	{
		wxLogWarning( _T( "MainFrame: No torrent is selected" ) );
		return;
	}

	/* shows selected torrent(s) property*/
	{
		for( i = 0; i < selecteditems.size(); i++ )
		{
			shared_ptr<torrent_t> torrent = m_btsession->GetTorrent( selecteditems[i] );
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
	const SwashListCtrl::itemlist_t selecteditems = m_torrentlistctrl->GetSelectedItems();

	if( selecteditems.size() <= 0 )
	{
		wxLogWarning( _T( "MainFrame: No torrent is selected" ) );
		return;
	}

	wxString torrentwarning;

	if( selecteditems.size() > 1 )
	{ torrentwarning = wxString( _( "torrents" ) ); }
	else
	{ torrentwarning = wxString( _( "torrent" ) ); }

	if( deletedata )
	{ torrentwarning += _( " and data" ); }

	wxMessageDialog dialog( NULL, _( "Remove " ) + torrentwarning + _T( "?" ),
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
		int offset = 0;

		/* remove data in ui list, and bt list */
		for( i = 0; i < selecteditems.size(); i++ )
		{
			//shared_ptr<torrent_t> torrent = m_torrentlistitems[selecteditems[i] - (offset++)];
			m_btsession->RemoveTorrent( selecteditems[i], deletedata );
		}

		/* mark for list refresh */
		UpdateUI();
	}
	else
	{ wxLogInfo( _T( "Torrent removal cancelled" ) ); }
}

void MainFrame::OnMenuTorrentRemove( wxCommandEvent& event )
{
	wxLogDebug( _T( "MainFrame: OnMenuTorrentRemove" ) );
	RemoveTorrent( false );
}

void MainFrame::OnMenuTorrentRemoveData( wxCommandEvent& event )
{
	wxLogDebug( _T( "MainFrame: OnMenuTorrentRemoveData" ) );
	RemoveTorrent( true );
}

void MainFrame::OnMenuTorrentMoveUp( wxCommandEvent& event )
{
	int i = 0;

	wxLogDebug( _T( "MainFrame: OnMenuTorrentMoveUp" ) );
	const SwashListCtrl::itemlist_t selecteditems = m_torrentlistctrl->GetSelectedItems();

	if( selecteditems.size() <= 0 )
	{
		wxLogWarning( _T( "MainFrame: No torrent is selected" ) );
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

	wxLogDebug( _T( "MainFrame: OnMenuTorrentMoveDown" ) );
	const SwashListCtrl::itemlist_t selecteditems = m_torrentlistctrl->GetSelectedItems();

	if( selecteditems.size() <= 0 )
	{
		wxLogWarning( _T( "MainFrame: No torrent is selected" ) );
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
	wxLogDebug( _T( "MainFrame: OnMenuTorrentReannounce" ) );
	const SwashListCtrl::itemlist_t selecteditems = m_torrentlistctrl->GetSelectedItems();

	if( selecteditems.size() <= 0 )
	{
		wxLogWarning( _T( "MainFrame: No torrent is selected" ) );
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

	wxLogDebug( _T( "MainFrame: OnMenuTorrentRecheck" ) );
	const SwashListCtrl::itemlist_t selecteditems = m_torrentlistctrl->GetSelectedItems();

	if( selecteditems.size() <= 0 )
	{
		wxLogWarning( _T( "MainFrame: No torrent is selected" ) );
		return;
	}

	/* reannounce selected torrent(s) */
	{
		for( i = 0; i < selecteditems.size(); i++ )
		{
			//m_btsession->ReannounceTorrent((m_torrentlistitems[selecteditems[i]]));
			//XXX Workaround for libtorrent lack of recheck redo if supported in core
			shared_ptr<torrent_t> torrent = m_btsession->GetTorrent( selecteditems[i] );
			if(!torrent || !torrent->isvalid)
			{
				return;
			}
			enum torrent_state prev_state = ( enum torrent_state ) torrent->config->GetTorrentState();

			if( ( prev_state == TORRENT_STATE_START ) ||
					( prev_state == TORRENT_STATE_FORCE_START ) ||
					( prev_state == TORRENT_STATE_PAUSE ) )
			{
				wxLogInfo( _T( "%s: Stop for checking" ), torrent->name.c_str() );
				m_btsession->StopTorrent( torrent );
			}

			wxString fastresumefile = wxGetApp().SaveTorrentsPath() + wxGetApp().PathSeparator() + torrent->hash + _T( ".fastresume" );

			if( wxFileExists( fastresumefile ) )
			{
				wxLogWarning( _T( "%s: Remove fast resume data" ), torrent->name.c_str() );

				if( ! wxRemoveFile( fastresumefile ) )
				{
					wxLogError( _T( "%s: Remove fast resume data: failed" ), torrent->name.c_str() );
				}
			}

			if( ( prev_state == TORRENT_STATE_START ) ||
					( prev_state == TORRENT_STATE_FORCE_START ) )
			{
				wxLogInfo( _T( "%s: Stop for checking" ), torrent->name.c_str() );
				m_btsession->StartTorrent( torrent, ( prev_state == TORRENT_STATE_FORCE_START ) );
			}
			else
				if( prev_state == TORRENT_STATE_PAUSE )
				{
					m_btsession->PauseTorrent( torrent );
				}
		}
	}
}

shared_ptr<torrent_t> MainFrame::GetSelectedTorrent()
{
	shared_ptr<torrent_t> torrent;

	const SwashListCtrl::itemlist_t selecteditems = m_torrentlistctrl->GetSelectedItems();

	if( selecteditems.size() > 0 )
	{
		torrent = m_btsession->GetTorrent( selecteditems[0] );
	}

	return torrent;
}

void MainFrame::ShowPreferences()
{
	if( m_swashsetting == 0 )
	{ m_swashsetting = new SwashSetting( this, m_config ); }

	if( m_swashsetting->ShowModal() == wxID_OK )
	{
		//XXX settorrent session
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
	wxLogDebug( _T( "Command  %d, %s, item %p" ), event.GetId(), langinfo->CanonicalName.c_str(), event.GetEventObject() );

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
	wxString numpeers/* = wxString::Format( _T( "%ld" ),  m_btsession->GetLibTorrent()->status().dht_nodes )*/;
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
	wxString tooltips = wxString::Format(_T("%s %s\n%-20s : %s\n%-20s : %s\n%-20s : %s\n%-20s : %s\n%-20s : %s"),
		APPNAME, BITSWASH_VERSION,
		_("Download Rate"), GetStatusDownloadRate().c_str(),
		_("Upload Rate"), GetStatusUploadRate().c_str(),
		_("Peers"), GetStatusPeers().c_str(),
		_("DHT Nodes"), GetStatusDHT().c_str(),
		_("Incoming"), GetStatusIncoming().c_str());
	m_swashtrayicon->SetIcon( m_trayicon, tooltips );
}

void MainFrame::ShowSystray( bool show )
{
	if( show )
	{
		m_swashtrayicon = new SwashTrayIcon( this );

		if( !m_swashtrayicon->SetIcon( m_trayicon, wxT( "Bitswash" ) ) )
		{ wxMessageBox( _( "Could not set icon." ) ); }

		SetIcon( m_frameicon );
	}
	else
	{
		if( m_swashtrayicon != NULL )
		{
			delete m_swashtrayicon;
			m_swashtrayicon = NULL;
		}
	}
}

void MainFrame::SetLogSeverity()
{
	//wxASSERT(s!= NULL);
	int wx_loglevel = wxLOG_Debug - m_config->GetLogSeverity();
	wxLog::SetLogLevel( wx_loglevel );

	if( wx_loglevel >= wxLOG_Info )
	{ wxLog::SetVerbose( true ); }
	else
	{ wxLog::SetVerbose( false ); }

	m_btsession->SetLogSeverity();
	//  s->set_severity_level((lt::alert::severity_t)m_config->GetLogSeverity());
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
	return torrentMenu;
}

