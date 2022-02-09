
#pragma once

#include <cstdint>
#include <string>
#include <jsonmapper/JsonMapper.h>

namespace FS {

struct FlexClientConfig {
    std::string tcpAcceptorAddr { "127.0.0.1:9007" };  ///< The tcp server port we are using
    std::string kcpServerAddr { "138.2.76.165:9008" }; ///< The remote kcp server we are going to connect
    std::string logFile {};

    JM_DEFINE_MAP(
        JMKVP(tcpAcceptorAddr),
        JMKVP(kcpServerAddr),
        JMKVP(logFile)
        //
    )
};

}