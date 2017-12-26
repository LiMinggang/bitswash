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
// Class: SwashListCtrl
//
// Created by: lim k. c. <admin@bitswash.org>
// Created on: Fri Aug 24 22:01:26 MYT 2007
//
//
#ifdef __WXMSW__
	#include <wx/msw/winundef.h>
#endif
#include <wx/app.h>
#include <wx/tokenzr.h>
#include "swashlistctrl.h"
#include "mainframe.h"

// SwashListCtrl

SwashListCtrl::SwashListCtrl( wxWindow *parent,
						long num_cols,
						SwashColumnItem *columns,
						const wxString settings,
						const wxWindowID id,
						const wxPoint& pos,
						const wxSize& size,
						long style )
	: m_attrodd( *wxBLUE, *wxLIGHT_GREY, wxNullFont ),
	m_attreven( *wxBLACK, *wxWHITE, wxNullFont ),
	m_showfrom( 0 ),
	m_showto( 0 )
{
#ifdef __POCKETPC__
	EnableContextMenu();
#endif
#ifdef _DEBUG
	wxLogDebug( _T( "SwashListCtrl creator, setting %s\n" ), settings.c_str() );
#endif
	m_settings = settings;
	/* XXX initialize list column based on array */
	Init( num_cols, columns );
	Create( parent, id, pos, size, style );

	Bind( wxEVT_LIST_BEGIN_DRAG, &SwashListCtrl::OnBeginDrag, this );
	Bind( wxEVT_LIST_BEGIN_RDRAG, &SwashListCtrl::OnBeginRDrag, this );
	Bind( wxEVT_LIST_BEGIN_LABEL_EDIT, &SwashListCtrl::OnBeginLabelEdit, this );
	Bind( wxEVT_LIST_END_LABEL_EDIT, &SwashListCtrl::OnEndLabelEdit, this );
	Bind( wxEVT_LIST_DELETE_ITEM, &SwashListCtrl::OnDeleteItem, this );
	Bind( wxEVT_LIST_DELETE_ALL_ITEMS, &SwashListCtrl::OnDeleteAllItems, this );

	Bind( wxEVT_LIST_ITEM_SELECTED, &SwashListCtrl::OnSelected, this );
	Bind( wxEVT_LIST_ITEM_DESELECTED, &SwashListCtrl::OnDeselected, this );
	Bind( wxEVT_LIST_KEY_DOWN, &SwashListCtrl::OnListKeyDown, this );
	Bind( wxEVT_LIST_ITEM_ACTIVATED, &SwashListCtrl::OnActivated, this );
	Bind( wxEVT_LIST_ITEM_FOCUSED, &SwashListCtrl::OnFocused, this );

	Bind( wxEVT_LIST_COL_CLICK, &SwashListCtrl::OnColClick, this );
	Bind( wxEVT_LIST_COL_RIGHT_CLICK, &SwashListCtrl::OnColRightClick, this );
	Bind( wxEVT_LIST_COL_BEGIN_DRAG, &SwashListCtrl::OnColBeginDrag, this );
	Bind( wxEVT_LIST_COL_DRAGGING, &SwashListCtrl::OnColDragging, this );
	Bind( wxEVT_LIST_COL_END_DRAG, &SwashListCtrl::OnColEndDrag, this );

	Bind( wxEVT_LIST_CACHE_HINT, &SwashListCtrl::OnCacheHint, this );

#if USE_CONTEXT_MENU
	Bind( wxEVT_CONTEXT_MENU, &SwashListCtrl::OnContextMenu, this );
#endif

	Bind( wxEVT_CHAR, &SwashListCtrl::OnChar, this );

	Bind( wxEVT_RIGHT_DOWN, &SwashListCtrl::OnRightClick, this );
	Bind( wxEVT_LEFT_DCLICK, &SwashListCtrl::OnLeftDClick, this );
}

