//
// File-system system calls.
// Mostly argument checking, since we don't trust
// user code, and calls into file.c and fs.c.
//

#include "types.h"
#include "defs.h"
#include "param.h"
#include "stat.h"
#include "mmu.h"
#include "proc.h"
#include "fs.h"
#include "ext2.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "file.h"
#include "fcntl.h"
#include "vfs.h"
// Fetch the nth word-sized system call argument as a file descriptor
// and return both the descriptor and the corresponding struct file.
static int
argfd(int n, int *pfd, struct file **pf)
{
  int fd;
  struct file *f;

  if(argint(n, &fd) < 0)
    return -1;
  if(fd < 0 || fd >= NOFILE || (f=myproc()->ofile[fd]) == 0)
    return -1;
  if(pfd)
    *pfd = fd;
  if(pf)
    *pf = f;
  return 0;
}

// Allocate a file descriptor for the given file.
// Takes over file reference from caller on success.
static int
fdalloc(struct file *f)
{
  int fd;
  struct proc *curproc = myproc();

  for(fd = 0; fd < NOFILE; fd++){
    if(curproc->ofile[fd] == 0){
      curproc->ofile[fd] = f;
      return fd;
    }
  }
  return -1;
}

int
sys_dup(void)
{
  struct file *f;
  int fd;

  if(argfd(0, 0, &f) < 0)
    return -1;
  if((fd=fdalloc(f)) < 0)
    return -1;
  filedup(f);
  return fd;
}

int
sys_read(void)
{
  struct file *f;
  int n;
  char *p;

  if(argfd(0, 0, &f) < 0 || argint(2, &n) < 0 || argptr(1, &p, n) < 0)
    return -1;
  return fileread(f, p, n);
}

int
sys_write(void)
{
  struct file *f;
  int n;
  char *p;

  if(argfd(0, 0, &f) < 0 || argint(2, &n) < 0 || argptr(1, &p, n) < 0)
    return -1;
  return filewrite(f, p, n);
}

int
sys_close(void)
{
  int fd;
  struct file *f;

  if(argfd(0, &fd, &f) < 0)
    return -1;
  myproc()->ofile[fd] = 0;
  fileclose(f);
  return 0;
}

int
sys_fstat(void)
{
  struct file *f;
  struct stat *st;

  if(argfd(0, 0, &f) < 0 || argptr(1, (void*)&st, sizeof(*st)) < 0)
    return -1;
  return filestat(f, st);
}

// Create the path new as a link to the same inode as old.
int
sys_link(void)
{
  char name[DIRSIZ], *new, *old;
  struct inode *dp, *ip;

  if(argstr(0, &old) < 0 || argstr(1, &new) < 0)
    return -1;

  begin_op();
  if((ip = namei(old)) == 0){
    end_op();
    return -1;
  }

  ip->file_type->iops->ilock(ip);
  if(ip->type == T_DIR){
    ip->file_type->iops->iunlockput(ip);
    end_op();
    return -1;
  }

  ip->nlink++;
  ip->file_type->iops->iupdate(ip);
  ip->file_type->iops->iunlock(ip);

  if((dp = nameiparent(new, name)) == 0)
    goto bad;
  ip->file_type->iops->ilock(dp);
  if(dp->dev != ip->dev || dp->file_type->iops->dirlink(dp, name, ip->inum) < 0){
    ip->file_type->iops->iunlockput(dp);
    goto bad;
  }
  ip->file_type->iops->iunlockput(dp);
  ip->file_type->iops->iput(ip);

  end_op();

  return 0;

bad:
  ip->file_type->iops->ilock(ip);
  ip->nlink--;
  ip->file_type->iops->iupdate(ip);
  ip->file_type->iops->iunlockput(ip);
  end_op();
  return -1;
}

