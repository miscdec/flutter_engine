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

import SettingsChannel, { PlatformBrightness } from '../engine/systemchannels/SettingsChannel'
import I18n from '@ohos.i18n'

export default class Settings {
  settingsChannel: SettingsChannel;

  constructor(settingsChannel: SettingsChannel) {
    this.settingsChannel = settingsChannel;
  }

  sendSettings(): void {
    this.settingsChannel.startMessage()
      .setAlwaysUse24HourFormat(I18n.System.is24HourClock())
      .setTextScaleFactor(1.0)
      .setNativeSpellCheckServiceDefined(false)
      .setBrieflyShowPassword(false)
      .setPlatformBrightness(PlatformBrightness.LIGHT)
      .send();
  }
}