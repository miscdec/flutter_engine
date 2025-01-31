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
import image from '@ohos.multimedia.image';

export interface TextureRegistry {

  createSurfaceTexture(): SurfaceTextureEntry;
  getTextureId(): number;
  registerTexture(textureId: number): SurfaceTextureEntry;
  registerSurfaceTexture(receiver: image.ImageReceiver): SurfaceTextureEntry;
  registerPixelMap(pixelMap: PixelMap): number;
  unregisterTexture(textureId: number): void;
  onTrimMemory(level: number) : void;
}

export interface SurfaceTextureEntry {
  getTextureId(): number;

  getSurfaceId(): number;

  getImageReceiver(): image.ImageReceiver;

  release(): void;
}

export interface OnFrameConsumedListener {
  onFrameConsumed(): void;
}

export interface OnTrimMemoryListener {
  onTrimMemory(level: number) : void;
}