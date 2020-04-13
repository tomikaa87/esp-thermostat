#pragma once

#include "Logger.h"

#include <ctime>
#include <functional>
#include <memory>
#include <string>

class asyncHTTPrequest;
class SystemClock;

class OtaUpdater
{
public:
    OtaUpdater(std::string updateBaseUrl, const SystemClock& systemClock);
    ~OtaUpdater();

    void task();

    void forceUpdate();

    enum class UpdateState
    {
        CheckingForUpdate,
        NoUpdateNeeded,
        DownloadingUpdate,
        RebootingAfterUpdate,
        CheckFailed,
        DownloadFailed
    };

    using UpdateStateChangedHandler = std::function<void(UpdateState)>;
    void setUpdateStateChangedHandler(UpdateStateChangedHandler&& handler);

private:
    const std::string _updateBaseUrl;
    const SystemClock& _systemClock;
    Logger _log{ "OtaUpdater" };
    UpdateStateChangedHandler _updateStateChangedHandler;

    enum class State
    {
        Idle,
        GetAvailableVersion,
        DownloadUpdate,
        Reboot
    };
    State _state = State::Idle;

    std::time_t _lastCheckedForUpdates = 0;
    std::time_t _updateCheckIntervalSeconds = 60 * 60;
    std::time_t _rebootTimestamp = 0;
    std::time_t _requestStartTimestamp = 0;
    bool _forceUpdateCheck = false;
    std::unique_ptr<asyncHTTPrequest> _httpClient;

    void createVersionInfoRequest();

    enum class VersionCheckResult
    {
        NoUpdateNeeded,
        UpdateNeeded,
        CannotCheckVersion
    };
    VersionCheckResult checkVersion(const std::string& versionCheckResponse);

    std::string getFwUpdateUrl() const;
};