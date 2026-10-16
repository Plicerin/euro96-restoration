#pragma once
#include <coroutine>
#include <exception>
#include <utility>

namespace actua {
// Owns application locals across browser animation callbacks. No references to
// main's stack survive its return; destruction also handles an abandoned task.
class FrameTask {
public:
    struct promise_type {
        int result=0;
        std::exception_ptr error;
        FrameTask get_return_object(){return FrameTask{std::coroutine_handle<promise_type>::from_promise(*this)};}
        std::suspend_always initial_suspend() noexcept{return {};}
        std::suspend_always final_suspend() noexcept{return {};}
        void return_value(int code) noexcept{result=code;}
        void unhandled_exception() noexcept{error=std::current_exception();}
    };
    explicit FrameTask(std::coroutine_handle<promise_type> h):handle(h){}
    FrameTask(FrameTask&& other) noexcept:handle(std::exchange(other.handle,{})){}
    FrameTask(const FrameTask&)=delete;
    ~FrameTask(){if(handle)handle.destroy();}
    bool step(){if(!handle.done())handle.resume();return !handle.done();}
    int result()const{if(handle.promise().error)std::rethrow_exception(handle.promise().error);return handle.promise().result;}
private:
    std::coroutine_handle<promise_type> handle;
};
}
