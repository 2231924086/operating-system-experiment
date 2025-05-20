#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"

// 比较目录条目名称与目标文件名
int namecmp(char *name, char *target) {
  int len = strlen(target);
  
  // 比较长度范围内的字符
  for(int i = 0; i < len; i++) {
    if(name[i] != target[i])
      return 0;  // 不匹配
  }
  
  // 确保目录条目名称不含其他非空白字符
  return name[len] == 0 || name[len] == ' ';
}

void find(char *path, char *target) {
  char buf[512], *p;
  int fd;
  struct dirent de;
  struct stat st;

  // 打开目录
  if((fd = open(path, O_RDONLY)) < 0) {
    fprintf(2, "find: cannot open %s\n", path);
    return;
  }

  // 获取文件/目录信息
  if(fstat(fd, &st) < 0) {
    fprintf(2, "find: cannot stat %s\n", path);
    close(fd);
    return;
  }

  // 确保是目录
  if(st.type != T_DIR) {
    close(fd);
    return;
  }

  // 检查缓冲区大小
  if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf) {
    printf("find: path too long\n");
    close(fd);
    return;
  }
  
  // 复制路径到缓冲区
  strcpy(buf, path);
  p = buf+strlen(buf);
  // 如果需要添加斜杠
  if(p > buf && p[-1] != '/')
    *p++ = '/';
  
  // 读取目录项
  while(read(fd, &de, sizeof(de)) == sizeof(de)) {
    if(de.inum == 0)
      continue;
    
    // 跳过 . 和 ..
    if(strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
      continue;
    
    // 将文件名添加到路径
    memmove(p, de.name, DIRSIZ);
    p[DIRSIZ] = 0;
    
    // 获取文件状态
    if(stat(buf, &st) < 0) {
      printf("find: cannot stat %s\n", buf);
      continue;
    }
    
    // 如果名称匹配目标，打印
    if(namecmp(de.name, target)) {
      printf("%s\n", buf);
    }
    
    // 如果是目录，递归搜索
    if(st.type == T_DIR) {
      // 创建路径副本用于递归
      char subdir[512];
      strcpy(subdir, buf);
      find(subdir, target);
    }
  }
  
  close(fd);
}

int main(int argc, char *argv[]) {
  if(argc < 2) {
    fprintf(2, "Usage: find [directory] filename\n");
    exit(1);
  }
  
  if(argc == 2) {
    // 如果只提供一个参数，在当前目录搜索
    find(".", argv[1]);
  } else {
    // 在指定目录搜索
    find(argv[1], argv[2]);
  }
  
  exit(0);
}
