# Online OJ Platform — 完整产品规格书 (SPEC)

---

## 1. 项目概述

仿 LeetCode 的在线判题（OJ）平台，面向小团队内部训练（1-20 人在线）。
支持 C++ 代码的在线编辑、编译、运行、判题，角色分为普通用户和管理员。

| 项目属性 | 决策 |
|----------|------|
| 用户规模 | 1-20 并发 |
| 前端 | 原生 HTML + CSS + JS，SPA 单页应用 |
| 后端 | C++ (cpp-httplib + 线程池) |
| 数据库 | MySQL / MariaDB |
| 判题沙箱 | Docker 容器（每次新建/销毁）+ IOI isolate |
| 支持语言 | 仅 C++（架构预留多语言扩展） |
| 部署方式 | 单机 Docker Compose |
| 构建系统 | CMake |

---

## 2. 系统架构

```
┌─────────────────────────────────────────────────────────────┐
│                        浏览器 (SPA)                         │
│  ┌──────┐ ┌──────┐ ┌──────┐ ┌──────┐ ┌──────┐ ┌──────┐   │
│  │ 登录 │ │ 题单 │ │ 做题 │ │ 结果 │ │ 记录 │ │ 管理 │   │
│  └──────┘ └──────┘ └──────┘ └──────┘ └──────┘ └──────┘   │
│             CodeMirror · 客户端路由 · JWT                  │
└──────────────────────┬──────────────────────────────────────┘
                       │  REST API (HTTP/JSON)
                       ▼
┌─────────────────────────────────────────────────────────────┐
│                   C++ 后端 (cpp-httplib)                    │
│  ┌──────────┐ ┌──────────┐ ┌──────────┐ ┌──────────────┐  │
│  │ JWT 认证  │ │ 路由分发  │ │ 线程池   │ │ Docker 调用   │  │
│  └──────────┘ └──────────┘ └──────────┘ └───────┬──────┘  │
│                    │ mysql-connector-cpp       │            │
└────────────────────┼───────────────────────────┼────────────┘
                     │                           │
                     ▼                           ▼
         ┌──────────────────┐     ┌──────────────────────────┐
         │  MySQL/MariaDB   │     │   Docker Judge Container  │
         │  · users         │     │   ┌────────────────────┐  │
         │  · problems      │     │   │  isolate 沙箱       │  │
         │  · test_cases    │     │   │  · g++ 编译         │  │
         │  · submissions   │     │   │  · 运行 + rlimit    │  │
         │  · sub_results   │     │   │  · 输出比对         │  │
         └──────────────────┘     │   └────────────────────┘  │
                                  └──────────────────────────┘
┌─────────────────────────────────────────────────────────────┐
│                   数据流 (判题流程)                          │
│                                                             │
│  用户提交代码                                                │
│    │                                                        │
│    ▼                                                        │
│  后端: 写入 MySQL (submission, status=pending)              │
│    │                                                        │
│    ▼                                                        │
│  后端: 线程池取出任务 → popen("docker run ...")              │
│    │                                                        │
│    ▼                                                        │
│  Docker容器:                                                │
│    1. 写入用户代码到文件                                     │
│    2. isolate --init → 创建沙箱                             │
│    3. g++ 编译代码 → 失败? → "CE"                            │
│    4. 逐测试用例: isolate --run → 运行 → 比对输出            │
│    5. 所有结果写入 stdout (JSON)                             │
│    │                                                        │
│    ▼                                                        │
│  后端: 解析结果 → 更新 MySQL → 返回给前端                    │
└─────────────────────────────────────────────────────────────┘
```

---

## 3. 数据库设计 (ER 图 & 表结构)

### 3.0 数据库连接信息

| 项目 | 值 |
|------|-----|
| 主机 | `127.0.0.1:3306` |
| 数据库名 | `oj_platform` |
| 用户名 | `root` |
| 密码 | `2791830200Zby..` |
| 字符集 | `utf8mb4` |

> 初始化脚本: `sql/init.sql`

### 3.1 ER 图

