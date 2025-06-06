# Sky NBS Player
&emsp;为《光·遇》国服PC端设计的自动演奏工具。该版本使用C语言开发。

**正在开发中，目前已实现基本功能。删除线下为未实现功能。**

## 项目特性
- 支持播放使用Minecraft Note Block Studio编辑的`.nbs`曲谱。
- 支持Sky Studio编辑的非加密`.txt`曲谱。
- ~~支持通过概率分布模型模拟手弹。~~

## 使用方式
1. 下载：进入[版本列表](https://github.com/HTMonkeyG/SkyNBSPlayer-C/releases)，并下载对应exe可执行文件至本地。
2. 配置：在可执行文件同级目录下创建`skynbs-config.txt`。必须创建该文件，否则软件无法打开。配置文件可以为空。
3. 运行：
   - 在运行程序前须先在游戏内打开音乐键盘界面。
   - 若直接在文件管理器中将有效的NBS或Sky Studio曲谱文件拖动至软件上，此时软件在播放完成后自动退出。
   - ~~若双击运行软件，则软件会在后台静默运行。使用配置文件中定义的快捷键进行交互。~~
   - 在播放时若游戏窗口失焦或退出音乐键盘，则软件会自动停止播放。可通过快捷键恢复播放。
4. 曲谱：
   - 对于NBS曲谱，仅harp乐器的C4~C6的15个全音会被识别并播放。
   - 对于Sky Studio曲谱，仅非加密曲谱可正常识别。

## 配置文件
&emsp;示例配置文件如下：
```
# 示例配置

# 游戏的平均帧率，由设置决定
# 该字段用于决定tick最小间隔
frame_rate: 60

# 高TPS模式
# 如果该字段非0则使用1000Hz主计时器
# 为0则使用100Hz
high_tps: 1

# 状态显示
# 通过浮窗显示当前播放状态
state_display: 1

# 播放快捷键
hotkey_play: Alt+O

# 暂停快捷键
# 留空则与播放快捷键相同
hotkey_pause:

# 停止快捷键
hotkey_stop: Alt+I

# 打开文件快捷键
hotkey_open: Ctrl+O
```

## 编译方式
&emsp;本项目使用MinGW32-x64编译，编译目标为x64架构的Windows 10桌面应用。
将仓库clone至本地，切换至仓库根目录并新建名为`dist`的文件夹，运行mingw32-make.exe即可。
