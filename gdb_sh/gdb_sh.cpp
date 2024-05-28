// g++ -std=c++20 -o app_con connect.cpp 
#include <iostream>
 
 
int main(int argc, char * argv[]) 
{
    std::cout<< argv[0] << "begin\n";
    getchar();
    std::cout<< argv[0] << "end\n";
    return argc;
}
