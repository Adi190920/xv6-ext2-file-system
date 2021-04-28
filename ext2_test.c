#include "types.h"
#include "stat.h"
#include "fs.h"
#include "ext2.h"
#include "vfs.h"
#include "user.h"
#include "fcntl.h"

int 
main(int argc, char *argv[])
{
  int fd;
  // char str[10];
  fd = open("/mnt/h.txt",O_CREATE | O_RDWR);
  printf(1, "file created\n");
  write(fd, "Hello I am aditya!", 19);
  printf(1, "file written\n");
  close(fd);
  printf(1, "file close\n");
  // fd = open("/mnt/h.txt",O_RDWR);
  // read(fd, str, 10);
  // printf(1, str);
  // printf(1, "\n");
  // close(fd);
  // printf(1, "file close\n");
  exit();
}