/**
 * @file SyncBarrier.h
 * @author yyx (you@domain.com)
 * @brief 用于多线程之间的同步
 * 适用于主线程以及分支线程的模式 主线程不得使用dispatch；
 * @version 0.1
 * @date 2023-09-18
 *
 * @copyright Copyright (c) 2023
 *
 */
#pragma once

#include <assert.h>
#include <atomic>
#include <unistd.h>
#include <workflow/WFFacilities.h>

class Dispath;
class SyncBarrier;

// class SyncBarrierclass SyncBarrier

Dispath dispath_f(SyncBarrier *syb);

class SyncBarrier
{
    friend class Dispath;

public:
    std::atomic_uint64_t AtomicInt;
    std::atomic_uint64_t INC_nb;
    std::atomic_uint64_t DEC_nb;
    WFFacilities::WaitGroup wag;
    bool waitflag;

    void Increase()
    {
        AtomicInt.fetch_add(1);
        INC_nb.fetch_add(1);
    }
    void Decrease()
    {
        AtomicInt.fetch_sub(1);
        DEC_nb.fetch_add(1);
    }
    void done()
    {
        wag.done();
        waitflag = 0;
    }
    void try_notify()
    {
        fprintf(stderr , "try notify \n");
        if (INC_nb == DEC_nb and AtomicInt == 0 and waitflag == 1)
        {

            fprintf(stderr , "notifyed \n");
            done();
            return ;
        }
        fprintf(stderr , "Exit   %zu ,%zu\n" ,INC_nb.load(),DEC_nb.load() );
    }

public:
    // friend class Dispath;
    void wait()
    {
        if (INC_nb == DEC_nb && AtomicInt == 0 && INC_nb != 0)return;
        waitflag = 1;
        wag.wait();
        
    }
    SyncBarrier() : wag(1)
    {
        AtomicInt.store(0);
        INC_nb.store(0);
        DEC_nb.store(0);
        waitflag = 0;
    }
    Dispath dispath();

    ~SyncBarrier()
    {
        if (AtomicInt != 0)
        {
            exit(-1);
        }
    }
};

class Dispath
{
private:
    mutable SyncBarrier *_syncb;
    Dispath(SyncBarrier *sync) : _syncb(sync)
    {
        assert(_syncb != nullptr);
        _syncb->Increase();
    }

public:
    friend class SyncBarrier;
    Dispath() : _syncb(nullptr) {}
    Dispath(Dispath &&arg) : _syncb(arg._syncb)
    {
        arg._syncb = nullptr;
    }
    Dispath(Dispath &arg) : _syncb(arg._syncb)
    {
        arg._syncb = nullptr;
    }

    Dispath(const Dispath &arg):_syncb(arg._syncb)
    {
        arg._syncb = nullptr;
    }

    // Dispath(Dispath &disp):_syncb(disp._syncb)
    // {
    //     disp._syncb=nullptr;

    // }
    ~Dispath()
    {

        if (_syncb == nullptr)
        {
            return;
        }
        fprintf( stderr , "destry a node %zu\n",(size_t)_syncb->AtomicInt.load());
        _syncb->Decrease();
        _syncb->try_notify();
    }

    bool IsSecurt() const
    {
        return (_syncb==nullptr)?true:false;
    }
    void Destroy()
    {
        this->_syncb=nullptr;
    }

    Dispath &operator=(Dispath src) = delete;
    Dispath &operator()(Dispath src) = delete;
    Dispath &operator()(SyncBarrier *sync) = delete;

    Dispath &operator=(Dispath &arg)
    {
        _syncb = arg._syncb;
        // _syncb=arg._syncb;
        arg._syncb = nullptr;
        return *this;
    }
    Dispath &operator=(Dispath &&arg)
    {
        _syncb = arg._syncb;
        arg._syncb = nullptr;
        return *this;
    }
};
