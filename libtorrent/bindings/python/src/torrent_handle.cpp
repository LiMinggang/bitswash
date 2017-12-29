// Copyright Daniel Wallin 2006. Use, modification and distribution is
// subject to the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "boost_python.hpp"
#include <boost/python/tuple.hpp>
#include <boost/python/stl_iterator.hpp>
#include <libtorrent/torrent_handle.hpp>
#include <libtorrent/torrent_info.hpp>
#include <libtorrent/torrent_status.hpp>
#include <libtorrent/entry.hpp>
#include <libtorrent/peer_info.hpp>
#include "libtorrent/announce_entry.hpp"
#include <libtorrent/storage.hpp>
#include <libtorrent/disk_interface.hpp>
#include "gil.hpp"

using namespace boost::python;
using namespace lt;

#ifdef _MSC_VER
#pragma warning(push)
// warning c4996: x: was declared deprecated
#pragma warning( disable : 4996 )
#endif

namespace
{

  list url_seeds(torrent_handle& handle)
  {
      list ret;
      std::set<std::string> urls;
      {
          allow_threading_guard guard;
          urls = handle.url_seeds();
      }

      for (std::set<std::string>::iterator i(urls.begin())
          , end(urls.end()); i != end; ++i)
          ret.append(*i);
      return ret;
  }

  list http_seeds(torrent_handle& handle)
  {
      list ret;
      std::set<std::string> urls;
      {
          allow_threading_guard guard;
          urls = handle.http_seeds();
      }

      for (std::set<std::string>::iterator i(urls.begin())
          , end(urls.end()); i != end; ++i)
          ret.append(*i);
      return ret;
  }

  list piece_availability(torrent_handle& handle)
  {
      list ret;
      std::vector<int> avail;
      {
          allow_threading_guard guard;
          handle.piece_availability(avail);
      }

      for (auto const a : avail)
          ret.append(a);
      return ret;
  }

  list piece_priorities(torrent_handle& handle)
  {
      list ret;
      std::vector<download_priority_t> prio;
      {
          allow_threading_guard guard;
          prio = handle.get_piece_priorities();
      }

      for (auto const p : prio)
          ret.append(p);
      return ret;
  }

} // namespace unnamed

list file_progress(torrent_handle& handle, int flags)
{
    std::vector<std::int64_t> p;

    {
        allow_threading_guard guard;
        std::shared_ptr<const torrent_info> ti = handle.torrent_file();
        if (ti)
        {
           p.reserve(ti->num_files());
           handle.file_progress(p, flags);
        }
    }

    list result;

    for (std::vector<std::int64_t>::iterator i(p.begin()), e(p.end()); i != e; ++i)
        result.append(*i);

    return result;
}

list get_peer_info(torrent_handle const& handle)
{
    std::vector<peer_info> pi;

    {
        allow_threading_guard guard;
        handle.get_peer_info(pi);
    }

    list result;

    for (std::vector<peer_info>::iterator i = pi.begin(); i != pi.end(); ++i)
        result.append(*i);

    return result;
}

namespace
{
   template <typename T>
   T extract_fn(object o)
   {
      return boost::python::extract<T>(o);
   }
}

void prioritize_pieces(torrent_handle& info, object o)
{
   stl_input_iterator<object> begin(o), end;
   if (begin == end) return;

   // determine which overload should be selected. the one taking a list of
   // priorities or the one taking a list of piece -> priority mappings
   bool const is_piece_list = extract<std::pair<piece_index_t, download_priority_t>>(*begin).check();

   if (is_piece_list)
   {
      std::vector<std::pair<piece_index_t, download_priority_t>> piece_list;
      std::transform(begin, end, std::back_inserter(piece_list)
         , &extract_fn<std::pair<piece_index_t, download_priority_t>>);
      info.prioritize_pieces(piece_list);
   }
   else
   {
      std::vector<download_priority_t> priority_vector;
      std::transform(begin, end, std::back_inserter(priority_vector)
         , &extract_fn<download_priority_t>);
      info.prioritize_pieces(priority_vector);
   }
}

void prioritize_files(torrent_handle& info, object o)
{
   stl_input_iterator<download_priority_t> begin(o), end;
   info.prioritize_files(std::vector<download_priority_t>(begin, end));
}

list file_priorities(torrent_handle& handle)
{
    list ret;
    std::vector<download_priority_t> priorities = handle.get_file_priorities();

    for (auto const p : priorities)
        ret.append(p);

    return ret;
}

download_priority_t file_prioritity0(torrent_handle& h, file_index_t index)
{
   return h.file_priority(index);
}

