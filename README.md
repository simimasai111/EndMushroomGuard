# EndMushroomGuard（末地防蘑菇 · C++ 原生版）

LeviLamina C++ 原生插件：玩家一旦在**末地**尝试放置蘑菇，操作会被**直接拦截**——
蘑菇方块根本不会生成、玩家手里的蘑菇也还在，同时弹一条红色文字提醒。

> 这是之前 JS/LLSE 版（清除 + 传送 + 提醒）的 **C++ 硬拦截**版本。JS 版用的
> `afterPlaceBlock` 事件无法取消，只能事后清除；本版用 **可取消** 的
> `PlayerPlacingBlockEvent`，在放置“之前” `cancel()`，是真·拦截。

## 满足的需求

1. **检测**：监听放置方块事件，判定维度 == 末地（基岩版 `DimensionId`：0 主世界 / 1 下界 / 2 末地）。
2. **拦截 + 提醒**：若主手物品是蘑菇类（`minecraft:red_mushroom` / `brown_mushroom` /
   `mushroom_stem` / `crimson_fungus` / `warped_fungus` 等），调用 `event.cancel()` 取消放置，
   并 `player.sendMessage("§c...")` 发送提醒。
3. **兼容 LL 最新版**：全程使用 LeviLamina 长期稳定的原生 API，无黑科技、无版本绑定坑。

## 目录结构

```
levilamina-endmushroom-cpp/
├── manifest.json                 # 模组元数据（必需）
├── tooth.json                    # lip 发布包描述（供 CI 打包）
├── xmake.lua                     # xmake 构建配置（官方推荐，CI 用此）
├── CMakeLists.txt                # CMake 备选构建配置
├── .github/workflows/build.yml   # GitHub Actions 自动构建
├── src/main.cpp                  # 插件源码
└── README.md
```

## 自动构建（GitHub Actions）

把仓库推到 GitHub 后，每次 `push` / PR / 手动 `workflow_dispatch` 都会在
**Windows runner（自带 VS2022，并自动从 xmake-repo 拉取 LeviLamina SDK）** 上用 xmake 编译，
产物（含 `EndMushroomGuard.dll` 与 `manifest.json` 的发布包）作为 Artifact 上传。
到仓库 **Actions → 对应 run → Artifacts** 下载 `bin/` 即可，无需本机装任何工具链。

## 本地编译

需要 **Windows + Visual Studio 2022（含 C++ 桌面开发，v143）+ Windows SDK**，并安装 LeviLamina SDK。

### CMake 方式

```bash
# 在 LeviLamina 安装目录下拿到 SDK 的 cmake 配置目录后：
cmake -B build -DLeviLamina_DIR=<LeviLamina SDK cmake 目录>
cmake --build build --config Release
```

产物：`build/Release/EndMushroomGuard.dll`。

### xmake 方式（官方推荐）

```bash
xmake repo -u
xmake f -m release
xmake
```

## 部署

把 `EndMushroomGuard.dll` 与 `manifest.json` 一起放进服务器的：

```
<LeviLamina>/mods/EndMushroomGuard/
├── EndMushroomGuard.dll
└── manifest.json
```

重启服务器（`bedrock_server_mod.exe`）即可，无需任何命令。

## 自定义

所有逻辑集中在 `src/main.cpp`：

| 位置 | 作用 |
|------|------|
| `isMushroom()` | 判定哪些物品算“蘑菇”。想收紧/放宽匹配（如只拦 `red_mushroom`），改这里。 |
| `(int)dimension.getDimensionId() != 2` | 维度判定。改数字可换到下界(`1`)或主世界(`0`)。 |
| `player.sendMessage(...)` 字符串 | 提醒文案（§c 为红色）。 |

## 进阶：拦截后“强制传送回主世界”

原始三点需求里 JS 版有“传送回主世界”。拦截版默认**只拦截 + 提醒**（蘑菇没出现，
玩家还在末地，拦截本身已经达成目的）。若您仍要传送作为惩罚，在 `event.cancel()` 之后加：

```cpp
// 方式 A：BDS Player 的 teleportTo（参数签名以您服务器 BDS 版本为准）
// player.teleportTo(Vec3{0, 100, 0}, false, false, 0);

// 方式 B：更稳妥——让玩家执行 /tp 回到主世界坐标（需 OP 或命令权限）
// player.runCommand("/tp @s 0 100 0");
```

> 注意：传送 API 属于 BDS 内部接口，跨版本可能微调。编译报错时按您服务器的
> LeviLamina / BDS 头文件调整即可。

## 版本兼容

- `manifest.json` 已锁定 `LeviLamina` 版本 `26.10.10`（对应 MC 26.10.x）。
- **LeviLamina 与 MC 版本强绑定**，换 MC 版本必须同步升级 LeviLamina，否则启动即崩。
- 事件名 `PlayerPlacingBlockEvent`、`NativeMod` 入口写法在 LeviLamina 1.x 长期稳定。
