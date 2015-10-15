#include <string>
#include <sstream>
#include "stringutils.h"
#include <iostream>

std::string escapeXMLAttribute(const std::string& value) {
    std::string res = value;
    std::size_t pos = 0;
    while ((pos = res.find("&",pos)) != std::string::npos) {
        res.replace(pos, 1, "&amp;"); ++pos;
    }
    while ((pos = res.find("<")) != std::string::npos) {
        res.replace(pos, 1, "&lt;");
    }
    while ((pos = res.find(">")) != std::string::npos) {
        res.replace(pos, 1, "&gt;");
    }
    while ((pos = res.find("\"")) != std::string::npos) {
        res.replace(pos, 1, "&quot;");
    }
    while ((pos = res.find("'")) != std::string::npos) {
        res.replace(pos, 1, "&apos;");
    }

    return res;
}
