#include "test_streams.h"
#include "pool.h"

#include <fk-data-protocol.h>
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

    EXPECT_EQ(writer.write(name1), strlen(name1));
    EXPECT_EQ(writer.write(name2), strlen(name2));

    {
        auto reader = fk::DirectReader{ writer.toBufferPtr() };

        char buffer1[256] = { 0 };
        EXPECT_EQ(strlen(name1), reader.read((uint8_t *)buffer1, (size_t)strlen(name1)));
        ASSERT_STREQ(name1, buffer1);

        char buffer2[256] = { 0 };
        EXPECT_EQ(strlen(name2), reader.read((uint8_t *)buffer2, (size_t)strlen(name2)));
        ASSERT_STREQ(name2, buffer2);

        ASSERT_EQ(-1, reader.read());
    }

    {
        auto reader = fk::DirectReader{ writer.toBufferPtr() };

        char buffer3[256] = { 0 };
        EXPECT_EQ(strlen(name1) + strlen(name2), reader.read((uint8_t *)buffer3, sizeof(buffer3)));

        auto names = "Jacob LewallenShah Selbe";
        ASSERT_STREQ(buffer3, names);
    }
}

/*
TEST_F(StreamsSuite, ConcatenatedReader) {
    auto buffer = fk::AlignedStorageBuffer<256>{};
    auto ptr = buffer.toBufferPtr();

    fk_module_WireMessageQuery message = fk_module_WireMessageQuery_init_default;
    message.type = fk_module_QueryType_QUERY_CAPABILITIES;
    message.queryCapabilities.version = FK_MODULE_PROTOCOL_VERSION;

    auto writer = fk::DirectWriter{ ptr };
    auto queryWriter = fk::ProtoBufMessageWriter{ writer };
    queryWriter.write(fk_module_WireMessageQuery_fields, &message);

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
*/

TEST_F(StreamsSuite, CircularStreamsCloseMidRead) {
    auto circularStreams = fk::CircularStreams<fk::RingBufferN<256>>{ };

    auto& reader = circularStreams.getReader();
    auto& writer = circularStreams.getWriter();

    auto name1 = "Jacob";
    auto name2 = "Shah";

    EXPECT_EQ(writer.write(name1), strlen(name1));
    EXPECT_EQ(writer.write(name2), strlen(name2));

    {
        char scratch[256] = { 0 };
        EXPECT_EQ(strlen(name1), reader.read((uint8_t *)scratch, strlen(name1)));
        scratch[strlen(name1)] = 0;
        ASSERT_STREQ(name1, scratch);

        writer.close();

        EXPECT_EQ(strlen(name2), reader.read((uint8_t *)scratch, strlen(name2)));
        scratch[strlen(name2)] = 0;
        ASSERT_STREQ(name2, scratch);

        ASSERT_EQ(-1, reader.read((uint8_t *)scratch, sizeof(scratch)));
    }
}

TEST_F(StreamsSuite, CircularStreamsBasics) {
    auto circularStreams = fk::CircularStreams<fk::RingBufferN<256>>{ };

    auto& reader = circularStreams.getReader();
    auto& writer = circularStreams.getWriter();

    auto name1 = "Jacob";
    auto name2 = "Shah";

    EXPECT_EQ(writer.write(name1), strlen(name1));

    {
        char scratch[256] = { 0 };
        EXPECT_EQ(strlen(name1), reader.read((uint8_t *)scratch, sizeof(scratch)));
        scratch[strlen(name1)] = 0;
        ASSERT_STREQ(name1, scratch);

        ASSERT_EQ(0, reader.read((uint8_t *)scratch, sizeof(scratch)));
    }

    EXPECT_EQ(writer.write(name2), strlen(name2));

    {
        char scratch[256] = { 0 };
        EXPECT_EQ(strlen(name2), reader.read((uint8_t *)scratch, sizeof(scratch)));
        scratch[strlen(name2)] = 0;
        ASSERT_STREQ(name2, scratch);

        EXPECT_EQ(0, reader.read((uint8_t *)scratch, sizeof(scratch)));
    }

    writer.close();

    {
        char scratch[256] = { 0 };
        ASSERT_EQ(-1, reader.read((uint8_t *)scratch, sizeof(scratch)));
    }
}

