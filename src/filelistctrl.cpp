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
#include "trackerlistctrl.h"
#include "filelistctrl.h"
#include "functions.h"
#include "mainframe.h"

// resources
#include "../icons/checked.xpm"
#include "../icons/unchecked.xpm"
#include "../icons/checked_dis.xpm"
#include "../icons/unchecked_dis.xpm"

//namespace lt = libtorrent;

const long FileListCtrl::FILELISTCTRL_MENU_PRIORITY0 = wxNewId();
const long FileListCtrl::FILELISTCTRL_MENU_PRIORITY1 = wxNewId();
const long FileListCtrl::FILELISTCTRL_MENU_PRIORITY2 = wxNewId();
const long FileListCtrl::FILELISTCTRL_MENU_PRIORITY3 = wxNewId();
const long FileListCtrl::FILELISTCTRL_MENU_PRIORITY4 = wxNewId();
const long FileListCtrl::FILELISTCTRL_MENU_PRIORITY5 = wxNewId();
const long FileListCtrl::FILELISTCTRL_MENU_PRIORITY6 = wxNewId();
const long FileListCtrl::FILELISTCTRL_MENU_PRIORITY7 = wxNewId();
const long FileListCtrl::FILELISTCTRL_MENU_OPENPATH = wxNewId();

// FileListCtrl

FileListCtrl::wxCmdEvtHandlerMap_t FileListCtrl::m_menu_evt_map[]=
{
	{FILELISTCTRL_MENU_PRIORITY0, &FileListCtrl::OnMenuPriority},
	{FILELISTCTRL_MENU_PRIORITY1, &FileListCtrl::OnMenuPriority},
	{FILELISTCTRL_MENU_PRIORITY2, &FileListCtrl::OnMenuPriority},
	{FILELISTCTRL_MENU_PRIORITY3, &FileListCtrl::OnMenuPriority},
	{FILELISTCTRL_MENU_PRIORITY4, &FileListCtrl::OnMenuPriority},
	{FILELISTCTRL_MENU_PRIORITY5, &FileListCtrl::OnMenuPriority},
	{FILELISTCTRL_MENU_PRIORITY6, &FileListCtrl::OnMenuPriority},
	{FILELISTCTRL_MENU_PRIORITY7, &FileListCtrl::OnMenuPriority},
	{FILELISTCTRL_MENU_OPENPATH,  &FileListCtrl::OnMenuOpenPath},
};

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
	{ id: FILELIST_COLUMN_DOWNLOAD, name: _( "Download" ), title: _( "Download" ), tooltip: _( "File priority" ), width: 88, show: true},
	{ id: FILELIST_COLUMN_PROGRESS, name: _( "Progress" ), title: _( "Progress" ), tooltip: _( "File download progress" ), width: 88, show: true },
};
#else
static SwashColumnItem filelistcols[] =
{
	{ FILELIST_COLUMN_SELECTED, _T( "Indicator" ), _T( "" ), _T( "" ), 24, true },
	{ FILELIST_COLUMN_FILE,  _( "File" ), _( "File" ), _( "Files in torrent" ), 315, true},
	{ FILELIST_COLUMN_SIZE, _( "Size" ), _( "Size" ), _( "File size" ), 88, true},
	{ FILELIST_COLUMN_DOWNLOAD, _( "Download" ),  _( "Download" ), _( "File priority" ), 88, true},
	{ FILELIST_COLUMN_PROGRESS,  _( "Progress" ), _( "Progress" ), _( "File download progress" ),  88, true },
};
#endif

// sort using a custom function object
struct fsDsc{
	bool operator()(long a, long b) const
	{
		return allfiles->file_size(lt::file_index_t(a)) > allfiles->file_size(lt::file_index_t(b));
	}
	lt::file_storage const * allfiles;
} ;

