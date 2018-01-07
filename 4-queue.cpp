
#include <unistd.h>

#include <iostream>
#include <sstream>
#include <mutex>
#include <queue>
#include <thread>

/*
 * QueueEntry : Tecnically, I saw so, that a string object sending is enough for test.
 *              If I would like to do someting with this suource, I can remove this
 *              alias & implement the real class.
 */

using QueueEntry = std::string;


/*
 * ThreadSafeQueue : It is an thread safe queue what was the task.
 */

class ThreadSafeQueue
{
   protected:

      static constexpr size_t const MaxQueueEntry = 1024;

      std::mutex             pushMutex,
                             popMutex;
      std::queue<QueueEntry> queue;
      QueueEntry             defaultMessage;
      QueueEntry             lastMessage;

   public:

      ThreadSafeQueue    (                        )
         : pushMutex     (                        )
         , popMutex      (                        )
         , queue         (                        )
         , defaultMessage( "No message available" )
         , lastMessage   ( ""                     )
      {}

   public:

      /*
       * Push : It works in strict mode, so it will wait to push its entry till
       *        this task is done.
       */

      void Push( QueueEntry& aEnt )
      {
         bool isSuccess = false;

         pushMutex.lock();

         while ( !isSuccess )
         {
            popMutex.lock();
            if ( queue.size() < MaxQueueEntry )
            {
               std::cout << "-> " << aEnt << std::endl;
               queue.push( aEnt );
               isSuccess = true;
            }
            else
            {
               std::cout << "Queue is full!" << std::endl;
            }
            popMutex.unlock();

            if ( !isSuccess )
            {
               // Queue is full, so it should wait a little bit to reader thread
               pushMutex.unlock();
               usleep( 100 ); // Give time for another threads to remove items from queue
               pushMutex.lock();
            }
         }

         pushMutex.unlock();
      }

      /*
       * Pop : read & remove an entry from queue.
       */

      QueueEntry& Pop()
      {
         pushMutex.lock();
         popMutex.lock();

         bool isEmpty = queue.size() <= 0;
         lastMessage  = !isEmpty ? queue.front() : defaultMessage;

         if ( !isEmpty )
         {
            std::cout << "<- " << lastMessage << std::endl;
            queue.pop();
         }

         popMutex.unlock();
         pushMutex.unlock();

         return lastMessage;
      }
};

/*
 * Thread: Generic thread starter class based on std::thread.
 */

class Thread
{
   protected:

      bool         isActive;
      std::thread* thread;
      std::string  thread_id;

   public:

     Thread()
        : isActive ( false   )
        , thread   ( nullptr )
        , thread_id( ""      )
     {}

     virtual ~Thread()
     {
        stop();
     }

   public:

      /*
       * Start of thred
       */

      virtual int start( int aForced = false )
      {
         printf( "Preparing start of thread...\n" );

         if ( thread != nullptr )
         {
            if ( !aForced )
            {
               std::cout << "Error: Could not start thread (threadId: " << thread->get_id() << "), due to it is already running!" << std::endl;
               return 1;
            }
            else
            {
               std::cout << "Warning: Stopping thread (theadId: " << thread->get_id() << "), due to forced start requested!" << std::endl;
               stop();
            }
         }

         std::cout << "Starting thread..." << std::endl;
         thread = new std::thread( startThread, this );
         std::cout << "Thread is started." << std::endl;
      }

      /*
       * Helper function to call the run function of this class.
       */

      static void startThread( Thread* thread )
      {
         std::stringstream id;
         thread->isActive = true;
         id << std::this_thread::get_id();
         id >> thread->thread_id;

         std::cout << "Start [" << thread->thread_id << "]" << std::endl;
         thread->run();
         std::cout << "Stop  [" << thread->thread_id << "]" << std::endl;
      }

      /*
       * Stop of thred
       */

      virtual void stop()
      {
         if ( thread == nullptr )
            return ;

         if ( !isActive )
            return ;

         std::cout << "Stopping thread..." << std::endl;

         isActive = false;
         thread->join();

         delete thread;
         thread = nullptr;

         std::cout << "Thread is stopped." << std::endl;
      }

   public:

      /*
       * Pure virtual function to implement the workflow of thread.
       */

      virtual void run() = 0;
};

/*
 * ThreadWithQueue : The both thread type will have the same queue, so I would like
 *                   to avoid the duplicated source whit this class.
 */

class ThreadWithQueue : public Thread
{
   using Inherited = Thread;

   protected:

      ThreadSafeQueue& queue;

   public:

      ThreadWithQueue( ThreadSafeQueue& aQueue )
         : queue( aQueue )
      {}

      virtual ~ThreadWithQueue() {};
};

/*
 * ProducerThread : This thread will push data into the queue.
 */

class ProducerThread : public ThreadWithQueue
{
   using Inherited = ThreadWithQueue;

   private:

      QueueEntry lastEntry;

   public:

      ProducerThread( ThreadSafeQueue& aQueue )
         : Inherited( aQueue )
         , lastEntry( ""     )
      {}

   public:

      void run()
      {
         // Source part from PDF
         while( isActive ) // I would like to stop threads so I added activate checking instead of simple 'true'
         {
            QueueEntry ent = CreateANewEntry();
            queue.Push( ent );
            usleep( 100 ); // Give time for another threads before push again
         }
      }

   protected:

      QueueEntry& CreateANewEntry()
      {
         lastEntry = "Message from " + thread_id;
         return lastEntry;
      }
};

/*
 * ConsumerThread : This thread will read & remove data from the queue.
 */

class ConsumerThread : public ThreadWithQueue
{
   using Inherited = ThreadWithQueue;

   public:

      ConsumerThread( ThreadSafeQueue& aQueue )
         : Inherited( aQueue )
      {}

   public:

      void run()
      {
         // Source part from PDF
         while ( isActive ) // I would like to stop threads so I added activate checking instead of simple 'true'
         {
            QueueEntry ent = queue.Pop();
            ProcessQueueEntry( ent );
         }
      }

   protected:

      void ProcessQueueEntry( QueueEntry& aEnt )
      {
         std::cout << "Message received: " << aEnt << std::endl;
      }
};

/*
 * Main
 */

int main( void )
{
   std::cout << "Program started" << std::endl;

   ThreadSafeQueue queue;
   ConsumerThread  reader ( queue );
   ProducerThread  writer1( queue ),
                   writer2( queue ),
                   writer3( queue ),
                   writer4( queue );

   reader.start();
   writer1.start();
   writer2.start();
   writer3.start();
   writer4.start();

   usleep(100000);

   writer1.stop();
   writer2.stop();
   writer3.stop();
   writer4.stop();
   reader.stop();

   std::cout << "Program ended" << std::endl;

   return 0;
}
