#ifndef POAC_IO_NETWORK_HPP
#define POAC_IO_NETWORK_HPP

#include <iostream>
#include <string>
#include <memory>
#include <cstdio>

#include <curl/curl.h>


namespace poac::io::network {
    size_t callback_write(char* ptr, size_t size, size_t nmemb, std::string* stream) {
        int dataLength = size * nmemb;
        stream->append(ptr, dataLength);
        return dataLength;
    }

    // TODO: Check if connecting network

    std::string get(const std::string& url) {
        std::string chunk;
        if (CURL* curl = curl_easy_init(); curl != nullptr) {
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, callback_write);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &chunk);
            if (CURLcode res = curl_easy_perform(curl); res != CURLE_OK)
                std::cerr << "curl_easy_perform() failed." << std::endl;
            curl_easy_cleanup(curl);
        }
        return chunk;
    }
    std::string get_github(const std::string& url) {
        std::string chunk;
        std::string useragent(std::string("curl/") + curl_version());
        if (CURL* curl = curl_easy_init(); curl != nullptr) {
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_USERAGENT, &useragent);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, callback_write);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &chunk);
            if (CURLcode res = curl_easy_perform(curl); res != CURLE_OK)
                std::cerr << "curl_easy_perform() failed." << std::endl;
            curl_easy_cleanup(curl);
        }
        return chunk;
    }

    void get_file(const std::string& from, const std::string& to) {
        if (CURL* curl = curl_easy_init(); curl != nullptr) {
            FILE* fp = std::fopen(to.data(), "wb");
            curl_easy_setopt(curl, CURLOPT_URL, from.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, std::fwrite);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
            // Switch on full protocol/debug output
//            curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
            if (CURLcode res = curl_easy_perform(curl); res != CURLE_OK)
                std::cerr << "curl told us " << res << std::endl;
            curl_easy_cleanup(curl);
            std::fclose(fp);
        }
    }
} // end namespace
#endif // !POAC_IO_NETWORK_HPP
