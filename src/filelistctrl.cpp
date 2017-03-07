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
// Class: FileListCtrl
//
// Created by: lim k. c. <admin@bitswash.org>
// Created on: Mon Feb  5 11:09:21 2007
//
#include <wx/utils.h> 
#include <wx/filename.h>

#include "torrentinfo.h"
#include "filelistctrl.h"
#include "functions.h"

// resources
#include "../icons/checked.xpm"
#include "../icons/unchecked.xpm"
#include "../icons/checked_dis.xpm"
#include "../icons/unchecked_dis.xpm"

enum
{
	FILELISTCTRL_MENU_PRIORITY0 = 10000,
	FILELISTCTRL_MENU_PRIORITY1,
	FILELISTCTRL_MENU_PRIORITY2,
	FILELISTCTRL_MENU_PRIORITY3,
	FILELISTCTRL_MENU_PRIORITY4,
	FILELISTCTRL_MENU_PRIORITY5,
	FILELISTCTRL_MENU_PRIORITY6,
	FILELISTCTRL_MENU_PRIORITY7,
	FILELISTCTRL_MENU_OPENPATH,
};


BEGIN_EVENT_TABLE( FileListCtrl, SwashListCtrl )
	EVT_MENU( FILELISTCTRL_MENU_PRIORITY0, FileListCtrl::OnMenuPriority )
	EVT_MENU( FILELISTCTRL_MENU_PRIORITY1, FileListCtrl::OnMenuPriority )
	EVT_MENU( FILELISTCTRL_MENU_PRIORITY2, FileListCtrl::OnMenuPriority )
	EVT_MENU( FILELISTCTRL_MENU_PRIORITY3, FileListCtrl::OnMenuPriority )
	EVT_MENU( FILELISTCTRL_MENU_PRIORITY4, FileListCtrl::OnMenuPriority )
	EVT_MENU( FILELISTCTRL_MENU_PRIORITY5, FileListCtrl::OnMenuPriority )
	EVT_MENU( FILELISTCTRL_MENU_PRIORITY6, FileListCtrl::OnMenuPriority )
	EVT_MENU( FILELISTCTRL_MENU_PRIORITY7, FileListCtrl::OnMenuPriority )
	EVT_MENU( FILELISTCTRL_MENU_OPENPATH,  FileListCtrl::OnMenuOpenPath )
	EVT_LEFT_DOWN(FileListCtrl::OnLeftDown)
END_EVENT_TABLE()
// FileListCtrl


enum torrentlistcolumnid
{
	FILELIST_COLUMN_SELECTED = 0,
	FILELIST_COLUMN_FILE,
	FILELIST_COLUMN_SIZE,
	FILELIST_COLUMN_DOWNLOAD,
	FILELIST_COLUMN_PROGRESS,
};

#ifndef __WXMSW__
static SwashColumnItem filelistcols[] =
{
	{ id: FILELIST_COLUMN_SELECTED, name: _T( "Indicator" ), title: _T( "" ), tooltip: _T( "" ), width: 24, show: true },
	{ id: FILELIST_COLUMN_FILE, name: _( "File" ), title: _( "File" ), tooltip: _( "Files in torrent" ), width: 315, show: true},
	{ id: FILELIST_COLUMN_SIZE, name: _( "Size" ), title: _( "Size" ), tooltip: _( "File size" ), width: 88, show: true},
	{ id: FILELIST_COLUMN_FILE, name: _( "Download" ), title: _( "Download" ), tooltip: _( "File priority" ), width: 88, show: true},
	{ id: FILELIST_COLUMN_PROGRESS, name: _( "Progress" ), title: _( "Progress" ), tooltip: _( "File download progress" ), width: 88, show: true },
};
#else
static SwashColumnItem filelistcols[] =
{
	{ FILELIST_COLUMN_SELECTED, _T( "Indicator" ), _T( "" ), _T( "" ), 24, true },
	{ FILELIST_COLUMN_FILE,  _( "File" ), _( "File" ), _( "Files in torrent" ), 315, true},
	{ FILELIST_COLUMN_SIZE, _( "Size" ), _( "Size" ), _( "File size" ), 88, true},
	{ FILELIST_COLUMN_FILE, _( "Download" ),  _( "Download" ), _( "File priority" ), 88, true},
	{ FILELIST_COLUMN_PROGRESS,  _( "Progress" ), _( "Progress" ), _( "File download progress" ),  88, true },
};

