#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/system/error_code.hpp>
#include <iostream>
#include <vector>
#include <iomanip>
#include <thread>
#include <exception>
#include <string>

//#include "../NetworkMonitor/utils.hpp"

namespace beast = boost::beast;
namespace http = beast::http;
namespace ws = beast::websocket;
namespace net = boost::asio;

namespace sync_connect 
{
    
using tcp = boost::asio::ip::tcp;

void Log(boost::system::error_code ec)
{
    std::cerr << "[" << std::setw(14) << std::this_thread::get_id() << "] "
              << (ec ? "Error: " : "OK")
              << (ec ? ec.message() : "")
              << std::endl;
}

void OnConnect(boost::system::error_code ec)
{
    Log(ec);
}

/*

37 love lane
ha5 3ee

020 88 66 57 66

*/

int sync_connect
    const std::string url{"ltnm.learncppthroughprojects.com"};
    const std::string port{"80"};

    // Always start with an I/O context object.
    boost::asio::io_context ioc{};

    // Create an I/O object. Every Boost.Asio I/O object API needs an io_context
    // as the first parameter.
    tcp::socket socket{ioc};

    // Under the hood, socket.connect uses I/O context to talk to the socket
    // and get a response back. The response is saved in ec.
    boost::system::error_code ec{};
    tcp::resolver resolver{ioc};
    auto resolverIt{resolver.resolve(url, port, ec)};
    if (ec)
    {
        Log(ec);
        return -1;
    }
    socket.connect(*resolverIt, ec);
    ws::stream<boost::beast::tcp_stream> ws_stream{std::move(socket)};

    std::string message{"hello"};
    ws_stream.handshake(url, "/echo", ec);

    if (ec)
    {
        printf("step 1 \n");
        Log(ec);
        return -1;
    }

    net::const_buffer write_buff{message.c_str(), message.size()};
    ws_stream.text(true);
    ws_stream.write(write_buff, ec);
    if (ec)
    {
        printf("step 2 \n");
        Log(ec);
        return -2;
    }

    beast::flat_buffer read_buff{};
    ws_stream.read(read_buff, ec);

    if (ec)
    {
        printf("step 3 \n");
        Log(ec);
        return -3;
    }

    std::cout << "ECHO: "
              << boost::beast::make_printable(read_buff.data())
              << std::endl;

    // for_each(read_buff.begin(), read_buff.end(),
    //          [](const auto &value)
    //          {
    //              std::cout << "Value from buffer : " << value << "\n";
    //          });

    //    std::cout << "TEST..." << " --" << message << "\n";

    return 0;
}
    
} // namespace sync_connect 
