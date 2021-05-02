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


// does chdir() call iput(p->cwd) in a transaction?
void
iputtest(void)
{
  printf(stdout, "iput test\n");

  if(mkdir("/mnt/iputdir") < 0){
    printf(stdout, "mkdir failed\n");
    exit();
  }
  if(chdir("mnt/iputdir") < 0){
    printf(stdout, "chdir iputdir failed\n");
    exit();
  }
  if(unlink("/mnt/iputdir") < 0){
    printf(stdout, "unlink ../iputdir failed\n");
    exit();
  }
  if(chdir("/mnt") < 0){
    printf(stdout, "chdir / failed\n");
    exit();
  }
  printf(stdout, "iput test ok\n");
}
void
openiputtest(void)
{
  int pid;

  printf(stdout, "openiput test\n");
  if(mkdir("mnt/oidir") < 0){
    printf(stdout, "mkdir oidir failed\n");
    exit();
  }
  pid = fork();
  if(pid < 0){
    printf(stdout, "fork failed\n");
    exit();
  }
  if(pid == 0){
    int fd = open("mnt/oidir", O_RDWR);
    if(fd >= 0){
      printf(stdout, "open directory for write succeeded\n");
      exit();
    }
    exit();
  }
  sleep(1);
  if(unlink("mnt/oidir") != 0){
    printf(stdout, "unlink failed\n");
    exit();
  }
  wait();
  printf(stdout, "openiput test ok\n");
}

// simple file system tests