#endif
FileListCtrl::FileListCtrl( wxWindow *parent,
							const wxString settings,
							const wxWindowID id,
							const wxPoint& pos,
							const wxSize& size,
							long style )
	: SwashListCtrl( parent, SWASHLISTCOL_SIZE( filelistcols ), filelistcols, settings, id, pos, size, style ), m_imageList(16, 16, TRUE)
{
	SetImageList(&m_imageList, wxIMAGE_LIST_SMALL);

	// the add order must respect the wxCLC_XXX_IMGIDX defines in the headers !
	/*0*/m_imageList.Add(wxIcon(unchecked_xpm));
	/*1*/m_imageList.Add(wxIcon(checked_xpm));
	/*2*/m_imageList.Add(wxIcon(unchecked_dis_xpm));
	/*3*/m_imageList.Add(wxIcon(checked_dis_xpm));
    
    /* XXX openfile */
    m_contextmenu.Append( FILELISTCTRL_MENU_PRIORITY0, _( "Do not download" ) );
    m_contextmenu.AppendSeparator();
    m_contextmenu.Append( FILELISTCTRL_MENU_PRIORITY1, _( "Lowest Priority" ) );
    m_contextmenu.Append( FILELISTCTRL_MENU_PRIORITY4, _( "Normal Priority" ) );
    m_contextmenu.Append( FILELISTCTRL_MENU_PRIORITY7, _( "Highest Priority" ) );
	m_contextmenu.AppendSeparator();
    m_contextmenu.Append( FILELISTCTRL_MENU_OPENPATH, _( "Open containing folder" ) );
}

FileListCtrl::~FileListCtrl()
{
}

wxString FileListCtrl::GetItemValue( long item, long columnid ) const
{
	wxString ret(_T( "" ));
	//XXX backward compatible
	int nopriority = 0;
	wxLogDebug( _T( "FileListCtrl column %ld of item %ld\n" ), columnid, item );
	shared_ptr<torrent_t> pTorrent;

	if( m_pTorrent )
	{
		pTorrent = m_pTorrent;
	}
	else
	{
        MainFrame* pMainFrame = ( MainFrame* )( wxGetApp().GetTopWindow() );
		pTorrent = pMainFrame->GetSelectedTorrent();
	}

	if( !pTorrent )
	{ return ret; }

	libtorrent::torrent_handle h = pTorrent->handle;
	libtorrent::file_entry f_entry;
	libtorrent::torrent_info const& torrent_info = *( pTorrent->info );
	std::vector<int> filespriority = pTorrent->config->GetFilesPriorities();

	if( filespriority.size() != torrent_info.num_files() )
	{
		nopriority = 1;
	}

	// some priority has no name and not made an option yet
	//
	const static wxChar* priority[] =
	{
		_( "No" ),
		_( "Lowest" ),
		_T( "2" ), /* unused priority level */
		_T( "3" ), /* unused priority level */
		_( "Normal" ),
		_T( "5" ), /* unused priority level */
		_T( "6" ), /* unused priority level */
		_( "Higest" )
	};
	std::vector<float> f_progress;
	f_entry = torrent_info.file_at( item );

	if( h.is_valid() )
	{ h.file_progress( f_progress ); }

	switch( columnid )
	{
	case FILELIST_COLUMN_FILE:
		ret = wxString( wxConvUTF8.cMB2WC( f_entry.path.c_str() ) );
		break;

	case FILELIST_COLUMN_SIZE:
		ret = HumanReadableByte( ( wxDouble ) f_entry.size );
		break;

	case FILELIST_COLUMN_DOWNLOAD:
		if( nopriority )
		{ ret = priority[BITTORRENT_FILE_NORMAL]; }
		else
		{ ret = priority[filespriority[item]]; }

		break;

	case FILELIST_COLUMN_PROGRESS:
		if( h.is_valid() )
		{ return wxString::Format( _T( "%.02f%%" ), ( ( wxDouble ) f_progress[item] * 100 ) ); }
		else
		{ ret = _T( "0.00" ); }

		break;

	default:
		break;
	}

	return ret;
}

int FileListCtrl::GetItemColumnImage(long item, long columnid) const
{
	int ret = -1;
	if(columnid == FILELIST_COLUMN_SELECTED)
	{
		//XXX backward compatible
		int nopriority = 0;
		wxLogDebug( _T( "FileListCtrl column %ld of item %ld\n" ), columnid, item );
		shared_ptr<torrent_t> pTorrent;

		if( m_pTorrent )
		{
			pTorrent = m_pTorrent;
		}
		else
		{
			MainFrame* pMainFrame = ( MainFrame* )( wxGetApp().GetTopWindow() );
			pTorrent = pMainFrame->GetSelectedTorrent();
		}

		if (pTorrent)
		{
			libtorrent::torrent_info const& torrent_info = *(pTorrent->info);
			std::vector<int> filespriority = pTorrent->config->GetFilesPriorities();

			if (filespriority.size() != torrent_info.num_files())
			{
				nopriority = 1;
			}
			/*0--unchecked_xpm*/
			/*1--checked_xpm*/
			/*2--unchecked_dis_xpm*/
			/*3--checked_dis_xpm*/

			switch (columnid)
			{
			case FILELIST_COLUMN_SELECTED:

				if (nopriority || filespriority[item] != BITTORRENT_FILE_NONE)
					ret = 1;
				else
					ret = 0;
				break;

			default:
				break;
			}
		}
	}
	return ret;
}

