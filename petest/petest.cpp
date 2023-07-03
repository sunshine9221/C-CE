// petest.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#include "PortableExecutableFile.h"
#include <iostream>

int main()
{
    PortableExecutableFile file;
    int ret = file.Load("E:\\edoyun\\public\\DataSearch\\Debug\\DataSearch.exe");
    return 0;
}
