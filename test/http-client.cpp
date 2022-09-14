//
// Created by Ruslan on 11/09/2022.
//

#include <network-monitor/file_downloader.hpp>

#include <boost/test/unit_test.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/http.hpp>

#include <string>

namespace beast = boost::beast;// from <boost/beast.hpp>
namespace http = beast::http;  // from <boost/beast/http.hpp>
namespace net = boost::asio;   // from <boost/asio.hpp>

BOOST_AUTO_TEST_SUITE(network_monitor);

BOOST_AUTO_TEST_CASE(file_downloader)
{
    const char* fileUrl {
            "https://ltnm.learncppthroughprojects.com/network-layout.json"
    };
//    const auto destination {
//            std::filesystem::temp_directory_path() / "network-layout.json"
//    };

    // Download the file.
    bool downloaded {file::handler::download_file(fileUrl, TESTS_NETWORK_LAYOUT_JSON)};
    BOOST_CHECK(downloaded);
    BOOST_CHECK(std::filesystem::exists(TESTS_NETWORK_LAYOUT_JSON));

    // Check the content of the file.
    // We cannot check the whole file content as it changes over time, but we
    // can at least check some expected file properties.
    {
        const std::string expectedString {"\"stations\": ["};
        std::ifstream file {TESTS_NETWORK_LAYOUT_JSON};
        std::string line {};
        bool foundExpectedString {false};
        while (std::getline(file, line)) {
            if (line.find(expectedString) != std::string::npos) {
                foundExpectedString = true;
                break;
            }
        }
        BOOST_CHECK(foundExpectedString);
    }

    // Clean up.
//    std::filesystem::remove(TESTS_NETWORK_LAYOUT_JSON);
}


BOOST_AUTO_TEST_CASE(parse_file)
{
    // Parse the file.
    const std::filesystem::path sourceFile {TESTS_NETWORK_LAYOUT_JSON};
    auto parsed = file::handler::ParseJsonFile(sourceFile);
    BOOST_CHECK(parsed.is_object());
    BOOST_CHECK(parsed.contains("lines"));
    BOOST_CHECK(parsed.at("lines").size() > 0);
    BOOST_CHECK(parsed.contains("stations"));
    BOOST_CHECK(parsed.at("stations").size() > 0);
    BOOST_CHECK(parsed.contains("travel_times"));
    BOOST_CHECK(parsed.at("travel_times").size() > 0);
}

BOOST_AUTO_TEST_SUITE_END();