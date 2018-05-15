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
constexpr uint32_t ScanDuration = 10 * 1000;

constexpr const char API_INGESTION_STREAM[] = FK_API_BASE "/messages/ingestion/stream";

constexpr uint8_t NumberOfTwoWireRetries = 3;

constexpr uint32_t MaximumTwoWireReply = 3000;

constexpr uint16_t ServerPort = 54321;
constexpr uint16_t FK_CORE_PORT_UDP = 54321;

// This is copied from WiFiSocket.cpp. I wish they were in a header.
constexpr size_t SOCKET_BUFFER_SIZE = 1472;

constexpr uint32_t PowerManagementInterval = 5000;

constexpr uint32_t GpsFixAttemptInterval = 10 * 1000;
constexpr uint32_t GpsStatusInterval = 1 * 500;

constexpr uint32_t PingInterval = 2500;

/**
 *
 */
constexpr uint32_t WifiConnectionMemory = 128;

/**
 *
 */
constexpr uint32_t WifiAwakenInterval = 1000 * 60 * 1;

constexpr uint32_t WifiInactivityTimeout = 60 * 1000 * 2;

constexpr uint32_t MaximumUpload = 1024 * 1024;

constexpr uint32_t DefaultPageSize = (size_t)(8 * 4096);

constexpr uint32_t IdleRebootInterval = 1000 * 60 * 10;
constexpr uint32_t ConnectionTimeout = 5000;

constexpr uint32_t RescanIntervalOrphaned = 30 * 1000;
constexpr uint32_t RescanExistingModulesInterval = 5 * 60 * 1000;

constexpr uint32_t TransmitBusyWaitMax = 10 * 1000;

constexpr uint32_t TransmitFileMaximumTries = 3;

}

#endif