struct fsAsc{
	bool operator()(long a, long b) const
	{
		return allfiles->file_size(lt::file_index_t(a)) < allfiles->file_size(lt::file_index_t(b));
	}
	lt::file_storage const * allfiles;
} ;

struct fnDsc{
	bool operator()(long a, long b) const
	{
		return allfiles->file_name(lt::file_index_t(a)) > allfiles->file_name(lt::file_index_t(b));
	}
	lt::file_storage const * allfiles;
} ;

struct fnAsc{
	bool operator()(long a, long b) const
	{
		return allfiles->file_name(lt::file_index_t(a)) < allfiles->file_name(lt::file_index_t(b));
	}
	lt::file_storage const * allfiles;
} ;

struct prgDsc{
	bool operator()(long a, long b) const
	{
		std::vector<boost::int64_t> f_progress;
		h.file_progress( f_progress, lt::torrent_handle::piece_granularity);

		return ((100 * f_progress[a])/ (allfiles->file_size(lt::file_index_t(a)))) > (( 100 * f_progress[b])/ ((wxDouble)allfiles->file_size(lt::file_index_t(b))));
	}
	lt::torrent_handle h;
	lt::file_storage const * allfiles;
} ;

struct prgAsc{
	bool operator()(long a, long b) const
	{
		std::vector<boost::int64_t> f_progress;
		h.file_progress( f_progress, lt::torrent_handle::piece_granularity);

		return ((100 * f_progress[a])/ (allfiles->file_size(lt::file_index_t(a)))) < (( 100 * f_progress[b])/ ((wxDouble)allfiles->file_size(lt::file_index_t(b))));
	}
	lt::torrent_handle h;
	lt::file_storage const * allfiles;
} ;

FileListCtrl::FileListCtrl( wxWindow *parent,
							const wxString settings,
							const wxWindowID id,
							const wxPoint& pos,
							const wxSize& size,
							long style )
	: SwashListCtrl( parent, SWASHLISTCOL_SIZE( filelistcols ), filelistcols, settings, id, pos, size, style ), m_imageList(16, 16, TRUE), m_lastclickcol(-1)
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
	for(size_t i = 0; i < sizeof(m_menu_evt_map)/sizeof(m_menu_evt_map[0]); ++i)
	{
		Bind( wxEVT_MENU, m_menu_evt_map[i].method, this, m_menu_evt_map[i].evtTag );
	}
	Bind( wxEVT_LEFT_DOWN, &FileListCtrl::OnLeftDown, this );
}

FileListCtrl::~FileListCtrl()
{
}

void FileListCtrl::OnColClick(wxListEvent& event)
{
	std::shared_ptr<torrent_t> pTorrent(m_pTorrent);

	if (pTorrent.use_count() <= 1)
	{
		MainFrame* pMainFrame = dynamic_cast< MainFrame* >( wxGetApp().GetTopWindow() );
		wxASSERT(pMainFrame != nullptr);
		pTorrent = pMainFrame->GetSelectedTorrent();
		
		if(m_pTorrent.use_count() == 1)
			m_pTorrent.reset();
	}

	int col = event.GetColumn();

	static bool second = false;
	second = (m_lastclickcol == col) ? (!second) : false;
	switch(col)
	{
		case FILELIST_COLUMN_FILE:
			if(second)
			{
				fnDsc st;
				st.allfiles = &(pTorrent->info->files());
				std::sort(pTorrent->fileindex.begin(), pTorrent->fileindex.end(), st);
			}
			else
			{
				fnAsc st;
				st.allfiles = &(pTorrent->info->files());
				std::sort(pTorrent->fileindex.begin(), pTorrent->fileindex.end(), st);
			}
			break;
		case FILELIST_COLUMN_PROGRESS:
			if(second)
			{
				prgDsc st;
				st.h = pTorrent->handle;
				st.allfiles = &(pTorrent->info->files());
				std::sort(pTorrent->fileindex.begin(), pTorrent->fileindex.end(), st);
			}
			else
			{
				prgAsc st;
				st.h = pTorrent->handle;
				st.allfiles = &(pTorrent->info->files());
				std::sort(pTorrent->fileindex.begin(), pTorrent->fileindex.end(), st);
			}
			break;
		case FILELIST_COLUMN_SIZE:
			if(second)
			{
				fsAsc st;
				st.allfiles = &(pTorrent->info->files());
				std::sort(pTorrent->fileindex.begin(), pTorrent->fileindex.end(), st);
				break;
			}
		default:
			fsDsc st;
			st.allfiles = &(pTorrent->info->files());
			std::sort(pTorrent->fileindex.begin(), pTorrent->fileindex.end(), st);
	}

	m_lastclickcol = col;
	Refresh();
}

