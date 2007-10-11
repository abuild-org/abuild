#include "Logger.hh"

int main()
{
    Logger* logger = Logger::getInstance();
    logger->logInfo("message 1");
    logger->logError("message 2");
    Logger::stopLogger();
    return 0;
}