void
opentest(void)
{
  int fd;

  printf(stdout, "open test\n");
  fd = open("doesnotexist", 0);
  if(fd >= 0){
    printf(stdout, "open doesnotexist succeeded!\n");
    exit();
  }
  printf(stdout, "open test ok\n");
}

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
    if(write(fd, buf, 2048) != 2048){
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
    i = read(fd, buf, 2048);
    if(i == 0){
      if(n == MAXFILE - 1){
        printf(stdout, "read only %d blocks from big", n);
        exit();
      }
      break;
    } else if(i != 2048){
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
  printf(stdout, "many creates done\n");
  name[5] = 'a';
  name[7] = '\0';
  for(i = 0; i < 10; i++){
    name[6] = '0' + i;
    unlink(name);
  }
  printf(stdout, "many creates, followed by unlink; ok\n");
}

// More file system tests

// two processes write to the same file descriptor
// is the offset shared? does inode locking work?
void
sharedfd(void)
{
  int fd, pid, i, n, nc, np;
  char buf[10];

  printf(1, "sharedfd test\n");

  unlink("sharedfd");
  fd = open("sharedfd", O_CREATE|O_RDWR);
  if(fd < 0){
    printf(1, "fstests: cannot open sharedfd for writing");
    return;
  }
  pid = fork();
  memset(buf, pid==0?'c':'p', sizeof(buf));
  for(i = 0; i < 1000; i++){
    if(write(fd, buf, sizeof(buf)) != sizeof(buf)){
      printf(1, "fstests: write sharedfd failed\n");
      break;
    }
  }
  if(pid == 0)
    exit();
  else
    wait();
  close(fd);
  fd = open("sharedfd", 0);
  if(fd < 0){
    printf(1, "fstests: cannot open sharedfd for reading\n");
    return;
  }
  nc = np = 0;
  while((n = read(fd, buf, sizeof(buf))) > 0){
    for(i = 0; i < sizeof(buf); i++){
      if(buf[i] == 'c')
        nc++;
      if(buf[i] == 'p')
        np++;
    }
  }
  close(fd);
  unlink("sharedfd");
  if(nc == 10000 && np == 10000){
    printf(1, "sharedfd ok\n");
  } else {
    printf(1, "sharedfd oops %d %d\n", nc, np);
    exit();
  }
}
// four processes write different files at the same
// time, to test block allocation.

// four processes create and delete different files in same directory
void
createdelete(void)
{
  enum { N = 20 };
  int pid, i, fd, pi;
  char name[32];

  printf(1, "createdelete test\n");
  name[0] = 'm';name[1] = 'n';name[2] = 't';name[3] = '/';
  for(pi = 0; pi < 4; pi++){
    pid = fork();
    if(pid < 0){
      printf(1, "fork failed\n");
      exit();
    }

    if(pid == 0){
      name[4] = 'p' + pi;
      name[6] = '\0';
      for(i = 0; i < N; i++){
        name[5] = '0' + i;
        fd = open(name, O_CREATE | O_RDWR);
        if(fd < 0){
          printf(1, "create failed\n");
          exit();
        }
        close(fd);
        if(i > 0 && (i % 2 ) == 0){
          name[5] = '0' + (i / 2);
          if(unlink(name) < 0){
            printf(1, "unlink failed\n");
            exit();
          }
        }
      }
      exit();
    }
  }

  for(pi = 0; pi < 4; pi++){
    wait();
  }

  for(i = 0; i < N; i++){
    for(pi = 0; pi < 4; pi++){
      name[4] = 'p' + pi;
      name[5] = '0' + i;
      fd = open(name, 0);
      if((i == 0 || i >= N/2) && fd < 0){
        printf(1, "oops createdelete %s didn't exist\n", name);
        exit();
      } else if((i >= 1 && i < N/2) && fd >= 0){
        printf(1, "oops createdelete %s did exist\n", name);
        exit();
      }
      if(fd >= 0)
        close(fd);
    }
  }

  for(i = 0; i < N; i++){
    for(pi = 0; pi < 4; pi++){
      name[4] = 'p' + i;
      name[5] = '0' + i;
      unlink(name);
    }
  }

  printf(1, "createdelete ok\n");
}

// can I unlink a file and still read it?
void
unlinkread(void)
{
  int fd, fd1;

  printf(1, "unlinkread test\n");
  fd = open("/mnt/unlinkread", O_CREATE | O_RDWR);
  if(fd < 0){
    printf(1, "create unlinkread failed\n");
    exit();
  }
  write(fd, "hello", 5);
  close(fd);

  fd = open("/mnt/unlinkread", O_RDWR);
  if(fd < 0){
    printf(1, "open unlinkread failed\n");
    exit();
  }
  if(unlink("/mnt/unlinkread") != 0){
    printf(1, "unlink unlinkread failed\n");
    exit();
  }

  fd1 = open("/mnt/unlinkread", O_CREATE | O_RDWR);
  write(fd1, "yyy", 3);
  close(fd1);

  if(read(fd, buf, sizeof(buf)) != 5){
    printf(1, "unlinkread read failed");
    exit();
  }
  if(buf[0] != 'h'){
    printf(1, "unlinkread wrong data\n");
    exit();
  }
  if(write(fd, buf, 10) != 10){
    printf(1, "unlinkread write failed\n");
    exit();
  }
  close(fd);
  unlink("unlinkread");
  printf(1, "unlinkread ok\n");
}
void
linktest(void)
{
  int fd;

  printf(1, "linktest\n");

  unlink("/mnt/lf1");
  unlink("/mnt/lf2");

  fd = open("/mnt/lf1", O_CREATE|O_RDWR);
  if(fd < 0){
    printf(1, "create lf1 failed\n");
    exit();
  }
  if(write(fd, "hello", 5) != 5){
    printf(1, "write lf1 failed\n");
    exit();
  }
  close(fd);

  if(link("/mnt/lf1", "/mnt/lf2") < 0){
    printf(1, "link lf1 lf2 failed\n");
    exit();
  }
  unlink("/mnt/lf1");

  if(open("/mnt/lf1", 0) >= 0){
    printf(1, "unlinked lf1 but it is still there!\n");
    exit();
  }

  fd = open("/mnt/lf2", 0);
  if(fd < 0){
    printf(1, "open lf2 failed\n");
    exit();
  }
  if(read(fd, buf, sizeof(buf)) != 5){
    printf(1, "read lf2 failed\n");
    exit();
  }
  close(fd);

  if(link("/mnt/lf2", "/mnt/lf2") >= 0){
    printf(1, "link lf2 lf2 succeeded! oops\n");
    exit();
  }

  unlink("/mnt/lf2");
  if(link("/mnt/lf2", "/mnt/lf1") >= 0){
    printf(1, "link non-existant succeeded! oops\n");
    exit();
  }

  if(link("mnt/.", "lf1") >= 0){
    printf(1, "link . lf1 succeeded! oops\n");
    exit();
  }

  printf(1, "linktest ok\n");
}

void
subdir(void)
{
  int fd, cc;

  printf(1, "subdir test\n");

  unlink("mnt/ff");
  if(mkdir("mnt/dd") != 0){
    printf(1, "subdir mkdir mnt/dd failed\n");
    exit();
  }

  fd = open("mnt/dd/ff", O_CREATE | O_RDWR);
  if(fd < 0){
    printf(1, "create mnt/dd/ff failed\n");
    exit();
  }
  write(fd, "ff", 2);
  close(fd);

  if(unlink("mnt/dd") >= 0){
    printf(1, "unlink mnt/dd (non-empty dir) succeeded!\n");
    exit();
  }

  if(mkdir("mnt/dd/dd") != 0){
    printf(1, "subdir mkdir mnt/dd/dd failed\n");
    exit();
  }

  fd = open("mnt/dd/dd/ff", O_CREATE | O_RDWR);
  if(fd < 0){
    printf(1, "create mnt/dd/dd/ff failed\n");
    exit();
  }
  write(fd, "FF", 2);
  close(fd);

  fd = open("mnt/dd/dd/../ff", 0);
  if(fd < 0){
    printf(1, "open mnt/dd/dd/../ff failed\n");
    exit();
  }
  cc = read(fd, buf, sizeof(buf));
  if(cc != 2 || buf[0] != 'f'){
    printf(1, "mnt/dd/dd/../ff wrong content\n");
    exit();
  }
  close(fd);

  if(link("mnt/dd/dd/ff", "mnt/dd/dd/ffff") != 0){
    printf(1, "link mnt/dd/dd/ff mnt/dd/dd/ffff failed\n");
    exit();
  }

  if(unlink("mnt/dd/dd/ff") != 0){
    printf(1, "unlink mnt/dd/dd/ff failed\n");
    exit();
  }
  if(open("mnt/dd/dd/ff", O_RDONLY) >= 0){
    printf(1, "open (unlinked) mnt/dd/dd/ff succeeded\n");
    exit();
  }

  if(chdir("mnt/dd") != 0){
    printf(1, "chdir mnt/dd failed\n");
    exit();
  }
  if(chdir("mnt/dd/../../dd") != 0){
    printf(1, "chdir mnt/dd/../../dd failed\n");
    exit();
  }
  if(chdir("mnt/dd/../../../dd") != 0){
    printf(1, "chdir mnt/dd/../../dd failed\n");
    exit();
  }
  if(chdir("mnt/./..") != 0){
    printf(1, "chdir ./.. failed\n");
    exit();
  }

  fd = open("mnt/dd/dd/ffff", 0);
  if(fd < 0){
    printf(1, "open mnt/dd/dd/ffff failed\n");
    exit();
  }
  if(read(fd, buf, sizeof(buf)) != 2){
    printf(1, "read mnt/dd/dd/ffff wrong len\n");
    exit();
  }
  close(fd);

  if(open("mnt/dd/dd/ff", O_RDONLY) >= 0){
    printf(1, "open (unlinked) mnt/dd/dd/ff succeeded!\n");
    exit();
  }

  if(open("mnt/dd/ff/ff", O_CREATE|O_RDWR) >= 0){
    printf(1, "create mnt/dd/ff/ff succeeded!\n");
    exit();
  }
  if(open("mnt/dd/xx/ff", O_CREATE|O_RDWR) >= 0){
    printf(1, "create mnt/dd/xx/ff succeeded!\n");
    exit();
  }
  if(open("mnt/dd", O_CREATE) >= 0){
    printf(1, "create mnt/dd succeeded!\n");
    exit();
  }
  if(open("mnt/dd", O_RDWR) >= 0){
    printf(1, "open mnt/dd rdwr succeeded!\n");
    exit();
  }
  if(open("mnt/dd", O_WRONLY) >= 0){
    printf(1, "open mnt/dd wronly succeeded!\n");
    exit();
  }
  if(link("mnt/dd/ff/ff", "mnt/dd/dd/xx") == 0){
    printf(1, "link mnt/dd/ff/ff dd/dd/xx succeeded!\n");
    exit();
  }
  if(link("mnt/dd/xx/ff", "mnt/dd/dd/xx") == 0){
    printf(1, "link mnt/dd/xx/ff mnt/dd/dd/xx succeeded!\n");
    exit();
  }
  if(link("mnt/dd/ff", "mnt/dd/dd/ffff") == 0){
    printf(1, "link mnt/dd/ff mnt/dd/dd/ffff succeeded!\n");
    exit();
  }
  if(mkdir("mnt/dd/ff/ff") == 0){
    printf(1, "mkdir mnt/dd/ff/ff succeeded!\n");
    exit();
  }
  if(mkdir("mnt/dd/xx/ff") == 0){
    printf(1, "mkdir mnt/dd/xx/ff succeeded!\n");
    exit();
  }
  if(mkdir("mnt/dd/dd/ffff") == 0){
    printf(1, "mkdir mnt/dd/dd/ffff succeeded!\n");
    exit();
  }
  if(unlink("dd/xx/ff") == 0){
    printf(1, "unlink dd/xx/ff succeeded!\n");
    exit();
  }
  if(unlink("mnt/dd/ff/ff") == 0){
    printf(1, "unlink mnt/dd/ff/ff succeeded!\n");
    exit();
  }
  if(chdir("mnt/dd/ff") == 0){
    printf(1, "chdir dd/ff succeeded!\n");
    exit();
  }
  if(chdir("mnt/dd/xx") == 0){
    printf(1, "chdir dd/xx succeeded!\n");
    exit();
  }

  if(unlink("mnt/dd/dd/ffff") != 0){
    printf(1, "unlink mnt/dd/dd/ff failed\n");
    exit();
  }
  if(unlink("mnt/dd/ff") != 0){
    printf(1, "unlink mnt/dd/ff failed\n");
    exit();
  }
  if(unlink("mnt/dd") == 0){
    printf(1, "unlink non-empty mnt/dd succeeded!\n");
    exit();
  }
  if(unlink("mnt/dd/dd") < 0){
    printf(1, "unlink mnt/dd/dd failed\n");
    exit();
  }
  if(unlink("mnt/dd") < 0){
    printf(1, "unlink mnt/dd failed\n");
    exit();
  }

  printf(1, "subdir ok\n");
}
void
bigfile(void)
{
  int fd, i, total, cc;

  printf(1, "bigfile test\n");

  unlink("/mnt/bigfile");
  fd = open("/mnt/bigfile", O_CREATE | O_RDWR);
  if(fd < 0){
    printf(1, "cannot create bigfile");
    exit();
  }
  for(i = 0; i < 20; i++){
    memset(buf, i, 600);
    if(write(fd, buf, 600) != 600){
      printf(1, "write bigfile failed\n");
      exit();
    }
  }
  close(fd);

  fd = open("/mnt/bigfile", 0);
  if(fd < 0){
    printf(1, "cannot open bigfile\n");
    exit();
  }
  total = 0;
  for(i = 0; ; i++){
    cc = read(fd, buf, 300);
    if(cc < 0){
      printf(1, "read bigfile failed\n");
      exit();
    }
    if(cc == 0)
      break;
    if(cc != 300){
      printf(1, "short read bigfile\n");
      exit();
    }
    if(buf[0] != i/2 || buf[299] != i/2){
      printf(1, "read bigfile wrong data\n");
      exit();
    }
    total += cc;
  }
  close(fd);
  if(total != 20*600){
    printf(1, "read bigfile wrong total\n");
    exit();
  }
  unlink("/mnt/bigfile");

  printf(1, "bigfile test ok\n");
}
void
dirfile(void)
{
  int fd;

  printf(1, "dir vs file\n");

  fd = open("/mnt/dirfile", O_CREATE);
  if(fd < 0){
    printf(1, "create /mnt/dirfile failed\n");
    exit();
  }
  close(fd);
  if(chdir("mnt/dirfile") == 0){
    printf(1, "chdir dirfile succeeded!\n");
    exit();
  }
  fd = open("/mnt/dirfile/xx", 0);
  if(fd >= 0){
    printf(1, "create /mnt/dirfile/xx succeeded!\n");
    exit();
  }
  fd = open("/mnt/dirfile/xx", O_CREATE);
  if(fd >= 0){
    printf(1, "create /mnt/dirfile/xx succeeded!\n");
    exit();
  }
  if(mkdir("/mnt/dirfile/xx") == 0){
    printf(1, "mkdir /mnt/dirfile/xx succeeded!\n");
    exit();
  }
  if(unlink("dirfile/xx") == 0){
    printf(1, "unlink dirfile/xx succeeded!\n");
    exit();
  }
  if(link("/mnt/README", "/mnt/dirfile/xx") == 0){
    printf(1, "link to /mnt/dirfile/xx succeeded!\n");
    exit();
  }
  if(unlink("/mnt/dirfile") != 0){
    printf(1, "unlink /mnt/dirfile failed!\n");
    exit();
  }

  fd = open("/mnt/.", O_RDWR);
  if(fd >= 0){
    printf(1, "open /mnt/. for writing succeeded!\n");
    exit();
  }
  fd = open("/mnt/.", 0);
  if(write(fd, "x", 1) > 0){
    printf(1, "write /mnt/. succeeded!\n");
    exit();
  }
  close(fd);

  printf(1, "dir vs file OK\n");
}

void
iref(void)
{
  int i, fd;

  printf(1, "empty file name\n");

  // the 50 is NINODE
  for(i = 0; i < 50 + 1; i++){
    if(i == 0)  {
      if(mkdir("/mnt/irefd") != 0){
        printf(1, "mkdir /mnt/irefd failed\n");
        exit();
      }
      if(chdir("/mnt/irefd") != 0){
        printf(1, "chdir /mnt/irefd failed\n");
        exit();
      }
    }
    else{
      if(mkdir("irefd") != 0){
        printf(1, "mkdir irefd failed\n");
        exit();
      }
      if(chdir("irefd") != 0) {
        printf(1, "chdir irefd failed\n");
        exit();
      }
    }

    mkdir("");
    link("README", "");
    fd = open("", O_CREATE);
    if(fd >= 0)
      close(fd);
    fd = open("xx", O_CREATE);
    if(fd >= 0)
      close(fd);
    unlink("xx");
  }

  chdir("/");
  printf(1, "empty file name OK\n");
}

int 
main(int argc, char *argv[])
{
  if(open("usertests.ran", 0) >= 0){
    printf(1, "already ran user tests -- rebuild ext2.img\n");
    exit();
  }
  close(open("usertests.ran", O_CREATE));
  sharedfd();
  opentest();
  writetest();
  writetest1();
  openiputtest();
  iputtest();
  bigfile();
  subdir();
  linktest();
  unlinkread();
  dirfile();
  iref();
  printf(stdout, "ALL TESTS PASSED\n");
  exit();
}