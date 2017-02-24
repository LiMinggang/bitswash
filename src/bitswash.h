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


#ifndef _BITSWASH_H_
#define _BITSWASH_H_

//#include <wx/defs.h>
#include <wx/wx.h>
#include <wx/app.h>
#include <wx/log.h>
#include <wx/filename.h>
#include <wx/imaglist.h>
#include <wx/intl.h>
#include <wx/ipc.h>
#include <wx/stackwalk.h>
#include <wx/snglinst.h>
#include <wx/dir.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>

#include <libtorrent/session.hpp>

#include "bittorrentsession.h"
#include "configuration.h"
#ifndef __WXMSW__
#include "version.h"
#else
#include "version_win.h"
#endif

#define APPNAME _T("Bitswash")
#define APPBINNAME _T("bitswash")
#define RESOURCE_DIR _T("icons")
#define BITSWASH_HOMEPAGE _T("http://www.bitswash.org")
#define BITSWASH_HELP_URL _T("http://www.bitswash.org/forum")
#define BITSWASH_GUIVERSION 3
//
//1 - gui versioning introduced
//2 - tracker list ctrl added
	
enum appicon_id
{
	BITSWASH_ICON_APP = 0,
	BITSWASH_ICON_APP256, 
	BITSWASH_ICON_TORRENT_OPEN,
	BITSWASH_ICON_TORRENT_CREATE,
	BITSWASH_ICON_TORRENT_OPENURL,
	BITSWASH_ICON_TORRENT_OPENDIR,
	BITSWASH_ICON_TORRENT_START,
	BITSWASH_ICON_TORRENT_PAUSE,
	BITSWASH_ICON_TORRENT_STOP,
	BITSWASH_ICON_TORRENT_PROPERTIES,
	BITSWASH_ICON_TORRENT_REMOVE, 
	BITSWASH_ICON_TORRENT_REMOVEDATA,
	BITSWASH_ICON_TORRENT_MOVEUP, 
	BITSWASH_ICON_TORRENT_MOVEDOWN,
	BITSWASH_ICON_OPTION_PREFERENCE,
	BITSWASH_ICON_MAX
};

class wxSingleInstanceChecker;
class BitSwashAppConn : public wxConnection
{
public:
    bool OnExecute(const wxString& topic,
#if wxMAJOR_VERSION < 2 || (wxMAJOR_VERSION == 2 && wxMINOR_VERSION < 9)
                            wxChar* data,
                            int WXUNUSED(size),
#else
                            const void * data,
                            size_t WXUNUSED(size),
#endif
                            wxIPCFormat WXUNUSED(format));
};

class BitSwashAppSrv : public wxServer
{
public:
    virtual wxConnectionBase* OnAcceptConnection(const wxString& topic);
};

class BitSwashAppClnt : public wxClient
{
public:
    virtual wxConnectionBase* OnMakeConnection() {return new BitSwashAppConn();}
};

#if (wxUSE_ON_FATAL_EXCEPTION == 1) && (wxUSE_STACKWALKER == 1)
class BitSwashStackWalker : public wxStackWalker
{
    wxFile * m_DumpFile;
public:
    BitSwashStackWalker():m_DumpFile(0) {}
    void SetDumpFile(wxFile * file){m_DumpFile = file;}
protected:
    inline void OnStackFrame(const wxStackFrame & frame);
};
#endif

class BitSwash : public wxApp
{
public:
	virtual bool OnInit();
	virtual int OnExit();
    virtual void OnInitCmdLine(wxCmdLineParser& cmdParser);
    virtual bool OnCmdLineParsed(wxCmdLineParser& cmdParser);
#if (wxUSE_ON_FATAL_EXCEPTION == 1) && (wxUSE_STACKWALKER == 1)
    void OnFatalException();
#endif

	wxString DataPath() { return m_datapath ;}
	wxString ConfigPath() { return m_configpath ;}
	wxString SaveTorrentsPath() { return m_savetorrentspath ;}
	wxString DHTStatePath() { return m_dhtstatepath ;} 
	wxString UserAgent() { return wxString::Format(_T("%s %s"), APPNAME, BITSWASH_VERSION );}
	wxString LogPath() { return m_logpath; }

	Configuration* GetConfig() { return m_config; }
	
	wxChar PathSeparator() { return wxFileName::GetPathSeparator(wxPATH_NATIVE); }

	int GetCountryFlag(const wxString & code);

	wxImageList* GetCountryFlagsList() { return m_imglist_ctryflags ;} 
	wxImageList* GetSettingIconsList() { return m_imglist_settingicons;} 

	bool SetLocale(wxString lang);

	void SetLogLevel();

	wxImage & GetAppIcon(enum appicon_id id);

	BitTorrentSession* GetBitTorrentSession() { return m_btsession ; }
	void BTInitDone() { m_btinitdone = true; }

private:
	BitTorrentSession* m_btsession;

	wxLog* m_logold;

	wxString m_datapath;
	wxString m_configpath;
	wxString m_savetorrentspath;
	wxString m_dhtstatepath;
	wxString m_logpath;

	Configuration *m_config;

	void LoadIcons();
	void LoadFlags();
	void LoadSettingIcons();

	wxImageList *m_imglist_ctryflags;
	wxImageList *m_imglist_settingicons;

	wxLocale *m_locale;
	bool m_btinitdone;
    wxArrayString m_FileNames;
    wxSingleInstanceChecker * m_SigleAppChecker;
    BitSwashAppSrv * m_AppServer;
#if (wxUSE_ON_FATAL_EXCEPTION == 1) && (wxUSE_STACKWALKER == 1)
    BitSwashStackWalker m_StackWalker;
#endif

};

DECLARE_APP(BitSwash)

#endif // BITSWASH_H
