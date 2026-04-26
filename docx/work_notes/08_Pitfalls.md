# 踩坑记录

### [2026-04-14] 压缩模板仍引用其他仓库路径
- **现象**：`docx/07_compress_context.md` 中的归档规则仍指向 `e:\\ultralytics\\md\\07_Current_Progress.md` 和 `e:\\ultralytics\\md\\08_Pitfalls.md`，与当前仓库 `E:\\STM32\\OV` 不一致。
- **原因**：压缩模板从其他项目复用后，没有针对当前仓库做路径本地化处理。
- **解决**：将规则文档中的目标路径改为当前仓库下的 [docx/07_Current_Progress.md](/E:/STM32/OV/docx/07_Current_Progress.md) 和 [docx/08_Pitfalls.md](/E:/STM32/OV/docx/08_Pitfalls.md)，并按同一结构正式归档本轮内容。
- **教训**：通用模板进入新仓库后，第一步应先检查路径、编号和引用对象是否仍然有效；上下文压缩文档尤其不能保留旧项目路径。

### [2026-04-14] Windows 沙箱账户触发 Git `safe.directory` 限制
- **现象**：执行 `git status --short` 时返回 `detected dubious ownership`，导致无法直接使用 Git 命令查看仓库状态。
- **原因**：仓库所有者是 `administor`，当前沙箱执行账户是 `CodexSandboxOffline`，Git 将其识别为潜在不安全目录。
- **解决**：本轮未修改 Git 配置，改为基于文件直接检查和文档归档完成任务；如后续确实需要 Git 状态校验，应由用户批准后配置 `safe.directory`。
- **教训**：在 Windows 共享工作区或沙箱环境下，不要默认 Git 命令可直接使用；需要先确认仓库所有权和 `safe.directory` 状态，再决定是否依赖 Git 做验证。