void SwashListCtrl::Init( long num_cols, SwashColumnItem *columns )
{
	m_numberofcolumns = num_cols;
	m_columns = columns;

	if( m_settings != wxEmptyString )
	{
		wxStringTokenizer settings_token( m_settings, _T( "|" ) );

		while( settings_token.HasMoreTokens() )
		{
			wxString list_setting = settings_token.GetNextToken();
			//wxLogDebug(_T("SwashListCtrl: Parsing %s"), list_setting.c_str());
			wxStringTokenizer col_setting( list_setting, _T( ":" ), wxTOKEN_RET_EMPTY_ALL );
			wxString col_name = col_setting.GetNextToken();
			long col_width;

			if( !col_setting.GetNextToken().ToLong( &col_width ) )
			{ col_width = 0; }

			wxString col_show = col_setting.GetNextToken();
			wxString col_sort = col_setting.GetNextToken();
			int i;

			for( i = 0; i < num_cols ; i++ )
			{
				if( wxGetTranslation( columns[i].name ) == col_name )
				{
					//wxLogDebug(_T("SwashListCtrl: Init col id %d name %s title %s width %d sort %s"), columns[i].id, columns[i].name.c_str(), columns[i].title.c_str(), col_width, col_sort.c_str());
					columns[i].width = col_width;

					if( col_show == _T( "true" ) )
					{
						columns[i].show = true;
					}
					else
					{
						columns[i].show = false;
					}

					//XXX set sort
					break;
				}
			}
		}
	}
}

long SwashListCtrl::ColumnId( long column ) const
{
	//SwashListCtrl* pThis = const_cast<SwashListCtrl*>( this );
	int i = 0;

	for( i = 0; i < m_numberofcolumns; i++ )
	{
		//wxLogMessage(_T("colname %s - title %s"), colname.c_str(), m_columns[i].title.c_str());
		if( ( m_columns[i].show ) && ( m_columns[i].column == column ) )
		{ return i; }
	}

	return -1;
}

wxString SwashListCtrl::Settings() const
{
	int i;
	SwashListCtrl* pThis = const_cast<SwashListCtrl*>( this );
	wxString settings = _T( "" );

	for( i = 0; i < m_numberofcolumns ; i++ )
	{
		settings += wxGetTranslation( m_columns[i].name );
		settings += _T( ":" );

		if( m_columns[i].show )
		{
			settings += wxString::Format( _T( "%d" ), pThis->GetColumnWidth( m_columns[i].column ) );
		}
		else
		{
			settings += wxString::Format( _T( "%d" ), m_columns[i].width );
		}

		//show or hide
		settings += _T( ":" );

		if( m_columns[i].show )
		{
			settings += _T( "true" );
		}
		else
		{
			settings += _T( "false" );
		}

		settings += _T( ":" );
		//XXX sorting
		settings += _T( "|" );
	}

	return settings;
}

bool SwashListCtrl::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
	wxListView::Create( parent, id, pos, size, style );
	int i = 0 ;
	int j = 0;

	for( i = 0; i < m_numberofcolumns ; ++i )
	{
		if( m_columns[i].show )
		{
			wxListItem listitem;
			listitem.SetText( wxGetTranslation( m_columns[i].title ) );
			listitem.SetId( m_columns[i].id );
			InsertColumn( j, listitem );
#ifdef _DEBUG
			wxLogDebug( _T( "Insert column %d name %s id %d\n" ), j, listitem.GetText(), listitem.GetId() );
#endif
			m_columns[i].column = j;

			if( m_columns[i].width )
			{ SetColumnWidth( j, m_columns[i].width ); }
			else
			{ SetColumnWidth( j, wxLIST_AUTOSIZE_USEHEADER ); }

			j++;
		}
		else
		{
			m_columns[i].column = -1;
		}
	}

	return true;
}

SwashListCtrl::~SwashListCtrl()
{
#ifdef _DEBUG
	wxLogDebug( _T( "SwashListCtrl::Destructor\n" ) );
#endif
}

void SwashListCtrl::OnCacheHint( wxListEvent& event )
{
	m_showfrom = event.GetCacheFrom();
	m_showto  = event.GetCacheTo();

#ifdef _DEBUG
	wxLogDebug( _T( "SwashListCtrl::OnCacheHint %d-%d\n" ), m_showfrom, m_showto );
#endif
}

