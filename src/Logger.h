#pragma once

#include <cstdarg>
#include <string>

class Logger
{
public:
    explicit Logger(std::string category);

    enum class Severity
    {
        Error,
        Warning,
        Info,
        Debug
    };

    void logv(Severity severity, const char* fmt, va_list args);
    void logf(Severity severity, const char* fmt, ...);
    void log(Severity severity, const char* msg);

    void errorf(const char* fmt, ...);
    void warningf(const char* fmt, ...);
    void infof(const char* fmt, ...);
    void debugf(const char* fmt, ...);

    void error(const char* s);
    void warning(const char* s);
    void info(const char* s);
    void debug(const char* s);

private:
    std::string _category;

    static char severityIndicator(Severity severity);
};