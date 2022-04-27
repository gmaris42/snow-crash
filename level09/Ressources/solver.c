#include <stdio.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char **argv)
{
	
	if (argc != 2)
		return 1;
	int fd = open(argv[1], O_RDONLY);
	if (fd == -1)
	{
		printf("open fail\n");
		return 1;
	}
	char buff[100];
	bzero(buff, 100);

	int ret = read(fd, buff, 100);
	close(fd);
	int i = 0;
	buff[ret - 1] = '\0';//-1 cause of the \n at the end
	printf("coded token = [%s]\n", buff);
	while (i < ret - 1) //-1 cause of the \n at the end
	{
		buff[i] -= i;
		++i;
	}
	printf("decoded token = [%s]\n", buff);
}