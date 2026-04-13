# Yukino SDK
Yukino SDK 是一个高性能的移动端漫画解析方案，集成 C++ 原生引擎，旨在为 Android 开发者提供极致的文档渲染体验。

## 🚀 特性
* **双引擎支持**：集成 **libmobi** (Mobi/Azw3) 解析内核。
* **极致性能**：核心逻辑由 C++ 编写，通过 JNI 桥接，响应速度极快。
* **体积优化**：经过精简的二进制库，且支持主流架构（arm64-v8a）。
* **现代架构**：完美适配 Jetpack Compose 项目。

## 📦 安装

### 1. 添加 JitPack 仓库
在你的项目根目录 settings.gradle.kts 中配置：
```kotlin
dependencyResolutionManagement {
  repositories {
    google()
    mavenCentral()
    maven { url = uri("https://jitpack.io") }
  }
}
```

### 2. 引入依赖
在模块级 build.gradle.kts 中添加：
```kotlin
dependencies {
  implementation("com.github.xuennai:yukino-sdk:v1.0.3")
}
```

## 📜 核心模块
| 模块名 | 对应内核 | 说明 |
|---|---|---|
| yukino_mobi | libmobi | 负责 Mobi/Azw3 格式解析 |

## 🤝 贡献
欢迎参与 Yukino SDK 的开发。如果你发现了 Bug 或有更好的优化建议，请直接提交 Issue 或 PR。

## 🙏 致谢
Yukino SDK 使用了以下开源项目：
- **[libmobi](https://github.com/bfabiszewski/libmobi)**  
  提供了对 Mobi 和 Azw3 格式的解析支持，其源码集成于 `yukino-sdk/src/main/cpp/libmobi` 文件夹下。

## 📄 License

This project is licensed under the MIT License.

### Third-Party Licenses

This project includes the following third-party libraries:

- libmobi (LGPL v3)

These libraries are licensed under their respective open source licenses.  
The full license texts are included in the `licenses/` directory.

In compliance with LGPL requirements, users are allowed to replace or modify
the linked native libraries (.so) used by this project.