void file_prioritity1(torrent_handle& h, file_index_t index, download_priority_t prio)
{
   return h.file_priority(index, prio);
}

void dict_to_announce_entry(dict d, announce_entry& ae)
{
   ae.url = extract<std::string>(d["url"]);
   if (d.has_key("tier"))
      ae.tier = extract<int>(d["tier"]);
   if (d.has_key("fail_limit"))
      ae.fail_limit = extract<int>(d["fail_limit"]);
}

void replace_trackers(torrent_handle& h, object trackers)
{
    object iter(trackers.attr("__iter__")());

    std::vector<announce_entry> result;

    for (;;)
    {
        handle<> entry(allow_null(PyIter_Next(iter.ptr())));

        if (entry == handle<>())
            break;

        if (extract<announce_entry>(object(entry)).check())
        {
           result.push_back(extract<announce_entry>(object(entry)));
        }
        else
        {
            dict d;
            d = extract<dict>(object(entry));
            announce_entry ae;
            dict_to_announce_entry(d, ae);
            result.push_back(ae);
        }
    }

    allow_threading_guard guard;
    h.replace_trackers(result);
}

void add_tracker(torrent_handle& h, dict d)
{
   announce_entry ae;
   dict_to_announce_entry(d, ae);
   h.add_tracker(ae);
}

namespace
{
#if defined BOOST_ASIO_HAS_STD_CHRONO
   using std::chrono::system_clock;
#else
   using boost::chrono::system_clock;
#endif

   time_t to_ptime(time_point tpt)
   {
      return system_clock::to_time_t(system_clock::now()
         + duration_cast<system_clock::duration>(tpt - clock_type::now()));
   }
}

list trackers(torrent_handle& h)
{
    list ret;
    std::vector<announce_entry> const trackers = h.trackers();
    for (std::vector<announce_entry>::const_iterator i = trackers.begin(), end(trackers.end()); i != end; ++i)
    {
        dict d;
        d["url"] = i->url;
        d["trackerid"] = i->trackerid;
        d["tier"] = i->tier;
        d["fail_limit"] = i->fail_limit;
        d["source"] = i->source;
        d["verified"] = i->verified;

#ifndef TORRENT_NO_DEPRECATE
        if (!i->endpoints.empty())
        {
            announce_endpoint const& aep = i->endpoints.front();
            d["message"] = aep.message;
            dict last_error;
            last_error["value"] = aep.last_error.value();
            last_error["category"] = aep.last_error.category().name();
            d["last_error"] = last_error;
            if (aep.next_announce > min_time()) {
                d["next_announce"] = to_ptime(aep.next_announce);
            }
            else {
                d["next_announce"] = object();
            }
            if (aep.min_announce > min_time()) {
                d["min_announce"] = to_ptime(aep.min_announce);
            }
            else {
                d["min_announce"] = object();
            }
            d["scrape_incomplete"] = aep.scrape_incomplete;
            d["scrape_complete"] = aep.scrape_complete;
            d["scrape_downloaded"] = aep.scrape_downloaded;
            d["fails"] = aep.fails;
            d["updating"] = aep.updating;
            d["start_sent"] = aep.start_sent;
            d["complete_sent"] = aep.complete_sent;
        }
        else
        {
            d["message"] = std::string();
            dict last_error;
            last_error["value"] = 0;
            last_error["category"] = "";
            d["last_error"] = last_error;
            d["next_announce"] = object();
            d["min_announce"] = object();
            d["scrape_incomplete"] = 0;
            d["scrape_complete"] = 0;
            d["scrape_downloaded"] = 0;
            d["fails"] = 0;
            d["updating"] = false;
            d["start_sent"] = false;
            d["complete_sent"] = false;
        }
#endif

        list aeps;
        for (auto const& aep : i->endpoints)
        {
            dict e;
            e["message"] = aep.message;
            e["local_address"] = boost::python::make_tuple(aep.local_endpoint.address().to_string(), aep.local_endpoint.port());
            dict last_error;
            last_error["value"] = aep.last_error.value();
            last_error["category"] = aep.last_error.category().name();
            e["last_error"] = last_error;
            if (aep.next_announce > min_time()) {
                e["next_announce"] = to_ptime(aep.next_announce);
            }
            else {
                e["next_announce"] = object();
            }
            if (aep.min_announce > min_time()) {
                e["min_announce"] = to_ptime(aep.min_announce);
            }
            else {
                e["min_announce"] = object();
            }
            e["scrape_incomplete"] = aep.scrape_incomplete;
            e["scrape_complete"] = aep.scrape_complete;
            e["scrape_downloaded"] = aep.scrape_downloaded;
            e["fails"] = aep.fails;
            e["updating"] = aep.updating;
            e["start_sent"] = aep.start_sent;
            e["complete_sent"] = aep.complete_sent;
            aeps.append(e);
        }
        d["endpoints"] = aeps;

#ifndef TORRENT_NO_DEPRECATE
        d["send_stats"] = i->send_stats;
#endif
        ret.append(d);
    }
    return ret;
}

