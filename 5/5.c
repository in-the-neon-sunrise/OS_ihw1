#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

const ssize_t buf_size = 5000;

ssize_t process_data(char *buf, ssize_t size) {
	int flag[size];
	for (int i = 0; i < size; i++) {
		flag[i]=buf[i];
	}
	int written = 0;
	for (int i = size-1; i >=0; i--){
		written += sprintf(buf + written, "%c", flag[i]);
	}
	return written;
}

int main(int argc, char *argv[]) {
	const char* inputFile = "input.txt";
	const char* outputFile = "output.txt";
	char name1[] = "5/fd1.fifo";
	char name2[] = "5/fd2.fifo";
	int fd, result;
	ssize_t size;
	char buf[buf_size];

	/* Delete old fifo channels */
	if (access(name1, F_OK) == 0) {
		if (unlink(name1) < 0) {
			printf("process 1: Can\'t delete old FIFO1");
			exit(-1);
		}
	}
	if (access(name2, F_OK) == 0) {
		if (unlink(name2) < 0) {
			printf("process 1: Can\'t delete old FIFO1");
			exit(-1);
		}
	}

	/* Open fifo channels */
	 mknod(name1, S_IFIFO | 0666, 0);
	 mknod(name2, S_IFIFO | 0666, 0);

	/* Fork child process 1 */
	result = fork();
	if (result < 0) {
		printf("parent: Can\'t fork child1\n");
		exit(-1);
	} else if (result == 0) { 
	/* Child process 1 */

		/* Open input file */
		if ((fd = open(inputFile, O_RDONLY)) < 0) {
			printf("child1: Can\'t open input file\n");
			exit(-1);
		}

		/* Read from input file and close it*/
		size = read(fd, buf, buf_size);
		if (size == -1) {
			printf("child1: Can\'t read from input file\n");
			exit(-1);
		}
		if (close(fd) < 0) {
			printf("child1: Can\'t close input file\n");
			exit(-1);
		}

		/* Write to fifo1 and close it*/
		if ((fd = open(name1, O_WRONLY)) < 0) {
			 printf("child1: Can\'t open FIFO1 for writting\n");
			 exit(-1);
		}
		if (write(fd, buf, size) < 0) {
			printf("child1: Can\'t write to FIFO1\n");
			exit(-1);
		}
		if (close(fd) < 0) {
			printf("child1: Can\'t close FIFO1\n");
			exit(-1);
		}
		//fd = open("5/a.txt", O_WRONLY);
		printf("child1 done\n");
		return 0;
	}

	/* Fork child process 2 */
	result = fork();
	if (result < 0) {
		printf("parent: Can\'t fork child2\n");
		exit(-1);
	} else if (result == 0) { 
	/* Child process 2 */

		/* Read data from fifo2 and close it*/
		if ((fd = open(name2, O_RDONLY)) < 0) {
			 printf("child2: Can\'t open FIFO2 for reading\n");
			 exit(-1);
		}
		size = read(fd, buf, buf_size);
		if (size < 0) {
			printf("child2: Can\'t read from FIFO2\n");
			exit(-1);
		}
		if (close(fd) < 0) {
			printf("child2: Can\'t close FIFO2\n");
			exit(-1);
		}

		/* Open output file */
			if ((fd = open(outputFile, O_WRONLY | O_CREAT | O_TRUNC, 0600)) < 0) {
			printf("child2: Can\'t open output file\n");
			exit(-1);
		}

		/* Write to output file and close it*/
		if (write(fd, buf, size) == -1) {
			printf("child2: Can\'t write to output file\n");
			exit(-1);
		}
		if (close(fd) < 0) {
			printf("child2: Can\'t close output file\n");
			exit(-1);
		}
		printf("child2 done\n");

		return 0;
	}

/* Parent process */

	/* Read data from fifo1 and close it*/
	if ((fd = open(name1, O_RDONLY)) < 0) {
		 printf("parent: Can\'t open FIFO1 for reading\n");
		 exit(-1);
	}
	size = read(fd, buf, buf_size);
	if (size < 0) {
		printf("parent: Can\'t read from FIFO1\n");
		exit(-1);
	}
	if (close(fd) < 0) {
		printf("parent: Can\'t close FIFO1\n");
		exit(-1);
	}

	/* Process data */
	size = process_data(buf, size);

	/* Write to fifo2 and close it*/
	if ((fd = open(name2, O_WRONLY)) < 0) {
		 printf("parent: Can\'t open FIFO2 for writting\n");
		 exit(-1);
	}
	if (write(fd, buf, size) < 0) {
		printf("parent: Can\'t write to FIFO2\n");
		exit(-1);
	}
	if (close(fd) < 0) {
		printf("parent: Can\'t close FIFO2\n");
		exit(-1);
	}
	printf("parent done\n");

	return 0;
}