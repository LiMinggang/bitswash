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
// miscellanous functions
//
// Created by: lim k. c. <admin@bitswash.org>
// Created on: 2007-04-13 
//
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <wx/dir.h>
#include <wx/filefn.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>
#include <wx/mimetype.h>
#include <wx/url.h>

#include <wx/regex.h>

#include "functions.h"

#define MAX_UNITS 5
#define MAX_TIME_UNITS 5

static wxChar units[MAX_UNITS][2] = { _T(""), _T("K"), _T("M"), _T("G"), _T("T") };
static wxChar timetunits[MAX_TIME_UNITS][2] = { _T(""), _T("S"), _T("M"), _T("H"), _T("D") };

wxString HumanReadableByte(wxDouble byte) 
{
	wxDouble vl =0 ;

	wxDouble ivl = 1024;
	int i=0;
	wxString res;
	//std::cout << byte << "\n";

	vl = byte;
	for (i=0; i< MAX_UNITS ; i++) 
	{
		if (byte < ivl ) break;

		vl = byte / ivl ;
		ivl *= ivl;
	}
	if (!i)
		res = wxString::Format(_T("%.0lf B"), vl );
	else
		res = wxString::Format(_T("%.02lf %sB"), vl, units[i]);

	//std::cout << vl << " " << units[i] << "\n";
	//std::cout << res.mb_str() << "\n";
	return res;

}

wxString HumanReadableTime(unsigned long second) 
{
	//XXX format time?
	unsigned long min = 60, hour = 3600, day = 3600*24;
	unsigned int days, hours, mins, secs;
	days = second/day;
	hours = (second%day)/hour;
	mins = (second%hour)/min;
	secs = (second%min);
	wxString time;
	if(days) time << days << _("D");
	if(hours) time << hours << _("H");
	if(mins) time << mins << _("M");
	if(secs) time << secs << _("S");
	return time;
}

int RemoveDirectory(wxString path) {

	wxDir dir (path);

	if ( !dir.IsOpened() )
	{
		wxLogError(_T("Error opening directory %s for processing\n" ), path.c_str());
		return 0;
	}
	//dive into directory recursively 
	wxString filename;
	wxChar dirsep = wxFileName::GetPathSeparator(wxPATH_NATIVE);

	bool cont = dir.GetFirst(&filename, _T("*"), wxDIR_DEFAULT);

	while (cont) 
	{
		wxString fullfilename = path + dirsep + filename;
		wxLogDebug(_T("Scandir: %s\n" ), filename.c_str());
		if (wxDirExists(fullfilename))
		{
			if (!RemoveDirectory(fullfilename))
			{
				wxLogError(_T("Error removing directory %s error %s\n" ), fullfilename.c_str(), wxSysErrorMsg(wxSysErrorCode()));
			}
		}
		else if (wxFileExists( fullfilename))
		{
			if (!wxRemoveFile(fullfilename)) 
			{
				wxLogError(_T("Error removing file %s\n" ), fullfilename.c_str());
			}
		} else
			wxLogError(_T("Error, %s returned is not file nor directory!\n" ), fullfilename.c_str());

		cont = dir.GetNext(&filename);
	}
	return wxRmdir(path);
}

int CopyDirectory(wxString frompath, wxString topath) {

	wxDir fromdir (frompath);

	if ( !fromdir.IsOpened() )
	{
		wxLogError(_T("Error opening directory %s for processing\n" ), frompath.c_str());
		return 0;
	}
	wxLogInfo(_T("Copying %s -> %s\n" ), frompath.c_str(), topath.c_str());
	//make first directory
	if (!wxDirExists(topath) )
	{
		wxMkdir(topath);
	}	
	
	//dive into directory recursively 
	wxString fromfilename;
	wxChar dirsep = wxFileName::GetPathSeparator(wxPATH_NATIVE);

	bool cont = fromdir.GetFirst(&fromfilename, _T("*"), wxDIR_DEFAULT);

	while (cont) 
	{
		wxString fromfullfilename = frompath + dirsep + fromfilename;
		wxString tofullfilename = topath + dirsep + fromfilename;
		wxLogInfo(_T("Scandir: %s\n" ), fromfilename.c_str());
		if (wxDirExists(fromfullfilename))
		{
			wxLogInfo(_T("Copying dir %s -> %s\n" ), fromfilename.c_str(), tofullfilename.c_str());
			if (!CopyDirectory(fromfullfilename, tofullfilename))
			{
				wxLogError(_T("Error copying directory %s -> %s: %s\n" ), fromfullfilename.c_str(), fromfullfilename.c_str(), wxSysErrorMsg(wxSysErrorCode()));
			} 
		}
		else if (wxFileExists( fromfullfilename))
		{
			wxLogInfo(_T("Copying file %s -> %s\n" ), fromfullfilename.c_str(), tofullfilename.c_str());
			if (!wxCopyFile(fromfullfilename, tofullfilename)) 
			{
				wxLogError(_T("Error copying file %s -> %s: %s\n" ), fromfullfilename.c_str(), tofullfilename.c_str(), wxSysErrorMsg(wxSysErrorCode()));
			} 
		} else
			wxLogError(_T("%s returned is not file nor directory!\n" ), fromfullfilename.c_str());

		cont = fromdir.GetNext(&fromfilename);
	}
	return 1;
	//return wxRmdir(frompath);
}