list get_download_queue(torrent_handle& handle)
{
    list ret;

    std::vector<partial_piece_info> downloading;

    {
        allow_threading_guard guard;
        handle.get_download_queue(downloading);
    }

    for (std::vector<partial_piece_info>::iterator i = downloading.begin()
        , end(downloading.end()); i != end; ++i)
    {
        dict partial_piece;
        partial_piece["piece_index"] = i->piece_index;
        partial_piece["blocks_in_piece"] = i->blocks_in_piece;
        list block_list;
        for (int k = 0; k < i->blocks_in_piece; ++k)
        {
            dict block_info;
            block_info["state"] = i->blocks[k].state;
            block_info["num_peers"] = i->blocks[k].num_peers;
            block_info["bytes_progress"] = i->blocks[k].bytes_progress;
            block_info["block_size"] = i->blocks[k].block_size;
            block_info["peer"] = boost::python::make_tuple(
                i->blocks[k].peer().address().to_string()
                , i->blocks[k].peer().port());
            block_list.append(block_info);
        }
        partial_piece["blocks"] = block_list;

        ret.append(partial_piece);
    }

    return ret;
}

void set_metadata(torrent_handle& handle, std::string const& buf)
{
   handle.set_metadata(buf);
}

#ifndef TORRENT_NO_DEPRECATE
#if BOOST_VERSION > 104200

std::shared_ptr<const torrent_info> get_torrent_info(torrent_handle const& h)
{
    allow_threading_guard guard;
    return h.torrent_file();
}

#else

std::shared_ptr<torrent_info> get_torrent_info(torrent_handle const& h)
{
    // I can't figure out how to expose shared_ptr<const torrent_info>
    // as well as supporting mutable instances. So, this hack is better
    // than compilation errors. It seems to work on newer versions of boost though
    allow_threading_guard guard;
    return std::const_pointer_cast<torrent_info>(h.torrent_file());
}

#endif

#endif // TORRENT_NO_DEPRECAE

void add_piece(torrent_handle& th, piece_index_t piece, char const *data
    , add_piece_flags_t const flags)
{
   th.add_piece(piece, data, flags);
}

class dummy5 {};
class dummy {};
class dummy4 {};
class dummy6 {};
class dummy7 {};
class dummy8 {};

