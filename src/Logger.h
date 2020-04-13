#pragma once

#include <string>

#include <Arduino.h>

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

    template <typename... Params>
    void log(Severity severity, const char* fmt, Params... params) const
    {
        Serial.printf("[%c][%s]: ", severityIndicator(severity), _category.c_str());

        if (sizeof...(params) == 0) {
            Serial.println(fmt);
        } else {
            Serial.printf(fmt, params...);
            Serial.println();
        }
    }

    template <typename... Params>
    void error(const char* fmt, Params... params) const
    {
        log(Severity::Error, fmt, params...);
    }

    template <typename... Params>
    void warning(const char* fmt, Params... params) const
    {
        log(Severity::Warning, fmt, params...);
    }

    template <typename... Params>
    void info(const char* fmt, Params... params) const
    {
        log(Severity::Info, fmt, params...);
    }

    template <typename... Params>
    void debug(const char* fmt, Params... params) const
    {
        log(Severity::Debug, fmt, params...);
    }

private:
    const std::string _category;

    static char severityIndicator(Severity severity);
};