int MoveDirectory(wxString frompath, wxString topath)
{
	CopyDirectory(frompath, topath);

	RemoveDirectory(frompath);

	return 0;
}

void SystemOpenURL(wxString url)
{
#ifdef __UNIX_LIKE__
	wxString desktop_session = wxEmptyString;

	if (wxGetEnv(_T("GNOME_DESKTOP_SESSION_ID"), &desktop_session))
	{
			wxLogDebug(_T("GNOME_DESKTOP_SESSION_ID=%s\n" ), desktop_session.c_str());
			wxString cmd = wxEmptyString;
			cmd.sprintf(_T("%s %s"), _T("gnome-open"), url.c_str());

			wxLogDebug(_T("Open url %s cmd %s\n" ), url.c_str(), cmd.c_str());
			::wxExecute(cmd);
	}
	else
	{

			wxString mimetype = _T("text/html");
			wxFileType *filetype = wxTheMimeTypesManager->GetFileTypeFromMimeType (mimetype);

			if (filetype) {
				wxString cmd;
				if (filetype->GetOpenCommand (&cmd, wxFileType::MessageParameters (url))) {
				//cmd.Replace(_T("file://"), wxEmptyString);
				wxLogDebug(_T("Open url %s cmd %s\n" ), url.c_str(), cmd.c_str());
				::wxExecute(cmd);
				}
				delete filetype;
			}
	}
#else
	wxString mimetype = _T("text/html");
	wxFileType *filetype = wxTheMimeTypesManager->GetFileTypeFromMimeType (mimetype);

	if (filetype) {
        	wxString cmd;
        	if (filetype->GetOpenCommand (&cmd, wxFileType::MessageParameters (url))) {
            	//cmd.Replace(_T("file://"), wxEmptyString);
				wxLogDebug(_T("Open url %s cmd %s\n" ), url.c_str(), cmd.c_str());

            	::wxExecute(cmd);
        	}
        	delete filetype;
    	}
#endif
}

wxString & GetExecutablePath()
{
	static bool	found =	false;
	static wxString	path;

	if (!found)	{
#ifdef __WXMSW__

		wxChar buf[512];
		*buf = wxT('\0');
		GetModuleFileName(NULL, buf, 511);
		path = buf;

#elif defined(__WXMAC__)

		ProcessInfoRec processinfo;
		ProcessSerialNumber	procno ;
		FSSpec fsSpec;

		procno.highLongOfPSN = NULL	;
		procno.lowLongOfPSN	= kCurrentProcess ;
		processinfo.processInfoLength =	sizeof(ProcessInfoRec);
		processinfo.processName	= NULL;
		processinfo.processAppSpec = &fsSpec;

		GetProcessInformation( &procno , &processinfo )	;
		path = wxMacFSSpec2MacFilename(&fsSpec);
#else
		wxString argv0 = wxTheApp->argv[0];

		if (wxIsAbsolutePath(argv0)) {
			path = argv0;
		}
		else {
			wxPathList pathlist;
			pathlist.AddEnvList(wxT("PATH"));
			path = pathlist.FindAbsoluteValidPath(argv0);
		}

		wxFileName filename(path);
		filename.Normalize();
		path = filename.GetFullPath();
#endif
		found =	true;
	}

	return path;
}

bool isUrl(const wxString &s, bool isScheme/* = true*/)
{
	wxString scheme = s;
	if(!isScheme)
        scheme = wxURL(s).GetScheme();
    wxRegEx is_url(_T("http[s]?|ftp"), wxRE_ICASE);
    return is_url.Matches(scheme);
}

