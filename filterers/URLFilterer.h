//
// Created by fo on 06/04/2021.
//
// This file contains the definition of class URLFilterer
// URLFilterer is responsible for filtering-out important data from URLS(e.g file names)
#include <string>

#ifndef DOWNLOADER_URLFILTERER_H
#define DOWNLOADER_URLFILTERER_H

class URLFilterer {
public:

    /**
     * @brief: filterers-out the present file-name from the passed URL.
     * This function assumes that the passed string is a URL and that the last "/" of the URI, if any is present, is followed by the name of the file that is to be filtered-out
     * @param url: the URL, from which the concerned File-name is to be filtered-out from
     * @return std::string: the file-name
     */
    std::string getFileName(const std::string &url) {
        //confirm that "/" is present in the URL, by reading the URL from the end
        if (int pos = url.rfind("/"); pos != std::string::npos) {       //"/" is present
            //confirm the pos is part of the URI, and not a part of the host-name
            if (url.substr(pos - 1, 1)== "/") {      //pos is a position of the last slash of the protocol in the hostname (e.g. http://)
                goto returnNil;
            }
            //confirm that there is, at least, 1 more character in the URL after the last "/" and form a sub-string from there, else return an empty string
            return (url.size() >= pos + 2) ? url.substr(pos + 1) : std::string("");
        }

        returnNil:
        return std::string("");
    }
};

#endif //DOWNLOADER_URLFILTERER_H
