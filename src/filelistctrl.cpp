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

#include "torrentinfo.h"
#include "filelistctrl.h"
#include "functions.h"

enum
{
    FILELISTCTRL_MENU_PRIORITY0 = 10000,
    FILELISTCTRL_MENU_PRIORITY1,
    FILELISTCTRL_MENU_PRIORITY2,
    FILELISTCTRL_MENU_PRIORITY3,
    FILELISTCTRL_MENU_PRIORITY4,
    FILELISTCTRL_MENU_PRIORITY5,
    FILELISTCTRL_MENU_PRIORITY6,
    FILELISTCTRL_MENU_PRIORITY7
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
END_EVENT_TABLE()
// FileListCtrl


enum torrentlistcolumnid
{
    FILELIST_COLUMN_FILE = 0,
    FILELIST_COLUMN_SIZE,
    FILELIST_COLUMN_DOWNLOAD,
    FILELIST_COLUMN_PROGRESS,
};

#ifndef __WXMSW__
static SwashColumnItem filelistcols[] =
{
    { id: FILELIST_COLUMN_FILE, name: _T( "file" ), title: _( "File" ), tooltip: _( "Files in torrent" ), width: 315, show: true},
    { id: FILELIST_COLUMN_SIZE, name: _T( "size" ), title: _( "Size" ), tooltip: _( "File size" ), width: 88, show: true},
    { id: FILELIST_COLUMN_FILE, name: _T( "download" ), title: _( "Download" ), tooltip: _( "File priority" ), width: 88, show: true},
    { id: FILELIST_COLUMN_PROGRESS, name: _T( "progress" ), title: _( "Progress" ), tooltip: _( "File download progress" ), width: 88, show: true },
};
#else
static SwashColumnItem filelistcols[] =
{
    { FILELIST_COLUMN_FILE,  _T( "file" ), _( "File" ), _( "Files in torrent" ), 315, true},
    { FILELIST_COLUMN_SIZE, _T( "size" ), _( "Size" ), _( "File size" ), 88, true},
    { FILELIST_COLUMN_FILE, _T( "download" ),  _( "Download" ), _( "File priority" ), 88, true},
    { FILELIST_COLUMN_PROGRESS,  _T( "progress" ), _( "Progress" ), _( "File download progress" ),  88, true },
};

#endif
FileListCtrl::FileListCtrl( wxWindow *parent,
                            const wxString settings,
                            const wxWindowID id,
                            const wxPoint& pos,
                            const wxSize& size,
                            long style )
    : SwashListCtrl( parent, SWASHLISTCOL_SIZE( filelistcols ), filelistcols, settings, id, pos, size, style ),
      m_pTorrent( NULL )
{
}

FileListCtrl::~FileListCtrl()
{
}

wxString FileListCtrl::GetItemValue( long item, long columnid ) const
{
    FileListCtrl* pThis = const_cast<FileListCtrl*>( this );
    MainFrame* pMainFrame = ( MainFrame* )( wxGetApp().GetTopWindow() );
    wxString ret;
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
        pTorrent = pMainFrame->GetSelectedTorrent();
    }

    if( !pTorrent )
    { return _T( "" ); }

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
    const wxChar* priority[] =
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
    wxString t_name;
    f_entry = torrent_info.file_at( item );

    if( h.is_valid() )
    { h.file_progress( f_progress ); }

    switch( columnid )
    {
    case FILELIST_COLUMN_FILE:
        t_name = wxString( wxConvUTF8.cMB2WC( f_entry.path.c_str() ) );
        ret = t_name;
        break;

    case FILELIST_COLUMN_SIZE:
        ret = HumanReadableByte( ( wxDouble ) f_entry.size );
        break;

    case FILELIST_COLUMN_DOWNLOAD:
        if( nopriority )
        { ret = priority[4]; }
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
        ret = _T( "" );
    }

    return ret;
}

void FileListCtrl::ShowContextMenu( const wxPoint& pos )
{
    wxMenu menu;
    /* XXX openfile */
    menu.Append( FILELISTCTRL_MENU_PRIORITY0, _( "Do not download" ) );
    menu.AppendSeparator();
    menu.Append( FILELISTCTRL_MENU_PRIORITY1, _( "Lowest Priority" ) );
    menu.Append( FILELISTCTRL_MENU_PRIORITY4, _( "Normal Priority" ) );
    menu.Append( FILELISTCTRL_MENU_PRIORITY7, _( "Highest Priority" ) );
    PopupMenu( &menu, pos );
}

/* update files priority to torrent config */
void FileListCtrl::OnMenuPriority( wxCommandEvent& event )
{
    MainFrame* pMainFrame = ( MainFrame* )wxGetApp().GetTopWindow();
    int priority = event.GetId() - FILELISTCTRL_MENU_PRIORITY0;
    shared_ptr<torrent_t> pTorrent;

    if( m_pTorrent )
    { pTorrent = m_pTorrent; }
    else
    { pTorrent = pMainFrame->GetSelectedTorrent(); }

    if( !pTorrent )
    { return; }

    //XXX get some default priority definition
    // Get existing priority from pTorrent->t_cfg;
    std::vector<int> & filespriority = pTorrent->config->GetFilesPriorities();
    std::vector<int> deffilespriority( pTorrent->info->num_files(), 4 );
    std::vector<int>::iterator file_it ;

    if( filespriority.size() != pTorrent->info->num_files() )
    {
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
    Refresh( false );
}