TEST_F(StreamsSuite, CircularStreamsBufferPtr) {
    auto buffer = fk::AlignedStorageBuffer<8>{};
    auto ptr = buffer.toBufferPtr();

    auto circularStreams = fk::CircularStreams<fk::RingBufferPtr>{ ptr };

    auto& reader = circularStreams.getReader();
    auto& writer = circularStreams.getWriter();

    auto name1 = "Jacob";
    auto name2 = "Shah";

    EXPECT_EQ(writer.write(name1), strlen(name1));

    {
        char scratch[256] = { 0 };
        EXPECT_EQ(strlen(name1), reader.read((uint8_t *)scratch, sizeof(scratch)));
        scratch[strlen(name1)] = 0;
        ASSERT_STREQ(name1, scratch);

        ASSERT_EQ(0, reader.read((uint8_t *)scratch, sizeof(scratch)));
    }

    EXPECT_EQ(writer.write(name2), strlen(name2));

    {
        char scratch[256] = { 0 };
        EXPECT_EQ(strlen(name2), reader.read((uint8_t *)scratch, sizeof(scratch)));
        scratch[strlen(name2)] = 0;
        ASSERT_STREQ(name2, scratch);

        EXPECT_EQ(0, reader.read((uint8_t *)scratch, sizeof(scratch)));
    }

    writer.close();

    {
        char scratch[256] = { 0 };
        ASSERT_EQ(-1, reader.read((uint8_t *)scratch, sizeof(scratch)));
    }
}

TEST_F(StreamsSuite, CircularStreamsFull) {
    auto buffer = fk::AlignedStorageBuffer<4>{};
    auto ptr = buffer.toBufferPtr();

    auto circularStreams = fk::CircularStreams<fk::RingBufferPtr>{ ptr };

    auto& reader = circularStreams.getReader();
    auto& writer = circularStreams.getWriter();

    auto name1 = "Jacob";

    EXPECT_EQ(writer.write(name1), 4);
    EXPECT_EQ(writer.write(name1), 0);

    {
        char scratch[256] = { 0 };
        EXPECT_EQ(4, reader.read((uint8_t *)scratch, sizeof(scratch)));
        scratch[4] = 0;
        ASSERT_STREQ("Jaco", scratch);

        ASSERT_EQ(0, reader.read((uint8_t *)scratch, sizeof(scratch)));
    }
}

TEST_F(StreamsSuite, CircularStreamsProtoRoundTrip) {
    fk::StaticPool<64> pool("Pool");

    fk_data_DataRecord incoming = fk_data_DataRecord_init_default;
    incoming.log.facility.arg = (void *)&pool;
    incoming.log.facility.funcs.decode = fk::pb_decode_string;
    incoming.log.message.arg = (void *)&pool;
    incoming.log.message.funcs.decode = fk::pb_decode_string;

    fk_data_DataRecord outgoing = fk_data_DataRecord_init_default;
    outgoing.log.uptime = millis();
    outgoing.log.time = millis();
    outgoing.log.level = (uint32_t)LogLevels::INFO;
    outgoing.log.facility.arg = (void *)"Facility";
    outgoing.log.facility.funcs.encode = fk::pb_encode_string;
    outgoing.log.message.arg = (void *)"Message";
    outgoing.log.message.funcs.encode = fk::pb_encode_string;

    auto buffer = fk::AlignedStorageBuffer<64>{};
    auto ptr = buffer.toBufferPtr();
    auto cs = fk::CircularStreams<fk::RingBufferPtr>{ ptr };

    auto& reader = cs.getReader();
    auto& writer = cs.getWriter();

    auto protoWriter = fk::ProtoBufMessageWriter{ writer };
    EXPECT_EQ(protoWriter.write(fk_data_DataRecord_fields, &outgoing), 24);

    writer.close();

    auto protoReader = fk::ProtoBufMessageReader{ reader };

    EXPECT_EQ(protoReader.read<64>(fk_data_DataRecord_fields, &incoming), 24);

    ASSERT_EQ(outgoing.log.uptime, incoming.log.uptime);
    ASSERT_STREQ((const char *)outgoing.log.facility.arg, (const char *)incoming.log.facility.arg);

    EXPECT_EQ(protoReader.read<64>(fk_data_DataRecord_fields, &incoming), -1);

    // Notice that we're writing to the writer that we closed earlier. I think
    // that eventually this makes more sense if we throw here. Then we can
    // require the pairs of readers/writers to be opened together to allow for
    // this scenario. The problem then because that we need to have "room" for
    // more than one reader/writer. I'm thinking that we can tell the
    // CircularStreams class how many readers/writers to allow, and free them
    // when both ends are closed.

    EXPECT_EQ(protoWriter.write(fk_data_DataRecord_fields, &outgoing), 24);

    incoming = fk_data_DataRecord_init_default;
    incoming.log.facility.arg = (void *)&pool;
    incoming.log.facility.funcs.decode = fk::pb_decode_string;
    incoming.log.message.arg = (void *)&pool;
    incoming.log.message.funcs.decode = fk::pb_decode_string;

    EXPECT_EQ(protoReader.read<64>(fk_data_DataRecord_fields, &incoming), 24);

    incoming = fk_data_DataRecord_init_default;
    incoming.log.facility.arg = (void *)&pool;
    incoming.log.facility.funcs.decode = fk::pb_decode_string;
    incoming.log.message.arg = (void *)&pool;
    incoming.log.message.funcs.decode = fk::pb_decode_string;

    EXPECT_EQ(protoReader.read<64>(fk_data_DataRecord_fields, &incoming), -1);
}
