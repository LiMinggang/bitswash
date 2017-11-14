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

#include <wx/wx.h>
#include <wx/buffer.h>

#include "infohash.h"

//namespace lt = libtorrent;
extern std::string to_hex(lt::sha1_hash const& s);

int hex_to_int(char in)
{
	if (in >= '0' && in <= '9') return int(in) - '0';
	if (in >= 'A' && in <= 'F') return int(in) - 'A' + 10;
	if (in >= 'a' && in <= 'f') return int(in) - 'a' + 10;
	return -1;
}

inline bool from_hex(char const *in, int len, char* out)
{
	for (auto i = in; i < (in + len); ++i, ++out)
	{
		int const t1 = hex_to_int(*i);
		if (t1 == -1) return false;
		*out = char(t1 << 4);
		++i;
		int const t2 = hex_to_int(*i);
		if (t2 == -1) return false;
		*out |= t2 & 15;
	}
	return true;
}

InfoHash::InfoHash()
    : m_valid(false)
{
}

InfoHash::InfoHash(const lt::sha1_hash &nativeHash)
    : m_valid(true)
    , m_nativeHash(nativeHash)
{
    std::string out = to_hex(m_nativeHash);
    m_hashString = wxString(out);
}

InfoHash::InfoHash(const wxString &hashString)
    : m_valid(false)
    , m_hashString(hashString)
{
    wxCharBuffer raw = m_hashString.ToAscii();
    if (raw.length() == 40)
        m_valid = from_hex(raw.data(), 40, (char*)&m_nativeHash[0]);
}

InfoHash::InfoHash(const InfoHash &other)
    : m_valid(other.m_valid)
    , m_nativeHash(other.m_nativeHash)
    , m_hashString(other.m_hashString)
{
}

bool InfoHash::isValid() const
{
    return m_valid;
}

InfoHash::operator lt::sha1_hash() const
{
    return m_nativeHash;
}


InfoHash::operator wxString() const
{
    return m_hashString;
}


bool InfoHash::operator==(const InfoHash &other) const
{
    return (m_nativeHash == other.m_nativeHash);
}

bool InfoHash::operator!=(const InfoHash &other) const
{
    return (m_nativeHash != other.m_nativeHash);
}

