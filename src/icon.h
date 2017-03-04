#ifndef __BITSWASHICON__
#define __BITSWASHICON__

struct AppIcons_t
{
	enum appicon_id id;
	wxImage *image;
	const unsigned char * filebuf;
	size_t filesize;
};

struct CountryFlags_t
{
	const wxChar code[4];
	wxString name;
    const unsigned char * filebuf;
    size_t filesize;
	int imgidx;             /* index in the imagelist */
};

struct SettingIcons_t
{
	const unsigned char * filebuf;
	size_t filesize;
};

extern struct AppIcons_t AppIcons[];
extern struct CountryFlags_t CountryFlags[];
extern const unsigned int N_COUNTRY;
extern struct SettingIcons_t SettingIcons[];
extern const unsigned int N_SETTINGS;
#endif
