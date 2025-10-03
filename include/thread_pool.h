#pragma once

#include <functional>
#include <atomic>
#include <cstddef>

class ThreadPool {
public:
    explicit ThreadPool(size_t numThreads);
    ~ThreadPool();
    
    // Interface planejada para enfileiramento de tarefas
    template<class F>
    void enqueue(F&& f);
    
    // Controle do pool
    void shutdown();
    size_t getActiveThreads() const;
    size_t getQueueSize() const;

private:
    // Implementação será desenvolvida nas próximas partes
    std::atomic<bool> stop_;
    std::atomic<size_t> activeThreads_;
};
