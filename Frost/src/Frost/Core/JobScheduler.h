#pragma once

#include "Frost/Core/Core.h"
#include "Frost/Utils/NoCopy.h"

#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <type_traits>
#include <vector>

namespace Frost
{
    class FROST_API JobScheduler : public NoCopy
    {
    public:
        using Job = std::function<void()>;

        JobScheduler();
        ~JobScheduler();

        bool AddJob(Job job);

        template<typename F, typename... Args>
        auto Submit(F&& f, Args&&... args) -> std::future<std::invoke_result_t<F, Args...>>
        {
            using ReturnType = std::invoke_result_t<F, Args...>;

            auto taskPtr = std::make_shared<std::packaged_task<ReturnType()>>(
                [func = std::forward<F>(f), ... capturedArgs = std::forward<Args>(args)]() mutable
                { return func(capturedArgs...); });

            std::future<ReturnType> future = taskPtr->get_future();
            if (!AddJob([taskPtr]() { (*taskPtr)(); }))
            {
                (*taskPtr)();
            }

            return future;
        }

        bool IsCurrentThreadWorker() const;

    private:
        const unsigned int max_threads_{ std::thread::hardware_concurrency() > 1
                                             ? std::thread::hardware_concurrency() - 1
                                             : 1u };

        std::vector<std::thread> workers_;
        std::vector<std::thread::id> workerIds_;
        std::queue<Job> jobs_;

        std::mutex queue_mutex_;
        std::condition_variable condition_;
        bool stop_{ false };

        void worker_loop();
    };
} // namespace Frost