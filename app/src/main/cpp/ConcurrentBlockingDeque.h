//
// Created by sg on 2024/1/10.
//

#ifndef MYPLAYER_CONCURRENTBLOCKINGDEQUE_H
#define MYPLAYER_CONCURRENTBLOCKINGDEQUE_H

#include <queue>
#include "pthread.h"

template<typename T>
class ConcurrentBlockingDeque {
    // 队列里元素的释放操作handler
    typedef void (*ReleaseHandler)(T &);

private:
    std::queue<T> queue;
    pthread_mutex_t mutex;
    pthread_cond_t cond;

    bool enable = false;

    ReleaseHandler release = 0;
public:
    void setReleaseHandler(ReleaseHandler _release);

public:
    ConcurrentBlockingDeque();

    virtual ~ConcurrentBlockingDeque();

    void put(T item);
    int get(T &item);
    void clear();
    bool empty();
    int size();

    void setEnable(bool enable);
};

template<typename T>
int ConcurrentBlockingDeque<T>::size() {
    int size = 0;
    pthread_mutex_lock(&mutex);
    size = queue.size();
    pthread_mutex_unlock(&mutex);
    return size;
}

template<typename T>
bool ConcurrentBlockingDeque<T>::empty() {
    bool empty = true;
    pthread_mutex_lock(&mutex);
    empty = queue.empty();
    pthread_mutex_unlock(&mutex);
    return empty;
}

template<typename T>
void ConcurrentBlockingDeque<T>::clear() {
    pthread_mutex_lock(&mutex);

    int size = queue.size();

    for (int i = 0; i < size; i++) {
        T item = queue.front();
        if (release) {
            release(item);
        }
        queue.pop();
    }

    pthread_mutex_unlock(&mutex);
}

template<typename T>
void ConcurrentBlockingDeque<T>::setReleaseHandler(ReleaseHandler _release) {
    release = _release;
}

template<typename T>
void ConcurrentBlockingDeque<T>::setEnable(bool _enable) {
    pthread_mutex_lock(&mutex);
    enable = _enable;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
}


template<typename T>
ConcurrentBlockingDeque<T>::ConcurrentBlockingDeque() {
    pthread_mutex_init(&mutex, 0);
    pthread_cond_init(&cond, 0);
}

template<typename T>
ConcurrentBlockingDeque<T>::~ConcurrentBlockingDeque() {
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);
}


template<typename T>
int ConcurrentBlockingDeque<T>::get(T &item) {
    int ret = 0;

    pthread_mutex_lock(&mutex);

    while (enable && queue.empty()) {
        pthread_cond_wait(&cond, &mutex);
    }
    if (!queue.empty()) {
        item = queue.front();
        queue.pop();
        ret = 1;
    }

    pthread_mutex_unlock(&mutex);
    return ret;
}

template<typename T>
void ConcurrentBlockingDeque<T>::put(T item) {
    pthread_mutex_lock(&mutex);
    if (enable) {
        queue.push(item);
        pthread_cond_signal(&cond);
    } else {
        if (release) {
            release(item);
        }
    }
    pthread_mutex_unlock(&mutex);
}

#endif //MYPLAYER_CONCURRENTBLOCKINGDEQUE_H
