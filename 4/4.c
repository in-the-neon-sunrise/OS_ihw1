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
	int fd, fd1[2], fd2[2], result;
	ssize_t size;
	char buf[buf_size];

	/* Open pipes */
	if (pipe(fd1) < 0) {
		printf("parent: Can\'t open pipe1\n");
		exit(-1);
	}
	if (pipe(fd2) < 0) {
		printf("parent: Can\'t open pipe2\n");
		exit(-1);
	}

	/* Fork child process 1 */
	result = fork();
	if (result < 0) {
		printf("parent: Can\'t fork child1\n");
		exit(-1);
	} else if (result == 0) { 
	/* Child process 1 */
		/* Close unused pipes */
		if (close(fd1[0]) < 0){
			printf("child1: Can\'t close reading side of pipe1\n");
			exit(-1);
		}
		if (close(fd2[0]) < 0){
			printf("child1: Can\'t close reading side of pipe2\n");
			exit(-1);
		}
		if (close(fd2[1]) < 0){
			printf("child1: Can\'t close writing side of pipe2\n");
			exit(-1);
		}

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

		/* Send data to pipe1 and close it*/
		if (write(fd1[1], buf, size) < 0) {
			printf("child1: Can\'t write to pipe1\n");
			exit(-1);
		}  
		if (close(fd1[1]) < 0){
			printf("child1: Can\'t close writing side of pipe1\n");
			exit(-1);
		}

		return 0;
	}

	/* Fork child process 2 */
	result = fork();
	if (result < 0) {
		printf("parent: Can\'t fork child2\n");
		exit(-1);
	} else if (result == 0) { 
	/* Child process 2 */
		/* Close unused pipes */
		if (close(fd1[0]) < 0){
			printf("child2: Can\'t close reading side of pipe1\n");
			exit(-1);
		}
		if (close(fd1[1]) < 0){
			printf("child2: Can\'t close writing side of pipe1\n");
			exit(-1);
		}
		if (close(fd2[1]) < 0){
			printf("child2: Can\'t close writing side of pipe2\n");
			exit(-1);
		}

		/* Read data from pipe2 and close it */
		size = read(fd2[0], buf, buf_size);
		if (size < 0) {
			printf("child2: Can\'t read from pipe2\n");
			exit(-1);
		}  
		if (close(fd2[0]) < 0){
			printf("child2: Can\'t close reading side of pipe2\n");             exit(-1);
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

		return 0;
	}

	/* Parent process */
	/* Close unused pipes */
	if (close(fd1[1]) < 0) {
		printf("parent: Can\'t close writing side of pipe1\n");
		exit(-1);
	}
	if (close(fd2[0]) < 0) {
		printf("parent: Can\'t close reading side of pipe2\n");
		exit(-1);
	}

	/* Read data from pipe1 and close it */
	size = read(fd1[0], buf, buf_size);
	if (size < 0) {
		printf("parent: Can\'t read from pipe1\n");
		exit(-1);
	}
	if (close(fd1[0]) < 0) {
		printf("parent: Can\'t close reading side of pipe1\n");
		exit(-1);
	}

	/* Process data */
	size = process_data(buf, size);

	/* Write to pipe2 and close it */
	if (write(fd2[1], buf, size) == -1) {
		printf("parent: Can\'t write to pipe2\n");
		exit(-1);
	}
	else {
		printf("ok");
}
	if (close(fd2[1]) < 0) {
		printf("parent: Can\'t close writing side of pipe2\n");
		exit(-1);
	}

	return 0;
}