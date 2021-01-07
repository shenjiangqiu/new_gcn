#ifndef BUFFER_H
#define BUFFER_H

#include <cassert>
#include <memory>
#include <queue>
#include <ramulator_wrapper.h>
#include <string>
#include <utility>
#include <types.h>

class Buffer_base {
public:
    [[nodiscard]] bool isCurrentEmpty() const {
        return current_empty;
    }

    [[nodiscard]] bool isCurrentReady() const {
        return current_ready;
    }

    [[nodiscard]] bool isNextEmpty() const {
        return next_empty;
    }

    [[nodiscard]] bool isNextReady() const {
        return next_ready;
    }

    explicit Buffer_base(std::string name) : name(std::move(name)) {}

    [[nodiscard]] const string &getName() const {
        return name;
    }

    //by default, do nothing
    virtual void cycle() {};

    [[nodiscard]] const shared_ptr<Req> &getCurrentReq() const {
        return current_req;
    }

    [[nodiscard]] const shared_ptr<Req> &getNextReq() const {
        return next_req;
    }

    virtual void add_current(std::shared_ptr<Req> req) {
        assert(current_empty and !current_ready);
        current_empty = true;
        current_req = std::move(req);
    }

    virtual void add_next(std::shared_ptr<Req> req) {
        assert(next_empty and !next_ready);
        next_empty = true;
        next_req = std::move(req);
    }

    virtual void finish_current_move_next() {
        current_req = std::move(next_req);
        current_empty = next_empty;
        current_ready = next_ready;

        next_req = nullptr;
        next_empty = true;
        next_ready = false;
    }

private:

    std::shared_ptr<Req> current_req;
    std::shared_ptr<Req> next_req;

    std::string name;
    bool current_empty{false};
    bool current_ready{false};
    bool next_empty{false};
    bool next_ready{false};
};

class Aggregator_buffer : Buffer_base {
public:


private:

    //
};

class Mem_buffer : public Buffer_base {
public:
    explicit Mem_buffer(string basicString);

    void cycle() override;

    void add_current(std::shared_ptr<Req> req) override {
        assert(!current_sent and !current_send_ready);
        Buffer_base::add_current(req);
    }

    void add_next(std::shared_ptr<Req> req) override {
        assert(!next_sent and !next_send_ready);
        Buffer_base::add_current(req);

    }

    void finish_current_move_next() override {
        current_send_ready = next_send_ready;
        current_sent = next_sent;


        next_send_ready = false;
        next_sent = false;
        Buffer_base::finish_current_move_next();

    }


    [[nodiscard]] bool isCurrentSent() const {
        return current_sent;
    }

    [[nodiscard]] bool isCurrentSendReady() const {
        return current_send_ready;
    }

    [[nodiscard]] bool isNextSendReady() const {
        return next_send_ready;
    }

    [[nodiscard]] bool isNextSent() const {
        return next_sent;
    }

private:

    bool current_sent{};
    bool current_send_ready{};

    bool next_send_ready{};
    bool next_sent{};


};

class WriteBuffer : Mem_buffer {
public:
    explicit WriteBuffer(const std::string &name = "Write Buffer") : Mem_buffer(name) {}

};


class ReadBuffer : Mem_buffer {
public:
    explicit ReadBuffer(const string &basicString);
};

#endif /* BUFFER_H */