void SwashListCtrl::SetColumnImage( int col, int image )
{
	wxListItem item;
	item.SetMask( wxLIST_MASK_IMAGE );
	item.SetImage( image );
	SetColumn( col, item );
}

void SwashListCtrl::OnColClick( wxListEvent& event )
{
#ifdef _DEBUG
	int col = event.GetColumn();
	// set or unset image
	//static bool x = false;
	//x = !x;
	//SetColumnImage( col, x ? 0 : -1 );
	wxLogDebug( wxT( "OnColumnClick at %d.\n" ), col );
#endif
}

void SwashListCtrl::OnColumnSelected( wxCommandEvent& event )
{
	long colnum = 0;
	int i;
	colnum = event.GetId() - SWASHLIST_POPUP_START;
	
#ifdef _DEBUG
	wxLogDebug( _T( "Command colnum %d start %d id %d (%d) %s\n" ), colnum, SWASHLIST_POPUP_START, event.GetId(), ( event.GetId() - SWASHLIST_POPUP_START ), ( event.IsChecked() ? _T( "checked" ) : _T( "uncheck" ) ) );
#endif

	if( event.IsChecked() )
	{
		wxListItem listitem;
		long insert = 0;
		listitem.SetText( wxGetTranslation( m_columns[colnum].title ) );

		if( colnum == 0 )
		{
			insert = 0;
		}
		else if( colnum >= GetColumnCount() )
		{
			insert = GetColumnCount();
		}
		else
		{
			for( i = colnum + 1; i < m_numberofcolumns; i++ )
			{
				if( m_columns[i].show )
				{
					wxListItem item;
					int j;

					for( j = 0; j < GetColumnCount(); j++ )
					{
						GetColumn( j, item );

						if( item.GetText() == wxGetTranslation( m_columns[i].title ) )
						{ insert = j; }
					}
#ifdef _DEBUG
					wxLogDebug( _T( "Insert before %s %d\n" ), m_columns[i].name.c_str(), insert );
#endif
					break;
				}
			}

			/* last visible column not checked */
			if( insert <= 0 )
			{
				insert = GetColumnCount();
			}
		}

		m_columns[colnum].show = true;
		m_columns[colnum].column = colnum;
		for( i = colnum + 1; i < m_numberofcolumns; ++i )
		{
			if(m_columns[i].show == true)
			{
				wxASSERT(m_columns[i].column >= 0);
				m_columns[i].column++;
			}
		}
#ifdef _DEBUG
		wxLogDebug( _T( "Insert column %s in position %d\n" ), listitem.GetText().c_str(), insert );
#endif
		InsertColumn( insert, listitem );

		if( m_columns[colnum].width )
		{ SetColumnWidth( insert, m_columns[colnum].width ); }
		else
		{ SetColumnWidth( insert, wxLIST_AUTOSIZE_USEHEADER ); }
	}
	else
	{
		int deleted = -1;
		m_columns[colnum].show = false;
		m_columns[colnum].column = -1;
		
		for( i = colnum + 1; i < m_numberofcolumns; ++i )
		{
			if(m_columns[i].show == true)
			{
				wxASSERT(m_columns[i].column >= 0);
				m_columns[i].column--;
			}
		}
		/* find column number */
		wxListItem item;
		item.SetMask( wxLIST_MASK_TEXT );

		for( i = 0; i < GetColumnCount(); ++i )
		{
			GetColumn( i, item );

			if( item.GetText() == wxGetTranslation( m_columns[colnum].title ) )
			{
				deleted = i;
				break;
			}
		}

		wxASSERT( deleted != -1 );

		if( deleted != -1 )
		{
			m_columns[colnum].width = GetColumnWidth( deleted );
			DeleteColumn( deleted );
		}
	}
}

