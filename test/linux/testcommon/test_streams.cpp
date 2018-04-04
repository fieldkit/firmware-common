#include "test_streams.h"

#include <fk-module-protocol.h>

StreamsSuite::StreamsSuite() {
}

StreamsSuite::~StreamsSuite() {};

void StreamsSuite::SetUp() {};

void StreamsSuite::TearDown() {};

TEST_F(StreamsSuite, BufferedReader) {
    auto buffer = fk::AlignedStorageBuffer<256>{};
    auto writer = fk::DirectWriter{ buffer.toBufferPtr() };

    auto name1 = "Jacob Lewallen";
    auto name2 = "Shah Selbe";

    writer.write(name1);
    writer.write(name2);

    auto reader = fk::DirectReader{ writer.toBufferPtr() };

    char buffer1[256] = { 0 };
    EXPECT_EQ(strlen(name1), reader.read((uint8_t *)buffer1, (size_t)strlen(name1)));

    char buffer2[256] = { 0 };
    EXPECT_EQ(strlen(name2), reader.read((uint8_t *)buffer2, (size_t)strlen(name2)));

    ASSERT_STREQ(name1, buffer1);
    ASSERT_STREQ(name2, buffer2);

    ASSERT_EQ(-1, reader.read());

    reader.beginning();

    char buffer3[256] = { 0 };
    EXPECT_EQ(strlen(name1) + strlen(name2), reader.read((uint8_t *)buffer3, sizeof(buffer3)));

    auto names = "Jacob LewallenShah Selbe";
    ASSERT_STREQ(buffer3, names);
}

TEST_F(StreamsSuite, Nothing) {
    auto buffer = fk::AlignedStorageBuffer<256>{};
    auto ptr = buffer.toBufferPtr();

    fk_module_WireMessageQuery message = fk_module_WireMessageQuery_init_default;
    message.type = fk_module_QueryType_QUERY_CAPABILITIES;
    message.queryCapabilities.version = FK_MODULE_PROTOCOL_VERSION;

    auto queryWriter = fk::ProtoBufMessageWriter{ ptr };
    queryWriter.write(fk_module_WireMessageQuery_fields, &message);

    auto outgoing = queryWriter.toBufferPtr();

    auto buffer1 = fk::AlignedStorageBuffer<256>{};
    auto buffer2 = fk::AlignedStorageBuffer<256>{};
    auto writer1 = fk::DirectWriter{ buffer1.toBufferPtr() };
    auto writer2 = fk::DirectWriter{ buffer2.toBufferPtr() };

    auto reader1 = fk::DirectReader{ writer1.toBufferPtr() };
    auto reader2 = fk::DirectReader{ writer2.toBufferPtr() };
    auto readers = fk::ConcatenatedReader{ &reader1, &reader2 };

    while (true) {
        auto r = readers.read();
        if (r < 0) {
            break;
        }
    }
}
