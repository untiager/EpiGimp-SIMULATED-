// Implementation of ConsoleErrorHandler class
#include "../../include/Utils/Implementations.hpp"
#include <iostream>

namespace EpiGimp {

ConsoleErrorHandler::ConsoleErrorHandler(EventDispatcher* dispatcher) 
    : eventDispatcher_(dispatcher) 
{
}

void ConsoleErrorHandler::handleError(const std::string& message)
{
    std::cerr << "[ERROR] " << message << std::endl;
    if (eventDispatcher_) {
        eventDispatcher_->emit<ErrorEvent>(message);
    }
}

void ConsoleErrorHandler::handleWarning(const std::string& message)
{
    std::cout << "[WARNING] " << message << std::endl;
}

void ConsoleErrorHandler::handleInfo(const std::string& message)
{
    std::cout << "[INFO] " << message << std::endl;
}

} // namespace EpiGimp