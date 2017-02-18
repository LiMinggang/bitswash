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

#include <libtorrent/bencode.hpp>
#include <libtorrent/error_code.hpp>
#include <libtorrent/magnet_uri.hpp>

#include "magneturi.h"

#if 0
namespace
{
    wxString bcLinkToMagnet(wxString& bcLink)
    {
        wxString rawBc(bcLink.ToUTF8());
        rawBc = rawBc.Mid(8); // skip bc://bt/
        rawBc = QByteArray::fromBase64(rawBc); // Decode base64
        // Format is now AA/url_encoded_filename/size_bytes/info_hash/ZZ
        QStringList parts = wxString(rawBc).split("/");
        if (parts.size() != 5) return wxString();

        wxString filename = parts.at(1);
        wxString hash = parts.at(3);
        wxString magnet = "magnet:?xt=urn:btih:" + hash;
        magnet += "&dn=" + filename;
        return magnet;
    }
}
#endif

namespace libt = libtorrent;

MagnetUri::MagnetUri(const wxString &source)
    : m_valid(false)
    , m_url(source)
{
	wxRegEx exp1 = "[^0-9A-Fa-f]", exp2 = "[^2-7A-Za-z]";
    if (source.IsEmpty()) return;

	size_t len = source.size();
	wxString tmp = source.Lower();
    if (tmp.StartsWith(_T("bc://bt/"))) {
        wxLogDebug(_T("Creating magnet link from bc link not supported"));
		return;
        //m_url = bcLinkToMagnet(source);
    }
	else if (((len == 60) || (len == 52)) && tmp.StartsWith(_T("magnet:?xt=urn:btih:")))
	{
		m_url = source;
	}
    else if (((len == 40) && !exp1.Matches(source))
             || ((len == 32) && !exp2.Matches(source))) {
        m_url = "magnet:?xt=urn:btih:" + source;
    }

    libt::error_code ec;
    libt::parse_magnet_uri(m_url.ToUTF8().data(), m_addTorrentParams, ec);
    if (ec) return;

    m_valid = true;
    m_hash = m_addTorrentParams.info_hash;
    m_name = wxString(m_addTorrentParams.name);

	size_t i = 0;
	for(i = 0; i < m_addTorrentParams.trackers.size(); ++i )
	{
		m_trackers.push_back(wxString(m_addTorrentParams.trackers[i]));
	}
	for(i = 0; i < m_addTorrentParams.url_seeds.size(); ++i )
	{
		m_urlSeeds.push_back(wxURL(m_addTorrentParams.url_seeds[i]));
	}
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

const std::list<TrackerEntry> & MagnetUri::trackers() const
{
    return m_trackers;
}

const std::list<wxURL> & MagnetUri::urlSeeds() const
{
    return m_urlSeeds;
}

wxString MagnetUri::url() const
{
    return m_url;
}

libtorrent::add_torrent_params MagnetUri::addTorrentParams() const
{
    return m_addTorrentParams;
}