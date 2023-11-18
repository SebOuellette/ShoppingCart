#ifndef HTTP_HPP
#define HTTP_HPP

#include "defines.hpp"
#include <iostream>
#include <string>
#include <vector>


typedef struct _res_t {
    CURLcode code;
    std::string body;
} res_t;


static class HTTP {
private:
    size_t _ResCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
        size_t totalSize = size * nmemb;
        
        output->append
    }

    // Initialize the curl object
    int init(CURL* curl) {
        curl = curl_easy_init();

        if (!curl) {
            std::cerr << "Failed to initialize curl. Failing." << std::endl;
            exit(1);
        }
        return 0;
    }

    // Cleanup a curl object
    void cleanup(CURL* curl, curl_slist* headers = nullptr) {
        curl_easy_cleanup(curl);

        if (headers != nullptr) {
            curl_slist_free_all(headers);
        }
    }

    // Clean all curl objects
    void cleanup() {
        curl_global_cleanup();
    }


public:

    // Raw request
    res_t request(std::string url, std::string method = "GET", std::vector<string> headers, std::string postbody = "") {
        CURL* curl;
        res_t res;
        this->init(curl);

        /// --- Begin HTTP Requsest ---

        // Set request URL
        curl_easy_setopt(curl, CURLOPT_URL, url);

        // Set request method
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method);

        // If this is a post request, send post body
        if (method == "POST" || method == "PUT") {
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postbody.c_str());
        }

        // Add any and all headers
        struct curl_slist* _headers = nullptr;
        for (int i=0;i<headers.size();i++) {
            _headers = curl_slist_append(_headers, headers[i]);
        }

        // If we were given at least one header, then we can actually add headers to the request
        if (headers.size() > 0) {
            curl_easy_setopt(curl, CURLOPT_httpheader, _headers);
        }

        // Response Body
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, this->_ResCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &res.body);

        // Perform the request
        res.code = curl_easy_perform(curl);

        if (res.code != CURLE_OK) {
            std::cerr << "HTTP Request to " << url << " failed." << std::endl;
        }

        this->cleanup(curl, _headers);

        return res;
    }


    // Example: "Post request to google. We are authorizing with our token aksdfj, setting the context type to normal text, and sending the word meatballs"
    // res_t res = HTTP.request("https://google.com/", "POST", {"Authorization: Bearer aksdfj", "Context-type: text/plain"}, "meatballs");

    // res.code is the statuscode
    // res.body is the body in an std::string



}

#endif