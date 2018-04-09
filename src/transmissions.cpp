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

ClearModuleData::ClearModuleData() {
}

void ClearModuleData::query(ModuleQueryMessage &message) {
    message.m().type = fk_module_QueryType_QUERY_DATA_CLEAR;
}

void ClearModuleData::reply(ModuleReplyMessage &message) {
    maximumBytes = message.m().data.size;
}

ModuleDataTransfer::ModuleDataTransfer(FileSystem &fileSystem, uint8_t file) : fileReader(fileSystem, file), buffer(), streamCopier{ buffer.toBufferPtr() } {
}

void ModuleDataTransfer::query(ModuleQueryMessage &message) {
    fileReader.open();
    bytesCopied = 0;

    message.m().type = fk_module_QueryType_QUERY_DATA_APPEND;
    message.m().data.size = fileReader.size();
}

void ModuleDataTransfer::reply(ModuleReplyMessage &message) {
    maximumBytes = message.m().data.size;
}

void ModuleDataTransfer::prepare(ModuleQueryMessage &message, Writer &outgoing) {
    query(message);

    auto protoWriter = ProtoBufMessageWriter{ outgoing };
    protoWriter.write(fk_module_WireMessageQuery_fields, &message.m());
}

void ModuleDataTransfer::tick(Writer &outgoing) {
    auto bytes = streamCopier.copy(fileReader, outgoing);
    if (bytes == Stream::EOS) {
        outgoing.close();
    }
    else {
        bytesCopied += bytes;
        if (bytesCopied > maximumBytes) {
            outgoing.close();
        }
    }
}

PrepareTransmissionData::PrepareTransmissionData(TwoWireBus &bus, CoreState &state, FileSystem &fileSystem, uint8_t file, ModuleCommunications &communications) :
    Task("PrepareTransmissionData"), state(&state), moduleDataTransfer(fileSystem, file), protocol(communications) {
}

void PrepareTransmissionData::enqueued() {
    protocol.push(9, clearModuleData);
}

TaskEval PrepareTransmissionData::task() {
    if (protocol.isBusy()) {
        auto finished = protocol.handle();
        if (finished) {
            if (finished.error()) {
                return TaskEval::error();
            }

            if (finished.is(clearModuleData)) {
                moduleDataTransfer.setMaximumBytes(clearModuleData.getMaximumBytes());
                protocol.push(9, moduleDataTransfer);
            }
            else {
                log("Done");
            }
        }
        return TaskEval::busy();
    }
    return TaskEval::done();
}

}
