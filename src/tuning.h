#ifndef FK_TUNING_H_INCLUDED
#define FK_TUNING_H_INCLUDED

#include <cstdint>
#include <cstring>

namespace fk {

#ifndef FK_API_BASE
#define FK_API_BASE "http://api.fkdev.org"
#endif

#ifdef FK_NATURALIST
constexpr const char *DefaultName = "FieldKit Naturalist";
#else
constexpr const char *DefaultName = "FieldKit Device";
#endif

constexpr uint32_t LivePollInactivity = 1000 * 10;

constexpr uint8_t TwoWireNumberOfRetries = 3;
constexpr uint32_t TwoWireMaximumReplyWait = 3000;

constexpr uint32_t PowerManagementInterval = 5000;

constexpr uint32_t ReadingsInterval = 30;
constexpr uint32_t CheckPowerInterval = 15 * 60;
constexpr uint32_t WifiTransmitInterval = 15 * 60;

constexpr uint32_t GpsFixAttemptInterval = 10 * 1000;
constexpr uint32_t GpsStatusInterval = 1 * 500;
constexpr bool     GpsEchoRaw = false;

constexpr uint16_t DiscoveryUdpPort = 54321;
constexpr uint32_t DiscoveryPingInterval = 2500;

constexpr uint32_t ModuleIdleRebootInterval = 1000 * 60 * 10;

constexpr const char WifiApiUrlBase[] = FK_API_BASE;
constexpr const char WifiApiUrlIngestionStream[] = FK_API_BASE "/messages/ingestion/stream";
constexpr uint16_t WifiServerPort = 54321;
constexpr uint32_t WifiConnectionMemory = 128;
constexpr uint32_t WifiConnectionTimeout = 5000;
constexpr uint32_t WifiTransmitBusyWaitMax = 10 * 1000;
constexpr uint32_t WifiTransmitFileMaximumTries = 3;
constexpr uint32_t WifiInactivityTimeout = 1000 * 60;
constexpr uint32_t WifiCaptivitiyTimeout = 1000 * 10;

/**
 * This is copied from SOCKET_BUFFER_SIZE in WiFiSocket.cpp.
 */
constexpr size_t WifiSocketBufferSize = 1472;

constexpr uint32_t NtpRetryAfter = 2000;
constexpr uint32_t NtpMaximumWait = 30 * 1000;

constexpr size_t RadioTransmitFileBufferSize = 256;
constexpr size_t RadioTransmitFileCopierBufferSize = 128;
constexpr uint32_t RadioTransmitFileMaximumSize = 32768;
constexpr bool RadioTransmitFileTruncateAfter = true;

constexpr uint32_t RescanIntervalOrphaned = 30 * 1000;
constexpr uint32_t RescanExistingModulesInterval = 5 * 60 * 1000;

constexpr uint32_t StatusInterval = 5000;
constexpr float BatteryStatusBlinkThreshold = 60.0f;
constexpr float BatteryLowPowerSleepThreshold = 10.0;
constexpr float BatteryLowPowerResumeThreshold = 30.0;

constexpr uint32_t SleepMaximumGranularity = 8192;

constexpr uint32_t FileCopyBufferSize = 256;
constexpr uint32_t FileCopyStatusInterval = 1000;
constexpr uint32_t FileCopyMaximumElapsed = 4000;

constexpr size_t FileSystemNumberOfFiles = 5;

constexpr uint32_t LedsBlinkButtonWarn = 100;
constexpr uint32_t LedsDisableAfter = 1000 * 60 * 60;

constexpr uint32_t ButtonTouchHysteresis = 100;
constexpr uint32_t ButtonShortPressDuration = 50;
constexpr uint32_t ButtonLongPressDuration = 2000;
constexpr uint32_t ButtonLedsWarnDuration = 1000;

}

#endif
