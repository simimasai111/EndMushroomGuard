// EndMushroomGuard —— LeviLamina C++ 原生插件（兼容 LeviLamina 1.9.9）
// 功能：拦截玩家在末地放置蘑菇（方块压根不出现，物品栏保留），并发送文字提醒。
//
// 与 LLSE 脚本版的区别：
//   LLSE 的 afterPlaceBlock 事件【无法拦截】（return false 无效），只能事后清除方块。
//   本插件使用【可取消】的 PlayerPlacingBlockEvent，在放置“之前”调用 event.cancel()，
//   蘑菇根本不会生成、玩家手里还留着那个蘑菇——这是脚本做不到的硬拦截。
//   且 C++ 原生 mod 不依赖 LSE 引擎，零额外依赖，不与 CoralFans 等抢版本。

#include <cctype>
#include <string>

#include "ll/api/mod/NativeMod.h"
#include "ll/api/mod/RegisterHelper.h"
#include "ll/api/event/EventBus.h"
#include "ll/api/event/player/PlayerPlaceBlockEvent.h"

#include "mc/world/actor/player/Player.h"

// 注意：LeviLamina 1.9.9 的 NativeMod 采用【组合】写法（不继承 NativeMod），
//       而是持有 NativeMod& 引用，通过 LL_REGISTER_MOD 宏把实例绑定到引擎。
class EndMushroomGuard {
public:
    // 构造时从当前正在加载的 mod 拿到 NativeMod 引用
    EndMushroomGuard() : mSelf(*ll::mod::NativeMod::current()) {}

    [[nodiscard]] ll::mod::NativeMod& getSelf() const { return mSelf; }

    static EndMushroomGuard& getInstance();

    // load/enable/disable 是普通成员方法（非虚），由 LL_REGISTER_MOD 通过 concept 自动绑定
    bool load();
    bool enable();
    bool disable();

private:
    ll::mod::NativeMod&    mSelf;
    ll::event::ListenerPtr mPlacingListener;
};

// 判断物品 ID 是否属于蘑菇 / 菌类（覆盖红菇、棕菇、蘑菇柄、绯红菌、诡异菌等）。
// item.getTypeName() 返回形如 "minecraft:red_mushroom" 的完整 ID。
static bool isMushroom(std::string const& rawId) {
    std::string id = rawId;
    for (auto& c : id) c = (char)std::tolower((unsigned char)c);
    return id.find("mushroom") != std::string::npos
        || id.find("fungus") != std::string::npos;
}

bool EndMushroomGuard::load() {
    getSelf().getLogger().info("EndMushroomGuard 加载中……");
    return true;
}

bool EndMushroomGuard::enable() {
    auto& logger = getSelf().getLogger();
    auto& bus    = ll::event::EventBus::getInstance();

    mPlacingListener = bus.emplaceListener<ll::event::player::PlayerPlacingBlockEvent>(
        [&logger](ll::event::player::PlayerPlacingBlockEvent& event) {
            auto& player = event.self();   // Player&

            // 1) 仅拦截末地（DimensionId：0=主世界，1=下界，2=末地）
            if ((int)player.getDimensionId() != 2) {
                return;
            }

            // 2) 取玩家主手物品（即“即将被放置”的方块）
            auto const& item   = player.getCarriedItem();
            std::string itemId = item.getTypeName();
            if (!isMushroom(itemId)) {
                return;
            }

            // 3) 取消事件：蘑菇压根不出现，玩家物品栏仍保留该蘑菇
            event.cancel();

            // 4) 文字提醒（§c = 红色）
            player.sendMessage(std::string("\u00a7c[系统] 末地不允许放置蘑菇！该操作已被拦截。"));
            logger.info("已拦截末地放置蘑菇({})", itemId);
        }
    );

    logger.info("EndMushroomGuard 已启用：末地防蘑菇拦截生效");
    return true;
}

bool EndMushroomGuard::disable() {
    auto& bus = ll::event::EventBus::getInstance();
    if (mPlacingListener) {
        bus.removeListener(mPlacingListener);
        mPlacingListener.reset();
    }
    getSelf().getLogger().info("EndMushroomGuard 已禁用");
    return true;
}

// 全局单例 + 注册到 LeviLamina
EndMushroomGuard& EndMushroomGuard::getInstance() {
    static EndMushroomGuard instance;
    return instance;
}

LL_REGISTER_MOD(EndMushroomGuard, EndMushroomGuard::getInstance());
