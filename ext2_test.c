#include "types.h"
#include "stat.h"
#include "fs.h"
#include "ext2.h"
#include "vfs.h"
#include "user.h"
#include "fcntl.h"

char buf[8192];
char name[8] = "/mnt/";
char *echoargv[] = { "echo", "ALL", "TESTS", "PASSED", 0 };
int stdout = 1;





void
writetest(void)
{
  int fd;
  int i;

  printf(stdout, "small file test\n");
  fd = open("/mnt/small", O_CREATE|O_RDWR);
  if(fd >= 0){
    printf(stdout, "creat small succeeded; ok\n");
  } else {
    printf(stdout, "error: creat small failed!\n");
    exit();
  }
  for(i = 0; i < 100; i++){
    if(write(fd, "aaaaaaaaaa", 10) != 10){
      printf(stdout, "error: write aa %d new file failed\n", i);
      exit();
    }
    if(write(fd, "bbbbbbbbbb", 10) != 10){
      printf(stdout, "error: write bb %d new file failed\n", i);
      exit();
    }
  }
  printf(stdout, "writes ok\n");
  close(fd);
  fd = open("/mnt/small", O_RDONLY);
  if(fd >= 0){
    printf(stdout, "open small succeeded ok\n");
  } else {
    printf(stdout, "error: open small failed!\n");
    exit();
  }
  i = read(fd, buf, 2000);
  if(i == 2000){
    printf(stdout, "read succeeded ok\n");
  } else {
    printf(stdout, "read failed\n");
    exit();
  }
  close(fd);

  if(unlink("/mnt/small") < 0){
    printf(stdout, "unlink small failed\n");
    exit();
  }
  printf(stdout, "small file test ok\n");
}

void
writetest1(void)
{
  int i, fd, n;

  printf(stdout, "big files test\n");

  fd = open("/mnt/big", O_CREATE|O_RDWR);
  if(fd < 0){
    printf(stdout, "error: creat big failed!\n");
    exit();
  }

  for(i = 0; i < MAXFILE; i++){
    ((int*)buf)[0] = i;
    if(write(fd, buf, 512) != 512){
      printf(stdout, "error: write big file failed\n", i);
      exit();
    }
  }

  close(fd);

  fd = open("/mnt/big", O_RDONLY);
  if(fd < 0){
    printf(stdout, "error: open big failed!\n");
    exit();
  }

  n = 0;
  for(;;){
    i = read(fd, buf, 512);
    if(i == 0){
      if(n == MAXFILE - 1){
        printf(stdout, "read only %d blocks from big", n);
        exit();
      }
      break;
    } else if(i != 512){
      printf(stdout, "read failed %d\n", i);
      exit();
    }
    if(((int*)buf)[0] != n){
      printf(stdout, "read content of block %d is %d\n",
             n, ((int*)buf)[0]);
      exit();
    }
    n++;
  }
  close(fd);
  if(unlink("/mnt/big") < 0){
    printf(stdout, "unlink big failed\n");
    exit();
  }
  printf(stdout, "big files ok\n");
}

void
createtest(void)
{
  int i, fd;

  printf(stdout, "many creates, followed by unlink test\n");

  name[5] = 'a';
  name[7] = '\0';
  for(i = 0; i < 52; i++){
    name[6] = '0' + i;
    fd = open(name, O_CREATE|O_RDWR);
    close(fd);
  }
  name[5] = 'a';
  name[7] = '\0';
  for(i = 0; i < 52; i++){
    name[6] = '0' + i;
    unlink(name);
  }
  printf(stdout, "many creates, followed by unlink; ok\n");
}


int 
main(int argc, char *argv[])
{
  writetest();  
  writetest1();
  createtest();
  exit();
}