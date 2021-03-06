#ifndef FORGE_EXECUTOR_HPP_INCLUDED
#define FORGE_EXECUTOR_HPP_INCLUDED

#include <vector>
#include <deque>
#include <functional>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <string>

namespace sweet
{

namespace process
{

class Environment;
class Process;

}

namespace forge
{

class Arguments;
class Context;
class Target;
class Filter;
class Forge;

/**
// A thread pool and queue of scan and execute calls to be executed in that
// thread pool.
*/
class Executor
{
    Forge* forge_; ///< The Forge that this Executor is part of.
    std::mutex jobs_mutex_; ///< The mutex that ensures exclusive access to this Executor.
    std::condition_variable jobs_empty_condition_; ///< The condition attribute that is used to notify threads that there are jobs ready to be processed.
    std::condition_variable jobs_ready_condition_; ///< The condition attribute that is used to notify threads that there are jobs ready to be processed.
    std::deque<std::function<void ()> > jobs_; ///< The functions to be executed in the thread pool.
    std::string forge_hooks_library_; ///< The full path to the build hooks library.
    int maximum_parallel_jobs_; ///< The maximum number of parallel jobs to allow.
    std::vector<std::thread*> threads_; ///< The thread pool of threads used to process Jobs.
    bool done_; ///< Whether or not this Executor has finished processing (indicates to the threads in the thread pool that they should return).

    public:
        Executor( Forge* forge );
        ~Executor();
        const std::string& forge_hooks_library() const;
        int maximum_parallel_jobs() const;
        void set_forge_hooks_library( const std::string& forge_hook_library );
        void set_maximum_parallel_jobs( int maximum_parallel_jobs );
        void execute( const std::string& command, const std::string& command_line, process::Environment* environment, Filter* dependencies_filter, Filter* stdout_filter, Filter* stderr_filter, Arguments* arguments, Context* context );

    private:
        static int thread_main( void* context );
        void thread_process();
        void thread_execute( const std::string& command, const std::string& command_line, process::Environment* environment, Filter* dependencies_filter, Filter* stdout_filter, Filter* stderr_filter, Arguments* arguments, Target* working_directory, Context* context );
        void start();
        void stop();
        process::Environment* inject_build_hooks_linux( process::Environment* environment, bool dependencies_filter_exists ) const;
        process::Environment* inject_build_hooks_macosx( process::Environment* environment, bool dependencies_filter_exists ) const;
        void inject_build_hooks_windows( process::Process* process, intptr_t write_dependencies_pipe ) const;
        void initialize_build_hooks_windows() const;
        bool is_64_bit_process_windows( process::Process* process ) const;
};

}

}

#endif
