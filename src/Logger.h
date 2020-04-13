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
        if (!_inBlock) {
            Serial.printf("[%c][%s]: ", severityIndicator(severity), _category.c_str());
        }

        if (sizeof...(params) == 0) {
            Serial.print(fmt);
        } else {
            Serial.printf(fmt, params...);
        }

        if (!_inBlock) {
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

    struct Block
    {
        Block(bool& inBlock)
            : _inBlock(inBlock)
        {}

        ~Block()
        {
            _inBlock = false;
            Serial.println();
        }

    private:
        bool& _inBlock;
    };

    template <typename... Params>
    Block logBlock(Severity severity, const char* fmt, Params... params) const
    {
        _inBlock = true;

        Serial.printf("[%c][%s]: ", severityIndicator(severity), _category.c_str());

        if (sizeof...(params) == 0) {
            Serial.print(fmt);
        } else {
            Serial.printf(fmt, params...);
        }

        return Block{ _inBlock };
    }

    template <typename... Params>
    Block debugBlock(const char* fmt, Params... params) const
    {
        return logBlock(Severity::Debug, fmt, params...);
    }

private:
    const std::string _category;
    mutable bool _inBlock = false;

    static char severityIndicator(Severity severity);
};