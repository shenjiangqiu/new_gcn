#include <buffer.h>

#include <utility>

#include "spdlog/spdlog.h"

void Mem_buffer::cycle() {
    if (!isCurrentEmpty() and !current_sent) {
        assert(!isCurrentReady());
        assert(getCurrentReq());
        //send the request
        current_sent = true;
        current_send_ready = true;

    }
    if (!isNextEmpty() and !next_sent) {
        assert(!isNextReady());
        assert(getNextReq());
        assert(!next_send_ready);
        next_sent = true;
        next_send_ready = true;

    }
}

Mem_buffer::Mem_buffer(const string &basicString) : Buffer_base(basicString) {

}

ReadBuffer::ReadBuffer(const string &basicString) : Mem_buffer(
        basicString) {}

Aggregator_buffer::Aggregator_buffer(string name) : name(std::move(name)) {}


void Aggregator_buffer::cycle() {
    if (write_ready and read_empty) {
        //ready to move;
        assert(!write_empty and !read_ready);
        write_empty = true;
        read_ready = true;

        write_ready = false;
        read_empty = false;
        read_window = std::move(write_window);

        spdlog::debug("aggbuffer move the write buffer to read buffer");


    }

}

bool Aggregator_buffer::isWriteEmpty() const {
    return write_empty;
}

bool Aggregator_buffer::isWriteReady() const {
    return write_ready;
}

bool Aggregator_buffer::isReadEmpty() const {
    return read_empty;
}

bool Aggregator_buffer::isReadReady() const {
    return read_ready;
}

const string &Aggregator_buffer::getName() const {
    return name;
}

void Aggregator_buffer::add_new_task(std::shared_ptr<Slide_window> window) {
    assert(write_empty and !write_ready and write_window == nullptr);
    write_empty = true;
    write_window = std::move(window);

}

void Aggregator_buffer::finish_write() {
    assert(!write_empty and !write_ready);
    write_ready = true;

}

void Aggregator_buffer::finish_read() {
    assert(!read_empty and !read_ready);
    read_ready = true;

}

const shared_ptr<Slide_window> &Aggregator_buffer::getReadWindow() const {
    return read_window;
}

const shared_ptr<Slide_window> &Aggregator_buffer::getWriteWindow() const {
    return write_window;
}

WriteBuffer::WriteBuffer(const string &name)
        : Mem_buffer(name) {}
