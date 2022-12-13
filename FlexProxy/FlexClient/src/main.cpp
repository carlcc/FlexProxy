#include "FlexClient/FlexClient.h"
#include "FlexCommon/Asio.h"
#include "FlexCommon/Log.h"
#include "FlexCommon/MiscUtils.h"
#include <iostream>
#include <jsonmapper/types/deserialize/string.h>
#include <jsonmapper/types/serialize/string.h>
#include <string>

int main(int argc, char** argv)
{
    FS::FlexClientConfig config {};
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

    FS::LoggerInit(config.logFile.empty() ? nullptr : config.logFile.c_str(), FS::LogLevel::kInfo);

    asio::io_context ioContext;
    FS::FlexClient client(ioContext, config);
    if (!client.Initialize()) {
        L_ERROR("Failed to initialize client");
        return 1;
    }
    client.Start();

    ioContext.run();

    return 0;
}