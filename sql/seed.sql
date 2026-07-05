-- Online OJ Platform — 预置题目数据

USE oj_platform;

-- 题目 1: 两数之和
INSERT INTO problems (title, description, difficulty, time_limit_ms, memory_limit_kb, created_by)
VALUES ('两数之和', '给定一个整数数组 nums 和一个整数目标值 target，请你在该数组中找出和为目标值 target 的那两个整数，并返回它们的数组下标。\n\n输入：第一行包含两个整数 n 和 target，第二行包含 n 个整数。\n输出：两个整数的下标（空格分隔），保证有唯一解。\n\n示例输入：\n4 9\n2 7 11 15\n\n示例输出：\n0 1', 'easy', 1000, 262144, 1);

SET @p1 = LAST_INSERT_ID();
INSERT INTO test_cases (problem_id, input, expected_output, is_sample, order_index) VALUES
(@p1, '4 9\n2 7 11 15', '0 1', TRUE, 0),
(@p1, '3 6\n3 2 4', '1 2', FALSE, 1),
(@p1, '2 6\n3 3', '0 1', FALSE, 2);

-- 题目 2: 反转字符串
INSERT INTO problems (title, description, difficulty, time_limit_ms, memory_limit_kb, created_by)
VALUES ('反转字符串', '编写一个函数，其作用是将输入的字符串反转过来。\n\n输入：一行字符串 s\n输出：反转后的字符串\n\n示例输入：\nhello\n\n示例输出：\nolleh', 'easy', 1000, 262144, 1);

SET @p2 = LAST_INSERT_ID();
INSERT INTO test_cases (problem_id, input, expected_output, is_sample, order_index) VALUES
(@p2, 'hello', 'olleh', TRUE, 0),
(@p2, 'world', 'dlrow', FALSE, 1),
(@p2, '', '', FALSE, 2);

-- 题目 3: 斐波那契数列
INSERT INTO problems (title, description, difficulty, time_limit_ms, memory_limit_kb, created_by)
VALUES ('斐波那契数列', '计算斐波那契数列的第 n 项。F(0)=0, F(1)=1, F(n)=F(n-1)+F(n-2)。\n\n输入：一个整数 n (0 <= n <= 30)\n输出：第 n 项的值\n\n示例输入：\n10\n\n示例输出：\n55', 'easy', 1000, 262144, 1);

SET @p3 = LAST_INSERT_ID();
INSERT INTO test_cases (problem_id, input, expected_output, is_sample, order_index) VALUES
(@p3, '10', '55', TRUE, 0),
(@p3, '0', '0', FALSE, 1),
(@p3, '1', '1', FALSE, 2),
(@p3, '20', '6765', FALSE, 3);

-- 题目 4: 最大子序和
INSERT INTO problems (title, description, difficulty, time_limit_ms, memory_limit_kb, created_by)
VALUES ('最大子序和', '给定一个整数数组 nums，找到一个具有最大和的连续子数组（子数组最少包含一个元素），返回其最大和。\n\n输入：第一行一个整数 n，第二行 n 个整数。\n输出：最大子序和\n\n示例输入：\n9\n-2 1 -3 4 -1 2 1 -5 4\n\n示例输出：\n6', 'medium', 1000, 262144, 1);

SET @p4 = LAST_INSERT_ID();
INSERT INTO test_cases (problem_id, input, expected_output, is_sample, order_index) VALUES
(@p4, '9\n-2 1 -3 4 -1 2 1 -5 4', '6', TRUE, 0),
(@p4, '1\n5', '5', FALSE, 1),
(@p4, '3\n-1 -2 -3', '-1', FALSE, 2);

-- 题目 5: 判断回文数
INSERT INTO problems (title, description, difficulty, time_limit_ms, memory_limit_kb, created_by)
VALUES ('判断回文数', '给你一个整数 x，如果 x 是一个回文整数，输出 YES，否则输出 NO。回文数是指正序和倒序读都是一样的整数。\n\n输入：一个整数 x\n输出：YES 或 NO\n\n示例输入：\n121\n\n示例输出：\nYES', 'easy', 1000, 262144, 1);

SET @p5 = LAST_INSERT_ID();
INSERT INTO test_cases (problem_id, input, expected_output, is_sample, order_index) VALUES
(@p5, '121', 'YES', TRUE, 0),
(@p5, '-121', 'NO', FALSE, 1),
(@p5, '10', 'NO', FALSE, 2),
(@p5, '12321', 'YES', FALSE, 3);

-- 题目 6: 阶乘计算
INSERT INTO problems (title, description, difficulty, time_limit_ms, memory_limit_kb, created_by)
VALUES ('阶乘计算', '计算 n 的阶乘 n! = 1 × 2 × ... × n。\n\n输入：一个整数 n (0 <= n <= 12)\n输出：n 的阶乘值\n\n示例输入：\n5\n\n示例输出：\n120', 'easy', 1000, 262144, 1);

SET @p6 = LAST_INSERT_ID();
INSERT INTO test_cases (problem_id, input, expected_output, is_sample, order_index) VALUES
(@p6, '5', '120', TRUE, 0),
(@p6, '0', '1', FALSE, 1),
(@p6, '10', '3628800', FALSE, 2);

-- 题目 7: 冒泡排序
INSERT INTO problems (title, description, difficulty, time_limit_ms, memory_limit_kb, created_by)
VALUES ('冒泡排序', '对给定的 n 个整数进行从小到大排序。\n\n输入：第一行一个整数 n，第二行 n 个整数（空格分隔）。\n输出：排序后的 n 个整数（空格分隔）\n\n示例输入：\n5\n3 1 4 1 5\n\n示例输出：\n1 1 3 4 5', 'medium', 2000, 262144, 1);

SET @p7 = LAST_INSERT_ID();
INSERT INTO test_cases (problem_id, input, expected_output, is_sample, order_index) VALUES
(@p7, '5\n3 1 4 1 5', '1 1 3 4 5', TRUE, 0),
(@p7, '3\n9 2 7', '2 7 9', FALSE, 1),
(@p7, '1\n42', '42', FALSE, 2);

-- 题目 8: 素数判断
INSERT INTO problems (title, description, difficulty, time_limit_ms, memory_limit_kb, created_by)
VALUES ('素数判断', '判断一个整数是否为素数。素数是指大于 1 的自然数中，除了 1 和它本身以外不再有其他因数的数。\n\n输入：一个整数 n (1 <= n <= 10^6)\n输出：YES 或 NO\n\n示例输入：\n7\n\n示例输出：\nYES', 'easy', 1000, 262144, 1);

SET @p8 = LAST_INSERT_ID();
INSERT INTO test_cases (problem_id, input, expected_output, is_sample, order_index) VALUES
(@p8, '7', 'YES', TRUE, 0),
(@p8, '1', 'NO', FALSE, 1),
(@p8, '12', 'NO', FALSE, 2),
(@p8, '97', 'YES', FALSE, 3);
