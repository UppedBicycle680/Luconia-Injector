#include "rpc_connection.h"
#include <cassert>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>
#include <limits>

static BaseConnection transport;
static std::vector<char> input;
static size_t offset;
static int reads;
BaseConnection* BaseConnection::Create() { return &transport; }
void BaseConnection::Destroy(BaseConnection*& c) { c = nullptr; }
bool BaseConnection::Open() { return isOpen = true; }
bool BaseConnection::Close() { isOpen = false; return true; }
bool BaseConnection::Write(const void*, size_t) { return true; }
bool BaseConnection::Read(void* out, size_t length) {
    ++reads;
    if (length > input.size() - offset) return false;
    memcpy(out, input.data() + offset, length);
    offset += length;
    return true;
}
int GetProcessId() { return 1; }

static RpcConnection frame(uint32_t length, const std::string& body,
                           RpcConnection::Opcode opcode = RpcConnection::Opcode::Frame) {
    RpcConnection::MessageFrameHeader header{opcode, length};
    input.resize(sizeof(header) + body.size());
    memcpy(input.data(), &header, sizeof(header));
    memcpy(input.data() + sizeof(header), body.data(), body.size());
    offset = 0;
    reads = 0;
    transport.isOpen = true;
    RpcConnection rpc;
    rpc.connection = &transport;
    rpc.state = RpcConnection::State::SentHandshake;
    return rpc;
}

static void rejected(const std::string& body) {
    auto rpc = frame(static_cast<uint32_t>(body.size()), body);
    JsonDocument doc;
    assert(!rpc.Read(doc));
    assert(!transport.isOpen);
}

static void overwriteStack() {
    volatile char scratch[128 * 1024];
    for (size_t i = 0; i < sizeof(scratch); ++i) scratch[i] = 'x';
}

int main() {
    constexpr uint32_t capacity = sizeof(RpcConnection::MessageFrame::message);
    for (uint32_t length : {capacity, capacity + 1, std::numeric_limits<uint32_t>::max()}) {
        auto rpc = frame(length, "");
        JsonDocument doc;
        assert(!rpc.Read(doc));
        assert(!transport.isOpen);
        assert(reads == 1); // Must reject the header before requesting payload bytes.
    }
    for (const std::string& body : {std::string(""), std::string("{"),
         std::string("null"), std::string("[]"), std::string("42"),
         std::string("{\"x\":\"\xff\"}")}) rejected(body);
    rejected(std::string("{}\0garbage", 10));
    {
        auto rpc = frame(12, "{}");
        JsonDocument doc;
        assert(!rpc.Read(doc));
        assert(!transport.isOpen);
    }
    std::string good = "{\"evt\":\"READY\",\"data\":{\"text\":\"retained\"}}";
    {
        auto rpc = frame(static_cast<uint32_t>(good.size()), good);
        JsonDocument doc;
        assert(rpc.Read(doc));
        overwriteStack();
        assert(std::string(GetStrMember(&doc, "evt")) == "READY");
        assert(std::string(GetStrMember(GetObjMember(&doc, "data"), "text")) == "retained");
        assert(transport.isOpen);
    }
    std::string wide = "{";
    for (int i = 0; i < 600; ++i) {
        if (i) wide += ',';
        wide += "\"key" + std::to_string(i) + "\":\"value\"";
    }
    wide += '}';
    {
        auto rpc = frame(static_cast<uint32_t>(wide.size()), wide);
        JsonDocument doc;
        assert(rpc.Read(doc));
        assert(doc.MemberCount() == 600);
    }
    {
        std::string deep = "{\"nested\":" + std::string(2000, '[') + "0" + std::string(2000, ']') + "}";
        auto rpc = frame(static_cast<uint32_t>(deep.size()), deep);
        JsonDocument doc;
        assert(rpc.Read(doc));
    }
    {
        std::string edge = "{\"x\":\"" + std::string(capacity - 9, 'a') + "\"}";
        assert(edge.size() == capacity - 1);
        auto rpc = frame(static_cast<uint32_t>(edge.size()), edge);
        JsonDocument doc;
        assert(rpc.Read(doc));
    }
    std::cout << "IPC security regression tests passed\n";
}
