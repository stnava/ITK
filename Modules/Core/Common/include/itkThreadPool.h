/*=========================================================================
 *
 *  Copyright Insight Software Consortium
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/
#ifndef itkThreadPool_h
#define itkThreadPool_h

#include "itkConfigure.h"
#include "itkIntTypes.h"

#include <deque>
#include <functional>
#include <future>
#include <condition_variable>
#include <thread>

#include "itkObject.h"
#include "itkObjectFactory.h"

namespace itk
{

/**
 * \class ThreadPool
 * \brief Thread pool maintains a constant number of threads.
 *
 * Thread pool is called and initialized from within the PoolMultiThreader.
 * Initially the thread pool is started with GlobalDefaultNumberOfThreads.
 * The jobs are submitted via AddWork method.
 *
 * This implementation heavily borrows from:
 * https://github.com/progschj/ThreadPool
 *
 * \ingroup OSSystemObjects
 * \ingroup ITKCommon
 */

struct ThreadPoolGlobals;

class ITKCommon_EXPORT ThreadPool : public Object
{
public:
  ITK_DISALLOW_COPY_AND_ASSIGN(ThreadPool);

  /** Standard class type aliases. */
  using Self = ThreadPool;
  using Superclass = Object;
  using Pointer = SmartPointer< Self >;
  using ConstPointer = SmartPointer<const Self>;

  /** Run-time type information (and related methods). */
  itkTypeMacro(ThreadPool, Object);

  /** Returns the global instance */
  static Pointer New();

  /** Returns the global singleton instance of the ThreadPool */
  static Pointer GetInstance();

  /** Add this job to the thread pool queue.
   *
   * This method returns an std::future, and calling get()
   * will block until the result is ready. Example usage:
   * auto result = pool.AddWork([](int param) { return param; }, 7);
   * std::cout << result.get() << std::endl; */
  template< class Function, class... Arguments >
  auto
  AddWork( Function&& function, Arguments&&... arguments )
    -> std::future< typename std::result_of< Function( Arguments... ) >::type >
  {
    using return_type = typename std::result_of< Function( Arguments... ) >::type;

    auto task = std::make_shared< std::packaged_task< return_type() > >(
      std::bind( std::forward< Function >( function ), std::forward< Arguments >( arguments )... ) );

    std::future< return_type > res = task->get_future();
    {
      std::unique_lock< std::mutex > lock( this->GetMutex() );
      m_WorkQueue.emplace_back( [task]() { ( *task )(); } );
    }
    m_Condition.notify_one();
    return res;
  }

  /** Can call this method if we want to add extra threads to the pool. */
  void AddThreads(ThreadIdType count);

  ThreadIdType GetMaximumNumberOfThreads() const
  {
    return m_Threads.size();
  }

  /** The approximate number of idle threads. */
  int GetNumberOfCurrentlyIdleThreads() const;

  /** Platform specific number of threads. Deprecated! */
  itkLegacyMacro( static ThreadIdType GetGlobalDefaultNumberOfThreadsByPlatform() );

  /** Examines environment variables and falls back to hyper-threaded core count */
  static ThreadIdType GetGlobalDefaultNumberOfThreads();

  /** Set/Get wait for threads.
  This function should be used carefully, probably only during static
  initialization phase to disable waiting for threads when ITK is built as a
  static library and linked into a shared library (Windows only).*/
  static bool GetDoNotWaitForThreads();
  static void SetDoNotWaitForThreads(bool doNotWaitForThreads);

  /** Set/Get the pointer to ThreadPoolGlobals.
   * Note that SetThreadPoolGlobals is not concurrent thread safe. */
  static ThreadPoolGlobals * GetThreadPoolGlobals();
  static void SetThreadPoolGlobals(::itk::ThreadPoolGlobals * globals);

protected:

  /* We need access to the mutex in AddWork, and the variable is only
   * visible in .cxx file, so this method returns it. */
  std::mutex& GetMutex();

  ThreadPool();
  ~ThreadPool() override;

private:

  /** This is a list of jobs submitted to the thread pool.
   * This is the only place where the jobs are submitted.
   * Filled by AddWork, emptied by ThreadExecute. */
  std::deque< std::function< void() > > m_WorkQueue;

  /** When a thread is idle, it is waiting on m_Condition.
   * AddWork signals it to resume a (random) thread. */
  std::condition_variable m_Condition;

  /** Vector to hold all thread handles.
   * Thread handles are used to delete (join) the threads. */
  std::vector< std::thread > m_Threads;

  /* Has destruction started? */
  bool m_Stopping;

  /** To lock on the internal variables */
  static ThreadPoolGlobals * m_ThreadPoolGlobals;

  /** The continuously running thread function */
  static void ThreadExecute();
};

}
#endif
