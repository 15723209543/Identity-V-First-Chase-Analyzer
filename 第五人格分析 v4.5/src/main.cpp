#include "first_chase_app.h"

#include <windows.h>

int main()
{
    // 使用中文控制台编码，配合工程的多字节字符集和 GB2312/GBK 源码。
    SetConsoleCP(936);
    SetConsoleOutputCP(936);

    first_chase_app app;
    return app.run();

}
