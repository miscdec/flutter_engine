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

import FlutterEngineGroup from './FlutterEngineGroup';

export default class FlutterEngineGroupCache {
  static readonly instance = new FlutterEngineGroupCache();

  private cachedEngineGroups = new Map<String, FlutterEngineGroup>();

  contains(engineGroupId: string): boolean {
    return this.cachedEngineGroups.has(engineGroupId);
  }

  get(engineGroupId: string): FlutterEngineGroup {
    return this.cachedEngineGroups.get(engineGroupId);
  }

  put(engineGroupId: string, engineGroup?: FlutterEngineGroup) {
    if (engineGroup != null) {
      this.cachedEngineGroups.set(engineGroupId, engineGroup);
    } else {
      this.cachedEngineGroups.delete(engineGroupId);
    }
  }

  clear(): void {
    this.cachedEngineGroups.clear();
  }
}