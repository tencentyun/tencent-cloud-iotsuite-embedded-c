## 交叉编译指南
### 编译 arm-linux 执行程序
#### 1. 准备编译器(可选)

如果已经准备好了编译器，则可跳过此步骤。

``` shell
# 安装编译器
sudo apt install gcc-arm-linux-gnueabihf gcc-arm-linux-gnueabi
```

#### 2. 修改交叉编译配置（a 或 b）
- a. 如果是使用 Ubuntu 系统的编译器，则只需要根据架构平台，配置对应的前缀。
```shell
# 根据对应平台指令架构提供，选择选择对应的前缀
# 例如，如果用的编译器是 arm-linux-gnueabi-gcc ，
# 则设置 TOOLCHAIN_PREFIX 为 arm-linux-gnueabi- 即可
set(TOOLCHAIN_PREFIX arm-linux-gnueabi-)
# or 
# set(TOOLCHAIN_PREFIX arm-linux-gnueabihf-)
```

- b. 如果是自行指定的编译器，则修改 CMAKE_C_COMPILER 变量的值，设置为编译器程序绝对路径即可。

```shell
# 将
# set(CMAKE_C_COMPILER ${TOOLCHAIN_PREFIX}gcc)
# 修改为编译器的绝对路径
set(CMAKE_C_COMPILER "/path/to/your/c/compiler")
```

#### 3. 编译程序文件
```shell
cd tencent-cloud-iotsuite-embedded-c
mkdir -p build
cd build

# 指定交叉编译配置文件
cmake -DCMAKE_TOOLCHAIN_FILE=../tools/cross-compile/arm-gcc-toolchain.cmake ../

make
```

#### 4.上传到目标系统
编译执行后，build/bin 目录下的二进制文件，上传到 arm-linux
硬件系统上，即可运行。

### 其他平台交叉编译
交叉编译其他平台的过程类似，同样需要：
1. 安装和替换成对应的编译器；
2. 修改交叉编译配置文件；
3. cmake 时，用参数 "-DCMAKE_TOOLCHAIN_FILE=/path/to/your/cross/compile/config"，
   指定交叉编译配置文件路径；