void SwashListCtrl::OnColRightClick( wxListEvent& event )
{
	int col = event.GetColumn();
	int i = 0;

	if( col != -1 )
	{
		SetColumnImage( col, -1 );
	}

	wxMenu menu( _( "Show/Hide" ) );

	for( i = 0; i < m_numberofcolumns ; i++ )
	{
		wxMenuItem* menuitem = menu.AppendCheckItem( SWASHLIST_POPUP_START + i, wxGetTranslation( m_columns[i].name ), wxGetTranslation( m_columns[i].tooltip ) );

		if( m_columns[i].show )
		{
			menuitem->Check( true );
		}
		else
		{
			menuitem->Check( false );
		}
	}

	// Connect to event handler
	Connect( SWASHLIST_POPUP_START, SWASHLIST_POPUP_START + i,
			wxEVT_COMMAND_MENU_SELECTED,
			( wxObjectEventFunction )
			( wxEventFunction )
			( wxCommandEventFunction )&SwashListCtrl::OnColumnSelected );
	// Show popupmenu at position
	PopupMenu( &menu, event.GetPoint() );
#ifdef _DEBUG
	wxLogDebug( wxT( "OnColumnRightClick at %d.\n" ), event.GetColumn() );
#endif
}

void SwashListCtrl::LogColEvent( const wxListEvent& event, const wxChar *name )
{
#ifdef _DEBUG
	const int col = event.GetColumn();
	wxLogDebug( wxT( "%s: column %d (width = %d or %d).\n" ),
				name,
				col,
				event.GetItem().GetWidth(),
				GetColumnWidth( col ) );
#endif
}

void SwashListCtrl::OnColBeginDrag( wxListEvent& event )
{
#ifdef _DEBUG
	LogColEvent( event, wxT( "OnColBeginDrag" ) );
	/*
	if ( event.GetColumn() == 0 )
	{
	    wxLogDebug(_T("Resizing this column shouldn't work."));

	    event.Veto();
	}
	*/
#endif
}

void SwashListCtrl::OnColDragging( wxListEvent& event )
{
#ifdef _DEBUG
	LogColEvent( event, wxT( "OnColDragging" ) );
#endif
	event.Skip();
}

void SwashListCtrl::OnColEndDrag( wxListEvent& event )
{
#ifdef _DEBUG
	LogColEvent( event, wxT( "OnColEndDrag" ) );
#endif
}

void SwashListCtrl::OnBeginDrag( wxListEvent& event )
{
#ifdef _DEBUG
	const wxPoint& pt = event.m_pointDrag;
	int flags;
	LogColEvent( event, wxT( "OnBeginDrag" ) );
	wxLogDebug( wxT( "OnBeginDrag at (%d, %d), item %ld." ),
				pt.x, pt.y, HitTest( pt, flags ) );
#endif
}

void SwashListCtrl::OnBeginRDrag( wxListEvent& event )
{
#ifdef _DEBUG
	LogColEvent( event, wxT( "OnBeginRDrag" ) );
	wxLogDebug( wxT( "OnBeginRDrag at %d,%d.\n" ),
				event.m_pointDrag.x, event.m_pointDrag.y );
#endif
}

void SwashListCtrl::OnBeginLabelEdit( wxListEvent& event )
{
#ifdef _DEBUG
	wxLogDebug( wxT( "OnBeginLabelEdit: %s\n" ), event.m_item.m_text.c_str() );
#endif
}

void SwashListCtrl::OnEndLabelEdit( wxListEvent& event )
{
#ifdef _DEBUG
	wxLogDebug( wxT( "OnEndLabelEdit: %s\n" ),
				event.IsEditCancelled() ? _T( "[cancelled]" )
				: event.m_item.m_text.c_str() );
#endif
}

void SwashListCtrl::OnDeleteItem( wxListEvent& event )
{
#ifdef _DEBUG
	LogListEvent( event, _T( "OnDeleteItem" ) );
	wxLogDebug( wxT( "Number of items when delete event is sent: %d\n" ), GetItemCount() );
#endif
}

void SwashListCtrl::OnDeleteAllItems( wxListEvent& event )
{
#ifdef _DEBUG
	LogListEvent( event, _T( "OnDeleteAllItems" ) );
#endif
}


