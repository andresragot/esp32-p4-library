/**

 */

#include "Thread_Pool.hpp"
#include <iostream>

namespace Ragot
{
    Thread_Pool & thread_pool = Thread_Pool::instance ();

    void Thread_Pool::thread_function (std::stop_token stop_token)
    {
        while (not stop_token.stop_requested())
        {
            auto task = tasks.get();
            if (task.has_value())
            {
                std::cout << "Thread " << std::this_thread::get_id() << " executing task..." << std::endl;
                task.value ()(stop_token);
            }
        }
    }
}
