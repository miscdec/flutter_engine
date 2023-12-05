@rem Copyright (c) 2021-2023 Huawei Device Co., Ltd.
@rem Licensed under the Apache License, Version 2.0 (the "License");
@rem you may not use this file except in compliance with the License.
@rem You may obtain a copy of the License at
@rem
@rem     http://www.apache.org/licenses/LICENSE-2.0
@rem
@rem Unless required by applicable law or agreed to in writing, software
@rem distributed under the License is distributed on an "AS IS" BASIS,
@rem WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
@rem See the License for the specific language governing permissions and
@rem limitations under the License.

@echo off
setlocal EnableDelayedExpansion

call python .\src\flutter\tools\gn --unoptimized --runtime-mode=debug --ohos --ohos-cpu=arm64
call python .\src\flutter\tools\gn --runtime-mode=release --ohos --ohos-cpu=arm64
ninja -C .\src\out\ohos_debug_unopt_arm64
ninja -C .\src\out\ohos_release_arm64

