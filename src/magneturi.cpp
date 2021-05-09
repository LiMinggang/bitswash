/*
 * Bittorrent Client using Qt and libtorrent.
 * Copyright (C) 2015  Vladimir Golovnev <glassez@yandex.ru>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * In addition, as a special exception, the copyright holders give permission to
 * link this program with the OpenSSL project's "OpenSSL" library (or with
 * modified versions of it that use the same license as the "OpenSSL" library),
 * and distribute the linked executables. You must obey the GNU General Public
 * License in all respects for all of the code used other than "OpenSSL".  If you
 * modify file(s), you may extend this exception to your version of the file(s),
 * but you are not obligated to do so. If you do not wish to do so, delete this
 * exception statement from your version.
 */
#include <algorithm>
#include <wx/wx.h>
#include <wx/regex.h>
#include <wx/base64.h> 
#include <wx/tokenzr.h>

#include <libtorrent/bencode.hpp>
#include <libtorrent/error_code.hpp>
#include <libtorrent/magnet_uri.hpp>

#include "magneturi.h"

//namespace lt = libtorrent;

namespace
{
	wxString bcLinkToMagnet(const wxString& bcLink)
	{
		const wxScopedCharBuffer sbuf = bcLink.ToUTF8();
		wxMemoryBuffer decoded = wxBase64Decode(((const char *)sbuf) + 8);
		wxString rawBc((const char *)(decoded.GetData()), decoded.GetDataLen());

		// Format is now AA/url_encoded_filename/size_bytes/info_hash/ZZ
		wxStringTokenizer tkz( rawBc, '/' );
		if (tkz.CountTokens() != 5) return wxString();

		int i = 0;
		wxString token, filename, hash;
		while( tkz.HasMoreTokens() )
		{
			token = tkz.GetNextToken();
			if(i == 1) filename = token;
			if(i == 3)
			{
				hash = token;
				break;
			}
			++i;
		}

		wxString magnet = "magnet:?xt=urn:btih:" + hash;
		magnet += "&dn=" + filename;
		return magnet;
	}
}

namespace libt = libtorrent;

MagnetUri::MagnetUri(const wxString &source)
	: m_valid(false)
	, m_url(source)
{
	wxRegEx exp1 = "[^0-9A-Fa-f]", exp2 = "[^2-7A-Za-z]";
	if (source.IsEmpty()) return;

	size_t len = source.size();
	wxString tmp = source.Left(20).Lower();
	if (tmp.StartsWith(_T("bc://bt/"))) {
		m_url = bcLinkToMagnet(source);
	}
	else if (((len == 60) || (len == 52)) && tmp.StartsWith(_T("magnet:?xt=urn:btih:")))
	{
		m_url = source;
	}
	else if (((len == 40) && !exp1.Matches(source))
			 || ((len == 32) && !exp2.Matches(source))) {
		m_url = "magnet:?xt=urn:btih:" + source;
	}

	lt::error_code ec;
	m_addTorrentParams = lt::parse_magnet_uri(std::move(std::string((const char *)(m_url.ToUTF8().data()))), ec);
	if (ec) return;

	m_valid = true;
	m_hash = InfoHash(m_addTorrentParams.info_hashes.get_best());
	if(!m_addTorrentParams.name.empty())
		m_name = wxString::FromUTF8(m_addTorrentParams.name.c_str());
	else
		m_name = m_hash;
}

bool MagnetUri::isValid() const
{
	return m_valid;
}

InfoHash MagnetUri::hash() const
{
	return m_hash;
}

wxString MagnetUri::name() const
{
	return m_name;
}

const std::vector<std::string> & MagnetUri::trackers() const
{
	return m_addTorrentParams.trackers;
}

const std::vector<std::string> & MagnetUri::urlSeeds() const
{
	return m_addTorrentParams.url_seeds;
}

wxString MagnetUri::url() const
{
	return m_url;
}

lt::add_torrent_params MagnetUri::addTorrentParams() const
{
	return m_addTorrentParams;
}
