#ifndef FK_TUNING_H_INCLUDED
#define FK_TUNING_H_INCLUDED

#include <cstdint>
#include <cstring>

namespace fk {

#ifndef FK_API_BASE
#define FK_API_BASE "http://api.fkdev.org"
#endif

constexpr const char *DefaultName = "FieldKit Device";

constexpr uint32_t LivePollInactivity = 1000 * 10;

constexpr uint8_t TwoWireNumberOfRetries = 3;
constexpr uint32_t TwoWireMaximumReplyWait = 3000;

constexpr uint32_t PowerManagementInterval = 5000;

constexpr uint32_t GpsFixAttemptInterval = 10 * 1000;
constexpr uint32_t GpsStatusInterval = 1 * 500;

constexpr uint16_t DiscoveryUdpPort = 54321;
constexpr uint32_t DiscoveryPingInterval = 2500;

constexpr uint32_t DefaultPageSize = (size_t)(8 * 4096);

constexpr uint32_t ModuleIdleRebootInterval = 1000 * 60 * 10;

constexpr uint16_t WifiServerPort = 54321;
constexpr const char WifiApiUrlBase[] = FK_API_BASE;
constexpr const char WifiApiUrlIngestionStream[] = FK_API_BASE "/messages/ingestion/stream";
constexpr uint32_t WifiScanDuration = 10 * 1000;
constexpr uint32_t WifiConnectionMemory = 128;
constexpr uint32_t WifiAwakenInterval = 1000 * 60 * 1;
constexpr uint32_t WifiInactivityTimeout = 60 * 1000 * 2;
constexpr uint32_t WifiTransmitFileMaximumSize = 1024 * 1024;
constexpr uint32_t WifiConnectionTimeout = 5000;
constexpr uint32_t WifiTransmitBusyWaitMax = 10 * 1000;
constexpr uint32_t WifiTransmitFileMaximumTries = 3;
// This is copied from WiFiSocket.cpp. I wish they were in a header.
constexpr size_t SOCKET_BUFFER_SIZE = 1472;

constexpr size_t RadioTransmitFileBufferSize = 256;
constexpr size_t RadioTransmitFileCopierBufferSize = 128;
constexpr uint32_t RadioTransmitFileMaximumSize = 32768;
constexpr bool RadioTransmitFileTruncateAfter = false;

constexpr uint32_t RescanIntervalOrphaned = 30 * 1000;
constexpr uint32_t RescanExistingModulesInterval = 5 * 60 * 1000;

}

#endif