long FileListCtrl::ConvertItemId(std::shared_ptr<torrent_t>& torrent, long item) const
{
	if(torrent->fileindex.size() != torrent->info->num_files())
	{
		torrent->fileindex.resize(torrent->info->num_files());
		for(int i = 0; i < torrent->fileindex.size(); ++i)
		{
			torrent->fileindex[i] = i;
		}
	}

	return torrent->fileindex[item];
}

wxString FileListCtrl::GetItemValue( long item, long columnid ) const
{
	wxString ret(_T( "" ));
	//XXX backward compatible
	//wxLogDebug( _T( "FileListCtrl column %ld of item %ld" ), columnid, item );
	std::shared_ptr<torrent_t> pTorrent(m_pTorrent);

	if (pTorrent.use_count() <= 1)
	{
		MainFrame* pMainFrame = dynamic_cast< MainFrame* >( wxGetApp().GetTopWindow() );
		wxASSERT(pMainFrame != nullptr);
		pTorrent = pMainFrame->GetSelectedTorrent();
	}

	if( !pTorrent )
	{ return ret; }

	wxASSERT(item < pTorrent->info->num_files());

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

	lt::torrent_info const& torrent_info = *( pTorrent->info );
	if(torrent_info.is_valid())
	{
		if(pTorrent->info->num_files() > 1)
		{
			item = ConvertItemId(pTorrent, item);
		}

		switch( columnid )
		{
		case FILELIST_COLUMN_FILE:
			{
				lt::file_storage const& allfiles = torrent_info.files();
				ret = wxString( wxConvUTF8.cMB2WC( (allfiles.file_name(lt::file_index_t(item))).to_string().c_str() ) );
				break;
			}
		case FILELIST_COLUMN_SIZE:
			{
				lt::file_storage const& allfiles = torrent_info.files();
				ret = HumanReadableByte( ( wxDouble ) (allfiles.file_size(lt::file_index_t(item))) );
				break;
			}
		case FILELIST_COLUMN_DOWNLOAD:
			{
				bool nopriority = false;
				std::vector<lt::download_priority_t>& filespriority = pTorrent->config->GetFilesPriorities();
				
				if( filespriority.size() != torrent_info.num_files() )
				{
					nopriority = true;
				}
				if( nopriority )
				{ ret = priority[BITTORRENT_FILE_NORMAL]; }
				else
				{
					wxASSERT(item < filespriority.size());
					wxASSERT(filespriority[item] < lt::download_priority_t(sizeof(priority)));
					ret = priority[std::uint8_t(filespriority[item])];
				}

				break;
			}
		case FILELIST_COLUMN_PROGRESS:
			{
				lt::torrent_handle h = pTorrent->handle;
				lt::file_storage const& allfiles = torrent_info.files();

				if( h.is_valid() )
				{
					std::vector<boost::int64_t> f_progress;
					h.file_progress( f_progress, lt::torrent_handle::piece_granularity);
					if(item < f_progress.size())
						return wxString::Format( _T( "%.02f%%" ), ( (( wxDouble ) f_progress[item])/ ((wxDouble)allfiles.file_size(lt::file_index_t(item))) * 100 ) );
				}

				ret = _T( "0.00" );

				break;
			}
		default:
			break;
		}
	}

	return ret;
}

