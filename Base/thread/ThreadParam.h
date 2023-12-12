#ifndef __BASE_THREAD_PARAMETER_H__
#define __BASE_THREAD_PARAMETER_H__

#include "../utility/SafeQueue.h"
#include <functional>
#include <future>
#include <type_traits>
#include <typeinfo>

namespace base
{
    namespace thd
    {
        template <typename T>
        struct TaskWithFuture
        {
            std::function<void()> mFunc;
            std::shared_future<T> mFuture;
        };

        template <typename T>
        using TaskWithFutureSafeQueueType = utility::SafeQueue<TaskWithFuture<T>>;

        constexpr size_t RTHREAD_MAX_LOAD_NUMBER() { return 10; }
        constexpr size_t LAST_EXPECTED_NUMBER() { return 4; }
        constexpr size_t MAX_THREADS_NUM() { return 256; }
    }
}

#endif