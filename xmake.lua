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
        set_toolchains("clang-cl")
    end
    add_packages("levilamina")
    set_kind("shared")
    set_languages("c++20")
    set_symbols("debug")
    add_headerfiles("src/**.h")
    add_files("src/**.cpp")
    add_includedirs("src")
