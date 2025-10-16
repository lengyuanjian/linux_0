#!/bin/bash

while true; do
    clear
    id=$(pgrep -f lyj_lib_test_release)
    printf "VmRSS：物理内存使用（等效于 RSS）-VmSize：虚拟内存使用（等效于 VSZ）-\nVmData：数据段内存-VmStk：栈内存-VmExe：代码段内存-VmLib：共享库内存\n"
    cat /proc/${id}/status | grep -E 'VmRSS|VmSize|VmData|VmStk|VmExe|VmLib'
    sleep 1
done