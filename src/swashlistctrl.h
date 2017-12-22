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
#ifndef _SWASHLISTCTRL_H_
#define _SWASHLISTCTRL_H_

#include <vector>

#include <wx/wx.h>
#include <wx/dynarray.h>
#include <wx/listctrl.h>

// not all ports have support for EVT_CONTEXT_MENU yet, don't define
// USE_CONTEXT_MENU for those which don't
#if defined(__WXMOTIF__) || defined(__WXPM__) || defined(__WXX11__) || defined(__WXMGL__)
	#define USE_CONTEXT_MENU 0
#else
	#define USE_CONTEXT_MENU 1
#endif

/* struct used for both column header setup 
 * and populate list data
 */

typedef struct column_t 
{
	int id;
	wxString name;
	wxString title;
	wxString tooltip;
	int width; /* initial pixel, 0 = auto header */
	bool show;
	long column;
	bool operator == (const struct column_t& t) const
	{ return name == t.name ;}
} SwashColumnItem;

#define SWASHLIST_POPUP_START 1000

#define SWASHLISTCOL_SIZE(x) sizeof(x)/sizeof(SwashColumnItem)

class SwashListCtrl: public wxListView
{
public:

	SwashListCtrl(wxWindow *parent,
			long num_cols = 0,
			SwashColumnItem *columns = nullptr,
			const wxString settings = wxEmptyString,
			const wxWindowID id = wxID_ANY,
			const wxPoint& pos = wxDefaultPosition,
			const wxSize& size = wxDefaultSize,
			long style = wxLC_VIRTUAL | wxLC_REPORT | wxSUNKEN_BORDER);

	virtual ~SwashListCtrl();

	void Init(long num_cols = 0, SwashColumnItem *columns = nullptr);

	bool Create(wxWindow* parent, 
					wxWindowID id, 
					const wxPoint& pos , 
					const wxSize& size ,
					long style);

	wxString Settings() const; //return list setting in comma separated wxString
	void Settings(wxString s); // set list settings with command seperated string

	typedef std::vector<long> itemlist_t;

	void UpdateSwashList();
	/* returns list of selected items */
	void GetSelectedItems(itemlist_t & items) const;

protected:

	virtual void OnColClick(wxListEvent& event);
	virtual void OnColRightClick(wxListEvent& event);
	virtual void OnColBeginDrag(wxListEvent& event);
	virtual void OnColDragging(wxListEvent& event);
	virtual void OnColEndDrag(wxListEvent& event);
	virtual void OnBeginDrag(wxListEvent& event);
	virtual void OnBeginRDrag(wxListEvent& event);
	virtual void OnBeginLabelEdit(wxListEvent& event);
	virtual void OnEndLabelEdit(wxListEvent& event);
	virtual void OnDeleteItem(wxListEvent& event);
	virtual void OnDeleteAllItems(wxListEvent& event);

	virtual void OnSelected(wxListEvent& event);
	virtual void OnDeselected(wxListEvent& event);
	virtual void OnListKeyDown(wxListEvent& event);
	virtual void OnActivated(wxListEvent& event);
	virtual void OnFocused(wxListEvent& event);
	virtual void OnCacheHint(wxListEvent& event);

	virtual void OnChar(wxKeyEvent& event);

#if USE_CONTEXT_MENU
	virtual void OnContextMenu(wxContextMenuEvent& event);
#endif

	virtual void OnRightClick(wxMouseEvent& event);
	virtual void OnLeftDClick(wxMouseEvent& event);

	virtual wxString OnGetItemText(long item, long column) const;
	virtual wxString GetItemValue(long item, long columnid) const; /* override to get value for shown columns */

	virtual void ShowContextMenu(const wxPoint& pos);

	virtual void SetColumnImage(int col, int image);


	virtual int GetItemColumnImage(long item, long columnid) const; /* override to get image for shown columns */
	virtual int OnGetItemColumnImage(long item, long column) const;

	virtual wxListItemAttr *OnGetItemAttr(long item) const;

	void OnColumnSelected(wxCommandEvent& event);
	long ColumnId(long column) const ;

	wxListItemAttr m_attreven;
	wxListItemAttr m_attrodd;

	int m_showfrom;
	int m_showto;

	wxString m_settings;

	//debugging
	void LogListEvent(const wxListEvent& event, const wxChar *eventName);
	void LogColEvent(const wxListEvent& event, const wxChar *eventName);

	SwashColumnItem *m_columns;
	int m_numberofcolumns;
};

#endif // _SWASHLISTCTRL_H_
