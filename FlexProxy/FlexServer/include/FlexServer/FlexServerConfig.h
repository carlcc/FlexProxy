
#pragma once

#include <cstdint>
#include <jsonmapper/JsonMapper.h>
#include <string>

namespace FS {

struct FlexServerConfig {
    std::string kcpBindAddr { "0.0.0.0:2022" };       ///< The udp server port for kcp
    std::string tcpProxyForAddr { "127.0.0.1:8888" }; ///< The tcp server port for who we are proxying for
    std::string logFile {};

    JM_DEFINE_MAP(
        JMKVP(kcpBindAddr),
        JMKVP(tcpProxyForAddr),
        JMKVP(logFile)
        //
    )
};

}