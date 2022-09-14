//
// Created by Ruslan on 06/09/2022.
//

#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>

namespace beast = boost::beast;// from <boost/beast.hpp>
namespace http = beast::http;  // from <boost/beast/http.hpp>
namespace net = boost::asio;   // from <boost/asio.hpp>
namespace ssl = net::ssl;      // from <boost/asio/ssl.hpp>
using tcp = net::ip::tcp;      // from <boost/asio/ip/tcp.hpp>

#ifndef NETWORK_MONITOR_HTTP_CLIENT_H
#define NETWORK_MONITOR_HTTP_CLIENT_H

namespace client {

    const int VERSION = 11;

    class Url {
    public:
        Url(std::string &prefix, std::string &host, int port, std::string &target) : prefix_(std::move(prefix)), host_(host), port_(port), target_(target){};
        ~Url() = default;

        const std::string &getPrefix();
        const std::string &getHost();
        int getPort();
        const std::string &getTarget();

    private:
        std::string prefix_;
        std::string host_;
        int port_;
        std::string target_;
    };

    class UrlParser {
    private:
        bool using_ssl = false;

    public:
        explicit UrlParser(bool using_ssl) : using_ssl(using_ssl){};

        Url operator()(const std::string &path) {
            auto prefixIndx = path.find(':');
            std::string prefix_ = path.substr(0, prefixIndx + 3);
            std::string host_ = path.substr(prefixIndx + 3, path.length());

            auto targetIndx = host_.find('/', 0);
            std::string target_;
            if (targetIndx != -1) {
                target_ = host_.substr(targetIndx, host_.length());
                host_ = host_.substr(0, targetIndx);
            } else {
                target_ = "/";
            };

            auto port_indx = host_.find(":", 0);
            int port_;
            if (port_indx != -1) {
                port_ = stoi(host_.substr(port_indx + 1, targetIndx));
                host_ = host_.substr(0, port_indx);
                std::cout << "port: " << port_ << std::endl;
                //        int port = stoi(url.substr(port_indx, host.length()));
            } else {
                port_ = using_ssl ? 443 : 80;
            }

            std::cout << "prefix: " << prefix_ << std::endl;
            std::cout << "host: " << host_ << std::endl;
            std::cout << "target: " << target_ << std::endl;
            std::cout << "port: " << port_ << std::endl;
            return Url{prefix_, host_, port_, target_};
        }
    };

    template<class WriteStream>
    struct stream_shutdown_handler {
        void operator()(WriteStream &stream) {
            throw std::runtime_error("Not supported");
        }
    };

    template<>
    struct stream_shutdown_handler<beast::tcp_stream> {
        void operator()(beast::tcp_stream &&stream) {
            beast::error_code ec;
            stream.socket().shutdown(tcp::socket::shutdown_both, ec);

            // not_connected happens sometimes
            // so don't bother reporting it.
            //
            if (ec && ec != beast::errc::not_connected)
                throw beast::system_error{ec};
        }
    };

    template<>
    struct stream_shutdown_handler<beast::ssl_stream<beast::tcp_stream>> {
        void operator()(beast::ssl_stream<beast::tcp_stream> &&stream) {
            beast::error_code ec;
            //                Sleep(1000);
            stream.shutdown(ec);

            // not_connected happens sometimes
            // so don't bother reporting it.
            //
            if (ec && ec != beast::errc::not_connected && ec.message() != "stream truncated") {
                std::cout << " ec message = " << ec.message() << std::endl;
                throw beast::system_error{ec};
            }
        }
    };

    class HttpClient {
    public:
        using resolved_result = boost::asio::ip::basic_resolver<tcp, boost::asio::any_io_executor>::results_type;

        explicit HttpClient(net::io_context &ioc) : resolver_{ioc}, ioc_{&ioc} {}
        ~HttpClient() = default;

        http::response<http::string_body> get_request(const std::string &path, bool using_ssl);

        template<class WriteStream>
        http::response<http::string_body> get_request(WriteStream &write_stream, Url &url, const resolved_result &results);

        //        template <class WriteStream>
        [[nodiscard]] beast::ssl_stream<beast::tcp_stream> init_ssl_stream(const resolved_result &results) const;

    private:
        tcp::resolver resolver_;
        net::io_context *ioc_;
    };

}// namespace client


#endif//NETWORK_MONITOR_HTTP_CLIENT_H
