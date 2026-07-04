# 项目依赖清单 & 安装命令

> 适用环境：空白 Ubuntu 24.04

---

## 1. 宿主机系统包

### 1.1 编译工具链

| 包名 | 用途 | apt 命令 |
|------|------|----------|
| `build-essential` | gcc/g++/make 等基础编译工具 | `sudo apt install -y build-essential` |
| `cmake` | CMake 构建系统 | `sudo apt install -y cmake` |
| `wget` | 下载 header-only 库 | `sudo apt install -y wget` |
| `git` | 克隆仓库 (jwt-cpp) | `sudo apt install -y git` |

### 1.2 MySQL 相关

| 包名 | 用途 | apt 命令 |
|------|------|----------|
| `libmysqlclient-dev` | MySQL C 客户端库 (mysql-connector-cpp 编译依赖) | `sudo apt install -y libmysqlclient-dev` |
| `libmysqlcppconn-dev` | Oracle 官方 C++ MySQL 连接器 | `sudo apt install -y libmysqlcppconn-dev` |
| `mysql-client` | 命令行 MySQL 客户端 (调试用) | `sudo apt install -y mysql-client` |

### 1.3 JWT / 密码哈希

| 包名 | 用途 | apt 命令 |
|------|------|----------|
| `libssl-dev` | OpenSSL (JWT 签名/验证、密码 SHA256 哈希) | `sudo apt install -y libssl-dev` |

### 1.4 Docker 引擎

| 包名 | 用途 | apt 命令 |
|------|------|----------|
| `docker.io` | Docker 引擎 (运行判题容器) | `sudo apt install -y docker.io` |
| `docker-compose-v2` | Docker Compose (编排后端 + MySQL) | `sudo apt install -y docker-compose-v2` |

### 1.5 JSON 库 (可选，系统包或源码)

| 包名 | 用途 | apt 命令 |
|------|------|----------|
| `nlohmann-json3-dev` | JSON 解析/序列化 | `sudo apt install -y nlohmann-json3-dev` |

---

## 2. C++ 第三方 Header-Only 库

> 放置路径：`backend/third_party/`

### 2.1 cpp-httplib — HTTP 服务端

| 属性 | 值 |
|------|-----|
| 文件 | `backend/third_party/httplib.h` |
| 来源 | `https://github.com/yhirose/cpp-httplib` |
| License | MIT |

```bash
mkdir -p backend/third_party
wget -O backend/third_party/httplib.h \
  https://raw.githubusercontent.com/yhirose/cpp-httplib/master/httplib.h
```

### 2.2 nlohmann/json — JSON 解析 / 序列化

> 如果未通过 apt 安装 `nlohmann-json3-dev`，则用源码：

| 属性 | 值 |
|------|-----|
| 文件 | `backend/third_party/json.hpp` |
| 来源 | `https://github.com/nlohmann/json` |
| License | MIT |

```bash
wget -O backend/third_party/json.hpp \
  https://raw.githubusercontent.com/nlohmann/json/develop/single_include/nlohmann/json.hpp
```

### 2.3 jwt-cpp — JWT 生成与验证

> 依赖 `libssl-dev`（OpenSSL）

| 属性 | 值 |
|------|-----|
| 文件 | `backend/third_party/jwt.h` |
| 来源 | `https://github.com/Thalhammer/jwt-cpp` |
| License | MIT |

```bash
git clone --depth 1 https://github.com/Thalhammer/jwt-cpp.git /tmp/jwt-cpp
cp /tmp/jwt-cpp/include/jwt-cpp/jwt.h backend/third_party/
rm -rf /tmp/jwt-cpp
```

---

## 3. Docker 镜像

### 3.1 判题容器镜像

> Dockerfile 位于 `judge_image/Dockerfile`，由 `docker build` 构建

| 基础镜像 | 内部软件 | 用途 |
|----------|----------|------|
| `ubuntu:22.04` | `g++` | 编译用户 C++ 代码 |
| | `isolate` | IOI 沙箱 (限制时间/内存/进程/网络) |
| | `python3` | isolate 运行依赖 |
| | `judge_entrypoint.sh` | 判题入口脚本 (项目自写) |

```bash
docker pull ubuntu:22.04
docker build -t oj-judge:latest judge_image/
```

### 3.2 MySQL 镜像

> 由 `docker-compose.yml` 自动拉取，无需手动

| 镜像 | 用途 |
|------|------|
| `mysql:8.0` | MySQL 数据库服务 |

### 3.3 后端应用镜像 (可选)

> 后续可通过 Dockerfile 构建后端镜像，纳入 `docker-compose.yml` 管理。
> 初期直接宿主机编译运行即可，不需要单独镜像。

---

## 4. 前端依赖

> 全部通过 CDN 加载，无需本地安装。如需离线化，可从以下 URL 下载到 `frontend/lib/`。