void SwashListCtrl::OnSelected( wxListEvent& event )
{
#ifdef _DEBUG
	LogListEvent( event, _T( "OnSelected" ) );
#endif
	static MainFrame * pMainFrame = nullptr;
	if(pMainFrame == nullptr)
	{
 		pMainFrame = dynamic_cast<MainFrame *>( wxGetApp().GetTopWindow() );
		wxASSERT(pMainFrame != nullptr);
	}
	pMainFrame->OnListItemClick( event.GetIndex() );
}

void SwashListCtrl::OnDeselected( wxListEvent& event )
{
#ifdef _DEBUG
	LogListEvent( event, _T( "OnDeselected" ) );
#endif
}

void SwashListCtrl::OnActivated( wxListEvent& event )
{
#ifdef _DEBUG
	LogListEvent( event, _T( "OnActivated" ) );
#endif
}

void SwashListCtrl::OnFocused( wxListEvent& event )
{
#ifdef _DEBUG
	LogListEvent( event, _T( "OnFocused" ) );
#endif
	event.Skip();
}

void SwashListCtrl::OnListKeyDown( wxListEvent& event )
{
#ifdef _DEBUG
	//    long item;

	switch( event.GetKeyCode() )
	{
	default:
		LogListEvent( event, _T( "OnListKeyDown" ) );
		event.Skip();
	}

#else
	event.Skip();
#endif
}

void SwashListCtrl::OnChar( wxKeyEvent& event )
{
#ifdef _DEBUG
	wxLogDebug( _T( "Got char event.\n" ) );

	switch( event.GetKeyCode() )
	{
	case 'n':
	case 'N':
	case 'c':
	case 'C':
		// these are the keys we process ourselves
		break;

	default:
		event.Skip();
	}

#else
	event.Skip();
#endif
}

void SwashListCtrl::OnRightClick( wxMouseEvent& event )
{
	if( !event.ControlDown() )
	{
		event.Skip();
		return;
	}
#ifdef _DEBUG
	int flags;
	long subitem = -1;
	long item = HitTest( event.GetPosition(), flags, &subitem );
	wxString where;

	switch( flags )
	{
	case wxLIST_HITTEST_ABOVE:
		where = _T( "above" );
		break;

	case wxLIST_HITTEST_BELOW:
		where = _T( "below" );
		break;

	case wxLIST_HITTEST_NOWHERE:
		where = _T( "nowhere near" );
		break;

	case wxLIST_HITTEST_ONITEMICON:
		where = _T( "on icon of" );
		break;

	case wxLIST_HITTEST_ONITEMLABEL:
		where = _T( "on label of" );
		break;

	case wxLIST_HITTEST_ONITEMRIGHT:
		where = _T( "right on" );
		break;

	case wxLIST_HITTEST_TOLEFT:
		where = _T( "to the left of" );
		break;

	case wxLIST_HITTEST_TORIGHT:
		where = _T( "to the right of" );
		break;

	default:
		where = _T( "not clear exactly where on" );
		break;
	}

	wxLogDebug( _T( "Right click %s item %s, subitem %s\n" ),
				where.c_str(), ( wxLongLong( item ).ToString() ).c_str(), ( wxLongLong( subitem ).ToString() ).c_str() );
#endif
}

void SwashListCtrl::OnLeftDClick(wxMouseEvent& event)
{
	if( !event.ControlDown() )
	{
		event.Skip();
		return;
	}

#ifdef _DEBUG
	int flags;
	long subitem = -1;
	long item = HitTest( event.GetPosition(), flags, &subitem );
	wxString where;

	switch( flags )
	{
	case wxLIST_HITTEST_ABOVE:
		where = _T( "above" );
		break;

	case wxLIST_HITTEST_BELOW:
		where = _T( "below" );
		break;

	case wxLIST_HITTEST_NOWHERE:
		where = _T( "nowhere near" );
		break;

	case wxLIST_HITTEST_ONITEMICON:
		where = _T( "on icon of" );
		break;

	case wxLIST_HITTEST_ONITEMLABEL:
		where = _T( "on label of" );
		break;

	case wxLIST_HITTEST_ONITEMRIGHT:
		where = _T( "right on" );
		break;

	case wxLIST_HITTEST_TOLEFT:
		where = _T( "to the left of" );
		break;

	case wxLIST_HITTEST_TORIGHT:
		where = _T( "to the right of" );
		break;

	default:
		where = _T( "not clear exactly where on" );
		break;
	}

	wxLogDebug( _T( "Left double click %s item %s, subitem %s\n" ),
				where.c_str(), ( wxLongLong( item ).ToString() ).c_str(), ( wxLongLong( subitem ).ToString() ).c_str() );
#endif
}

