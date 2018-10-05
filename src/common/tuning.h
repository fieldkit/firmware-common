#ifndef FK_TUNING_H_INCLUDED
#define FK_TUNING_H_INCLUDED

#include <cinttypes>
#include <cstring>

namespace fk {

constexpr uint32_t Seconds = 1000;
constexpr uint32_t Minutes = 60 * Seconds;
constexpr uint32_t Hours = 60 * Minutes;

// constexpr uint8_t TwoWireNumberOfRetries = 3;
constexpr uint32_t TwoWireMaximumReplyWait = 1 * Seconds;
constexpr uint32_t TwoWireDefaultSpeed = 400000;
constexpr uint32_t TwoWireDefaultReplyWait = 100;
constexpr uint32_t TwoWireStreamingWait = 1000;

/***
 * How many times to retry a given TwoWire query before giving up.
 *
 * This is high because we retry fast and some operations, like DATA_PREPARE can
 * take a few seconds. Most other operations will require a smaller number of
 * retries, so maybe this is kind of ugly.
 */
constexpr uint32_t TwoWireRetries = 30;

constexpr uint32_t PowerManagementQueryInterval = 30 * Seconds;
constexpr uint32_t PowerManagementAlertInterval = 1 * Minutes;

constexpr uint16_t DiscoveryUdpPort = 54321;
constexpr uint32_t DiscoveryPingInterval = 2500;

constexpr uint16_t WifiServerPort = 54321;
constexpr uint32_t WifiConnectionMemory = 128;

/**
 * This is copied from SOCKET_BUFFER_SIZE in WiFiSocket.cpp.
 */
constexpr size_t WifiSocketBufferSize = 1472;

constexpr size_t RadioTransmitFileBufferSize = 256;
constexpr size_t RadioTransmitFileCopierBufferSize = 128;

constexpr uint32_t SleepMaximumGranularity = 8192;
constexpr uint32_t SleepMinimumSeconds = 5;
constexpr uint32_t SleepLeadingWakeupSeconds = 5;

constexpr uint32_t FileCopyBufferSize = 256;
constexpr uint32_t FileCopyStatusInterval = 1 * Seconds;
constexpr uint32_t FileCopyMaximumElapsed = 4 * Seconds;

constexpr size_t FileSystemNumberOfFiles = 5;

constexpr uint32_t ButtonTouchHysteresis = 100;
constexpr uint32_t ButtonShortPressDuration = 1 * Seconds;
constexpr uint32_t ButtonLongPressDuration = 5 * Seconds;

constexpr uint32_t SchedulerCheckInterval = 500;

constexpr uint32_t SelfFlashWaitPeriod = 20 * Seconds;

// TODO: The following should be moved to friendly configuration area.

constexpr uint32_t StatusInterval = 5 * Seconds;
constexpr float BatteryStatusBlinkThreshold = 60.0f;
constexpr float BatteryLowPowerSleepThreshold = 10.0;
constexpr float BatteryLowPowerResumeThreshold = 30.0;
constexpr uint32_t LowPowerSleepDurationSeconds = 60 * 10;

constexpr uint32_t RadioTransmitFileMaximumSize = 32768;
constexpr bool RadioTransmitFileTruncateAfter = true;

constexpr uint32_t WifiConnectionTimeout = 20 * Seconds;
constexpr uint32_t WifiTransmitBusyWaitMax = 10 * Seconds;
constexpr uint32_t WifiTransmitFileMaximumTries = 3;
constexpr uint32_t WifiNtpMaximumWait = 10 * Seconds;

constexpr uint32_t MaximumWaitBeforeReturnToIdle = 70 * Minutes;
constexpr uint32_t ModuleIdleRebootInterval = 120 * Minutes;

constexpr uint32_t LivePollInactivity = 10 * Seconds;

}

#endif