// Is the directory dp empty except for "." and ".." ?
int
isdirempty(struct inode *dp)
{
  int off;
  struct dirent de;

  for(off=2*sizeof(de); off<dp->size; off+=sizeof(de)){
    if(dp->file_type->iops->readi(dp, (char*)&de, off, sizeof(de)) != sizeof(de))
      panic("isdirempty: readi");
    if(de.inum != 0)
      return 0;
  }
  return 1;
}
int
ext2_isdirempty(struct inode *dp)
{
  int off;
  struct ext2_dir_entry_2 de;

  for(off=24; off<dp->size; off+=de.rec_len){
    if(dp->file_type->iops->readi(dp, (char*)&de, off, sizeof(de)) != sizeof(de))
      panic("isdirempty: readi");
    if(de.inode != 0)
      return 0;
  }
  return 1;
}
void
fs_unlink(struct inode *dp, uint off)
{
  struct dirent de;
  memset(&de, 0, sizeof(de));
  if(dp->file_type->iops->writei(dp, (char*)&de, off, sizeof(de)) != sizeof(de))
    panic("unlink: writei");
}
void
ext2_unlink(struct inode *dp, uint off)
{
  struct ext2_dir_entry_2 de, de1;
  if (dp->file_type->iops->readi(dp, (char *)&de1, off, sizeof(de1)) != sizeof(de1))
    panic("dirlookup read");
  memset(&de, 0, sizeof(de));
  de.rec_len = de1.rec_len;
  if(dp->file_type->iops->writei(dp, (char*)&de, off, de1.rec_len) != de1.rec_len)
    panic("unlink: writei");
}
//PAGEBREAK!
int
sys_unlink(void)
{
  struct inode *ip, *dp;
  char name[DIRSIZ], *path;
  uint off;

  if(argstr(0, &path) < 0)
    return -1;

  begin_op();
  if((dp = nameiparent(path, name)) == 0){
    end_op();
    return -1;
  }

  dp->file_type->iops->ilock(dp);

  // Cannot unlink "." or "..".
  if(dp->file_type->iops->namecmp(name, ".") == 0 || dp->file_type->iops->namecmp(name, "..") == 0)
    goto bad;

  if((ip = dp->file_type->iops->dirlookup(dp, name, &off)) == 0)
    goto bad;
  ip->file_type->iops->ilock(ip);

  if(ip->nlink < 1)
    panic("unlink: nlink < 1");
  if(ip->type == T_DIR && !ip->file_type->iops->isdirempty(ip)){
    ip->file_type->iops->iunlockput(ip);
    goto bad;
  }
  dp->file_type->iops->unlink(dp, off);
  if(ip->type == T_DIR){
    dp->nlink--;
    dp->file_type->iops->iupdate(dp);
  }
  dp->file_type->iops->iunlockput(dp);

  ip->nlink--;
  ip->file_type->iops->iupdate(ip);
  ip->file_type->iops->iunlockput(ip);

  end_op();

  return 0;

bad:
  dp->file_type->iops->iunlockput(dp);
  end_op();
  return -1;
}

static struct inode*
create(char *path, short type, short major, short minor)
{
  struct inode *ip, *dp;
  char name[DIRSIZ];
  if((dp = nameiparent(path, name)) == 0)
    return 0;
  dp->file_type->iops->ilock(dp);

  if((ip = dp->file_type->iops->dirlookup(dp, name, 0)) != 0){
    dp->file_type->iops->iunlockput(dp);
    ip->file_type->iops->ilock(ip);
    if(type == T_FILE && ip->type == T_FILE)
      return ip;
    ip->file_type->iops->iunlockput(ip);
    return 0;
  }
  if((ip = dp->file_type->iops->ialloc(dp->dev, type)) == 0)
    panic("create: ialloc");

  ip->file_type->iops->ilock(ip);
  ip->major = major;
  ip->minor = minor;
  ip->nlink = 1;
  ip->file_type->iops->iupdate(ip);

  if(type == T_DIR){  // Create . and .. entries.
    dp->nlink++;  // for ".."
    dp->file_type->iops->iupdate(dp);
    // No ip->nlink++ for ".": avoid cyclic ref count.
    if(ip->file_type->iops->dirlink(ip, ".", ip->inum) < 0 || ip->file_type->iops->dirlink(ip, "..", dp->inum) < 0)
      panic("create dots");
  }

  if(dp->file_type->iops->dirlink(dp, name, ip->inum) < 0)
    panic("create: dirlink");
  dp->file_type->iops->iunlockput(dp);
  return ip;
}

