#include "FlexCommon/Log.h"
#include "FlexCommon/MiscUtils.h"
#include "FlexServer/FlexServer.h"
#include <jsonmapper/types/serialize/string.h>
#include <jsonmapper/types/deserialize/string.h>
#include <boost/asio.hpp>
#include <iostream>
#include <string>

int main(int argc, char** argv)
{
    FS::FlexServerConfig config {};
    if (argc == 2) {
        std::string content;
        if (!FS::MiscUtils::LoadFileContent(content, argv[1])) {
            std::cout << "Failed to load config file '" << argv[1] << "'" << std::endl;
            return 1;
        }

        jsonmapper::DeserializeContext context {};
        if (!jsonmapper::DeserializeFromJsonString(config, content, context)) {
            std::cout << "Failed to load config file '" << argv[1] << "'" << std::endl;
            std::cout << context.GetErrorString() << std::endl;
            return 1;
        }
    }
    {
        std::string content;
        if (jsonmapper::SerializeToJsonString(config, content, true)) {
            std::cout << "The final config is: " << content << std::endl;
        }
    }
    boost::asio::io_context ioContext;

    FS::LoggerInit(config.logFile.empty() ? nullptr : config.logFile.c_str(), FS::LogLevel::kInfo);
    FS::FlexServer server(ioContext, config);
    if (!server.Initialize()) {
        L_ERROR("Failed to initialize server");
        return 1;
    }
    server.Start();

    ioContext.run();

    return 0;
}