void FileListCtrl::OnLeftDown(wxMouseEvent& event)
{
	int flags;
	long subitem = -1;
#if wxCHECK_VERSION(2,8,0)
	long item = HitTest(event.GetPosition(), flags, &subitem);
#else
	long item = HitTest(event.GetPosition(), flags );
#endif

	switch ( flags )
	{
		case wxLIST_HITTEST_ONITEMICON: /*FILELIST_COLUMN_SELECTED*/
			{
				//XXX backward compatible
				int nopriority = 0;
				shared_ptr<torrent_t> pTorrent;

				if( m_pTorrent )
				{
					pTorrent = m_pTorrent;
				}
				else
				{
					MainFrame* pMainFrame = ( MainFrame* )( wxGetApp().GetTopWindow() );
					pTorrent = pMainFrame->GetSelectedTorrent();
				}

				if (pTorrent)
				{
 					libtorrent::torrent_info const& torrent_info = *(pTorrent->info);
					std::vector<int>& filespriority = pTorrent->config->GetFilesPriorities();

					if ((item < filespriority.size() ) && filespriority[item] != 0)
					{
						filespriority[item] = BITTORRENT_FILE_NONE;
					}
					else
					{
						filespriority[item] = BITTORRENT_FILE_NORMAL; /*Normal*/
					}
					Refresh(false);
					return;
				}
				break;
			}
		default: 
			break;
	}
    event.Skip();
}

