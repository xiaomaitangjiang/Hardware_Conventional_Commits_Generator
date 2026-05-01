#include <cstdio>
#include <iostream>

#include "types.hpp"
#include "ui.hpp"

namespace {

void RegisterDefaults(hwcommit::TypeRegistry& types,
                      hwcommit::ScopeRegistry& scopes)
{
  using hwcommit::CommitScope;
  using hwcommit::CommitType;

  types.Register("feat",
                 CommitType{"feat", "feat - 新功能",
                            "新功能/模块添加，如新增模块、IP核、外设驱动"});
  types.Register("fix", CommitType{"fix", "fix - 修复",
                                   "时序违例、信号竞争、硬件bug修复"});
  types.Register("docs", CommitType{"docs", "docs - 文档",
                                    "架构文档、接口说明、约束文件说明"});
  types.Register("test", CommitType{"test", "test - 测试",
                                    "测试平台、仿真脚本、测试用例"});
  types.Register("refactor", CommitType{"refactor", "refactor - 重构",
                                        "代码优化、模块重组（不改变功能）"});
  types.Register("perf", CommitType{"perf", "perf - 性能优化",
                                    "时序优化、面积优化、功耗优化"});
  types.Register("style",
                 CommitType{"style", "style - 格式", "代码风格、命名规范调整"});
  types.Register(
      "build", CommitType{"build", "build - 构建", "Makefile、脚本、依赖更新"});
  types.Register(
      "ci", CommitType{"ci", "ci - CI/CD", "GitHub Actions、Jenkinsfile配置"});
  types.Register("chore", CommitType{"chore", "chore - 杂项",
                                     "文件移动、依赖更新等杂项任务"});

  scopes.Register("top", CommitScope{"top", "top - 顶层", "顶层设计"});
  scopes.Register("module", CommitScope{"module", "module - 模块", "模块级别"});
  scopes.Register("ip", CommitScope{"ip", "ip - IP核", "IP核"});
  scopes.Register("interface", CommitScope{"interface", "interface - 接口/总线",
                                           "接口/总线"});
  scopes.Register("board", CommitScope{"board", "board - 板级", "板级支持包"});
  scopes.Register("constraint",
                  CommitScope{"constraint", "constraint - 约束", "约束文件"});
  scopes.Register("sim", CommitScope{"sim", "sim - 仿真", "仿真相关"});
  scopes.Register("synthesis",
                  CommitScope{"synthesis", "synthesis - 综合", "综合相关"});
  scopes.Register("pkg", CommitScope{"pkg", "pkg - 包/库", "包/库文件"});
  scopes.Register("driver", CommitScope{"driver", "driver - 驱动", "设备驱动"});
}

}  // anonymous namespace

int main()
{
  hwcommit::TypeRegistry type_registry;
  hwcommit::ScopeRegistry scope_registry;
  RegisterDefaults(type_registry, scope_registry);

  auto result = hwcommit::RunTUI(type_registry, scope_registry);

  if (!result.empty()) {
    std::cout << '\n' << result << '\n' << '\n';
#ifdef _WIN32
    // Pipe to clip.exe for automatic clipboard copy. Silently skip if
    // unavailable.
    FILE* clip = _popen("clip", "w");
    if (clip != nullptr) {
      std::ignore = fwrite(result.c_str(), 1, result.size(), clip);
      _pclose(clip);
      std::cout << "[Copied to clipboard]\n";
    }
#endif
  }

  return 0;
}
