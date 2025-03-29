#include <execinfo.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <cxxabi.h>
#include <dlfcn.h>  // 用于获取更详细的符号信息

// 反修饰C++符号并打印更详细的堆栈信息
void print_stacktrace() {
    void* buffer[100];
    int nptrs = backtrace(buffer, sizeof(buffer)/sizeof(buffer[0]));
    
    printf("\nStack trace:\n");
    for (int i = 0; i < nptrs; i++) {
        Dl_info info;
        if (dladdr(buffer[i], &info)) {
            // 反修饰C++符号
            int status;
            char* demangled = abi::__cxa_demangle(info.dli_sname, NULL, NULL, &status);
            
            printf("#%-2d %p %s + %p (%s)\n",
                   i, buffer[i],
                   (demangled ? demangled : info.dli_sname),
                   (void*)((char*)buffer[i] - (char*)info.dli_saddr),
                   info.dli_fname);
            
            free(demangled);
        } else {
            printf("#%-2d %p [unknown symbol]\n", i, buffer[i]);
        }
    }
}

void handler(int /*sig*/) {  // 使用注释消除未使用参数警告
    print_stacktrace();
    exit(1);
}

void test() {
    int* p = nullptr;  // 使用C++11的nullptr
    *p = 42;          // 故意触发段错误
}

int main() {
    printf("core dump test\n");
    signal(SIGSEGV, handler);
    test();
    return 0;
}