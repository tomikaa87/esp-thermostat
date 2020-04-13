#include "FirmwareVersion.h"
#include "OtaUpdater.h"
#include "SystemClock.h"

#include <asyncHTTPrequest.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiSTA.h>
#include <ESP8266httpUpdate.h>

OtaUpdater::OtaUpdater(std::string updateBaseUrl, const SystemClock& systemClock)
    : _updateBaseUrl(std::move(updateBaseUrl))
    , _systemClock(systemClock)
{
    _log.info("initializing");
}

OtaUpdater::~OtaUpdater() = default;

void OtaUpdater::task()
{
    switch (_state) {
        case State::Idle:
            if ((_lastCheckedForUpdates > 0 && _systemClock.utcTime() - _lastCheckedForUpdates >= _updateCheckIntervalSeconds)
                    || _forceUpdateCheck) {
                _log.info("checking for updates");

                _state = State::GetAvailableVersion;
                _forceUpdateCheck = false;

                if (_updateStateChangedHandler) {
                    _updateStateChangedHandler(UpdateState::CheckingForUpdate);
                }

                _requestStartTimestamp = _systemClock.utcTime();

                createVersionInfoRequest();
                if (!_httpClient->send()) {
                    _log.error("failed to send HTTP request");

                    _httpClient.reset();
                    _state = State::Idle;

                    if (_updateStateChangedHandler) {
                        _updateStateChangedHandler(UpdateState::CheckFailed);
                    }
                }
            }
            break;

        case State::GetAvailableVersion:
            if (_httpClient->readyState() != 4) {
                if (_systemClock.utcTime() - _requestStartTimestamp >= 10) {
                    _log.warning("version check request timed out");

                    _state = State::Idle;
                    _httpClient.reset();

                    if (_updateStateChangedHandler) {
                        _updateStateChangedHandler(UpdateState::CheckFailed);
                    }
                }
                break;
            }
            if (_httpClient->responseHTTPcode() != 200) {
                _log.warning("HTTP request failed, response code: %d", _httpClient->responseHTTPcode());
                _state = State::Idle;
                _httpClient.reset();
                break;
            }
            switch (checkVersion(_httpClient->responseText().c_str())) {
                case VersionCheckResult::NoUpdateNeeded:
                    _log.info("firmware is up-to-date");

                    _state = State::Idle;
                    _httpClient.reset();

                    if (_updateStateChangedHandler) {
                        _updateStateChangedHandler(UpdateState::NoUpdateNeeded);
                    }
                    break;

                case VersionCheckResult::CannotCheckVersion: {
                    _log.warning("cannot check update version");

                    _state = State::Idle;
                    _httpClient.reset();

                    if (_updateStateChangedHandler) {
                        _updateStateChangedHandler(UpdateState::CheckFailed);
                    }
                    break;
                }

                case VersionCheckResult::UpdateNeeded: {
                    _log.info("firmware update is necessary");

                    _state = State::DownloadUpdate;

                    if (_updateStateChangedHandler) {
                        _updateStateChangedHandler(UpdateState::DownloadingUpdate);
                    }
                    break;
                }

                default:
                    _log.error("invalid version check result");

                    _state = State::Idle;
                    _httpClient.reset();

                    break;
            }
            break;

        case State::DownloadUpdate: {
            _log.info("downloading and updating firmware");

            WiFiClient wifiClient;

            switch (ESPhttpUpdate.update(wifiClient, getFwUpdateUrl().c_str())) {
                case HTTP_UPDATE_OK:
                    _log.info("update succeeded, procedding to reboot");

                    _rebootTimestamp = _systemClock.utcTime();
                    _state = State::Reboot;

                    if (_updateStateChangedHandler) {
                        _updateStateChangedHandler(UpdateState::RebootingAfterUpdate);
                    }

                    break;

                default:
                    _log.error("update failed, error: %s", ESPhttpUpdate.getLastErrorString().c_str());

                    _state = State::Idle;

                    if (_updateStateChangedHandler) {
                        _updateStateChangedHandler(UpdateState::DownloadFailed);
                    }

                    break;
            }
        }

        case State::Reboot: {
            if (_systemClock.utcTime() - _rebootTimestamp >= 5) {
                break;
            }

            _log.info("rebooting");

            ESP.restart();

            break;
        }
    }
}

void OtaUpdater::forceUpdate()
{
    _log.info("forcing update check");

    _forceUpdateCheck = true;
}

void OtaUpdater::setUpdateStateChangedHandler(UpdateStateChangedHandler&& handler)
{
    _updateStateChangedHandler = std::move(handler);
}

void OtaUpdater::createVersionInfoRequest()
{
    if (_httpClient) {
        _log.warning("HTTP client already created");
    }

    std::string url{ _updateBaseUrl };
    url.append("/").append(WiFi.macAddress().c_str()).append("/version");

    _log.debug("creating version info request, URL: %s", url.c_str());

    _httpClient.reset(new asyncHTTPrequest);
    _httpClient->setDebug(false);
    _httpClient->setTimeout(5); // seconds
    _httpClient->open("GET", url.c_str());
}

OtaUpdater::VersionCheckResult OtaUpdater::checkVersion(const std::string& versionCheckResponse)
{
    if (versionCheckResponse.empty()) {
        return VersionCheckResult::CannotCheckVersion;
    }

    int major = 0;
    int minor = 0;
    int patch = 0;

    sscanf(versionCheckResponse.c_str(), "%u.%u.%u", &major, &minor, &patch);

    if (major < FW_VER_MAJOR) {
        return VersionCheckResult::NoUpdateNeeded;
    }

    if (major > FW_VER_MAJOR) {
        return VersionCheckResult::UpdateNeeded;
    }

    if (minor < FW_VER_MINOR) {
        return VersionCheckResult::NoUpdateNeeded;
    }

    if (minor > FW_VER_MINOR) {
        return VersionCheckResult::UpdateNeeded;
    }

    if (patch < FW_VER_PATCH) {
        return VersionCheckResult::NoUpdateNeeded;
    }

    if (patch > FW_VER_PATCH) {
        return VersionCheckResult::UpdateNeeded;
    }

    return VersionCheckResult::NoUpdateNeeded;
}

std::string OtaUpdater::getFwUpdateUrl() const
{
    std::string url{ _updateBaseUrl };
    url.append("/").append(WiFi.macAddress().c_str()).append("/firmware.bin");

    return url;
}