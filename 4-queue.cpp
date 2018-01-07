
#include <unistd.h>

#include <iostream>
#include <sstream>
#include <mutex>
#include <queue>
#include <thread>

#define DEBUG( str ) std::cout << "DEBUG: " << std::string( str ) << std::endl


class QueueEntry
{
   private:

      std::string message;

   public:

      QueueEntry( std::string aMessage )
         : message( aMessage )
      {}

   public:

      inline void setMessage( std::string aMessage ) { message = aMessage; }
      inline std::string& getMessage() { return message; }
};

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

      void Push( QueueEntry& aEnt )
      {
         bool isSuccess = false;

         pushMutex.lock();

         while ( !isSuccess )
         {
            popMutex.lock();
            if ( queue.size() < MaxQueueEntry )
            {
               DEBUG( "Pushed " + aEnt.getMessage() );
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
               // Keep time for Pop method call
               pushMutex.unlock();
               usleep( 100 );
               pushMutex.lock();
            }
         }

         pushMutex.unlock();
         usleep( 100 );
      }

      QueueEntry& Pop()
      {
         pushMutex.lock();
         popMutex.lock();

         bool isEmpty = queue.size() <= 0;
         lastMessage  = !isEmpty ? queue.front() : defaultMessage;

         if ( !isEmpty )
            queue.pop();

         popMutex.unlock();
         pushMutex.unlock();

         return lastMessage;
      }
};

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

         std::cout  <<"Starting thread..." << std::endl;
         thread = new std::thread( startThread, this );
         std::cout << "Thread is started." << std::endl;
      }

      static void startThread( Thread* thread )
      {
         std::stringstream id;
         thread->isActive = true;
         id << std::this_thread::get_id();
         id >> thread->thread_id;
         thread->run();
      }

      virtual void stop()
      {
         if ( thread == nullptr )
            return ;

         if ( !isActive )
            return ;

         isActive = false;
         thread->join();

         delete thread;
         thread = nullptr;
      }

   public:

      virtual void run() = 0;
};

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
         while( isActive )
         {
            QueueEntry ent = CreateANewEntry();
            queue.Push( ent );
         }
      }

   protected:

      QueueEntry& CreateANewEntry()
      {
         lastEntry.setMessage( "Message from " + thread_id );
         return lastEntry;
      }
};

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
         while ( isActive )
         {
            QueueEntry ent = queue.Pop();
            ProcessQueueEntry( ent );
         }
      }

   protected:

      void ProcessQueueEntry( QueueEntry& aEnt )
      {
         std::cout << aEnt.getMessage() << std::endl;
      }
};

int main( void )
{
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

   usleep(10000);

   writer1.stop();
   writer2.stop();
   writer3.stop();
   writer4.stop();
   reader.stop();

   return 0;
}
