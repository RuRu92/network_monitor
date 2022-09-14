//
// Created by Ruslan on 06/09/2022.
//

#include "network-monitor/http-client.h"

using client::HttpClient;
using client::Url;
using client::UrlParser;

http::response<http::string_body> HttpClient::get_request(const std::string &path, bool using_ssl = false) {
    UrlParser url_parser{using_ssl};
    Url url = url_parser(path);

    auto const results = resolver_.resolve(url.getHost(), std::to_string(url.getPort()));

    if (!using_ssl) {
        auto stream = beast::tcp_stream{*ioc_};
        stream_shutdown_handler<beast::tcp_stream> shutdown_handler;
        auto res = get_request(stream, url, results);
        shutdown_handler(std::forward<beast::tcp_stream>(stream));
        return res;
    } else {
        auto stream = init_ssl_stream(results);
        stream_shutdown_handler<beast::ssl_stream<beast::tcp_stream>> shutdown_handler;
        auto res = get_request(stream, url, results);
        shutdown_handler(std::forward<beast::ssl_stream<beast::tcp_stream>>(stream));
        return res;
    }
};


template<class WriteStream>
http::response<http::string_body> HttpClient::get_request(WriteStream &write_stream, Url &url, const resolved_result &results) {

    http::request<http::string_body> req{http::verb::get, url.getTarget(), VERSION};
    req.set(http::field::host, url.getHost());
    req.set(http::field::timeout, "1000");
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

    // Send the HTTP request to the remote host
    http::write(write_stream, req);

    // This buffer is used for reading and must be persisted
    beast::flat_buffer buffer;

    // Declare a container to hold the response
    http::response<http::string_body> res;

    // Receive the HTTP response
    http::read(write_stream, buffer, res);

    return res;
}

[[nodiscard]] beast::ssl_stream<beast::tcp_stream> HttpClient::init_ssl_stream(const resolved_result &results) const {
    // The SSL context is required, and holds certificates
    ssl::context ctx(ssl::context::sslv23_client);

    // This holds the root certificate used for verification
    ctx.load_verify_file(TESTS_CACERT_PEM);

    // Verify the remote server's certificate
    ctx.set_verify_mode(ssl::verify_peer);

    beast::ssl_stream<beast::tcp_stream> stream_(*ioc_, ctx);// Set up an HTTP GET request message

    beast::get_lowest_layer(stream_).connect(results);
    // Perform the SSL handshake
    beast::error_code handshake_ec;
    stream_.handshake(ssl::stream_base::client, handshake_ec);
    //
    if (handshake_ec) {
        std::cout << "error in handshake: " << handshake_ec.message() << "\n";
    };
    return stream_;
};

// --------------------------- URL ---------------------------------

const std::string &client::Url::getPrefix() {
    return prefix_;
}
const std::string &client::Url::getHost() {
    return host_;
}
int client::Url::getPort() {
    return port_;
}
const std::string &client::Url::getTarget() {
    return target_;
}
