# 数据库目录

应用运行时，SQLite 数据库文件 `timemaster.db` 会被自动创建在这里。

## 路径解析逻辑（src/core/Database.cpp）

启动时从可执行文件位置向上查找：
1. 找到包含 `src/` 和 `CMakeLists.txt` 的目录 → 用 `<找到的目录>/database/timemaster.db`
2. 否则用 `<exe 同级目录>/database/timemaster.db`
3. 兜底：系统 AppData

这样开发时（exe 在 `build/Release/` 下），数据库会落在项目根的 `database/` 里，
即与 `src/` 同级 —— 方便备份和检查。

## 备份

复制本目录下的 `timemaster.db` 就是完整备份。导入到其他机器时放在相同的相对位置即可。