| 库 | 版本 | CDN URL | 用途 |
|----|------|---------|------|
| **CodeMirror 6** | latest | `https://cdn.jsdelivr.net/npm/codemirror@6/` | 在线代码编辑器 |
| **marked.js** | latest | `https://cdn.jsdelivr.net/npm/marked/marked.min.js` | Markdown 渲染 (题目描述) |

```bash
# 离线化 (可选)
mkdir -p frontend/lib
wget -O frontend/lib/marked.min.js \
  https://cdn.jsdelivr.net/npm/marked/marked.min.js

# CodeMirror 6 模块化复杂，建议保持 CDN 或后续用 npm 打包
```

---

## 5. 依赖拓扑图

```
宿主系统
├── apt 包
│   ├── build-essential, cmake        → C++ 编译
│   ├── libmysqlclient-dev            → MySQL C 客户端
│   ├── libmysqlcppconn-dev           → MySQL C++ 连接器
│   ├── libssl-dev                    → JWT (jwt-cpp) + 密码 SHA256
│   ├── nlohmann-json3-dev            → JSON 解析 (可选)
│   ├── docker.io                     → 运行判题容器
│   └── docker-compose-v2             → 编排服务
│
├── backend/third_party/ (header-only, #include 即可)
│   ├── httplib.h       (cpp-httplib)  → HTTP 服务端路由
│   ├── json.hpp        (nlohmann)     → JSON 解析 (备选)
│   └── jwt.h           (jwt-cpp)      → JWT 生成 / 验证
│                 └── 依赖 → libssl-dev
│
├── Docker 镜像
│   ├── mysql:8.0                      → docker-compose 自动拉取
│   └── oj-judge:latest                → 手动 docker build
│       ├── ubuntu:22.04               → base 自动拉取
│       ├── g++, isolate, python3      → Dockerfile 内 apt 安装
│       └── judge_entrypoint.sh        → 项目自编脚本
│
└── 前端 (CDN / 下载)
    ├── CodeMirror 6                   → 代码编辑器
    └── marked.js                      → Markdown 渲染
```

---

## 6. 一键安装脚本

```bash
#!/bin/bash
# install_all.sh — 空白 Ubuntu 24.04 一键安装全部依赖
set -e

echo "=== [1/5] 系统包 ==="
sudo apt update
sudo apt install -y \
  build-essential cmake wget git \
  libmysqlclient-dev libmysqlcppconn-dev mysql-client \
  libssl-dev nlohmann-json3-dev \
  docker.io docker-compose-v2

echo "=== [2/5] Docker 权限 ==="
sudo usermod -aG docker "$USER" 2>/dev/null || true

echo "=== [3/5] header-only 库 ==="
mkdir -p backend/third_party

wget -q -O backend/third_party/httplib.h \
  https://raw.githubusercontent.com/yhirose/cpp-httplib/master/httplib.h

wget -q -O backend/third_party/json.hpp \
  https://raw.githubusercontent.com/nlohmann/json/develop/single_include/nlohmann/json.hpp

git clone --depth 1 https://github.com/Thalhammer/jwt-cpp.git /tmp/jwt-cpp
cp /tmp/jwt-cpp/include/jwt-cpp/jwt.h backend/third_party/
rm -rf /tmp/jwt-cpp

echo "=== [4/5] 判题 Docker 镜像 ==="
docker pull ubuntu:22.04

echo "=== [5/5] 完成 ==="
echo ""
echo "依赖安装完成。请重新登录以使 docker 权限生效。"
echo "后续步骤:"
echo "  docker compose up -d          # 启动 MySQL + 后端"
echo "  bash install_all.sh    # 如需构建判题镜像"
```

---

## 7. 依赖汇总表

| # | 依赖 | 类型 | 安装方式 | 关键度 |
|---|------|------|----------|--------|
| 1 | build-essential | apt | `sudo apt install` | 必须 |
| 2 | cmake | apt | `sudo apt install` | 必须 |
| 3 | libmysqlclient-dev | apt | `sudo apt install` | 必须 |
| 4 | libmysqlcppconn-dev | apt | `sudo apt install` | 必须 |
| 5 | libssl-dev | apt | `sudo apt install` | 必须 |
| 6 | docker.io | apt | `sudo apt install` | 必须 |
| 7 | docker-compose-v2 | apt | `sudo apt install` | 必须 |
| 8 | nlohmann-json3-dev | apt | `sudo apt install` | 推荐 |
| 9 | mysql-client | apt | `sudo apt install` | 推荐 |
| 10 | cpp-httplib | 头文件 | `wget` | 必须 |
| 11 | nlohmann/json | 头文件 | `wget` 或 apt | 必须 |
| 12 | jwt-cpp | 头文件 | `git clone` + `cp` | 必须 |
| 13 | ubuntu:22.04 | Docker 镜像 | `docker pull` | 必须 |
| 14 | mysql:8.0 | Docker 镜像 | docker-compose 自动 | 必须 |
| 15 | CodeMirror 6 | CDN | HTML `<script>` 引用 | 必须 |
| 16 | marked.js | CDN | HTML `<script>` 引用 | 必须 |
