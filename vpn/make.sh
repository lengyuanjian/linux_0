#!/bin/bash

# 保存当前目录路径
CUR_PWD=$(pwd)

# 项目目录路径
PROJECT_DIR=("./com_lib/net_lib" "./server" "./client")
# PROJECT_DIR+=("./server_test")
for dir in "${PROJECT_DIR[@]}"; do
    # 切换到项目目录
    cd "$dir" || { echo "Failed to change directory to $PROJECT_DIR"; exit 1; }

    # 执行编译操作
    echo -e "\033[33mStarting build process... Eneter Directory $dir \033[0m"
    make $1 || { echo "Build failed"; cd "$CUR_PWD"; exit 1; }

    # 返回原目录
    cd "$CUR_PWD" || { echo "Failed to return to $CUR_PWD"; exit 1; }
done


echo -e "\033[33mBuild completed successfully.\033[0m"