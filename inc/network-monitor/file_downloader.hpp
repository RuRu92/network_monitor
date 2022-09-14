//
// Created by Ruslan on 11/09/2022.
//

#ifndef NETWORK_MONITOR_FILE_DOWNLOADER_H
#define NETWORK_MONITOR_FILE_DOWNLOADER_H


#include "network-monitor/http-client.h"
#include <nlohmann/json.hpp>
#include <vector>
#include <string>
#include <fstream>
#include <filesystem>


namespace file::handler {
        bool download_file(const char *path, char *filename) {
            std::vector<http::response<http::string_body>> results{};
            net::io_context ioc;

            client::HttpClient client{ioc};

            http::response<http::string_body> response = client.get_request(path, true);

            std::string parsed_data = response.body();

            if(parsed_data.empty()) {
                return false;
            };

            std::ofstream ofstream{filename};
            ofstream << parsed_data;

            if(ofstream.bad()) {
                ofstream.close();
                return false;
            }

            ofstream.close();
            return true;
        };

        nlohmann::json ParseJsonFile(
                const std::filesystem::path& source
        )
        {
            nlohmann::json parsed {};
            if (!std::filesystem::exists(source)) {
                return parsed;
            }
            try {
                std::ifstream file {source};
                file >> parsed;
            } catch (...) {
                // Will return an empty object.
            }
            return parsed;
        }

    }// namespace file


// namespace file

#endif//NETWORK_MONITOR_FILE_DOWNLOADER_H
