#include "transmissions.h"

namespace fk {

FileReader::FileReader(FileSystem &fileSystem, uint8_t file) : iterator(fileSystem.fkfs(), file) {
}

void FileReader::open() {
    iterator.beginning();
}

int32_t FileReader::read() {
    fk_assert(false);
    return EOS;
}

int32_t FileReader::read(uint8_t *ptr, size_t size) {
    if (iterator.isFinished()) {
        return EOS;
    }

    auto position = 0;
    auto remaining = size;
    while (remaining > 0 && !iterator.isFinished()) {
        auto data = iterator.read(remaining);
        if (data) {
            fk_assert(data.size <= size);
            memcpy(ptr + position, data.ptr, data.size);
            remaining -= data.size;
            position += data.size;
            iterator.moveNext(data);
        }
    }

    return position;
}

void FileReader::close() {
}

ModuleDataTransfer::ModuleDataTransfer(FileSystem &fileSystem, uint8_t file) : fileReader(fileSystem, file), buffer(), streamCopier{ buffer.toBufferPtr() } {
}

void ModuleDataTransfer::prepare(ModuleQueryMessage &message, Writer &outgoing) {
    query(message);

    auto protoWriter = ProtoBufMessageWriter{ outgoing };
    protoWriter.write(fk_module_WireMessageQuery_fields, &message.m());

    fileReader.open();
}

void ModuleDataTransfer::tick(Writer &outgoing) {
    if (streamCopier.copy(fileReader, outgoing) == Stream::EOS) {
        outgoing.close();
        return;
    }
}

PrepareTransmissionData::PrepareTransmissionData(TwoWireBus &bus, CoreState &state, FileSystem &fileSystem, uint8_t file, ModuleCommunications &communications) :
    Task("PrepareTransmissionData"), state(&state), moduleDataTransfer(fileSystem, file), protocol(communications) {
}

void PrepareTransmissionData::enqueued() {
    protocol.push(9, moduleDataTransfer);
}

TaskEval PrepareTransmissionData::task() {
    if (protocol.isBusy()) {
        auto finished = protocol.handle();
        if (finished) {
            log("Done");
        }
        return TaskEval::busy();
    }
    return TaskEval::done();
}

}
