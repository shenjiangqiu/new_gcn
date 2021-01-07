#include <buffer.h>


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

Mem_buffer::Mem_buffer(const string basicString) : Buffer_base(basicString) {

}

ReadBuffer::ReadBuffer(const string &basicString) : Mem_buffer(basicString) {}
