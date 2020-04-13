#include "Logger.h"

#include <cstdio>

Logger::Logger(std::string category)
    : _category{ std::move(category) }
{}

void Logger::logv(Severity severity, const char* fmt, va_list args)
{
    // TODO this code doesn't work properly without fflush() and
    // doesn't print the line end characters (it does with "double" fflush)
    printf("[%c][%s]: ", severityIndicator(severity), _category.c_str());
    vprintf(fmt, args);
    fflush(stdout);
    printf("\r\n");
    fflush(stdout);
}

void Logger::logf(const Severity severity, const char* fmt, ...)
{
    std::va_list args;
    va_start(args, fmt);
    logv(severity, fmt, args);
    va_end(args);
}

void Logger::log(Severity severity, const char* msg)
{
    printf("[%c][%s]: %s\r\n",
        severityIndicator(severity),
        _category.c_str(),
        msg
    );
}

void Logger::errorf(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    logv(Severity::Error, fmt, args);
    va_end(args);
}

void Logger::warningf(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    logv(Severity::Warning, fmt, args);
    va_end(args);
}

void Logger::infof(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    logv(Severity::Info, fmt, args);
    va_end(args);
}

void Logger::debugf(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    logv(Severity::Debug, fmt, args);
    va_end(args);
}

void Logger::error(const char* s)
{
    log(Severity::Error, s);
}

void Logger::warning(const char* s)
{
    log(Severity::Warning, s);
}

void Logger::info(const char* s)
{
    log(Severity::Info, s);
}

void Logger::debug(const char* s)
{
    log(Severity::Debug, s);
}

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