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

import Log from '../../util/Log';
import { FlutterView } from '../../view/FlutterView';
import { EmbeddingNodeController } from './EmbeddingNodeController';
import FlutterManager from './FlutterManager';

const TAG = "FlutterPage";

/**
 * 基础page组件，承载XComponent组件
 */
@Component
export struct FlutterPage {
  @Prop viewId: string = ""
  @Prop xComponentType: XComponentType = XComponentType.SURFACE

  @Builder doNothingBuilder() {}
  @BuilderParam splashScreenView: () => void = this.doNothingBuilder;
  @State showSplashScreen: boolean = true;

  @State checkFullScreen: boolean = true;
  @State checkKeyboard: boolean = true;
  @State checkGesture: boolean = true;

  @StorageLink('nodeWidth') storageLinkWidth: number = 0;
  @StorageLink('nodeHeight') storageLinkHeight: number = 0;

  private flutterView?: FlutterView | null
  private nodeController: EmbeddingNodeController | undefined = undefined
  private lastArea?: Area;

  aboutToAppear() {
    this.flutterView = FlutterManager.getInstance().getFlutterView(this.viewId);
    this.nodeController = this.flutterView!!.getEmbeddingNodeController();
    this.flutterView?.addFirstFrameListener(this)
    this.flutterView?.setCheckFullScreen(this.checkFullScreen)
    this.flutterView?.setCheckKeyboard(this.checkKeyboard)
    this.flutterView?.setCheckGesture(this.checkGesture)
  }

  aboutToDisappear() {
    this.flutterView?.removeFirstFrameListener(this);
  }

  onFirstFrame() {
    this.showSplashScreen = false;
  }

  build() {
    Stack() {
      NodeContainer(this.nodeController)
        .width(this.storageLinkWidth)
        .height(this.storageLinkHeight)

      Text('')
        .id('emptyFocusText' + this.viewId)
        .size({ width: 0, height: 0 })
        .opacity(0)
        .focusable(true)

      XComponent({ id: this.viewId, type: this.xComponentType, libraryname: 'flutter' })
        .focusable(true)
        .focusOnTouch(true)
        .onLoad((context) => {
          this.flutterView?.onSurfaceCreated()
          Log.d(TAG, "XComponent onLoad ");
        })
        .onDestroy(() => {
          Log.d(TAG, "XComponent onDestroy ");
          this.flutterView?.onSurfaceDestroyed()
        })
        .backgroundColor(Color.Transparent)

      if (this.showSplashScreen) {
        this.splashScreenView();
      }
    }
    .onAreaChange((oldValue: Area, newValue: Area) => {
      if (!this.lastArea || oldValue.width != newValue.width
        || oldValue.height != newValue.height) {
        Log.d(TAG, "onAreaChange, old=" + JSON.stringify(oldValue));
        Log.d(TAG, "onAreaChange, new=" + JSON.stringify(newValue));
        this.lastArea = newValue;
        this.flutterView?.onAreaChange(newValue)
      }
    })
    .onKeyEvent((event: KeyEvent) => {
      Log.d(TAG, "onKeyEvent " + event.type);
      this.flutterView?.onKeyEvent(event)
    })
  }
}
