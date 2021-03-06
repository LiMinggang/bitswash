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
// wxwidgets 2.6 compatibility headers
// Created by: lim k. c. <admin@bitswash.org>
// Created on: Sat May 12 11:18:47 MYT 2007
//

#ifndef __COMPAT_H__
#define __COMPAT_H__
#include <wx/toolbar.h>

#include <wx/aui/aui.h>
#include <wx/hyperlink.h>
#define AUIPANEINFO wxAuiPaneInfo()

#define SWASHTOOLBAR_STYLE wxTB_TOP | wxTB_TEXT | wxNO_BORDER | wxTB_HORIZONTAL | wxTB_3DBUTTONS | wxTB_FLAT | wxCLIP_CHILDREN
#define SWASHAUITOOLBAR_STYPE (wxAUI_TB_HORIZONTAL | wxAUI_TB_PLAIN_BACKGROUND | wxAUI_TB_GRIPPER)

#endif