void FileListCtrl::OnLeftDClick(wxMouseEvent& event)
{
	shared_ptr<torrent_t> pTorrent;

	if( m_pTorrent )
	{ pTorrent = m_pTorrent; }
	else
	{
		MainFrame* pMainFrame = ( MainFrame* )wxGetApp().GetTopWindow();
		pTorrent = pMainFrame->GetSelectedTorrent();
	}

	if( pTorrent )
	{
		int selectedfiles = GetFirstSelected();
		if( selectedfiles != -1 )
		{
			libtorrent::torrent_info const& torrentinfo = *(pTorrent->info);
			std::vector<int> & filespriority = pTorrent->config->GetFilesPriorities();
			libtorrent::file_entry f_entry = torrentinfo.file_at( selectedfiles );
			if(filespriority.at(selectedfiles) != BITTORRENT_FILE_NONE)
			{
				f_entry = torrentinfo.file_at( selectedfiles );
				wxFileName filename = pTorrent->config->GetDownloadPath() + wxString::FromUTF8(f_entry.path.c_str());
				filename.MakeAbsolute();
#if  defined(__WXMSW__) 
				wxExecute(_T("Explorer ")+filename.GetFullName(), wxEXEC_ASYNC, NULL); 
#elif defined(__APPLE__)
				wxExecute(_T("/usr/bin/open "+filename.GetFullName(), wxEXEC_ASYNC, NULL);
#elif defined(__WXGTK__)
				wxLaunchDefaultBrowser(_T("file://")+filename.GetFullName());
#endif
			}
		}
	}
}


void FileListCtrl::ShowContextMenu( const wxPoint& pos )
{
	static int menuids[] = {
		FILELISTCTRL_MENU_PRIORITY0,
		FILELISTCTRL_MENU_PRIORITY1,
		//FILELISTCTRL_MENU_PRIORITY2,
		//FILELISTCTRL_MENU_PRIORITY3,
		FILELISTCTRL_MENU_PRIORITY4,
		//FILELISTCTRL_MENU_PRIORITY5,
		//FILELISTCTRL_MENU_PRIORITY6,
		FILELISTCTRL_MENU_PRIORITY7,
	};

	bool enable_openpath = false;
	int disable_priority = FILELISTCTRL_MENU_PRIORITY0;
	if(GetSelectedItemCount() == 1)
	{
		shared_ptr<torrent_t> pTorrent;
		if( m_pTorrent )
		{ pTorrent = m_pTorrent; }
		else
		{
			MainFrame* pMainFrame = ( MainFrame* )wxGetApp().GetTopWindow();
			pTorrent = pMainFrame->GetSelectedTorrent();
		}

		if(!pTorrent) return;
		wxString filepath = pTorrent->config->GetDownloadPath();
		enable_openpath = wxFileName::DirExists( filepath );
		std::vector<int> & filespriority = pTorrent->config->GetFilesPriorities();
		std::vector<int>::iterator file_it ;
		
		if( filespriority.size() != pTorrent->info->num_files() )
		{
			std::vector<int> deffilespriority( pTorrent->info->num_files(), BITTORRENT_FILE_NORMAL );
			filespriority.swap( deffilespriority );
		}
		
		int selectedfiles = GetFirstSelected();

		if( selectedfiles != -1 )
		{
			file_it = filespriority.begin() + selectedfiles;
			disable_priority = (*file_it + FILELISTCTRL_MENU_PRIORITY0);
			enable_openpath = (enable_openpath && (*file_it));
		}
		else
			enable_openpath = false;
	}
	else
	{
		disable_priority = FILELISTCTRL_MENU_PRIORITY7 + 1000;
	}

	m_contextmenu.Enable(FILELISTCTRL_MENU_OPENPATH, enable_openpath);
	for(int i = 0; i < (sizeof(menuids)/sizeof(menuids[0])); ++i)
	{
		m_contextmenu.Enable(menuids[i], (disable_priority != menuids[i]));
	}

	PopupMenu( &m_contextmenu, pos );
}

/* update files priority to torrent config */
void FileListCtrl::OnMenuPriority( wxCommandEvent& event )
{
	int priority = event.GetId() - FILELISTCTRL_MENU_PRIORITY0;
	shared_ptr<torrent_t> pTorrent;

	if( m_pTorrent )
	{ pTorrent = m_pTorrent; }
	else
	{
        MainFrame* pMainFrame = ( MainFrame* )wxGetApp().GetTopWindow();
	    pTorrent = pMainFrame->GetSelectedTorrent();
    }

	if( !pTorrent )
	{ return; }

	//XXX get some default priority definition
	// Get existing priority from pTorrent->t_cfg;
	std::vector<int> & filespriority = pTorrent->config->GetFilesPriorities();
	std::vector<int>::iterator file_it ;

	if( filespriority.size() != pTorrent->info->num_files() )
	{
        std::vector<int> deffilespriority( pTorrent->info->num_files(), BITTORRENT_FILE_NORMAL );
		filespriority.swap( deffilespriority );
	}

	int selectedfiles = GetFirstSelected();

	while( selectedfiles != -1 )
	{
		file_it = filespriority.begin() + selectedfiles;
		*file_it = priority;
		selectedfiles = GetNextSelected( selectedfiles );
	}

	//pTorrent->config->SetFilesPriority(filespriority);
	if( !m_pTorrent )
	{ pTorrent->config->Save(); }

	wxGetApp().GetBitTorrentSession()->ConfigureTorrentFilesPriority( pTorrent );
	libtorrent::file_entry f_entry;
	wxULongLong_t total_selected = 0;
	libtorrent::torrent_info const& torrentinfo = *(pTorrent->info);

	for(size_t i = 0; i < filespriority.size(); ++i)
	{
		if(filespriority.at(i) != BITTORRENT_FILE_NONE)
		{
			f_entry = torrentinfo.file_at( i );
			total_selected += f_entry.size;
		}
	}

	pTorrent->config->SetSelectedSize(total_selected);
	Refresh( false );
}

void FileListCtrl::OnMenuOpenPath( wxCommandEvent& event )
{
	shared_ptr<torrent_t> pTorrent;

	if( m_pTorrent )
	{ pTorrent = m_pTorrent; }
	else
	{
        MainFrame* pMainFrame = ( MainFrame* )wxGetApp().GetTopWindow();
	    pTorrent = pMainFrame->GetSelectedTorrent();
    }

	if( pTorrent )
	{
		int selectedfiles = GetFirstSelected();
		if( selectedfiles != -1 )
		{
			libtorrent::torrent_info const& torrentinfo = *(pTorrent->info);
			std::vector<int> & filespriority = pTorrent->config->GetFilesPriorities();
			libtorrent::file_entry f_entry = torrentinfo.file_at( selectedfiles );
			if(filespriority.at(selectedfiles) != BITTORRENT_FILE_NONE)
			{
				f_entry = torrentinfo.file_at( selectedfiles );
				wxFileName filename = pTorrent->config->GetDownloadPath() + wxString::FromUTF8(f_entry.path.c_str());
				filename.MakeAbsolute();
#if  defined(__WXMSW__) 
				wxExecute(_T("Explorer ")+filename.GetFullPath(), wxEXEC_ASYNC, NULL); 
#elif defined(__APPLE__)
				wxExecute(_T("/usr/bin/open "+filename.GetFullPath(), wxEXEC_ASYNC, NULL);
#elif defined(__WXGTK__)
				wxLaunchDefaultBrowser(_T("file://")+filename.GetFullPath());
#endif
			}
		}
	}
}

