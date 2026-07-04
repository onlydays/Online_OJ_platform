# AGENTS.md — 项目约定

## Git 提交规范
- 每次较大版本变更后，自动执行 `git add` + `git commit` + `git push`
- Commit message 风格: `type: 简短描述`，如 `feat:`, `docs:`, `fix:`, `refactor:`

## 技术栈
- 后端: C++ (cpp-httplib + 线程池)，CMake 构建
- 前端: 原生 HTML + CSS + JS，SPA，CodeMirror 编辑器
- 数据库: MySQL / MariaDB
- 判题: Docker 容器 + IOI isolate 沙箱
- 部署: 单机 Docker Compose

## 项目结构
见 `SPEC.md` 第 7 节
