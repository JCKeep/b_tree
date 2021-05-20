#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

struct Test
{
   int a;
   char c[100];
};

int main()
{
        int fd;
        struct  Test  data[2] = {{1, "bitchffregeberagreagaegggaegggg"},{2, "bitch"}};
        struct  Test  data1[2];

        fd = open("file1", O_RDWR | O_CREAT);

        int n_write = write(fd, &data, sizeof(struct Test) * 2);
        lseek(fd,0,SEEK_SET);

        int n_read = pread(fd, &data1, sizeof(struct Test) * 2, 0);
        printf("read:%d,%s\n",data1[0].a,data1[0].c);
	printf("read:%d,%s\n",data1[1].a,data1[1].c);
        close(fd);
        return 0;
}


