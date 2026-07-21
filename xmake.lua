add_rules("mode.debug", "mode.release")

add_repositories("levimc-repo https://github.com/LiteLDev/xmake-repo.git")

-- 锁定 zlib 到 1.3.1：levilamina 内部的 libcurl 依赖 zlib 1.3.1，但官方 xmake-repo
-- 当前默认解析到 zlib 1.3.2，导致 libcurl 的构建引用了不存在的 1.3.1 路径而失败。
-- 显式钉死全局 zlib 版本，让整张依赖图（含 libcurl）统一使用 1.3.1。
add_requires("zlib 1.3.1")

option("target_type")
    set_default("server")
    set_showmenu(true)
    set_values("server", "client")
option_end()

-- 锁定 LeviLamina 1.9.9（与老板服务端版本一致），避免 CI 拉到 26.20.x 编译出 ABI 不兼容的 DLL。
add_requires("levilamina 1.9.9", {configs = {target_type = get_config("target_type")}})
add_requires("levibuildscript")

if not has_config("vs_runtime") then
    set_runtimes("MD")
end

target("EndMushroomGuard")
    add_rules("@levibuildscript/linkrule")
    add_rules("@levibuildscript/modpacker")
    if is_plat("windows") then
        add_defines("NOMINMAX", "UNICODE")
        set_exceptions("none") -- 避免与 /EHa 冲突
        add_cxflags("/EHa", "/utf-8", "/W4", "/w44265", "/w44289", "/w44296", "/w45263", "/w44738", "/w45204")
        add_cxflags(
            "/EHs",
            "-Wno-microsoft-cast",
            "-Wno-invalid-offsetof",
            "-Wno-c++2b-extensions",
            "-Wno-microsoft-include",
            "-Wno-overloaded-virtual",
            "-Wno-ignored-qualifiers",
            "-Wno-missing-field-initializers",
            "-Wno-potentially-evaluated-expression",
            "-Wno-pragma-system-header-outside-header",
            {tools = {"clang_cl"}}
        )
        -- 关键修复：使用 MSVC(cl.exe) 而非 clang-cl。
        -- 原因：LeviLamina 依赖的 rapidjson v1.1.0 在 document.h:319 有一个
        -- 非标准的 GenericStringRef::operator=，它向 const 成员 length 赋值，
        -- 这在标准 C++ 中是非法代码。MSVC/GCC 会容忍它，但现代 Clang（含 runner
        -- 自带的、llvm-prebuilt 提供的 clang 16/18）会直接报 hard error：
        --   "cannot assign to non-static data member 'length' with const-qualified type"
        -- LeviLamina 自身的 Windows target 不设置 toolchain，xmake 默认回退到 MSVC，
        -- 因此它的源码能编译过 rapidjson。本插件此前强制 clang-cl，被 runner 上过新的
        -- 系统 LLVM 命中而失败。改为 MSVC 后即与 LeviLamina 自身构建使用同一工具链。
        -- （注意：插件产物 DLL 的 ABI 与 clang-cl 完全一致，均可被 LeviLamina 加载。）
        set_toolchains("msvc")
    end
    add_packages("levilamina")
    set_kind("shared")
    set_languages("c++20")
    set_symbols("debug")
    add_headerfiles("src/**.h")
    add_files("src/**.cpp")
    add_includedirs("src")
