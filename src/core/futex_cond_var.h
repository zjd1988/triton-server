// Copyright 2021, NVIDIA CORPORATION & AFFILIATES. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//  * Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//  * Neither the name of NVIDIA CORPORATION nor the names of its
//    contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#pragma once

#include <atomic>
#include <memory>
#include <linux/futex.h> 
#include <sys/syscall.h>
 #include <unistd.h>

class FutexMutex {
  public:
  void lock()
  {
    if (state.exchange(locked, std::memory_order_acquire) == unlocked)
      return;
    while (state.exchange(sleeper, std::memory_order_acquire) != unlocked) {
      syscall(
          SYS_futex, &state, FUTEX_WAIT_PRIVATE, sleeper, nullptr, nullptr, 0);
    }
  }
  void unlock()
  {
    if (state.exchange(unlocked, std::memory_order_release) == sleeper)
      syscall(SYS_futex, &state, FUTEX_WAKE_PRIVATE, 1, nullptr, nullptr, 0);
  }

 private:
  std::atomic<unsigned> state{unlocked};

  static constexpr unsigned unlocked = 0;
  static constexpr unsigned locked = 1;
  static constexpr unsigned sleeper = 2;
};

class FutexConditionVariable {
 public:
  void wait(std::unique_lock<FutexMutex>& lock)
  {
    unsigned old_state = state.load(std::memory_order_relaxed);
    lock.unlock();
    syscall(
        SYS_futex, &state, FUTEX_WAIT_PRIVATE, old_state, nullptr, nullptr, 0);
    lock.lock();
  }
  template <typename Predicate>
  void wait(std::unique_lock<FutexMutex>& lock, Predicate&& predicate)
  {
    while (!predicate()) {
      wait(lock);
    }
  }
  void notify_one()
  {
    state.fetch_add(1, std::memory_order_relaxed);
    syscall(SYS_futex, &state, FUTEX_WAKE_PRIVATE, 1, nullptr, nullptr, 0);
  }
  void notify_all()
  {
    state.fetch_add(1, std::memory_order_relaxed);
    syscall(
        SYS_futex, &state, FUTEX_WAKE_PRIVATE, std::numeric_limits<int>::max(),
        nullptr, nullptr, 0);
  }

 private:
  std::atomic<unsigned> state{0};
};