```
┌──────────┐       ┌──────────────┐       ┌───────────────┐
│  users   │ 1───N │ submissions  │ 1───N │ sub_results   │
│          │       │              │       │               │
│ id (PK)  │       │ id (PK)      │       │ id (PK)       │
│ username │       │ user_id (FK) │       │ submission_id │
│ password │       │ problem_id   │       │ test_case_id  │
│ role     │       │ status       │       │ status        │
└──────────┘       │ code_path    │       │ time_used_ms  │
       │           │ time_used_ms │       │ memory_used_kb│
       │           │ memory_used  │       │ actual_output │
       │           └──────────────┘       └───────────────┘
       │                                        │
       │ 1                                     N │
       ▼                                        ▼
┌──────────────┐       ┌──────────────────────┐
│  problems    │ 1───N │   test_cases         │
│              │       │                      │
│ id (PK)      │       │ id (PK)              │
│ title        │       │ problem_id (FK)      │
│ description  │       │ input (TEXT)         │
│ difficulty   │       │ expected_output(TEXT)│
│ time_limit   │       │ is_sample (BOOLEAN)  │
│ memory_limit │       │ order_index (INT)    │
│ created_by   │───────│                      │
└──────────────┘       └──────────────────────┘
```

### 3.2 建表 SQL

```sql
CREATE TABLE users (
    id INT AUTO_INCREMENT PRIMARY KEY,
    username VARCHAR(50) UNIQUE NOT NULL,
    password_hash VARCHAR(255) NOT NULL,
    role ENUM('user', 'admin') DEFAULT 'user',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE problems (
    id INT AUTO_INCREMENT PRIMARY KEY,
    title VARCHAR(200) NOT NULL,
    description TEXT NOT NULL,
    difficulty ENUM('easy', 'medium', 'hard') DEFAULT 'medium',
    time_limit_ms INT DEFAULT 1000,
    memory_limit_kb INT DEFAULT 262144,
    created_by INT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    FOREIGN KEY (created_by) REFERENCES users(id) ON DELETE SET NULL
);

CREATE TABLE test_cases (
    id INT AUTO_INCREMENT PRIMARY KEY,
    problem_id INT NOT NULL,
    input TEXT NOT NULL,
    expected_output TEXT NOT NULL,
    is_sample BOOLEAN DEFAULT FALSE,
    order_index INT DEFAULT 0,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (problem_id) REFERENCES problems(id) ON DELETE CASCADE
);

CREATE TABLE submissions (
    id INT AUTO_INCREMENT PRIMARY KEY,
    user_id INT NOT NULL,
    problem_id INT NOT NULL,
    code_path VARCHAR(500) NOT NULL,
    status ENUM('pending','compiling','running',
                'accepted','wrong_answer',
                'compilation_error','time_limit_exceeded',
                'memory_limit_exceeded','runtime_error',
                'presentation_error') DEFAULT 'pending',
    time_used_ms INT DEFAULT 0,
    memory_used_kb INT DEFAULT 0,
    compile_error TEXT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
    FOREIGN KEY (problem_id) REFERENCES problems(id) ON DELETE CASCADE
);

CREATE TABLE submission_results (
    id INT AUTO_INCREMENT PRIMARY KEY,
    submission_id INT NOT NULL,
    test_case_id INT NOT NULL,
    status ENUM('accepted','wrong_answer',
                'time_limit_exceeded','memory_limit_exceeded',
                'runtime_error','presentation_error') NOT NULL,
    time_used_ms INT DEFAULT 0,
    memory_used_kb INT DEFAULT 0,
    actual_output TEXT,
    FOREIGN KEY (submission_id) REFERENCES submissions(id) ON DELETE CASCADE,
    FOREIGN KEY (test_case_id) REFERENCES test_cases(id) ON DELETE CASCADE
);
```

---

## 4. REST API 接口清单

### 4.1 认证模块

| 方法 | 路径 | 说明 | 鉴权 |
|------|------|------|------|
| POST | `/api/auth/register` | 注册 | 无 |
| POST | `/api/auth/login` | 登录，返回 JWT | 无 |

**POST /api/auth/register**
```json
// Request
{ "username": "alice", "password": "123456" }
// Response 201
{ "id": 1, "username": "alice", "role": "user" }
```

**POST /api/auth/login**
```json
// Request
{ "username": "alice", "password": "123456" }
// Response 200
{ "token": "eyJhbGci...", "user": { "id": 1, "username": "alice", "role": "user" } }
```

### 4.2 用户模块

| 方法 | 路径 | 说明 | 鉴权 |
|------|------|------|------|
| GET | `/api/users/me` | 获取当前用户信息 | JWT |
| GET | `/api/users/me/stats` | 个人做题统计 | JWT |

**GET /api/users/me/stats**
```json
// Response 200
{
  "total_submissions": 42,
  "accepted": 15,
  "ac_rate": 35.7,
  "problems_solved": 10
}
```

### 4.3 题目模块

