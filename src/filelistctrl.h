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
// Created by: lim k. c. <admin@bitswash.org>
// Created on: Mon Feb  5 11:09:21 2007
//

#ifndef _FILELISTCTRL_H_
#define _FILELISTCTRL_H_

#include <vector>

#include <wx/wx.h>
#include <wx/listctrl.h>
#include <wx/imaglist.h>

#include <libtorrent/torrent_handle.hpp>

#define ID_FILELIST_CTRL_START wxID_HIGHEST + 1

#include "swashlistctrl.h"

#include "bittorrentsession.h"

class FileListCtrl : public SwashListCtrl
{
public:
// IDs for the menu commands
	FileListCtrl(wxWindow *parent,
		const wxString settings,
		const wxWindowID id,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxLC_VIRTUAL | wxLC_REPORT);
           
	~FileListCtrl();

	void SetStaticHandle(std::shared_ptr<torrent_t>& torrent) { m_pTorrent = torrent; }
	void DeleteStaticHandle() { m_pTorrent.reset(); }
	virtual void OnColClick(wxListEvent& event);
    virtual int GetItemColumnImage(long item, long columnid) const; /* override to get image for shown columns */
    virtual void OnLeftDown(wxMouseEvent& event);
    virtual void OnLeftDClick(wxMouseEvent& event);
protected:
	// FileListCtrl variables

private:
	typedef struct 
	{
		const long evtTag;
		void (FileListCtrl::*method)( wxCommandEvent &);
	} wxCmdEvtHandlerMap_t;

	wxString GetItemValue(long item, long columnid) const;
	void ShowContextMenu(const wxPoint& pos);
	void OnMenuPriority(wxCommandEvent &event);
	void OnMenuOpenPath(wxCommandEvent &event);
	long ConvertItemId(std::shared_ptr<torrent_t>& torrent, long item) const;
	std::shared_ptr<torrent_t> m_pTorrent;
	wxImageList m_imageList;
    wxMenu m_contextmenu;
	static wxCmdEvtHandlerMap_t m_menu_evt_map[];
	long m_lastclickcol;

	static const long FILELISTCTRL_MENU_PRIORITY0;
	static const long FILELISTCTRL_MENU_PRIORITY1;
	static const long FILELISTCTRL_MENU_PRIORITY2;
	static const long FILELISTCTRL_MENU_PRIORITY3;
	static const long FILELISTCTRL_MENU_PRIORITY4;
	static const long FILELISTCTRL_MENU_PRIORITY5;
	static const long FILELISTCTRL_MENU_PRIORITY6;
	static const long FILELISTCTRL_MENU_PRIORITY7;
	static const long FILELISTCTRL_MENU_OPENPATH;

	DECLARE_NO_COPY_CLASS(FileListCtrl)
};	
	
	
#endif	//_FILELISTCTRL_H_

