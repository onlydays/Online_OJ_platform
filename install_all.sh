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

echo "=== [3/5] header-only 库 (通过 jsdelivr CDN) ==="
mkdir -p backend/third_party

wget -O backend/third_party/httplib.h \
  https://cdn.jsdelivr.net/gh/yhirose/cpp-httplib@master/httplib.h

wget -O backend/third_party/json.hpp \
  https://cdn.jsdelivr.net/gh/nlohmann/json@develop/single_include/nlohmann/json.hpp

wget -O backend/third_party/jwt.h \
  https://cdn.jsdelivr.net/gh/Thalhammer/jwt-cpp@master/include/jwt-cpp/jwt.h

echo "=== [4/5] Docker 基础镜像预热 ==="
sudo docker pull ubuntu:22.04

echo "=== [5/5] 完成 ==="
echo ""
echo "依赖安装完成。请重新登录以使 docker 权限生效。"
echo "后续步骤:"
echo "  docker compose up -d                   # 启动 MySQL + 后端"
echo "  docker build -t oj-judge:latest judge_image/   # 构建判题镜像"