| 方法 | 路径 | 说明 | 鉴权 |
|------|------|------|------|
| GET | `/api/problems` | 题目列表（支持筛选） | JWT |
| GET | `/api/problems/:id` | 题目详情（含样例用例） | JWT |
| POST | `/api/problems` | 新增题目 | Admin |
| PUT | `/api/problems/:id` | 编辑题目 | Admin |
| DELETE | `/api/problems/:id` | 删除题目 | Admin |

**GET /api/problems?difficulty=easy&page=1&size=20**
```json
// Response 200
{
  "total": 50,
  "problems": [
    { "id": 1, "title": "两数之和", "difficulty": "easy", "ac_rate": 75.5 }
  ]
}
```

**GET /api/problems/1**
```json
// Response 200
{
  "id": 1,
  "title": "两数之和",
  "description": "## 题目描述\n...(Markdown)...",
  "difficulty": "easy",
  "time_limit_ms": 1000,
  "memory_limit_kb": 262144,
  "sample_testcases": [
    { "input": "2 7\n1 2 3 4 5", "expected_output": "0 1" }
  ]
}
```

**POST /api/problems** (Admin)
```json
// Request
{
  "title": "新题目",
  "description": "## 描述...",
  "difficulty": "medium",
  "time_limit_ms": 2000,
  "memory_limit_kb": 131072
}
// Response 201
{ "id": 2, ... }
```

### 4.4 测试用例模块

| 方法 | 路径 | 说明 | 鉴权 |
|------|------|------|------|
| POST | `/api/problems/:id/testcases` | 批量添加测试用例 | Admin |
| GET | `/api/problems/:id/testcases` | 查看所有测试用例 | Admin |
| DELETE | `/api/testcases/:id` | 删除单个用例 | Admin |

**POST /api/problems/:id/testcases** (Admin)
```json
// Request
{
  "testcases": [
    { "input": "1 2", "expected_output": "3", "is_sample": true },
    { "input": "5 5", "expected_output": "10", "is_sample": false }
  ]
}
// Response 201
{ "count": 2 }
```

### 4.5 提交判题模块

| 方法 | 路径 | 说明 | 鉴权 |
|------|------|------|------|
| POST | `/api/submissions` | 提交代码 | JWT |
| GET | `/api/submissions` | 提交历史（可分页） | JWT |
| GET | `/api/submissions/:id` | 单次提交详情（含逐用例结果） | JWT |

**POST /api/submissions**
```json
// Request
{ "problem_id": 1, "code": "#include <iostream>\nint main() {...}" }
// Response 201
{
  "id": 100,
  "status": "pending",
  "created_at": "2026-07-04T12:00:00Z"
}
```

**GET /api/submissions/:id** (轮询判题结果)
```json
// Response 200
{
  "id": 100,
  "problem_id": 1,
  "status": "wrong_answer",
  "time_used_ms": 15,
  "memory_used_kb": 4096,
  "compile_error": null,
  "results": [
    { "status": "accepted", "time_used_ms": 5, "memory_used_kb": 1024 },
    { "status": "wrong_answer", "time_used_ms": 15, "memory_used_kb": 4096 }
  ],
  "created_at": "2026-07-04T12:00:00Z"
}
```

---

## 5. 前端页面 & 组件设计

### 5.1 页面清单

| # | 路由 | 页面名称 | 权限 | 核心元素 |
|---|------|----------|------|----------|
| 1 | `/login` | 登录/注册 | 公开 | 表单切换（登录/注册），JWT 存入 localStorage |
| 2 | `/` | 题目列表 | 登录后 | 搜索框、难度筛选、题目表格、分页 |
| 3 | `/problems/:id` | 题目详情+做题 | 登录后 | Markdown 渲染题目描述、CodeMirror 编辑器、语言选择（当前仅 C++）、提交按钮、样例用例展示 |
| 4 | `/submissions/:id` | 判题结果页 | 登录后 | 整体状态 Badge、时间/内存消耗、逐用例结果列表 |
| 5 | `/history` | 提交记录 | 登录后 | 按题目/状态筛选、历史列表表格 |
| 6 | `/admin` | 管理后台 | Admin | 题目 CRUD 表格、测试用例管理面板 |
| 7 | `/profile` | 个人主页 | 登录后 | 做题统计卡片、AC 率图表 |

### 5.2 技术选型清单