int
sys_open(void)
{
  char *path;
  int fd, omode;
  struct file *f;
  struct inode *ip;

  if(argstr(0, &path) < 0 || argint(1, &omode) < 0)
    return -1;

  begin_op();
  if(omode & O_CREATE){
    ip = create(path, T_FILE, 0, 0);
    if(ip == 0){
      end_op();
      return -1;
    }
  } else {
    if((ip = namei(path)) == 0){
      end_op();
      return -1;
    }
    ip->file_type->iops->ilock(ip);
    if(ip->type == T_DIR && omode != O_RDONLY){
      ip->file_type->iops->iunlockput(ip);
      end_op();
      return -1;
    }
  }
  if((f = filealloc()) == 0 || (fd = fdalloc(f)) < 0){
    if(f)
      fileclose(f);
    ip->file_type->iops->iunlockput(ip);
    end_op();
    return -1;
  }
  ip->file_type->iops->iunlock(ip);
  end_op();

  f->type = FD_INODE;
  f->ip = ip;
  f->off = 0;
  f->readable = !(omode & O_WRONLY);
  f->writable = (omode & O_WRONLY) || (omode & O_RDWR);
  return fd;
}

int
sys_mkdir(void)
{
  char *path;
  struct inode *ip;

  begin_op();
  if(argstr(0, &path) < 0 || (ip = create(path, T_DIR, 0, 0)) == 0){
    end_op();
    return -1;
  }
  ip->file_type->iops->iunlockput(ip);
  end_op();
  return 0;
}

int
sys_mknod(void)
{
  struct inode *ip;
  char *path;
  int major, minor;
  begin_op();
  if((argstr(0, &path)) < 0 ||
     argint(1, &major) < 0 ||
     argint(2, &minor) < 0 ||
     (ip = create(path, T_DEV, major, minor)) == 0){
    end_op();
    return -1;
  }
  ip->file_type->iops->iunlockput(ip);
  end_op();
  return 0;
}

int
sys_chdir(void)
{
  char *path;
  struct inode *ip;
  struct proc *curproc = myproc();
  
  begin_op();
  if(argstr(0, &path) < 0 || (ip = namei(path)) == 0){
    end_op();
    return -1;
  }
  ip->file_type->iops->ilock(ip);
  if(ip->type != T_DIR){
    ip->file_type->iops->iunlockput(ip);
    end_op();
    return -1;
  }
  ip->file_type->iops->iunlock(ip);
  curproc->cwd->file_type->iops->iput(curproc->cwd);
  end_op();
  curproc->cwd = ip;
  return 0;
}

int
sys_exec(void)
{
  char *path, *argv[MAXARG];
  int i;
  uint uargv, uarg;

  if(argstr(0, &path) < 0 || argint(1, (int*)&uargv) < 0){
    return -1;
  }
  memset(argv, 0, sizeof(argv));
  for(i=0;; i++){
    if(i >= NELEM(argv))
      return -1;
    if(fetchint(uargv+4*i, (int*)&uarg) < 0)
      return -1;
    if(uarg == 0){
      argv[i] = 0;
      break;
    }
    if(fetchstr(uarg, &argv[i]) < 0)
      return -1;
  }
  return exec(path, argv);
}

int
sys_pipe(void)
{
  int *fd;
  struct file *rf, *wf;
  int fd0, fd1;

  if(argptr(0, (void*)&fd, 2*sizeof(fd[0])) < 0)
    return -1;
  if(pipealloc(&rf, &wf) < 0)
    return -1;
  fd0 = -1;
  if((fd0 = fdalloc(rf)) < 0 || (fd1 = fdalloc(wf)) < 0){
    if(fd0 >= 0)
      myproc()->ofile[fd0] = 0;
    fileclose(rf);
    fileclose(wf);
    return -1;
  }
  fd[0] = fd0;
  fd[1] = fd1;
  return 0;
}
