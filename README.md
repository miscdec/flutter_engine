Flutter Engine
==============

原始仓来源：https://github.com/flutter/engine

## 仓库说明：
本仓库是基于flutter官方engine仓库拓展，可构建支持在OpenHarmony设备上运行的flutter engine程序。

## 构建说明：

* 构建环境：
1. 目前支持在Linux与Mac中构建，Window环境中支持构建gen_snapshot;
2. 请确保当前构建环境可以访问DEPS文件中配置的字段allowed_hosts列表。

* 构建步骤：
1. 构建基础环境：可参照[官网](https://github.com/flutter/flutter/wiki/Setting-up-the-Engine-development-environment)；

   需要安装的基础库：

   ```
   sudo apt install python3
   sudo apt install pkg-config
   sudo apt install ninja-build
   ```

   配置node：下载 `node` 并解压，且配置到环境变量中：

   ```
   # nodejs
   export NODE_HOME=/home/<user>/env/node-v14.19.1-linux-x64
   export PATH=$NODE_HOME/bin:$PATH
   ```

   Windows构建环境：
   可参考[官网](https://github.com/flutter/flutter/wiki/Compiling-the-engine#compiling-for-windows) 
   "Compiling for Windows" 章节搭建Windows构建环境


2. 配置文件：创建空文件夹engine，engine内新建.gclient文件，编辑文件：

   ```
   solutions = [
     {
       "managed": False,
       "name": "src/flutter",
       "url": "git@gitee.com:openharmony-sig/flutter_engine.git",
       "custom_deps": {},
       "deps_file": "DEPS",
       "safesync_url": "",
     },
   ]
   ```

3. 同步代码：在engine目录，执行`gclient sync`；这里会同步engine源码、官方packages仓，还有执行ohos_setup任务；

4. 下载sdk： 从[鸿蒙套件列表](https://developer.harmonyos.com/deveco-developer-suite/enabling/kit?currentPage=1&pageSize=100)下载配套开发工具，暂不支持非该渠道下载的套件

```sh
# 需要设置的环境变量: HarmonyOS SDK, ohpm, hvigor, node
export TOOL_HOME=/Applications/DevEco-Studio-5.0.3.300.app/Contents # mac环境
export DEVECO_SDK_HOME=$TOOL_HOME/sdk # command-line-tools/sdk
export PATH=$TOOL_HOME/tools/ohpm/bin:$PATH # command-line-tools/ohpm/bin
export PATH=$TOOL_HOME/tools/hvigor/bin:$PATH # command-line-tools/hvigor/bin
export PATH=$TOOL_HOME/tools/node/bin:$PATH # command-line-tools/tool/node/bin
```

5. 开始构建：在engine目录，执行`./ohos`，即可开始构建支持ohos设备的flutter engine。
   
6. 更新代码：在engine目录，执行`./ohos -b master`

## Engine构建产物

  [构建产物](https://docs.qq.com/sheet/DUnljRVBYUWZKZEtF?tab=BB08J2)

## FAQ
1. 运行项目工程报Member notfound:'isOhos'的错误：请确保src/third_party/dart目录下应用了所有的dart patch（补丁位于src/flutter/attachment/repos目录，可使用git apply应用patch）应用patch后重新编译engine

2. 提示Permission denied: 执行chmod +x <脚本文件> 添加执行权限

3. 单独编译debug/release/profile模式的engine：`./ohos -t debug|release|profile`

4. 查看帮助：`./ohos -h`

5. 由于windows和mac、linux对换行符处理方式不同，在应用dart补丁时会造成dart vm snapshot hash结果不同，可通过以下方法获取当前snapshot hash值

   ```shell
   python xxx/src/third_party/dart/tools/make_version.py --format='{{SNAPSHOT_HASH}}'
   ```

   其中xxx为创建的engine路径

   如果获取到的值不是“8af474944053df1f0a3be6e6165fa7cf”那么就需要检查xxx/src/third_party/dart/runtime/vm/dart.cc文件和xxx/src/third_party/dart/runtime/vm/image_snapshot.cc文件中全部行的结尾是不是以LF结尾的，windows可以使用notepad++查看，其它系统具体方法请自行查询


## embedding层代码构建指导

1. 编辑shell/platform/ohos/flutter_embedding/local.properties：

    ```
    sdk.dir=<OpenHarmony的sdk目录>
    nodejs.dir=<nodejs的sdk目录>
    ```

2. 你需要从编译后的 `engine` 目录中，复制文件到 `shell/platform/ohos/flutter_embedding/flutter/libs/arm64-v8a/`
   1. debug/release，复制 `libflutter.so`
   2. profile，复制 `libflutter.so` 和 `libvmservice_snapshot.so`

3. 在shell/platform/ohos/flutter_embedding目录下，执行 

    ```
    # buildMode可选值为: debug release profile
    hvigorw --mode module -p module=flutter@default -p product=default -p buildMode=debug assembleHar --no-daemon
    ```

4. har文件输出路径为：`shell/platform/ohos/flutter_embedding/flutter/build/default/outputs/default/flutter.har`

5. 获得 har 文件后，按 `flutter.har.BUILD_TYPE.API` 格式重命名文件，其中`BUILD_TYPE`指`debug`、`release`、`profile`，`API`指当前 SDK 版本，如 api11 就是 11；比如当前构建的是 api11 的 debug 版本，则重命名为 `flutter.har.debug.11`；

6. 替换 `flutter_flutter/packages/flutter_tools/templates/app_shared/ohos.tmpl/har/har_product.tmpl/` 目录下对应文件，重新运行项目工程即可生效。

ps:如果你使用的是DevEco Studio的Beta版本，编译工程时遇到“must have required property 'compatibleSdkVersion', location: build-profile.json5:17:11"错误，请参考《DevEco Studio环境配置指导.docx》中的‘6 创建工程和运行Hello World’【配置插件】章节修改 shell/platform/ohos/flutter_embedding/hvigor/hvigor-config.json5文件。