| 组件 | 选择 | 原因 |
|------|------|------|
| HTTP 客户端 | Fetch API | 原生 JS，无额外依赖 |
| 代码编辑器 | CodeMirror 6 | 轻量、适合嵌入式代码编辑 |
| Markdown 渲染 | marked.js | 极小体积，渲染题目描述 |
| 客户端路由 | 手写简易 HashRouter | ~50 行代码，无需引入框架 |
| CSS 方案 | 原生 CSS Grid/Flexbox | 无框架依赖 |

---

## 6. 判题核心设计

### 6.1 判题容器镜像

Dockerfile 内容规划：
```dockerfile
FROM ubuntu:22.04
RUN apt-get update && apt-get install -y g++ isolate python3
# isolate 需从源码编译或 apt 安装
COPY judge_entrypoint.sh /usr/local/bin/
RUN chmod +x /usr/local/bin/judge_entrypoint.sh
ENTRYPOINT ["/usr/local/bin/judge_entrypoint.sh"]
```

### 6.2 判题入口脚本 (judge_entrypoint.sh) 流程

```
输入 (stdin JSON): { "code": "...", "time_limit_ms": 1000, "memory_limit_kb": 262144,
                     "testcases": [{ "input": "...", "expected": "..." }, ...] }

1. echo "$CODE" > /tmp/solution.cpp
2. isolate --init --box-id=0
3. cp /tmp/solution.cpp /var/local/lib/isolate/0/box/solution.cpp
4. isolate --run --box-id=0 \
     --processes=10 \
     --time=${TIME_LIMIT_SECS} \
     --mem=${MEM_LIMIT_KB} \
     -- /usr/bin/g++ -O2 -std=c++17 solution.cpp -o solution
5. 若编译失败 → 输出 {"status": "compilation_error", "error": "..."} → 退出
6. for each testcase:
   a. echo "$INPUT" > /var/local/lib/isolate/0/box/input.txt
   b. isolate --run --box-id=0 \
        --processes=1 \
        --time=${TIME_LIMIT_SECS} \
        --mem=${MEM_LIMIT_KB} \
        -- /box/solution < input.txt > output.txt
   c. 读取 isolate 退出码和 meta 文件 → 判定 TLE/MLE/RE
   d. diff output.txt expected.txt → 判定 WA/PE/AC
7. isolate --cleanup --box-id=0
8. 输出 (stdout JSON): { "status": "...", "results": [...], "time_ms": ..., "memory_kb": ... }
```

### 6.3 判题状态流转

```
                 ┌─────────┐
                 │ pending  │
                 └────┬─────┘
                      │
                      ▼
              ┌───────────────┐
              │   compiling   │
              └───┬───────┬───┘
                  │       │ (编译失败)
                  │       └──────► compilation_error
                  │
                  ▼
              ┌───────────────┐
              │   running     │ (逐用例)
              └───┬───┬───┬───┘
     ┌────────────┼───┼───┼──────────────┐
     ▼            ▼   ▼   ▼              ▼
 accepted    TLE   MLE  RE        wrong_answer
                              presentation_error
```

---

## 7. 项目目录结构

```
Online_OJ_platform/
├── SPEC.md                     # 本文件
├── docker-compose.yml          # 编排后端 + MySQL
├── CMakeLists.txt              # 顶层 CMake
├── backend/
│   ├── CMakeLists.txt
│   ├── src/
│   │   ├── main.cpp            # 入口，启动 httplib 服务
│   │   ├── router.cpp/.h      # 路由注册 & 分发
│   │   ├── auth.cpp/.h        # JWT 生成/验证中间件
│   │   ├── handlers/
│   │   │   ├── auth_handler.cpp/.h
│   │   │   ├── problem_handler.cpp/.h
│   │   │   ├── submission_handler.cpp/.h
│   │   │   └── user_handler.cpp/.h
│   │   ├── models/
│   │   │   ├── user.cpp/.h
│   │   │   ├── problem.cpp/.h
│   │   │   ├── submission.cpp/.h
│   │   │   └── testcase.cpp/.h
│   │   ├── judge/
│   │   │   ├── judge_manager.cpp/.h   # Docker 调用 + 线程池
│   │   │   └── judge_result.cpp/.h    # 结果解析
│   │   ├── db/
│   │   │   └── connection_pool.cpp/.h # MySQL 连接池
│   │   └── utils/
│   │       ├── json.cpp/.h            # JSON 解析/序列化 (nlohmann/json)
│   │       └── string_utils.cpp/.h
│   └── third_party/
│       ├── cpp-httplib/               # header-only
│       ├── nlohmann/json/             # header-only
│       └── jwt-cpp/                   # JWT 库 (header-only)
├── judge_image/
│   ├── Dockerfile
│   └── judge_entrypoint.sh
├── frontend/
│   ├── index.html              # SPA 入口
│   ├── css/
│   │   └── style.css
│   ├── js/
│   │   ├── app.js              # 路由、初始化
│   │   ├── api.js              # API 请求封装
│   │   ├── auth.js             # 登录/注册逻辑 + JWT 管理
│   │   ├── router.js           # 简易 HashRouter
│   │   └── pages/
│   │       ├── login.js
│   │       ├── problemList.js
│   │       ├── problemDetail.js
│   │       ├── submissionResult.js
│   │       ├── history.js
│   │       ├── admin.js
│   │       └── profile.js
│   └── lib/
│       ├── codemirror/         # CodeMirror 6 打包
│       └── marked.min.js       # Markdown 渲染
└── sql/
    └── init.sql                # 建表 + 初始管理员账号
```

