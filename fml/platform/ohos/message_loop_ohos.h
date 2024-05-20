/*
 * Copyright (c) 2023 Hunan OpenValley Digital Industry Development Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef FLUTTER_FML_PLATFORM_OHOS_MESSAGE_LOOP_OHOS_H_
#define FLUTTER_FML_PLATFORM_OHOS_MESSAGE_LOOP_OHOS_H_

#include <uv.h>
#include "flutter/fml/macros.h"
#include "flutter/fml/message_loop_impl.h"
#include "flutter/fml/unique_fd.h"
#include <thread>
#include <atomic>

namespace fml {

class MessageLoopOhos : public MessageLoopImpl {
 private:
  uv_async_t async_handle_;
  uv_loop_t loop_;
	fml::UniqueFD epoll_fd_;
  fml::UniqueFD timer_fd_;
  std::atomic<bool> running_;
  std::thread timerhandleThread;

  MessageLoopOhos(void* platform_loop);

  ~MessageLoopOhos() override;

  // |fml::MessageLoopImpl|
  void Run() override;

  // |fml::MessageLoopImpl|
  void Terminate() override;

  // |fml::MessageLoopImpl|
  void WakeUp(fml::TimePoint time_point) override;

  void OnEventFired();

  void TimerFdWatcher();

  bool AddOrRemoveTimerSource(bool add);

  FML_FRIEND_MAKE_REF_COUNTED(MessageLoopOhos);
  FML_FRIEND_REF_COUNTED_THREAD_SAFE(MessageLoopOhos);
  FML_DISALLOW_COPY_AND_ASSIGN(MessageLoopOhos);

 public:
  static void OnAsyncCallback(uv_async_t* handle);
  static void OnAsyncHandleClose(uv_handle_t* handle);
};

}  // namespace fml

#endif  // FLUTTER_FML_PLATFORM_OHOS_MESSAGE_LOOP_OHOS_H_
