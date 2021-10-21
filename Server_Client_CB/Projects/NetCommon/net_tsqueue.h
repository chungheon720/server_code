#pragma once

#ifndef NET_TSQUEUE_H_INCLUDED
#define NET_TSQUEUE_H_INCLUDED
#include "net_common.h"

namespace olc
{
    namespace net
    {
        template<typename T>
        class tsqueue
        {
        public:
            tsqueue() = default;
            tsqueue(const tsqueue<T>&) = delete;
            virtual ~tsqueue() { clear(); }
        public:
            //Returns and pops the item at the front of the Queue
            T pop_front()
            {
                std::scoped_lock lock(muxQueue);
                auto t = std::move(deqQueue.front());
                deqQueue.pop_front();
                return t;
            }

            //Returns and pops the item at the back of the Queue
            T pop_back()
            {
                std::scoped_lock lock(muxQueue);
                auto t = std::move(deqQueue.back());
                deqQueue.pop_back();
                return t;
            }
            //Returns and maintains the item at the front of the Queue
            const T& front()
            {
                std::scoped_lock lock(muxQueue);
                return deqQueue.front();
            }

            //Returns and maintains the item at the back of the Queue
            const T& back()
            {
                std::scoped_lock lock(muxQueue);
                return deqQueue.back();
            }

            //Add item to the front of the Queue
            void push_front(const T& item)
            {
                std::scoped_lock lock(muxQueue);
                return deqQueue.emplace_front(std::move(item));
            }

            //Add item to the back of the Queue
            void push_back(const T& item)
            {
                std::scoped_lock lock(muxQueue);
                return deqQueue.emplace_back(std::move(item));
            }

            //Clear the queue
            void clear()
            {
                std::scoped_lock lock(muxQueue);
                deqQueue.clear();
            }

            //Check if queue is empty
            bool empty()
            {
                std::scoped_lock lock(muxQueue);
                return deqQueue.empty();
            }

            //Returns the number of items in the queue
            size_t count()
            {
                std::scoped_lock lock(muxQueue);
                return deqQueue.size();
            }


        protected:
            std::mutex muxQueue;
            std::deque<T> deqQueue;
        };
    }
}

#endif // NET_TSQUEUE_H_INCLUDED
