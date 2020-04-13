#include "Logger.h"

#include <Arduino.h>

Logger::Logger(std::string category)
    : _category(std::move(category))
{}

char Logger::severityIndicator(const Severity severity)
{
    switch (severity) {
        case Severity::Error:
            return 'E';
            break;
        case Severity::Warning:
            return 'W';
            break;
        case Severity::Info:
            return 'I';
            break;
        case Severity::Debug:
            return 'D';
            break;
    }

    return '?';
}