#ifndef SCHED_HH_
#define SCHED_HH_

#include "arch-thread-state.hh"
#include <functional>
#include "tls.hh"
#include "elf.hh"

namespace sched {

class thread;
void schedule();

extern "C" {
    void thread_main_c(thread* t);
};

class thread {
public:
    explicit thread(std::function<void ()> func, bool main = false);
    ~thread();
    template <class Pred>
    static void wait_until(Pred pred);
    void wake();
    static thread* current();
private:
    void main();
    void switch_to();
    void prepare_wait();
    void wait();
    void stop_wait();
    void init_stack();
    void setup_tcb();
    void setup_tcb_main();
    static void on_thread_stack(thread* t);
    void switch_to_thread_stack();
private:
    std::function<void ()> _func;
    thread_state _state;
    thread_control_block* _tcb;
    bool _on_runqueue;
    bool _waiting;
    char _stack[64*1024] __attribute__((aligned(16)));
    friend void thread_main_c(thread* t);
    friend class wait_guard;
    friend void schedule();
};

thread* current();

class wait_guard {
public:
    wait_guard(thread* t) : _t(t) { t->prepare_wait(); }
    ~wait_guard() { _t->stop_wait(); }
private:
    thread* _t;
};

void init(elf::program& prog);

template <class Pred>
void thread::wait_until(Pred pred)
{
    thread* me = current();
    wait_guard waiter(me);
    while (!pred()) {
        me->wait();
    }
}

}

#endif /* SCHED_HH_ */
