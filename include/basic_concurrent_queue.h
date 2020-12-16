/*****************************************************************************
  File: basic_concurrent_queue.h

  Version: 1.0
  Author: Carlos Faria <carlosfaria89@gmail.com>
  Maintainer: Carlos Faria <carlosfaria89@gmail.com>

  Copyright (C) 2017 Carlos André de Oliveira Faria. All rights reserved.

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *****************************************************************************/

#ifndef V_REPEXT_BASIC_CONCURRENT_QUEUE
#define V_REPEXT_BASIC_CONCURRENT_QUEUE

#include <deque>
#include <thread>
#include <mutex>

template <typename T>
class BasicConcurrentQueue {
public:
    BasicConcurrentQueue();

    void push_front(const T t);
    void push_back(const T t);

    T back();
    T only_back();
    void pop_back();
    void clear();

    bool empty();
    int size();

private:
    std::deque<T> m_deque;
    std::mutex m_mutex;

};

template<typename T>
BasicConcurrentQueue<T>::BasicConcurrentQueue()
{}

template<typename T>
void BasicConcurrentQueue<T>::push_front(const T t) {
    std::lock_guard<std::mutex> lk(m_mutex);
    m_deque.push_front(t);
}

template<typename T>
void BasicConcurrentQueue<T>::push_back(const T t) {
    std::lock_guard<std::mutex> lk(m_mutex);
    m_deque.push_back(t);
}

template<typename T>
T BasicConcurrentQueue<T>::back() {
    std::lock_guard<std::mutex> lk(m_mutex);
    return m_deque.back();
}

template<typename T>
T BasicConcurrentQueue<T>::only_back() {
    std::lock_guard<std::mutex> lk(m_mutex);
    T val = m_deque.back();
    m_deque.clear();
    return val;
}

template<typename T>
void BasicConcurrentQueue<T>::pop_back() {
    std::lock_guard<std::mutex> lk(m_mutex);
    m_deque.pop_back();
}

template<typename T>
void BasicConcurrentQueue<T>::clear() {
    std::lock_guard<std::mutex> lk(m_mutex);
    m_deque.clear();
}

template<typename T>
bool BasicConcurrentQueue<T>::empty() {
    std::lock_guard<std::mutex> lk(m_mutex);
    return m_deque.empty();
}

template<typename T>
int BasicConcurrentQueue<T>::size() {
    std::lock_guard<std::mutex> lk(m_mutex);
    return m_deque.size();
}

#endif //V_REPEXT_BASIC_CONCURRENT_QUEUE
