// EndMushroomGuard —— LeviLamina C++ 原生插件
// 功能：拦截玩家在末地放置蘑菇（方块压根不出现，物品栏保留），并发送文字提醒。
// 兼容 LeviLamina 最新版（CI 已验证 26.20.4 ↔ MC 26.20.x）。
//
// 与 JS/LLSE 版的区别：
//   LLSE 的 afterPlaceBlock 事件【无法拦截】（return false 无效），只能事后清除方块。
//   本插件使用【可取消】的 PlayerPlacingBlockEvent，在放置“之前”调用 event.cancel()，
//   蘑菇根本不会生成、玩家手里还留着那个蘑菇——这是 JS 做不到的硬拦截。

#include <cctype>
#include <memory>
#include <string>

#include "ll/api/mod/NativeMod.h"
#include "ll/api/event/EventBus.h"
#include "ll/api/event/player/PlayerPlaceBlockEvent.h"

#include "mc/world/actor/player/Player.h"

class EndMushroomGuard : public ll::mod::NativeMod {
public:
    EndMushroomGuard() : NativeMod() {}
    ~EndMushroomGuard() override = default;

    bool enable() override;
    bool disable() override;

private:
    // 返回当前 mod 实例（用于拿 logger / 配置目录）
    static EndMushroomGuard& getSelf();

    ll::event::ListenerPtr mPlacingListener;
};

// 判断物品 ID 是否属于蘑菇 / 菌类。
// 覆盖：红菇、棕菇、蘑菇柄、绯红菌、诡异菌、绯红菌根、诡异菌根等。
// item.getTypeName() 返回形如 "minecraft:red_mushroom" 的完整 ID。
static bool isMushroom(std::string const& rawId) {
    std::string id = rawId;
    for (auto& c : id) c = (char)std::tolower((unsigned char)c);
    return id.find("mushroom") != std::string::npos
        || id.find("fungus") != std::string::npos;
}

EndMushroomGuard& EndMushroomGuard::getSelf() {
    // NativeMod::current() 返回当前正在启用 / 加载的 mod 实例。
    // EndMushroomGuard 是 NativeMod 的唯一子类实例，static_cast 安全（BDS 缺 RTTI，禁用 dynamic_cast）。
    return static_cast<EndMushroomGuard&>(ll::mod::NativeMod::current());
}

bool EndMushroomGuard::enable() {
    auto& logger = getSelf().getLogger();
    auto& bus    = ll::event::EventBus::getInstance();

    mPlacingListener = bus.emplaceListener<ll::event::player::PlayerPlacingBlockEvent>(
        [&logger](ll::event::player::PlayerPlacingBlockEvent& event) {
            auto& player = event.self();   // Player&

            // 1) 仅拦截末地（基岩版 DimensionId：0=主世界，1=下界，2=末地）
            //    事件自带 pos()，BlockPos::getDimension() 返回 DimensionId 枚举，比从 Player 取维度更稳。
            if ((int)event.pos().getDimension() != 2) {
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

// 入口：LeviLamina 加载 DLL 时会构造此全局实例并自动注册为 NativeMod。
// 请确保 manifest.json 的 entry / name 与此类名一致，且 DLL 文件名相同。
std::shared_ptr<EndMushroomGuard> mod = std::make_shared<EndMushroomGuard>();
