source set_env.gdb
# // break n （简写b n）:在第n行处设置断点
# // （可以带上代码路径和代码名称： b OAGUPDATE.cpp:578）
# // b fn1 if a＞b：条件断点设置
# // break func（break缩写为b）：在函数func()的入口处设置断点，如：break cb_button
# // delete 断点号n：删除第n个断点
# // disable 断点号n：暂停第n个断点
# // enable 断点号n：开启第n个断点
# // clear 行号n：清除第n行的断点
# // info b （info breakpoints） ：显示当前程序的断点设置情况
# // delete breakpoints：清除所有断点

# // gdb中使用“x”命令来打印内存的值，格式为“x/nfu addr”。含义为以f格式打印从addr开始的n个长度单元为u的内存值。参数具体含义如下：
# // *）n：输出单元的个数。
# // *）f：是输出格式。比如x是以16进制形式输出，
# //         o: 是以8进制形式输出,
# //         d：以十进制形式输出
# //         u：以十进制形式输出，但是不带符号
# //         t：以二进制形式输出
# //         a：以地址的形式输出
# //         c：以字符形式输出
# //         f：以浮点数形式输出
# //         s：以字符串形式输出,
# // *）u：标明一个单元的长度。b是一个byte，h是两个byte（halfword），w是四个byte（word），g是八个byte（giant word）。
b main
commands
set $i=0
  while $i < 5
    printf "i = %d\n", $i
    set $i=$i+1
  end
  c
end
r