---

## 8. TODO 开发任务拆分

### Phase 1: 基础设施 (约 2-3 天)

| # | 任务 | 优先级 | 产出物 |
|---|------|--------|--------|
| 1.1 | 搭建 C++ 后端骨架 (CMake + cpp-httplib Hello World) | P0 | `backend/` 目录，可编译运行 |
| 1.2 | MySQL 建表 + 初始数据 | P0 | `sql/init.sql`，数据库就绪 |
| 1.3 | 数据库连接池实现 | P0 | `db/connection_pool.cpp` |
| 1.4 | JSON 工具封装 (nlohmann/json) | P0 | `utils/json.cpp` |
| 1.5 | Docker Compose 编排文件 | P0 | `docker-compose.yml` |

### Phase 2: 认证与用户 (约 1-2 天)

| # | 任务 | 优先级 | 产出物 |
|---|------|--------|--------|
| 2.1 | 用户模型 (CRUD) | P0 | `models/user.cpp` |
| 2.2 | 密码哈希 (bcrypt/SHA256) | P0 | `auth.cpp` 中的哈希函数 |
| 2.3 | JWT 生成与验证中间件 | P0 | `auth.cpp` |
| 2.4 | 注册/登录 API | P0 | `handlers/auth_handler.cpp` |
| 2.5 | 前端登录/注册页面 | P0 | `pages/login.js` |

### Phase 3: 题目管理 (约 2 天)

| # | 任务 | 优先级 | 产出物 |
|---|------|--------|--------|
| 3.1 | 题目 + 测试用例模型 | P0 | `models/problem.cpp`, `models/testcase.cpp` |
| 3.2 | 题目 CRUD API | P0 | `handlers/problem_handler.cpp` |
| 3.3 | 测试用例管理 API (Admin) | P0 | `handlers/problem_handler.cpp` |
| 3.4 | 前端题目列表页 | P0 | `pages/problemList.js` |
| 3.5 | 前端管理后台页 | P1 | `pages/admin.js` |

### Phase 4: 判题核心 (约 3-4 天) ⚠️ 关键路径

| # | 任务 | 优先级 | 产出物 |
|---|------|--------|--------|
| 4.1 | 判题 Docker 镜像编写 | P0 | `judge_image/Dockerfile`, `judge_entrypoint.sh` |
| 4.2 | Docker 调用封装 (popen) | P0 | `judge/judge_manager.cpp` |
| 4.3 | 判题结果解析 | P0 | `judge/judge_result.cpp` |
| 4.4 | 线程池实现 (用于异步判题) | P0 | `judge/judge_manager.cpp` 内嵌或独立 |
| 4.5 | 提交 API (POST + GET + history) | P0 | `handlers/submission_handler.cpp` |
| 4.6 | isolate 沙箱调通 + 全面测试 (CE/TLE/MLE/RE/WA/PE/AC) | P0 | 测试报告 |

### Phase 5: 前端完整化 (约 2-3 天)

| # | 任务 | 优先级 | 产出物 |
|---|------|--------|--------|
| 5.1 | 题目详情页 + CodeMirror 集成 | P0 | `pages/problemDetail.js` |
| 5.2 | 判题结果页 (逐用例展示) | P0 | `pages/submissionResult.js` |
| 5.3 | 提交记录页 | P1 | `pages/history.js` |
| 5.4 | 个人主页页 | P1 | `pages/profile.js` |
| 5.5 | 前端路由 + 页面切换 | P0 | `router.js`, `app.js` |
| 5.6 | CSS 样式整体美化 | P1 | `css/style.css` |