int FileListCtrl::GetItemColumnImage(long item, long columnid) const
{
	int ret = -1;
	const static lt::download_priority_t file_none(BITTORRENT_FILE_NONE);
	const static lt::download_priority_t file_normal(BITTORRENT_FILE_NORMAL);
	if(columnid == FILELIST_COLUMN_SELECTED)
	{
		//XXX backward compatible
		bool nopriority = false;
		//wxLogDebug( _T( "FileListCtrl column %ld of item %ld\n" ), columnid, item );
		std::shared_ptr<torrent_t> pTorrent(m_pTorrent);

		if (pTorrent.use_count() <= 1)
		{
			MainFrame* pMainFrame = dynamic_cast< MainFrame* >( wxGetApp().GetTopWindow() );
			wxASSERT(pMainFrame != nullptr);
			pTorrent = pMainFrame->GetSelectedTorrent();
		}

		if (pTorrent)
		{
			lt::torrent_info const& torrent_info = *(pTorrent->info);
			std::vector<lt::download_priority_t>& filespriority = pTorrent->config->GetFilesPriorities();

			if(pTorrent->info->num_files() > 1)
			{
				item = ConvertItemId(pTorrent, item);
			}
			if (filespriority.size() != torrent_info.num_files())
			{
				std::vector<lt::download_priority_t> deffilespriority( pTorrent->info->num_files(), file_normal );
				filespriority.swap( deffilespriority );
				nopriority = true;
			}
			
			/*0--unchecked_xpm*/
			/*1--checked_xpm*/
			/*2--unchecked_dis_xpm*/
			/*3--checked_dis_xpm*/
			switch (columnid)
			{
			case FILELIST_COLUMN_SELECTED:

				if (nopriority || filespriority[item] != file_none)
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
			bool nopriority = false;
			std::shared_ptr<torrent_t> pTorrent(m_pTorrent);

			if (pTorrent.use_count() <= 1)
			{
				MainFrame* pMainFrame = dynamic_cast< MainFrame* >( wxGetApp().GetTopWindow() );
				wxASSERT(pMainFrame != nullptr);
				pTorrent = pMainFrame->GetSelectedTorrent();
				
				if(m_pTorrent.use_count() == 1)
					m_pTorrent.reset();
			}

			if (pTorrent)
			{
				if(pTorrent->info->num_files() > 1)
				{
					item = ConvertItemId(pTorrent, item);
				}
				lt::torrent_info const& torrent_info = *(pTorrent->info);
				std::vector<lt::download_priority_t>& filespriority = pTorrent->config->GetFilesPriorities();
				if( filespriority.size() != torrent_info.num_files() )
				{
					nopriority = true;
					
					if( filespriority.size() != pTorrent->info->num_files() )
					{
						std::vector<lt::download_priority_t> deffilespriority( pTorrent->info->num_files(), lt::download_priority_t(BITTORRENT_FILE_NORMAL) );
						filespriority.swap( deffilespriority );
					}
				}

				if (nopriority || ((item < filespriority.size() ) && filespriority[item] != lt::download_priority_t(BITTORRENT_FILE_NONE)))
				{
					filespriority[item] = lt::download_priority_t(BITTORRENT_FILE_NONE);
				}
				else
				{
					filespriority[item] = lt::download_priority_t(BITTORRENT_FILE_NORMAL); /*Normal*/
				}

				wxGetApp().GetBitTorrentSession()->ConfigureTorrentFilesPriority( pTorrent );
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
	std::shared_ptr<torrent_t> pTorrent(m_pTorrent);

	if(pTorrent.use_count() <= 1)
	{
 		MainFrame* pMainFrame = dynamic_cast< MainFrame* >( wxGetApp().GetTopWindow() );
		wxASSERT(pMainFrame != nullptr);
		pTorrent = pMainFrame->GetSelectedTorrent();
		
		if(m_pTorrent.use_count() == 1)
			m_pTorrent.reset();
	}

	if( pTorrent )
	{
		int selectedfiles = GetFirstSelected();
		if( selectedfiles != -1 )
		{
			bool nopriority = false;
			lt::torrent_info const& torrentinfo = *(pTorrent->info);
			std::vector<lt::download_priority_t> & filespriority = pTorrent->config->GetFilesPriorities();
			
			if( filespriority.size() != torrentinfo.num_files() )
			{
				nopriority = true;
			}

			if(pTorrent->info->num_files() > 1)
				selectedfiles = ConvertItemId(pTorrent, selectedfiles);
			if(nopriority || filespriority.at(selectedfiles) != lt::download_priority_t(BITTORRENT_FILE_NONE))
			{
				lt::file_storage const& allfiles = torrentinfo.files();
				wxString fname = pTorrent->config->GetDownloadPath() + wxConvUTF8.cMB2WC( torrentinfo.name().c_str() ) + wxFileName::GetPathSeparator() + wxString::FromUTF8((allfiles.file_name(lt::file_index_t(selectedfiles))).to_string().c_str());
				//std::vector<std::string> const& allp = allfiles.paths();
				wxFileName filename (fname);
				filename.MakeAbsolute();
				//wxLogDebug(_T("File path %s\n"), filename.GetFullPath().c_str());
				if(wxFileName::FileExists(filename.GetFullPath()))
				{
					wxLaunchDefaultApplication(filename.GetFullPath()); 
				}
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
		std::shared_ptr<torrent_t> pTorrent;
		if( m_pTorrent.use_count() > 1)
		{ pTorrent = m_pTorrent; }
		else
		{
			MainFrame* pMainFrame = dynamic_cast< MainFrame* >( wxGetApp().GetTopWindow() );
			wxASSERT(pMainFrame != nullptr);
			pTorrent = pMainFrame->GetSelectedTorrent();
			if(m_pTorrent.use_count() == 1)
				m_pTorrent.reset();
		}

		if(!pTorrent) return;
		wxString filepath = pTorrent->config->GetDownloadPath();
		enable_openpath = wxFileName::DirExists( filepath );
		std::vector<lt::download_priority_t> & filespriority = pTorrent->config->GetFilesPriorities();
		std::vector<lt::download_priority_t>::iterator file_it ;
		
		if( filespriority.size() != pTorrent->info->num_files() )
		{
			std::vector<lt::download_priority_t> deffilespriority( pTorrent->info->num_files(), lt::download_priority_t(BITTORRENT_FILE_NORMAL ));
			filespriority.swap( deffilespriority );
		}
		
		int selectedfiles = GetFirstSelected();

		if( selectedfiles != -1 )
		{
			if(pTorrent->info->num_files() > 1)
				selectedfiles = ConvertItemId(pTorrent, selectedfiles);
			file_it = filespriority.begin() + selectedfiles;
			disable_priority = (std::uint8_t(*file_it) + FILELISTCTRL_MENU_PRIORITY0);
			enable_openpath = (enable_openpath && ((*file_it) != lt::download_priority_t(FILELISTCTRL_MENU_PRIORITY0)));
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
	std::shared_ptr<torrent_t> pTorrent;

	if( m_pTorrent.use_count() > 1 )
	{ pTorrent = m_pTorrent; }
	else
	{
		MainFrame* pMainFrame = dynamic_cast< MainFrame* >( wxGetApp().GetTopWindow() );
		wxASSERT(pMainFrame != nullptr);
		pTorrent = pMainFrame->GetSelectedTorrent();
		
		if(m_pTorrent.use_count() == 1)
			m_pTorrent.reset();
	}

	if( !pTorrent )
	{ return; }

	//XXX get some default priority definition
	// Get existing priority from pTorrent->t_cfg;
	std::vector<lt::download_priority_t> & filespriority = pTorrent->config->GetFilesPriorities();
	std::vector<lt::download_priority_t>::iterator file_it ;

	if( filespriority.size() != pTorrent->info->num_files() )
	{
		std::vector<lt::download_priority_t> deffilespriority( pTorrent->info->num_files(), lt::download_priority_t(BITTORRENT_FILE_NORMAL) );
		filespriority.swap( deffilespriority );
	}

	int selectedfiles = GetFirstSelected();

	while( selectedfiles != -1 )
	{
		int item = selectedfiles;
		if(pTorrent->info->num_files() > 1) item = ConvertItemId(pTorrent, selectedfiles);
		file_it = filespriority.begin() + item;
		*file_it = lt::download_priority_t(priority);
		selectedfiles = GetNextSelected( selectedfiles );
	}

	//pTorrent->config->SetFilesPriority(filespriority);
	pTorrent->config->Save();

	lt::torrent_info const& torrent_info = *(pTorrent->info);

	wxGetApp().GetBitTorrentSession()->ConfigureTorrentFilesPriority( pTorrent );
	lt::file_storage const& allfiles = torrent_info.files();
	
	wxULongLong_t total_selected = 0;
	lt::torrent_info const& torrentinfo = *(pTorrent->info);

	for(size_t i = 0; i < filespriority.size(); ++i)
	{
		if(filespriority.at(i) != lt::download_priority_t(BITTORRENT_FILE_NONE))
		{
			total_selected += allfiles.file_size(lt::file_index_t(i));
		}
	}

	pTorrent->config->SetSelectedSize(total_selected);
	Refresh( false );
}

void FileListCtrl::OnMenuOpenPath( wxCommandEvent& event )
{
	std::shared_ptr<torrent_t> pTorrent;

	if( m_pTorrent.use_count() > 1)
	{ pTorrent = m_pTorrent; }
	else
	{
		MainFrame* pMainFrame = dynamic_cast< MainFrame* >( wxGetApp().GetTopWindow() );
		wxASSERT(pMainFrame != nullptr);
		pTorrent = pMainFrame->GetSelectedTorrent();
		
		if(m_pTorrent.use_count() == 1)
			m_pTorrent.reset();
	}

	if( pTorrent )
	{
		int selectedfiles = GetFirstSelected();
		if( selectedfiles != -1 )
		{
			bool nopriority = false;
			lt::torrent_info const& torrentinfo = *(pTorrent->info);
			std::vector<lt::download_priority_t> & filespriority = pTorrent->config->GetFilesPriorities();
			if(pTorrent->info->num_files() > 1)
				selectedfiles = ConvertItemId(pTorrent, selectedfiles);
			if(selectedfiles < filespriority.size() && filespriority.at(selectedfiles) != lt::download_priority_t(BITTORRENT_FILE_NONE))
			{
				lt::file_storage const& allfiles = torrentinfo.files();
				wxString fname = pTorrent->config->GetDownloadPath() + wxConvUTF8.cMB2WC( torrentinfo.name().c_str() ) + wxFileName::GetPathSeparator() + wxString::FromUTF8((allfiles.file_name(lt::file_index_t(selectedfiles))).to_string().c_str());
				//std::vector<std::string> const& allp = allfiles.paths();
				wxFileName filename(fname);
				filename.MakeAbsolute();
				//wxLogDebug(_T("File path %s\n"), filename.GetFullPath().c_str());
#if  defined(__WXMSW__) 
				wxExecute(_T("Explorer ")+filename.GetPath(), wxEXEC_ASYNC, nullptr); 
#elif defined(__APPLE__)
				wxExecute(_T("/usr/bin/open ")+filename.GetPath(), wxEXEC_ASYNC, nullptr);
#elif defined(__WXGTK__)
//wxFileSystem::FileNameToURL(const wxFileName &filename)
				wxLaunchDefaultBrowser(_T("file://")+filename.GetPath());
#endif
			}
		}
	}
}

