#include "mainframe.h"
#include "magneturihandler.h"

bool MagnetUriHanlder::CanOpen( const wxString& location )
{
	return (GetProtocol(location) == "magnet");
}

wxFSFile * MagnetUriHanlder::OpenFile (wxFileSystem &fs, const wxString &location)
{
	if(m_frame)
		m_frame->OpenMagnetURI(location);
	return 0;
}

