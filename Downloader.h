//
// Created by fo on 06/04/2021.
//
// This file contains the definition of class Downloader
// Downloader is responsible for For downloading files from remote sources and storing them locally, will maintaining their names
#include "filterers/URLFilterer.h"

#include <vector>
#include <fcntl.h>
#include <curl/curl.h>
#include <system_error>
#include <iostream>
#include <cstring>

//include OS-dependent dependencies
#include <unistd.h>


#ifndef DOWNLOADER_DOWNLOADER_H
#define DOWNLOADER_DOWNLOADER_H

class Downloader {
    /**
     * @brief: the collection of source-URLs for the file(s) to be downloaded. Each URL is separated from its neighbour by a newline
     */
    std::string _urls;

    /**
     * @brief: the URL of the file containing the list of URLs of the files to be downloaded
     */
    std::string _urlsSrc;

    /**
     * @brief: filters-out filenames from URLs
     */
    URLFilterer _urlFilterer;

    /**
     * @brief: the present working directory(i.e the directory of the executable, of which this Downloader is component)
     */
    std::string _pwd;

    /**
     * @brief: realizes remote-access
     */
    CURL *_curl{nullptr};

    /**
     * @brief: writes the contents in the passed source into the passed sink
     * @param src: the source
     * @param charSize: the size of 1 character in the source
     * @param charCount: the total number of characters in the source
     * @param sink: the sink
     * @return int: the size of contents written
     */
    static size_t writeContent(char *src, size_t charSize, size_t charCount, std::string *sink) {
        //append the contents in src to sink
        sink->append(src, charSize * charCount);

        return (charCount * charSize);
    }

    /**
     * @brief: writes the contents in passed source into the file with the passed file-descriptor
     * @param src: the source
     * @param charSize: the size of 1 character in the source
     * @param charCount: the total number of characters in the source
     * @param sinkFd: the file-descriptor of the sink
     * @return int: the size of the contents written
     */
    static size_t writeFile(char *src, size_t charSize, size_t charCount, int *sinkFd) {
        //reset errno
        errno = 0;

        //write the contents in source into the file
        write(*sinkFd, (const char *)src, charSize * charCount);

        //confirm that error did not occur
        if (errno) {        //error occurred
            //create the error string
            std::string error = "In Downloader::writeFile: ";
            error += strerror(errno);

            //report the error
            throw error;
        }

        return (charCount *charSize);
    }

    /**
     * @brief: downloads the source-URLS for the files to be later downloaded from the node identified by the passed source-URL
     * @param srcURL: the source-URL
     */
    inline void getURLs() {
        //set the necessary options
        curl_easy_setopt(_curl, CURLOPT_URL, _urlsSrc.c_str());
        curl_easy_setopt(_curl, CURLOPT_WRITEFUNCTION, &Downloader::writeContent);
        curl_easy_setopt(_curl, CURLOPT_WRITEDATA, &_urls);

        if (CURLcode res = curl_easy_perform(_curl); res) {     //an error occurred
            //create the error string
            std::string error = "In Downloader::getURLs: ";
            error += curl_easy_strerror(res);

            //report the error
            throw error;
        }
    }

public:
    explicit Downloader(const std::string &linksURL) : _curl(curl_easy_init()), _urlsSrc(linksURL) {
        //get the current working directory
        _pwd = get_current_dir_name();
        _pwd += "/";
    }

    /**
     * @brief: begins the process of downloading remote-files and storing them locally
     */
    void start() {
        //get the source-URLs
        getURLs();

        //visit all the nodes for identified by the source-URLs and download the files
        while (!_urls.empty()) {        //there are still URLs in _urls
            std::string url;        //the present URL

            //find the position of the next newline and cut _urls there
            if (int pos = _urls.find('\n'); pos != std::string::npos) {     //there is a newline
                //get the next URL
                url = _urls.substr(0, pos);

                //cut-off the present URL from _urls
                _urls = (_urls.size() >= pos + 1) ? _urls.substr(pos + 1) : std::string("");
            } else {        //there is no newline
                url = _urls;
                //empty _urls
                {
                    std::string tmp;
                    _urls.swap(tmp);
                }
            }

            //filter filename from present URL
            std::string fileName = _urlFilterer.getFileName(url);
            if (fileName.empty()) {     //there's no filename in the URL
                //url has no filename
                fileName = "remoteFile";
            }

            int counter = 0;        //counts the number of passed iteration in the next loop
            int file;       //the file-descriptor of the file to be opened
            //try to create a new file with filename
            do {
                if (errno == EEXIST) {      //a file with filename already exists
                    fileName += std::to_string(++counter);
                } else if (errno) {        //some other error has occurred
                    //create the error-message
                    std::string error = "In Downloader::start: ";
                    error += strerror(errno);

                    //report the error
                    throw std::system_error(std::error_code(errno, std::system_category()));
                }

                //create and open a new file to write the next remote file to
                file = creat(fileName.c_str(), 0664);
            } while (errno);

            //reset errno
            errno = 0;

            //set the necessary Options
            curl_easy_setopt(_curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(_curl, CURLOPT_WRITEFUNCTION, &Downloader::writeFile);
            curl_easy_setopt(_curl, CURLOPT_WRITEDATA, &file);

            //download the file
            if (CURLcode res = curl_easy_perform(_curl); res) {     //error has occurred
                //create the error-message
                std::string error = "In Download::start: ";
                error += curl_easy_strerror(res);

                //report the error
                throw std::system_error(std::error_code(errno, std::system_category()));
            }

            //close the opened file
            close(file);
        }
    }

    ~Downloader() {
        //free the memory allocated to _curl
        curl_easy_cleanup(_curl);
    }
};

#endif //DOWNLOADER_DOWNLOADER_H
