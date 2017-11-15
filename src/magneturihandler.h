#ifndef __MAGNETURIHANDLER_H__
#define __MAGNETURIHANDLER_H__

#include <wx/filesys.h>

class MainFrame;

class MagnetUriHanlder: public wxFileSystemHandler
{
public:
	explicit MagnetUriHanlder(MainFrame * frame):m_frame(frame){}
	~MagnetUriHanlder(){}
	virtual bool CanOpen( const wxString& location );
	virtual wxFSFile * 	OpenFile (wxFileSystem &fs, const wxString &location);
private:
	MainFrame *m_frame;
};
#endif
