#ifndef FK_TUNING_H_INCLUDED
#define FK_TUNING_H_INCLUDED

#include <cinttypes>
#include <cstring>

namespace fk {

#ifndef FK_API_BASE
#define FK_API_BASE "http://api.fkdev.org"
#endif

#ifdef FK_NATURALIST
constexpr const char *DefaultName = "FieldKit Naturalist";
constexpr const char *ModuleName = "fk-naturalist";
#else
constexpr const char *DefaultName = "FieldKit Device";
constexpr const char *ModuleName = "fk-core";
#endif

constexpr uint32_t Seconds = 1000;
constexpr uint32_t Minutes = 60 * Seconds;

constexpr uint32_t LivePollInactivity = 10 * Seconds;

constexpr uint8_t TwoWireNumberOfRetries = 3;
constexpr uint32_t TwoWireMaximumReplyWait = 1 * Seconds;
constexpr uint32_t TwoWireDefaultSpeed = 400000;
constexpr uint32_t TwoWireDefaultReplyWait = 100;
constexpr uint32_t TwoWireStreamingWait = 1000;

/***
 * This is high because we retry fast
 * and some operations, like
 * DATA_PREPARE can take a few seconds.
 */
constexpr uint32_t TwoWireRetries = 30;

constexpr uint32_t PowerManagementQueryInterval = 30 * Seconds;
constexpr uint32_t PowerManagementAlertInterval = 1 * Minutes;

constexpr uint32_t ReadingsInterval = 30 * 5;
constexpr uint32_t CheckPowerInterval = 15 * 60;
constexpr uint32_t WifiTransmitInterval = 60 * 5;
constexpr uint32_t RadioTransmitInterval = 60 * 5;

constexpr uint32_t GpsFixAttemptInterval = 10 * Seconds;
constexpr uint32_t GpsStatusInterval = 1 * 500;
constexpr bool     GpsEchoRaw = false;

constexpr uint16_t DiscoveryUdpPort = 54321;
constexpr uint32_t DiscoveryPingInterval = 2500;

constexpr uint32_t MaximumWaitBeforeReturnToIdle = 70 * Minutes;
constexpr uint32_t ModuleIdleRebootInterval = 120 * Minutes;
constexpr uint32_t NoModulesRebootWait = 10 * Minutes;

constexpr const char WifiApiUrlBase[] = FK_API_BASE;
constexpr const char WifiApiUrlIngestionStream[] = FK_API_BASE "/messages/ingestion/stream";
constexpr const char WifiApiUrlFirmware[] = FK_API_BASE "/devices/%s/%s/firmware";
constexpr uint16_t WifiServerPort = 54321;
constexpr uint32_t WifiConnectionMemory = 128;
constexpr uint32_t WifiConnectionTimeout = 20 * Seconds;
constexpr uint32_t WifiTransmitBusyWaitMax = 10 * Seconds;
constexpr uint32_t WifiTransmitFileMaximumTries = 3;
constexpr uint32_t WifiInactivityTimeout = 1 * Minutes;
constexpr uint32_t WifiCaptivitiyTimeout = 10 * Seconds;

/**
 * This is copied from SOCKET_BUFFER_SIZE in WiFiSocket.cpp.
 */
constexpr size_t WifiSocketBufferSize = 1472;

constexpr uint32_t NtpRetryAfter = 2 * Seconds;
constexpr uint32_t NtpMaximumWait = 30 * Seconds;

constexpr size_t RadioTransmitFileBufferSize = 256;
constexpr size_t RadioTransmitFileCopierBufferSize = 128;
constexpr uint32_t RadioTransmitFileMaximumSize = 32768;
constexpr bool RadioTransmitFileTruncateAfter = true;

constexpr uint32_t RescanIntervalOrphaned = 30 * Seconds;
constexpr uint32_t RescanExistingModulesInterval = 5 * Minutes;

constexpr uint32_t StatusInterval = 5 * Seconds;
constexpr float BatteryStatusBlinkThreshold = 60.0f;
constexpr float BatteryLowPowerSleepThreshold = 10.0;
constexpr float BatteryLowPowerResumeThreshold = 30.0;
constexpr uint32_t LowPowerSleepDurationSeconds = 60 * 10;

constexpr uint32_t SleepMaximumGranularity = 8192;
constexpr uint32_t SleepMinimumSeconds = 5;
constexpr uint32_t SleepLeadingWakeupSeconds = 5;

constexpr uint32_t FileCopyBufferSize = 256;
constexpr uint32_t FileCopyStatusInterval = 1 * Seconds;
constexpr uint32_t FileCopyMaximumElapsed = 4 * Seconds;

constexpr size_t FileSystemNumberOfFiles = 5;

constexpr uint32_t LedsBlinkButtonWarn = 100;
constexpr uint32_t LedsDisableAfter = 60 * Minutes;

constexpr uint32_t ButtonTouchHysteresis = 100;
constexpr uint32_t ButtonShortPressDuration = 1 * Seconds;
constexpr uint32_t ButtonLongPressDuration = 5 * Seconds;

constexpr uint32_t SchedulerCheckInterval = 500;

constexpr uint32_t SelfFlashWaitPeriod = 20 * Seconds;

}

#endif
