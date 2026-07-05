-- Online OJ Platform — 数据库初始化脚本
-- 适用: MySQL 8.0 / MariaDB

CREATE DATABASE IF NOT EXISTS oj_platform
  CHARACTER SET utf8mb4
  COLLATE utf8mb4_unicode_ci;

USE oj_platform;

-- 用户表
CREATE TABLE users (
    id INT AUTO_INCREMENT PRIMARY KEY,
    username VARCHAR(50) UNIQUE NOT NULL,
    password_hash VARCHAR(255) NOT NULL,
    role ENUM('user', 'admin') DEFAULT 'user',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- 题目表
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

-- 测试用例表
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

-- 提交记录表
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

-- 逐用例判题结果表
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

-- 初始管理员账号 (密码: admin123, SHA-256)
INSERT INTO users (username, password_hash, role)
VALUES ('admin',
        '240be518fabd2724ddb6f04eeb1da5967448d7e831c08c8fa822809f74c720a9',
        'admin');
