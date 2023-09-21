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
import MessageChannelUtils from '../../util/MessageChannelUtils';
import { BinaryMessageHandler, BinaryMessenger, BinaryReply, TaskQueue } from './BinaryMessenger';
import MethodCall from './MethodCall';
import MethodCodec from './MethodCodec';
/**
 * A named channel for communicating with the Flutter application using asynchronous method calls.
 *
 * <p>Incoming method calls are decoded from binary on receipt, and Java results are encoded into
 * binary before being transmitted back to Flutter. The {@link MethodCodec} used must be compatible
 * with the one used by the Flutter application. This can be achieved by creating a <a
 * href="https://api.flutter.dev/flutter/services/MethodChannel-class.html">MethodChannel</a>
 * counterpart of this channel on the Dart side. The Java type of method call arguments and results
 * is {@code Object}, but only values supported by the specified {@link MethodCodec} can be used.
 *
 * <p>The logical identity of the channel is given by its name. Identically named channels will
 * interfere with each other's communication.
 */

export default class MethodChannel {
  static TAG = "MethodChannel#";
  private messenger: BinaryMessenger;
  private name: string;
  private codec: MethodCodec;
  private taskQueue: TaskQueue;

  constructor(messenger: BinaryMessenger, name: string, codec?: MethodCodec, taskQueue?: TaskQueue) {
    this.messenger = messenger
    this.name = name
    this.codec = codec
    this.taskQueue = taskQueue
  }

  /**
   * Invokes a method on this channel, optionally expecting a result.
   *
   * <p>Any uncaught exception thrown by the result callback will be caught and logged.
   *
   * @param method the name String of the method.
   * @param arguments the arguments for the invocation, possibly null.
   * @param callback a {@link Result} callback for the invocation result, or null.
   */
  invokeMethod(method: string, args: any, callback?: MethodResult): void {
    this.messenger.send(this.name, this.codec.encodeMethodCall(new MethodCall(method, args)), callback == null ? null : new IncomingResultHandler(callback, this.codec));
  }

  /**
   * Registers a method call handler on this channel.
   *
   * <p>Overrides any existing handler registration for (the name of) this channel.
   *
   * <p>If no handler has been registered, any incoming method call on this channel will be handled
   * silently by sending a null reply. This results in a <a
   * href="https://api.flutter.dev/flutter/services/MissingPluginException-class.html">MissingPluginException</a>
   * on the Dart side, unless an <a
   * href="https://api.flutter.dev/flutter/services/OptionalMethodChannel-class.html">OptionalMethodChannel</a>
   * is used.
   *
   * @param handler a {@link MethodCallHandler}, or null to deregister.
   */
  setMethodCallHandler(handler: MethodCallHandler): void {
    // We call the 2 parameter variant specifically to avoid breaking changes in
    // mock verify calls.
    // See https://github.com/flutter/flutter/issues/92582.
    if (this.taskQueue != null) {
      this.messenger.setMessageHandler(
        this.name, handler == null ? null : new IncomingMethodCallHandler(handler, this.codec), this.taskQueue);
    } else {
      this.messenger.setMessageHandler(
        this.name, handler == null ? null : new IncomingMethodCallHandler(handler, this.codec));
    }
  }

  /**
   * Adjusts the number of messages that will get buffered when sending messages to channels that
   * aren't fully set up yet. For example, the engine isn't running yet or the channel's message
   * handler isn't set up on the Dart side yet.
   */
  resizeChannelBuffer(newSize: number): void {
    MessageChannelUtils.resizeChannelBuffer(this.messenger, this.name, newSize);
  }
}

/** A handler of incoming method calls. */
export interface MethodCallHandler {
  /**
   * Handles the specified method call received from Flutter.
   *
   * <p>Handler implementations must submit a result for all incoming calls, by making a single
   * call on the given {@link Result} callback. Failure to do so will result in lingering Flutter
   * result handlers. The result may be submitted asynchronously and on any thread. Calls to
   * unknown or unimplemented methods should be handled using {@link Result#notImplemented()}.
   *
   * <p>Any uncaught exception thrown by this method will be caught by the channel implementation
   * and logged, and an error result will be sent back to Flutter.
   *
   * <p>The handler is called on the platform thread (Android main thread) by default, or
   * otherwise on the thread specified by the {@link BinaryMessenger.TaskQueue} provided to the
   * associated {@link MethodChannel} when it was created. See also <a
   * href="https://github.com/flutter/flutter/wiki/The-Engine-architecture#threading">Threading in
   * the Flutter Engine</a>.
   *
   * @param call A {@link MethodCall}.
   * @param result A {@link Result} used for submitting the result of the call.
   */
  onMethodCall(call: MethodCall, result: MethodResult): void;
}

/**
 * Method call result callback. Supports dual use: Implementations of methods to be invoked by
 * Flutter act as clients of this interface for sending results back to Flutter. Invokers of
 * Flutter methods provide implementations of this interface for handling results received from
 * Flutter.
 *
 * <p>All methods of this class can be invoked on any thread.
 */
export interface MethodResult {
  /**
   * Handles a successful result.
   *
   * @param result The result, possibly null. The result must be an Object type supported by the
   *     codec. For instance, if you are using {@link StandardMessageCodec} (default), please see
   *     its documentation on what types are supported.
   */
  success(result: any): void;

  /**
   * Handles an error result.
   *
   * @param errorCode An error code String.
   * @param errorMessage A human-readable error message String, possibly null.
   * @param errorDetails Error details, possibly null. The details must be an Object type
   *     supported by the codec. For instance, if you are using {@link StandardMessageCodec}
   *     (default), please see its documentation on what types are supported.
   */
  error(errorCode: string, errorMessage: string, errorDetails: any): void;

  /** Handles a call to an unimplemented method. */
  notImplemented(): void;
}

class IncomingResultHandler implements BinaryReply {
  private callback: MethodResult;
  private codec: MethodCodec;

  constructor(callback: MethodResult, codec: MethodCodec) {
    this.callback = callback;
    this.codec = codec
  }

  reply(reply: ArrayBuffer): void {
    try {
      if (reply == null) {
        this.callback.notImplemented();
      } else {
        try {
          this.callback.success(this.codec.decodeEnvelope(reply));
        } catch (e) {
          this.callback.error(e.code, e.getMessage(), e.details);
        }
      }
    } catch (e) {
      Log.e(MethodChannel.TAG, "Failed to handle method call result", e);
    }
  }
}

class IncomingMethodCallHandler implements BinaryMessageHandler {
  private handler: MethodCallHandler;
  private codec: MethodCodec;

  constructor(handler: MethodCallHandler, codec: MethodCodec) {
    this.handler = handler;
    this.codec = codec
  }

  onMessage(message: ArrayBuffer, reply: BinaryReply): void {
    const call = this.codec.decodeMethodCall(message);
    try {
      this.handler.onMethodCall(
        call, {
        success: (result: any): void => {
          reply.reply(this.codec.encodeSuccessEnvelope(result));
        },

        error: (errorCode: string, errorMessage: string, errorDetails: any): void => {
          reply.reply(this.codec.encodeErrorEnvelope(errorCode, errorMessage, errorDetails));
        },

        notImplemented: (): void => {
          reply.reply(null);
        }
      });
    } catch (e) {
      Log.e(MethodChannel.TAG, "Failed to handle method call", e);
      reply.reply(this.codec.encodeErrorEnvelopeWithStacktrace("error", e.getMessage(), null, e));
    }
  }
}