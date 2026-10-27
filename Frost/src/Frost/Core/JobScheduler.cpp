#include "Frost/Core/JobScheduler.h"

#include <algorithm>

namespace Frost
{
    JobScheduler::JobScheduler()
    {
        workers_.reserve(max_threads_);
        workerIds_.reserve(max_threads_);
        for (unsigned int i = 0; i < max_threads_; ++i)
        {
            workers_.emplace_back([this]() { worker_loop(); });
            workerIds_.push_back(workers_.back().get_id());
        }
    }

    JobScheduler::~JobScheduler()
    {
        {
            std::scoped_lock lock(queue_mutex_);
            stop_ = true;
        }
        condition_.notify_all();

        for (auto& worker : workers_)
        {
            if (worker.joinable())
            {
                worker.join();
            }
        }
    }

    bool JobScheduler::AddJob(Job job)
    {
        {
            std::scoped_lock lock(queue_mutex_);
            if (stop_)
            {
                return false;
            }
            jobs_.push(std::move(job));
        }

        condition_.notify_one();
        return true;
    }

    bool JobScheduler::IsCurrentThreadWorker() const
    {
        const std::thread::id currentId = std::this_thread::get_id();
        return std::find(workerIds_.begin(), workerIds_.end(), currentId) != workerIds_.end();
    }

    void JobScheduler::worker_loop()
    {
        while (true)
        {
            Job job;

            {
                std::unique_lock<std::mutex> lock(queue_mutex_);
                condition_.wait(lock, [this]() { return stop_ || !jobs_.empty(); });

                if (stop_ && jobs_.empty())
                {
                    return;
                }

                job = std::move(jobs_.front());
                jobs_.pop();
            }

            if (job)
            {
                job();
            }
        }
    }
} // namespace Frost