### Phase 6: 联调 & 部署 (约 1-2 天)

| # | 任务 | 优先级 | 产出物 |
|---|------|--------|--------|
| 6.1 | 全流程联调测试 | P0 | 测试用例表 |
| 6.2 | Docker Compose 一键部署验证 | P0 | 部署文档 |
| 6.3 | 预置 5-10 道题目 | P1 | SQL insert 脚本 |
| 6.4 | 前端静态文件由 cpp-httplib 托管 | P0 | `main.cpp` 中静态文件路由 |

---

## 9. 验收标准

### 9.1 功能验收

| 功能 | 验收条件 |
|------|----------|
| 注册/登录 | 用户可用用户名密码注册并登录，错误密码提示明确 |
| JWT 鉴权 | 未登录无法访问任何需要登录的接口，返回 401 |
| 题目列表 | 展示所有题目，支持难度筛选和分页 |
| 题目详情 | 显示 Markdown 渲染的描述、样例用例、时间/内存限制 |
| 代码编辑 | CodeMirror 正常加载，C++ 语法高亮 |
| 提交判题 | 提交后状态从 pending → 最终结果，刷新可获取 |
| AC (Accepted) | 所有测试用例通过 |
| WA (Wrong Answer) | 至少一个用例输出不匹配 |
| CE (Compilation Error) | 代码编译失败，返回编译错误信息 |
| TLE (Time Limit Exceeded) | 运行时间超过题目限制 |
| MLE (Memory Limit Exceeded) | 使用内存超过题目限制 |
| RE (Runtime Error) | 运行时异常（段错误等） |
| PE (Presentation Error) | 输出基本正确但格式有差异 |
| 提交记录 | 用户可查看自己的历史提交，按题目/状态筛选 |
| 题目管理 (Admin) | 管理员可新增/编辑/删除题目和测试用例 |
| 普通用户权限限制 | 普通用户无法访问管理 API，返回 403 |
| 个人统计 | 个人主页展示提交数、AC 数、AC 率 |

### 9.2 非功能验收

| 维度 | 验收条件 |
|------|----------|
| 并发 | 20 人同时提交判题，所有请求在 30 秒内返回结果 |
| 安全-沙箱 | 用户代码无法访问宿主机文件系统，无法发起网络请求 |
| 安全-注入 | SQL 注入防护（使用参数化查询） |
| 安全-密码 | 密码哈希存储，无明文 |
| 安全-溢出 | 单次判题内存/时间严格限制，不可破坏 Docker 宿主 |
| 性能 | 题目列表 API 响应 < 100ms，判题提交 API < 200ms |
| 可用性 | 后端异常崩溃后 Docker 自动重启 |
| 代码存储 | 历史提交代码可查看，存储在文件系统，路径存数据库 |

### 9.3 边缘案例

| 场景 | 预期行为 |
|------|----------|
| 空代码提交 | 返回错误提示 "代码不能为空" |
| 超长代码 (>64KB) | 拒绝提交，返回 413 错误 |
| 不存在的题目 ID | 返回 404 |
| 不存在的提交 ID | 返回 404 |
| 并发提交同一题 | 每次提交独立 Docker 容器，互不影响 |
| Docker 不可用 | 提交返回 503 "判题服务暂时不可用" |
| MySQL 连接断开 | 连接池自动重连 |
| JWT 过期 | 前端自动跳转登录页，返回 401 |
| 测试用例为空 | 管理后台禁止保存无测试用例的题目 |
| 用户删除题目 | 级联删除关联的测试用例和提交记录 |

---

## 10. 潜在风险与缓解措施

| 风险 | 影响 | 概率 | 缓解 |
|------|------|------|------|
| isolate 在 Docker 内嵌套使用有问题 | 判题核心不可用 | 中 | 备选方案：Docker 自带 cgroup + rlimit，不用 isolate |
| cpp-httplib 单线程瓶颈 | 20 人并发时响应慢 | 低 | 线程池已规划，httplib 支持多线程模式 |
| Docker 容器启动开销 | 单次判题延迟高 (1-3s) | 高 | 可接受范围内；后续可改为容器池复用 |
| mysql-connector-cpp 编译/链接复杂 | 开发环境搭建困难 | 中 | Docker Compose 中预置编译环境 |
| 前端原生 JS 维护性差 | 代码混乱难以扩展 | 中 | 模块化拆分 (pages/)，ES Modules，保持简洁 |

---

*文档版本: v1.0 · 生成时间: 2026-07-04 · 状态: 待评审*
