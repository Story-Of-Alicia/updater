#include "libupdate/libupdate.hpp"

#include <iostream>
#include <boost/beast.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>

#include "libpak/libpak.hpp"

namespace beast = boost::beast; // from <boost/beast.hpp>
namespace http = beast::http; // from <boost/beast/http.hpp>
namespace net = boost::asio; // from <boost/asio.hpp>
namespace ssl = net::ssl; // from <boost/asio/ssl.hpp>
using tcp = net::ip::tcp; // from <boost/asio/ip/tcp.hpp>

constexpr std::string host = "localhost";

void libupdate::update::update_manifest() {
    net::io_context ioc;
    ssl::context ctx(ssl::context::tlsv12_client);
    ctx.set_verify_mode(ssl::verify_none);

    tcp::resolver resolver(ioc);
    beast::ssl_stream<beast::tcp_stream> stream(ioc, ctx);

    if (!SSL_set_tlsext_host_name(stream.native_handle(), host.c_str())) {
        beast::error_code const ec{static_cast<int>(::ERR_get_error()), net::error::get_ssl_category()};
        throw beast::system_error{ec};
    }

    auto const results = resolver.resolve(host, "443");
    beast::get_lowest_layer(stream).connect(results);

    stream.handshake(ssl::stream_base::client);
    http::request<http::string_body> req{http::verb::get, "/update/res.pak.manifest", 11};
    req.set(http::field::host, host);
    req.set(http::field::user_agent, "libupdate");

    http::write(stream, req);
    beast::flat_buffer buffer;
    http::response<http::string_body> resp;
    http::read(stream, buffer, resp);

    _manifest.clear();

    beast::error_code ec;
    stream.shutdown(ec);
    if (ec == net::error::eof || ec == ssl::error::stream_truncated) {
        ec = {};
    }
    if (ec) {
        throw beast::system_error{ec};
    }

    char path[256] = {};
    char crc[9] = {};
    size_t offset = 0;

    while (true) {
        if (offset >= resp.body().size())
            break;

        memset(path, 0, sizeof(path));
        memset(crc, 0, sizeof(crc));

        // parse path up until the first colon ':'
        // incrementing the path index, but also the offset from the start of the file
        for (unsigned index = 0; index < sizeof(path); ++index, ++offset) {
            if (offset >= resp.body().size()) {
                throw std::runtime_error("unexpected eof when parsing a path in the manifest");
            }
            char const c = resp.body().at(offset);
            //if (!isalnum(c) && (c != '/' or c != '\\' or c != '_' or c != '-'))
            //    continue; // ignore invalid character

            if (c == ':')
                break; // end of path
            path[index] = c;
        }

        offset += 1;

        for (unsigned index = 0; index < 8; ++index, ++offset) {
            if (offset >= resp.body().size()) {
                throw std::runtime_error("unexpected eof when parsing manifest, missing crc");
            }
            crc[index] = resp.body().at(offset);
        }
        // since uint32_t is larger than int we must first parse it as long long
        // and then cast it to uint32_t
        long long crc_l;
        try {
            crc_l = std::stoll(std::string(crc), nullptr, 16); //TODO: sto(uint32_t)?
        } catch (std::invalid_argument const& ex)
        {
           throw std::runtime_error(std::format("invalid crc for '{}'", std::string(path)));
        }
        catch (std::out_of_range const& ex)
        {
           throw std::runtime_error(std::format("invalid crc for '{}' - out of range", std::string(path)));
        }
        auto crc_i = static_cast<uint32_t>(crc_l);
        _manifest.emplace(std::string(path), crc_i);
        ++offset; // ignore then newline at the end of the line
    }
}

libupdate::progress libupdate::update::get_progress() const noexcept {
    std::scoped_lock (_mutex);
    return _progress;
}

void libupdate::update::initiate() {
    _progress.state = CHECK;
    update_manifest();

    auto r = libpak::resource("res.pak");
    r.read(false);
    std::vector<std::string> marked {};

    for (auto &[path, asset]  : r.assets) {
        if (!_manifest.contains(path)) {
            marked.emplace_back(path);
            continue;
        }

        if (_manifest[path] != asset.header.crc_embedded) {
            marked.emplace_back(path);
            continue;
        }
    }

    assert(marked.empty());
}

void libupdate::update::terminate() {
    _progress.state = NONE;
}

void libupdate::update::pause(bool const val) {
    _paused = val;
    if (val) {
        _progress.state = PAUSE;
    }
}