void SwashListCtrl::LogListEvent( const wxListEvent& event, const wxChar *eventName )
{
#ifdef _DEBUG
	wxLogDebug( _T( "Item %s: %s (item text = %s, data = %s)\n" ),
				( wxLongLong( event.GetIndex() ).ToString() ).c_str(), eventName,
				event.GetText().c_str(), ( wxLongLong( event.GetData() ).ToString() ).c_str() );
#endif
}

wxString SwashListCtrl::OnGetItemText( long item, long column ) const
{
	//SwashListCtrl* pThis = const_cast<SwashListCtrl*>( this );
	//long index_item = item;
	//wxListItem listitem;
	//  pThis->GetColumn(column, listitem);
	//wxLogDebug(_T("OnGetItemText: Item %d column %d listitem %s ColumnCount %d id %d"), item, column, listitem.GetText().c_str(), pThis->GetColumnCount(), listitem.GetId());
	return GetItemValue( item, ColumnId( column ) );
}

wxString SwashListCtrl::GetItemValue( long item, long columnid ) const
{
	return _T( "" );
}

int SwashListCtrl::GetItemColumnImage( long item, long columnid ) const
{
	return -1;
}

int SwashListCtrl::OnGetItemColumnImage( long item, long column ) const
{
	//long index_item = item;
	wxListItem listitem;
	GetColumn( column, listitem );
	//wxLogDebug(_T("OnGetItemText: Item %d column %d"), item, column);
	return GetItemColumnImage( item, ColumnId( column ) );
}

wxListItemAttr *SwashListCtrl::OnGetItemAttr( long item ) const
{
#ifdef __WXMSW__
	/* alternate color on msw is kinda.. weird */
	return ( wxListItemAttr * )&m_attreven;
#else
	return ( item % 2 ) ? ( wxListItemAttr * )&m_attreven : ( wxListItemAttr * )&m_attrodd;
#endif
}

#if USE_CONTEXT_MENU
void SwashListCtrl::OnContextMenu( wxContextMenuEvent& event )
{
	wxPoint point = event.GetPosition();

	// If from keyboard
	if( point.x == -1 && point.y == -1 )
	{
		wxSize size = GetSize();
		point.x = size.x / 2;
		point.y = size.y / 2;
	}
	else
	{
		point = ScreenToClient( point );
	}

	ShowContextMenu( point );
}
#endif

void SwashListCtrl::ShowContextMenu( const wxPoint& pos )
{
#ifdef _DEBUG
	wxLogDebug( _T( "SwashListCtrl: ShowContextMenu (%d, %d)\n" ), pos.x, pos.y );
#endif
}

void SwashListCtrl::UpdateSwashList()
{
	//SwashListCtrl* pThis = const_cast<SwashListCtrl*>( this );

	if( GetItemCount() == 0 )
	{ return ; }

	m_showto = ( m_showto >= GetItemCount() ) ? ( GetItemCount() - 1 ) : m_showto;
#ifdef _DEBUG
	wxLogDebug( _T( "UpdateSwashList %d - %d\n" ), m_showfrom, m_showto );
#endif

	if( m_showfrom > m_showto )
	{ m_showfrom = m_showto; }

	RefreshItems( m_showfrom, m_showto );
}

void SwashListCtrl::GetSelectedItems(SwashListCtrl::itemlist_t & items) const
{
	//wxLogDebug( _T( "SwashListCtrl: Selected items %d" ), GetSelectedItemCount() );
	items.clear();
	if( GetSelectedItemCount() <= 0 )
	{
		return;
	}

	long item = GetFirstSelected();

	while( item != -1 )
	{
		items.push_back( item );
		item = GetNextSelected( item );
	}
}
