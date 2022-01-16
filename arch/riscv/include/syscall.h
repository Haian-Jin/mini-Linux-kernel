#define SYS_WRITE   64
#define SYS_GETPID  172


int sys_getpid();
void sys_write(unsigned int fd, const char* buf, unsigned int count);