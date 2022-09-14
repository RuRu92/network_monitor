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
namespace websocket = beast::websocket;
namespace http = beast::http;
namespace net = boost::asio;

using tcp = boost::asio::ip::tcp;

namespace ws
{
    namespace async
    {
        namespace callback
        {

            void Log(const std::string &where, boost::system::error_code ec)
            {
                std::cerr << "[" << std::setw(20) << where << "] "
                          << (ec ? "Error: " : "OK")
                          << (ec ? ec.message() : "")
                          << std::endl;
            }

            void OnReceive(
                // --> Start of shared data
                beast::flat_buffer &rBuffer,
                // <-- End of shared data
                const boost::system::error_code &ec)
            {
                if (ec)
                {
                    Log("OnReceive", ec);
                    return;
                }

                // Print the echoed message.
                std::cout << "ECHO: "
                          << beast::make_printable(rBuffer.data())
                          << std::endl;
            }

            void OnSend(
                // --> Start of shared data
                websocket::stream<boost::beast::tcp_stream> &ws,
                boost::beast::flat_buffer &rBuffer,
                // <-- End of shared data
                const boost::system::error_code &ec)
            {
                if (ec)
                {
                    Log("OnSend", ec);
                    return;
                }

                // Read the echoed message back.
                ws.async_read(rBuffer,
                              [&rBuffer](auto ec, auto nBytesRead)
                              {
                                  OnReceive(rBuffer, ec);
                              });
            }

            void OnConnect(
                // --> Start of shared data
                websocket::stream<boost::beast::tcp_stream> &ws,
                const std::string &url,
                const std::string &endpoint,
                const net::const_buffer &wBuffer,
                beast::flat_buffer &rBuffer,
                // <-- End of shared data
                const boost::system::error_code &ec)
            {
              if (ec) {
                Log("OnConnect", ec);
                if (_onConnect) {
                  onConnect_(ec);
                }
                return;
              }

              // Now that the TCP socket is connected, we can reset the timeout to
              // whatever Boost.Beast recommends.
              _ws.next_layer().expires_never();
              _ws.set_option(websocket::stream_base::timeout::suggested(
                  boost::beast::role_type::client
                  ));

              // Attempt a WebSocket handshake.
              _ws.async_handshake(_url, _endpoint,
                                  [this](auto ec) {
                                    OnHandshake(ec);
                                  }
              );
            }

            void OnHandshake(
                // --> Start of shared data
                websocket::stream<boost::beast::tcp_stream> &ws,
                const net::const_buffer &wBuffer,
                boost::beast::flat_buffer &rBuffer,
                // <-- End of shared data
                const boost::system::error_code &ec)
            {
                if (ec)
                {
                    Log("OnHandshake", ec);
                    return;
                }

                // Tell the WebSocket object to exchange messages in text format.
                ws.text(true);

                // Send a message to the connected WebSocket server.
                ws.async_write(wBuffer,
                               [&ws, &rBuffer](auto ec, auto nBytesWritten)
                               {
                                   OnSend(ws, rBuffer, ec);
                               });
            }

            void OnResolve(
                // --> Start of shared data
                websocket::stream<beast::tcp_stream> &ws,
                const std::string &url,
                const std::string &endpoint,
                const net::const_buffer &wBuffer,
                beast::flat_buffer &rBuffer,
                // <-- End of shared data
                const boost::system::error_code &ec,
                tcp::resolver::iterator resolverIt)
            {
                if (ec)
                {
                    Log("OnResolve", ec);
                    return;
                }

                // Connect to the TCP socket.
                // Instead of constructing the socket and the ws objects separately, the
                // socket is now embedded in ws, and we access it through next_layer().
                ws.next_layer().async_connect(*resolverIt,
                                              [&ws, &url, &endpoint, &wBuffer, &rBuffer](auto ec)
                                              {
                                                  OnConnect(ws, url, endpoint, wBuffer, rBuffer, ec);
                                              });
            }

            /*

            37 love lane
            ha5 3ee

            020 88 66 57 66

            */

            int run()
            {
                // Connection targets
                const std::string url{"ltnm.learncppthroughprojects.com"};
                const std::string endpoint{"/echo"};
                const std::string port{"80"};
                const std::string message{"Hello WebSocket"};

                std::cerr << "[" << std::setw(14) << std::this_thread::get_id() << "] main"
                          << std::endl;

                // Always start with an I/O context object.
                boost::asio::io_context ioc{};

                // Create the objects that will be shared by the connection callbacks.
                websocket::stream<beast::tcp_stream> ws{ioc};

                net::const_buffer wBuffer{message.c_str(), message.size()};
                beast::flat_buffer rBuffer{};

                tcp::resolver resolver{ioc};
                resolver.async_resolve(url, port,
                                       [&ws, &url, &endpoint, &wBuffer, &rBuffer](auto ec, auto resolverIt)
                                       {
                                           OnResolve(ws, url, endpoint, wBuffer, rBuffer, ec, resolverIt);
                                       });

                // We must call io_context::run for asynchronous callbacks to run.
                ioc.run();

                // We only succeed if the buffer contains the same message we sent out
                // originally.
                if (boost::beast::buffers_to_string(rBuffer.data()) != message)
                {
                    // rBuffer.data().size() == 0) {
                    return -1;
                }

                return 0;
            }
        } // namespace callback

    }
} // namespace ws
