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


    virtual void add_next(std::shared_ptr<Req> req) {
        assert(next_empty and !next_ready);
        next_empty = false;
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

protected:
    virtual void add_current(std::shared_ptr<Req> req) {
        assert(current_empty and !current_ready);
        current_empty = false;
        current_req = std::move(req);
    }

    void setCurrentEmpty(bool currentEmpty) {
        current_empty = currentEmpty;
    }

    void setCurrentReady(bool currentReady) {
        current_ready = currentReady;
    }

    void setNextEmpty(bool nextEmpty) {
        next_empty = nextEmpty;
    }

    void setNextReady(bool nextReady) {
        next_ready = nextReady;
    }

private:

    std::shared_ptr<Req> current_req;
    std::shared_ptr<Req> next_req;

    std::string name;
    bool current_empty{true};
    bool current_ready{false};
    bool next_empty{true};
    bool next_ready{false};
};

class Aggregator_buffer : public Buffer_base {
public:
    Aggregator_buffer(const string &name);

private:

    //
};

class Mem_buffer : public Buffer_base {
public:
    explicit Mem_buffer(string basicString);

    void cycle() override;

    virtual std::shared_ptr<Req> pop_current_req() {
        current_send_ready = false;
        return getCurrentReq();
    }

    virtual std::shared_ptr<Req> pop_next_req() {
        next_send_ready = false;
        return getNextReq();
    }

    void add_current(std::shared_ptr<Req> req) override {
        assert(!current_sent and !current_send_ready);
        Buffer_base::add_current(req);
    }

    void add_next(std::shared_ptr<Req> req) override {
        assert(!next_sent and !next_send_ready);
        Buffer_base::add_next(req);

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

    void accept_req(std::shared_ptr<Req> req) {
        if (getNextReq() and getNextReq()->id == req->id) {
            assert(!isNextReady() and !isNextEmpty());
            setNextReady(true);
        } else {
            assert(getCurrentReq()->id == req->id and !isCurrentEmpty() and !isCurrentReady());
            setCurrentReady(true);
        }
    }

private:

    bool current_sent{};
    bool current_send_ready{};

    bool next_send_ready{};
    bool next_sent{};


};

class WriteBuffer : public Mem_buffer {
public:
    shared_ptr<Req> pop_current_req() override {
        setCurrentReady(true);
        return Mem_buffer::pop_current_req();
    }

    shared_ptr<Req> pop_next_req() override {
        setNextReady(true);
        return Mem_buffer::pop_next_req();
    }

    explicit WriteBuffer(const std::string &name = "Write Buffer") : Mem_buffer(name) {}

};


class ReadBuffer : public Mem_buffer {
public:
    explicit ReadBuffer(const string &basicString);
};

#endif /* BUFFER_H */