using by_value = return_value_policy<return_by_value>;
void bind_torrent_handle()
{
    // arguments are: number of seconds and tracker index
    void (torrent_handle::*force_reannounce0)(int, int) const = &torrent_handle::force_reannounce;

#ifndef TORRENT_NO_DEPRECATE
    bool (torrent_handle::*super_seeding0)() const = &torrent_handle::super_seeding;
    void (torrent_handle::*super_seeding1)(bool) const = &torrent_handle::super_seeding;
#endif
    void (torrent_handle::*set_flags0)(torrent_flags_t) const = &torrent_handle::set_flags;
    void (torrent_handle::*set_flags1)(torrent_flags_t, torrent_flags_t) const = &torrent_handle::set_flags;

    download_priority_t (torrent_handle::*piece_priority0)(piece_index_t) const = &torrent_handle::piece_priority;
    void (torrent_handle::*piece_priority1)(piece_index_t, download_priority_t) const = &torrent_handle::piece_priority;

    void (torrent_handle::*move_storage0)(std::string const&, lt::move_flags_t) const = &torrent_handle::move_storage;
    void (torrent_handle::*rename_file0)(file_index_t, std::string const&) const = &torrent_handle::rename_file;

#if !defined TORRENT_NO_DEPRECATE
    void (torrent_handle::*move_storage1)(std::wstring const&, int) const = &torrent_handle::move_storage;
    void (torrent_handle::*rename_file1)(file_index_t, std::wstring const&) const = &torrent_handle::rename_file;
#endif

    std::vector<open_file_state> (torrent_handle::*file_status0)() const = &torrent_handle::file_status;

#define _ allow_threads

    enum_<move_flags_t>("move_flags_t")
        .value("always_replace_files", move_flags_t::always_replace_files)
        .value("fail_if_exist", move_flags_t::fail_if_exist)
        .value("dont_replace", move_flags_t::dont_replace)
    ;

#ifndef TORRENT_NO_DEPRECATE
   enum_<deprecated_move_flags_t>("deprecated_move_flags_t")
        .value("always_replace_files", deprecated_move_flags_t::always_replace_files)
        .value("fail_if_exist", deprecated_move_flags_t::fail_if_exist)
        .value("dont_replace", deprecated_move_flags_t::dont_replace)
    ;
#endif

    class_<torrent_handle>("torrent_handle")
        .def(self == self)
        .def(self != self)
        .def(self < self)
        .def("__hash__", (std::size_t (*)(torrent_handle const&))&libtorrent::hash_value)
        .def("get_peer_info", get_peer_info)
        .def("status", _(&torrent_handle::status), arg("flags") = 0xffffffff)
        .def("get_download_queue", get_download_queue)
        .def("file_progress", file_progress, arg("flags") = 0)
        .def("trackers", trackers)
        .def("replace_trackers", replace_trackers)
        .def("add_tracker", add_tracker)
        .def("add_url_seed", _(&torrent_handle::add_url_seed))
        .def("remove_url_seed", _(&torrent_handle::remove_url_seed))
        .def("url_seeds", url_seeds)
        .def("add_http_seed", _(&torrent_handle::add_http_seed))
        .def("remove_http_seed", _(&torrent_handle::remove_http_seed))
        .def("http_seeds", http_seeds)
        .def("torrent_file", _(&torrent_handle::torrent_file))
        .def("set_metadata", set_metadata)
        .def("is_valid", _(&torrent_handle::is_valid))
        .def("pause", _(&torrent_handle::pause), arg("flags") = 0)
        .def("resume", _(&torrent_handle::resume))
        .def("clear_error", _(&torrent_handle::clear_error))
        .def("queue_position", _(&torrent_handle::queue_position))
        .def("queue_position_up", _(&torrent_handle::queue_position_up))
        .def("queue_position_down", _(&torrent_handle::queue_position_down))
        .def("queue_position_top", _(&torrent_handle::queue_position_top))
        .def("queue_position_bottom", _(&torrent_handle::queue_position_bottom))

        .def("add_piece", add_piece)
        .def("read_piece", _(&torrent_handle::read_piece))
        .def("have_piece", _(&torrent_handle::have_piece))
        .def("set_piece_deadline", _(&torrent_handle::set_piece_deadline)
            , (arg("index"), arg("deadline"), arg("flags") = 0))
        .def("reset_piece_deadline", _(&torrent_handle::reset_piece_deadline), (arg("index")))
        .def("clear_piece_deadlines", _(&torrent_handle::clear_piece_deadlines), (arg("index")))
        .def("piece_availability", &piece_availability)
        .def("piece_priority", _(piece_priority0))
        .def("piece_priority", _(piece_priority1))
        .def("prioritize_pieces", &prioritize_pieces)
        .def("get_piece_priorities", &piece_priorities)
        .def("prioritize_files", &prioritize_files)
        .def("get_file_priorities", &file_priorities)
        .def("file_priority", &file_prioritity0)
        .def("file_priority", &file_prioritity1)
        .def("file_status", _(file_status0))
        .def("save_resume_data", _(&torrent_handle::save_resume_data), arg("flags") = 0)
        .def("need_save_resume_data", _(&torrent_handle::need_save_resume_data))
        .def("force_reannounce", _(force_reannounce0)
            , (arg("seconds") = 0, arg("tracker_idx") = -1))
#ifndef TORRENT_DISABLE_DHT
        .def("force_dht_announce", _(&torrent_handle::force_dht_announce))
#endif
        .def("scrape_tracker", _(&torrent_handle::scrape_tracker), arg("index") = -1)
        .def("flush_cache", &torrent_handle::flush_cache)
        .def("set_upload_limit", _(&torrent_handle::set_upload_limit))
        .def("upload_limit", _(&torrent_handle::upload_limit))
        .def("set_download_limit", _(&torrent_handle::set_download_limit))
        .def("download_limit", _(&torrent_handle::download_limit))
        .def("connect_peer", &torrent_handle::connect_peer, (arg("endpoint"), arg("source")=0, arg("flags")=0xd))
        .def("set_max_uploads", &torrent_handle::set_max_uploads)
        .def("max_uploads", _(&torrent_handle::max_uploads))
        .def("set_max_connections", &torrent_handle::set_max_connections)
        .def("max_connections", _(&torrent_handle::max_connections))
        .def("move_storage", _(move_storage0), (arg("path"), arg("flags") = move_flags_t::always_replace_files))
        .def("info_hash", _(&torrent_handle::info_hash))
        .def("force_recheck", _(&torrent_handle::force_recheck))
        .def("rename_file", _(rename_file0))
        .def("set_ssl_certificate", &torrent_handle::set_ssl_certificate, (arg("cert"), arg("private_key"), arg("dh_params"), arg("passphrase")=""))
        .def("flags", _(&torrent_handle::flags))
        .def("set_flags", _(set_flags0))
        .def("set_flags", _(set_flags1))
        .def("unset_flags", _(&torrent_handle::unset_flags))
        // deprecated
#ifndef TORRENT_NO_DEPRECATE
        .def("piece_priorities", &piece_priorities)
        .def("file_priorities", &file_priorities)
        .def("stop_when_ready", _(&torrent_handle::stop_when_ready))
        .def("super_seeding", super_seeding1)
        .def("auto_managed", _(&torrent_handle::auto_managed))
        .def("set_priority", _(&torrent_handle::set_priority))
        .def("get_torrent_info", &get_torrent_info)
        .def("super_seeding", super_seeding0)
        .def("write_resume_data", _(&torrent_handle::write_resume_data))
        .def("is_seed", _(&torrent_handle::is_seed))
        .def("is_finished", _(&torrent_handle::is_finished))
        .def("has_metadata", _(&torrent_handle::has_metadata))
        .def("use_interface", &torrent_handle::use_interface)
        .def("name", _(&torrent_handle::name))
        .def("is_paused", _(&torrent_handle::is_paused))
        .def("is_auto_managed", _(&torrent_handle::is_auto_managed))
        .def("set_upload_mode", _(&torrent_handle::set_upload_mode))
        .def("set_share_mode", _(&torrent_handle::set_share_mode))
        .def("apply_ip_filter", &torrent_handle::apply_ip_filter)
        .def("set_sequential_download", _(&torrent_handle::set_sequential_download))
        .def("set_peer_upload_limit", &torrent_handle::set_peer_upload_limit)
        .def("set_peer_download_limit", &torrent_handle::set_peer_download_limit)
        .def("set_ratio", _(&torrent_handle::set_ratio))
        .def("save_path", _(&torrent_handle::save_path))
        .def("set_tracker_login", &torrent_handle::set_tracker_login)
        .def("move_storage", _(move_storage1), (arg("path"), arg("flags") = always_replace_files))
        .def("rename_file", _(rename_file1))
#endif
        ;

    class_<open_file_state>("open_file_state")
       .add_property("file_index", make_getter((&open_file_state::file_index), by_value()))
       .def_readonly("last_use", &open_file_state::last_use)
       .def_readonly("open_mode", &open_file_state::open_mode)
    ;

    {
    scope s = class_<dummy>("file_open_mode");
    s.attr("read_only") = file_open_mode::read_only;
    s.attr("write_only") = file_open_mode::write_only;
    s.attr("read_write") = file_open_mode::read_write;
    s.attr("rw_mask") = file_open_mode::rw_mask;
    s.attr("sparse") = file_open_mode::sparse;
    s.attr("no_atime") = file_open_mode::no_atime;
    s.attr("random_access") = file_open_mode::random_access;
#ifndef TORRENT_NO_DEPRECATE
    s.attr("locked") = 0;
#endif
    }

    enum_<torrent_handle::file_progress_flags_t>("file_progress_flags")
        .value("piece_granularity", torrent_handle::piece_granularity)
    ;

    {
    scope s = class_<dummy6>("add_piece_flags_t");
    s.attr("overwrite_existing") = torrent_handle::overwrite_existing;
    }

    {
    scope s = class_<dummy7>("pause_flags_t");
    s.attr("graceful_pause") = torrent_handle::graceful_pause;
    }

    {
    scope s = class_<dummy4>("save_resume_flags_t");
    s.attr("flush_disk_cache") = torrent_handle::flush_disk_cache;
    s.attr("save_info_dict") = torrent_handle::save_info_dict;
    s.attr("only_if_modified") = torrent_handle::only_if_modified;
    }

    {
    scope s = class_<dummy8>("deadline_flags_t");
    s.attr("alert_when_available") = torrent_handle::alert_when_available;
    }

	 {
	 scope s = class_<dummy5>("status_flags_t");
    s.attr("query_distributed_copies") = torrent_handle::query_distributed_copies;
    s.attr("query_accurate_download_counters") = torrent_handle::query_accurate_download_counters;
    s.attr("query_last_seen_complete") = torrent_handle::query_last_seen_complete;
    s.attr("query_pieces") = torrent_handle::query_pieces;
    s.attr("query_verified_pieces") = torrent_handle::query_verified_pieces;
	 }

}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
