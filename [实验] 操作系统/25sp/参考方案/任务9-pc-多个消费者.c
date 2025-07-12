#define __LIBRARY__
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
	
#define BUFFER_SIZE 50		/* 缓冲区数量 */
#define NUM_PRODUCE 130		/* 生产产品总数 */
#define NUM_CONSUME 25      /* 每个消费者消费的产品数 */
#define CONSUMERS_COUNT 5	/* 消费者数量 */

#define __NR_sem_open 87
#define __NR_sem_wait 88
#define __NR_sem_post 89
#define __NR_sem_unlink 90

typedef void sem_t;  /* 下面用到sem_t*的，其实质都是指针，故在此将sem_t*类型定义为void*类型，
						就不用在此重新定义sem_t的结构体，因此也方便了对结构体的管理：如果在
						此重新定义此结构体，内核中也已经定义了此结构体，如需对其修改，则同时
						需要修改此处和内存处两处的结构体 */

_syscall2(int, sem_open, const char*, name, unsigned int, value)
_syscall1(int, sem_wait, sem_t *, sem)
_syscall1(int, sem_post, sem_t *, sem)
_syscall1(int, sem_unlink, const char *, name)

int main()
{	
	/* 注意: 在应用程序中不能使用断点等调试功能 */
	int i, j;
	int s, t;
	int consumeNum = 0; /* 消费者消费的产品号 */
	int produceNum = 0; /* 生产者生产的产品号 */
	int consume_pos = 0; /* 消费者从共享缓冲区中取出产品消费的位置 */
	int produce_pos = 0; /* 生产者生产产品向共享缓冲区放入的位置 */
	
	sem_t *empty, *full, *mutex;
	FILE *fp = NULL;
	pid_t producer_pid, consumer_pids[CONSUMERS_COUNT];
	
	/* 创建empty、full、mutex三个信号量 */
	empty = (sem_t*)sem_open("empty", BUFFER_SIZE);
	full  = (sem_t*)sem_open("full", 0);
	mutex = (sem_t*)sem_open("mutex", 1);
	
	/* 用文件建立一个共享缓冲区 */
	fp=fopen("filebuffer.txt", "wb+");
	
	/* 创建生产者进程 */
	if( !fork() )
	{
		producer_pid = getpid();
		printf("Producer pid=%d create success!\n", producer_pid);
		for( i = 0 ; i < NUM_PRODUCE; i++)
		{
			sem_wait(empty);
			sem_wait(mutex);
			
			produceNum = i;
			
			/* 移动文件的游标，向其中放入产品 */
			fseek(fp, produce_pos * sizeof(int), SEEK_SET);
			fwrite(&produceNum, sizeof(int), 1, fp);
			fflush(fp); 
			
			/* 输出生产产品的信息 */
			printf("Producer pid=%d : #%02d at BUF[%d]\n", producer_pid, produceNum, produce_pos); 
			fflush(stdout);
			
			/* 生产者的游标向后移动一个位置 */
			produce_pos = (produce_pos + 1) % BUFFER_SIZE;
			
			sem_post(mutex);
			sem_post(full);
			
			sleep(2);
		}
		exit(0);
	}
	
	/* 创建多个消费者进程 */
	for (s = 0; s < CONSUMERS_COUNT; s++) {
		if( !fork() )
		{
			consumer_pids[s] = getpid();
			printf("\t\t\tConsumer[%d] (pid=%d) create success!\n", s, consumer_pids[s]);
			for( j = 0; j < NUM_CONSUME; j++ ) 
			{
				sem_wait(full);
				sem_wait(mutex);
				
				/* 移动文件的游标，取出其中的产品 */
				fseek(fp, consume_pos * sizeof(int), SEEK_SET);
				fread(&consumeNum, sizeof(int), 1, fp);
				fflush(fp);
				
				/* 输出消费产品的信息 */
				printf("\t\t\tConsumer[%d] (pid=%d): #%02d at BUF[%d]\n", s, consumer_pids[s], consumeNum, consume_pos);
				fflush(stdout);
				
				/* 消费者的游标向后移动一个位置 */
				consume_pos = (consume_pos + 1) % BUFFER_SIZE;
		
				sem_post(mutex);
				sem_post(empty);
				
				if(j<4)	sleep(8);
				else sleep(1);
			}
			exit(0);
		}
	}

	waitpid(producer_pid, NULL, 0);	/* 等待生产者进程结束 */
	/* 等待所有消费者进程结束 */
	for (t = 0; t < CONSUMERS_COUNT; t++) {
        waitpid(consumer_pids[t], NULL, 0);
    }

	/* 关闭所有信号量 */
	sem_unlink("empty");
	sem_unlink("full");
	sem_unlink("mutex");
	
	/* 关闭文件 */
	fclose(fp);
	
	return 0;
}
