#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <ctype.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <regex.h>
#include <pthread.h>


#define tofindSum   "^sum\\(([^)]*)\\)[,]*$"
#define tofindBPrime   "^bprime\\(([^)]*)\\)[,]*$"
#define tofindLcm   "^lcm\\(([^)]*)\\)[,]*$"

#define tofindSub   "^sub\\(([^)]*)\\)[,]*$"
#define tofindMul   "^mul\\(([^)]*)\\)[,]*$"
#define tofindDiv   "^div\\(([^)]*)\\)[,]*$"
#define HISTORY_COUNT 20

extern int errno;
int totalForSum = 0;

typedef void(*sighandler_t)(int);
static char *my_argv[100], *my_envp[100];
static char *search_path[10];

int  current = 0;
void handle_signal(int signo)
{
	printf("\nkxg15840 $> ");
	fflush(stdout);
}

void fill_argv(char *tmp_argv)
{

	char *foo = tmp_argv;
	int index = 0;
	char ret[100];
	bzero(ret, 100);
	while (*foo != '\0') {
		if (index == 10)
			break;

		if (*foo == ' ') {
			if (my_argv[index] == NULL)
				my_argv[index] = (char *)malloc(sizeof(char) * strlen(ret) + 1);
			else {
				bzero(my_argv[index], strlen(my_argv[index]));
			}
			strncpy(my_argv[index], ret, strlen(ret));
			strncat(my_argv[index], "\0", 1);
			bzero(ret, 100);
			index++;
		}
		else {
			strncat(ret, foo, 1);
		}
		foo++;
		/*printf("foo is %c\n", *foo);*/
	}
	my_argv[index] = (char *)malloc(sizeof(char) * strlen(ret) + 1);
	strncpy(my_argv[index], ret, strlen(ret));
	strncat(my_argv[index], "\0", 1);
}


void call_execve(char *cmd)
{
	int i;
	printf("cmd is %s\n", cmd);
	if (fork() == 0) {
		i = execve(cmd, my_argv, my_envp);
		printf("errno is %d\n", errno);
		if (i < 0) {
			printf("%s: %s\n", cmd, "command not found");
			exit(1);
		}
	}
	else {
		wait(NULL);
	}
}

void free_argv()
{
	int index;
	for (index = 0; my_argv[index] != NULL; index++) {
		bzero(my_argv[index], strlen(my_argv[index]) + 1);
		my_argv[index] = NULL;
		free(my_argv[index]);
	}
}


// Sum operation
void *getSum(void *arg){
	int *val_p = (int *)arg;
	int sum = 0;
	int i = 0;
	for (i = 0; i < totalForSum; i++){
		sum += val_p[i];
	}
	printf("%d\n", sum);
}

// Substraction operation
void *getSub(void *arg){
	int *val_p = (int *)arg;
	int sub = 0;
	int i = 0;
	for (i = 0; i < totalForSum; i++){
		sub = val_p[i]-sub;
	}
	printf("%d\n", sub);
}

// Multiplication operation
void *getMul(void *arg){
	int *val_p = (int *)arg;
	int mul = 1;
	int i = 0;
	for (i = 0; i < totalForSum; i++){
		mul = val_p[i]*mul;
	}
	printf("%d\n", mul);
}

// Divison operation
void *getDiv(void *arg){
	int *val_p = (int *)arg;
	int div = 0;
	int i = 0;
	for (i = 0; i < totalForSum; i++){
		div = div/val_p[i];
	}
	printf("%d\n", div);
}

//Biggest Prime operation
void *biggestPrime(void *params)
{
	int  *numb = (int *)params;
	int primeNumber = 0, status = 0;
	int currentNumber = 0;
	int i = 0;
	while (currentNumber< 4)
	{
		int num = numb[currentNumber];
		status = 0;

		for (i = 2; i <= num / 2; ++i)
		{
			if (num%i == 0)
			{
				status = 1;
				break;
			}
		}
		if ((status == 0) && primeNumber<num)
		{
			primeNumber = num;
		}
		currentNumber++;
	}
	if (primeNumber != 0)
	{
		printf("%d\n", primeNumber);
	}
	else
	{
		printf("No prime number's are found in the Given numbers\n");
	}
}

//Lcm operation
void *lcm(void *params)
{
	int  *numb = (int *)params;
	int lcmvalue = 0;
	int TempNumb = numb[0];
	int TempSecondNumb = numb[1];

	while (TempNumb != TempSecondNumb)
	{

		if (TempNumb>TempSecondNumb)
			TempNumb -= TempSecondNumb;
		else
			TempSecondNumb -= TempNumb;


	}
	lcmvalue = (numb[0] * numb[1]) / TempNumb;


	TempNumb = numb[2];
	TempSecondNumb = numb[3];


	while (TempNumb != TempSecondNumb)
	{

		if (TempNumb>TempSecondNumb)
			TempNumb -= TempSecondNumb;
		else
			TempSecondNumb -= TempNumb;


	}
	int lcmvalue2 = (numb[2] * numb[3]) / TempNumb;

	TempNumb = lcmvalue;
	TempSecondNumb = lcmvalue2;


	while (TempNumb != TempSecondNumb)
	{

		if (TempNumb>TempSecondNumb)
			TempNumb -= TempSecondNumb;
		else
			TempSecondNumb -= TempNumb;


	}
	lcmvalue = (lcmvalue*lcmvalue2) / TempNumb;


	printf("\nThe LCM: %d ", lcmvalue);

}

int history(char *hist[],int current)
{
		
        int i = current;
        int hist_num = 1;

        do {
                if (hist[i]) {
                        printf("%s\n", hist[i]);
                        hist_num++;
                }

                i = (i + 1) % HISTORY_COUNT;

        } while (i != current);

        return 0;
}

void parseCommand(char *command, char *hist[]) {
	char* substr;
	char* quitStr = "quit";
	const char *ptr = strchr(command, '(');
	if (ptr) {
		if (command[strlen(command) - 1] == '\n')
            command[strlen(command) - 1] = '\0';
			//free(hist[current]);
		int index = ptr - command;
		char* otherString = 0;
		char* sumStr = "sum";
		char* bprimeStr = "bprime";
		char* lcmStr = "lcm";
		char* subStr = "sub";
		char* mulStr = "mul";
		char* divStr = "div";
		char* sqrtStr = "sqrt";
		char* historyStr = "history";
		otherString = (char*)malloc(index);
		memcpy(otherString, command, index);
		otherString[index + 1] = 0;
		if (strcasecmp(otherString, sumStr) == 0) {

			int result = sumchildProcess(command);
			if(result > 0) {
			//Logic to trace the history.
			hist[current] = strdup(command);
			current = (current + 1) % HISTORY_COUNT;		
			}

		}
		else if (strcasecmp(otherString, bprimeStr) == 0) {
			int result = bPrimechildProcess(command);
			if(result > 0) {
			//Logic to trace the history.
			hist[current] = strdup(command);
			current = (current + 1) % HISTORY_COUNT;		
			}
		}
		else if (strcasecmp(otherString, lcmStr) == 0) {
			int result = lcmchildProcess(command);
			if(result > 0) {
			//Logic to trace the history.
			hist[current] = strdup(command);
			current = (current + 1) % HISTORY_COUNT;		
			}
		}
		else if (strcasecmp(otherString, subStr) == 0) {
			int result = subchildProcess(command);
			if(result > 0) {
			//Logic to trace the history.
			hist[current] = strdup(command);
			current = (current + 1) % HISTORY_COUNT;		
			}
		}
		else if (strcasecmp(otherString, mulStr) == 0) {
			int result = mulchildProcess(command);
			if(result > 0) {
			//Logic to trace the history.
			hist[current] = strdup(command);
			current = (current + 1) % HISTORY_COUNT;		
			}
		}
		else if (strcasecmp(otherString, divStr) == 0) {
			int result = divchildProcess(command);
			if(result > 0) {
			//Logic to trace the history.
			hist[current] = strdup(command);
			current = (current + 1) % HISTORY_COUNT;		
			}
		}
		else if (strcasecmp(otherString, sqrtStr) == 0) {
			printf("\n sqrt is ... %s\n", otherString);
		}
		
	}
	if (strcasecmp(command, "sum") == 0) {
		printf("Please enter proper parameters: sum(parameter1, parameter2....\n");
	}
	if (strcasecmp(command, "bprime") == 0) {
		printf("Please enter proper parameters: bprime(parameter1, parameter2....\n");
	}
	if (strcasecmp(command, "lcm") == 0) {
		printf("Please enter proper parameters: lcm(parameter1, parameter2....\n");
	}
	if (strcasecmp(command, "sub") == 0) {
		printf("Please enter proper parameters: sub(parameter1, parameter2....\n");
	}
	if (strcasecmp(command, "mul") == 0) {
		printf("Please enter proper parameters: mul(parameter1, parameter2....\n");
	}
	if (strcasecmp(command, "div") == 0) {
		printf("Please enter proper parameters: div(parameter1, parameter2\n");
	}
	if (strcasecmp(command, quitStr) == 0) {
		printf("bye\n");
		exit(0);
	}
	if (strcasecmp(command, "history") == 0) {
			history(hist,current);
			
		}

}

/* Child process for sum:
*  When fork() returns 0, we are in
*  the child process.
*/
int sumchildProcess(char *line) {
	int arr[7];
	int i = 0;
	pthread_t thread1, thread2;
	int  iret1, iret2;
	regex_t re;
	regmatch_t rm[2];
	int retval = 0;
	int pid = fork();
	if (pid == -1) {
		/* Error:
		* When fork() returns -1, an error happened
		* (for example, number of processes reached the limit).
		*/
		fprintf(stderr, "can't fork, error %d\n", errno);
		exit(EXIT_FAILURE);
	}
	if (pid == 0) {

		if (regcomp(&re, tofindSum, REG_EXTENDED) != 0) {
			fprintf(stderr, "Failed to compile regex '%s'\n", tofindSum);
			return EXIT_FAILURE;
		}
		
		int *nums = (int*)malloc((8)*sizeof(int));
		if ((retval = regexec(&re, line, 2, rm, 0)) == 0)
		{
			char *src = line + rm[1].rm_so;
			char *end = line + rm[1].rm_eo;

			while (src < end)
			{
				size_t len = strcspn(src, ",");
				if (src + len > end)
					len = end - src;
				char *sqlAnswers = malloc(len);
				sprintf(sqlAnswers, "%.*s", (int)len, src);
				nums[totalForSum] = atoi(sqlAnswers);
				//printf("String %d ", atoi(sqlAnswers));
				src += len;
				src += strspn(src, ",");
				totalForSum++;
			}


			/* Create independent threads each of which will execute function */
			iret1 = pthread_create(&thread1, NULL, getSum, (void*)nums);
			if (iret1)
			{
				fprintf(stderr, "Error - pthread_create() return code: %d\n", iret1);
				exit(EXIT_FAILURE);
			}

			/* Wait till threads are complete before main continues. Unless we  */
			/* wait we run the risk of executing an exit which will terminate   */
			/* the process and all threads before the threads have completed.   */
			pthread_join(thread1, NULL);
			exit(EXIT_SUCCESS);
		}
		else {
			printf("Please enter proper parameters: sum(parameter1, parameter2....\n");
			return 0;
		}
	}
	else {
		wait(NULL);
	}
}

/* Child process for bprime:
*  When fork() returns 0, we are in
*  the child process.
*/
int bPrimechildProcess(char *line) {
	int i = 0;
	pthread_t thread1;
	int  iret1;
	regex_t re;
	regmatch_t rm[2];
	int retval = 0;
	int pid = fork();
	if (pid == -1) {
		/* Error:
		* When fork() returns -1, an error happened
		* (for example, number of processes reached the limit).
		*/
		fprintf(stderr, "can't fork, error %d\n", errno);
		exit(EXIT_FAILURE);
	}
	if (pid == 0) {

		if (regcomp(&re, tofindBPrime, REG_EXTENDED) != 0) {
			fprintf(stderr, "Failed to compile regex '%s'\n", tofindBPrime);
			return EXIT_FAILURE;
		}
		int *nums = (int*)malloc((8)*sizeof(int));
		if ((retval = regexec(&re, line, 2, rm, 0)) == 0)
		{
			char *src = line + rm[1].rm_so;
			char *end = line + rm[1].rm_eo;

			while (src < end)
			{
				size_t len = strcspn(src, ",");
				if (src + len > end)
					len = end - src;
				char *sqlAnswers = malloc(len);
				sprintf(sqlAnswers, "%.*s", (int)len, src);
				nums[totalForSum] = atoi(sqlAnswers);
				//printf("String %d ", atoi(sqlAnswers));
				src += len;
				src += strspn(src, ",");
				totalForSum++;
			}


			/* Create independent threads each of which will execute function */
			iret1 = pthread_create(&thread1, NULL, biggestPrime, (void*)nums);
			if (iret1)
			{
				fprintf(stderr, "Error - pthread_create() return code: %d\n", iret1);
				exit(EXIT_FAILURE);
			}

			/* Wait till threads are complete before main continues. Unless we  */
			/* wait we run the risk of executing an exit which will terminate   */
			/* the process and all threads before the threads have completed.   */
			pthread_join(thread1, NULL);
			exit(EXIT_SUCCESS);
		}
		else {
			printf("Please enter proper parameters: bprime(parameter1, parameter2....\n");
			return 0;
		}
	}
	else {
		wait(NULL);
	}
}

/* Child process for lcm:
*  When fork() returns 0, we are in
*  the child process.
*/
int lcmchildProcess(char *line) {
	int i = 0;
	pthread_t thread1;
	int  iret1;
	regex_t re;
	regmatch_t rm[2];
	int retval = 0;
	int pid = fork();
	if (pid == -1) {
		/* Error:
		* When fork() returns -1, an error happened
		* (for example, number of processes reached the limit).
		*/
		fprintf(stderr, "can't fork, error %d\n", errno);
		exit(EXIT_FAILURE);
	}
	if (pid == 0) {

		if (regcomp(&re, tofindLcm, REG_EXTENDED) != 0) {
			fprintf(stderr, "Failed to compile regex '%s'\n", tofindLcm);
			return EXIT_FAILURE;
		}
		int *nums = (int*)malloc((8)*sizeof(int));
		if ((retval = regexec(&re, line, 2, rm, 0)) == 0)
		{
			char *src = line + rm[1].rm_so;
			char *end = line + rm[1].rm_eo;

			while (src < end)
			{
				size_t len = strcspn(src, ",");
				if (src + len > end)
					len = end - src;
				char *sqlAnswers = malloc(len);
				sprintf(sqlAnswers, "%.*s", (int)len, src);
				nums[totalForSum] = atoi(sqlAnswers);
				//printf("String %d ", atoi(sqlAnswers));
				src += len;
				src += strspn(src, ",");
				totalForSum++;
			}


			/* Create independent threads each of which will execute function */
			iret1 = pthread_create(&thread1, NULL, lcm, (void*)nums);
			if (iret1)
			{
				fprintf(stderr, "Error - pthread_create() return code: %d\n", iret1);
				exit(EXIT_FAILURE);
			}

			/* Wait till threads are complete before main continues. Unless we  */
			/* wait we run the risk of executing an exit which will terminate   */
			/* the process and all threads before the threads have completed.   */
			pthread_join(thread1, NULL);
			exit(EXIT_SUCCESS);
		}
		else {
			printf("Please enter proper parameters: lcm(parameter1, parameter2....\n");
			return 0;
		}
	}
	else {
		wait(NULL);
	}
}

/* Child process for substraction:
*  When fork() returns 0, we are in
*  the child process.
*/
int subchildProcess(char *line) {
	int arr[7];
	int i = 0;
	pthread_t thread1, thread2;
	int  iret1, iret2;
	regex_t re;
	regmatch_t rm[2];
	int retval = 0;
	int pid = fork();
	if (pid == -1) {
		/* Error:
		* When fork() returns -1, an error happened
		* (for example, number of processes reached the limit).
		*/
		fprintf(stderr, "can't fork, error %d\n", errno);
		exit(EXIT_FAILURE);
	}
	if (pid == 0) {

		if (regcomp(&re, tofindSub, REG_EXTENDED) != 0) {
			fprintf(stderr, "Failed to compile regex '%s'\n", tofindSub);
			return EXIT_FAILURE;
		}
		int *nums = (int*)malloc((8)*sizeof(int));
		if ((retval = regexec(&re, line, 2, rm, 0)) == 0)
		{
			char *src = line + rm[1].rm_so;
			char *end = line + rm[1].rm_eo;

			while (src < end)
			{
				size_t len = strcspn(src, ",");
				if (src + len > end)
					len = end - src;
				char *sqlAnswers = malloc(len);
				sprintf(sqlAnswers, "%.*s", (int)len, src);
				nums[totalForSum] = atoi(sqlAnswers);
				//printf("String %d ", atoi(sqlAnswers));
				src += len;
				src += strspn(src, ",");
				totalForSum++;
			}


			/* Create independent threads each of which will execute function */
			iret1 = pthread_create(&thread1, NULL, getSub, (void*)nums);
			if (iret1)
			{
				fprintf(stderr, "Error - pthread_create() return code: %d\n", iret1);
				exit(EXIT_FAILURE);
			}

			/* Wait till threads are complete before main continues. Unless we  */
			/* wait we run the risk of executing an exit which will terminate   */
			/* the process and all threads before the threads have completed.   */
			pthread_join(thread1, NULL);
			exit(EXIT_SUCCESS);
		}
		else {
			printf("Please enter proper parameters: sub(parameter1, parameter2....\n");
			return 0;
		}
	}
	else {
		wait(NULL);
	}
}

/* Child process for Multiplication:
*  When fork() returns 0, we are in
*  the child process.
*/
int mulchildProcess(char *line) {
	int arr[7];
	int i = 0;
	pthread_t thread1, thread2;
	int  iret1, iret2;
	regex_t re;
	regmatch_t rm[2];
	int retval = 0;
	int pid = fork();
	if (pid == -1) {
		/* Error:
		* When fork() returns -1, an error happened
		* (for example, number of processes reached the limit).
		*/
		fprintf(stderr, "can't fork, error %d\n", errno);
		exit(EXIT_FAILURE);
	}
	if (pid == 0) {

		if (regcomp(&re, tofindMul, REG_EXTENDED) != 0) {
			fprintf(stderr, "Failed to compile regex '%s'\n", tofindMul);
			return EXIT_FAILURE;
		}
		int *nums = (int*)malloc((8)*sizeof(int));
		if ((retval = regexec(&re, line, 2, rm, 0)) == 0)
		{
			char *src = line + rm[1].rm_so;
			char *end = line + rm[1].rm_eo;

			while (src < end)
			{
				size_t len = strcspn(src, ",");
				if (src + len > end)
					len = end - src;
				char *sqlAnswers = malloc(len);
				sprintf(sqlAnswers, "%.*s", (int)len, src);
				nums[totalForSum] = atoi(sqlAnswers);
				//printf("String %d ", atoi(sqlAnswers));
				src += len;
				src += strspn(src, ",");
				totalForSum++;
			}


			/* Create independent threads each of which will execute function */
			iret1 = pthread_create(&thread1, NULL, getMul, (void*)nums);
			if (iret1)
			{
				fprintf(stderr, "Error - pthread_create() return code: %d\n", iret1);
				exit(EXIT_FAILURE);
			}

			/* Wait till threads are complete before main continues. Unless we  */
			/* wait we run the risk of executing an exit which will terminate   */
			/* the process and all threads before the threads have completed.   */
			pthread_join(thread1, NULL);
			exit(EXIT_SUCCESS);
		}
		else {
			printf("Please enter proper parameters: mul(parameter1, parameter2....\n");
			return 0;
		}
	}
	else {
		wait(NULL);
	}
}

/* Child process for divison:
*  When fork() returns 0, we are in
*  the child process.
*/
int divchildProcess(char *line) {
	int arr[7];
	int i = 0;
	pthread_t thread1, thread2;
	int  iret1, iret2;
	regex_t re;
	regmatch_t rm[2];
	int retval = 0;
	int pid = fork();
	if (pid == -1) {
		/* Error:
		* When fork() returns -1, an error happened
		* (for example, number of processes reached the limit).
		*/
		fprintf(stderr, "can't fork, error %d\n", errno);
		exit(EXIT_FAILURE);
	}
	if (pid == 0) {

		if (regcomp(&re, tofindDiv, REG_EXTENDED) != 0) {
			fprintf(stderr, "Failed to compile regex '%s'\n", tofindDiv);
			return EXIT_FAILURE;
		}
		int *nums = (int*)malloc((8)*sizeof(int));
		if ((retval = regexec(&re, line, 2, rm, 0)) == 0)
		{
			char *src = line + rm[1].rm_so;
			char *end = line + rm[1].rm_eo;

			while (src < end)
			{
				size_t len = strcspn(src, ",");
				if (src + len > end)
					len = end - src;
				char *sqlAnswers = malloc(len);
				sprintf(sqlAnswers, "%.*s", (int)len, src);
				nums[totalForSum] = atoi(sqlAnswers);
				//printf("String %d ", atoi(sqlAnswers));
				src += len;
				src += strspn(src, ",");
				totalForSum++;
			}


			/* Create independent threads each of which will execute function */
			iret1 = pthread_create(&thread1, NULL, getDiv, (void*)nums);
			if (iret1)
			{
				fprintf(stderr, "Error - pthread_create() return code: %d\n", iret1);
				exit(EXIT_FAILURE);
			}

			/* Wait till threads are complete before main continues. Unless we  */
			/* wait we run the risk of executing an exit which will terminate   */
			/* the process and all threads before the threads have completed.   */
			pthread_join(thread1, NULL);
			exit(EXIT_SUCCESS);
		}
		else {
			printf("Please enter proper parameters: div(parameter1, parameter2\n");
			return 0;
		}
	}
	else {
		wait(NULL);
	}
}

int main(int argc, char *argv[], char *envp[])
{
	char* substr;
	char c;
	int i, fd;
	
	char *tmp = (char *)malloc(sizeof(char) * 100);
	char *path_str = (char *)malloc(sizeof(char) * 256);
	char *cmd = (char *)malloc(sizeof(char) * 100);
	char *hist[HISTORY_COUNT];
	signal(SIGINT, SIG_IGN);
	signal(SIGINT, handle_signal);

	int ik;

 
	
	if (fork() == 0) {
		execve("/usr/bin/clear", argv, my_envp);
		exit(1);
	}
	else {
		wait(NULL);
	}
	printf("kxg15840 $> ");
	fflush(stdout);
	
	for (ik = 0; ik < HISTORY_COUNT; ik++) {
        hist[ik] = NULL;
	}
	while (c != EOF) {
		c = getchar();
		switch (c) {
		case '\n': if (tmp[0] == '\0') {
			printf("kxg15840 $> ");
		}
				   else {
					   fill_argv(tmp);
					   strncpy(cmd, my_argv[0], strlen(my_argv[0]));
					   strncat(cmd, "\0", 1);

					   parseCommand(cmd,hist);
					   fflush(stdout);
					   free_argv();
					   printf("kxg15840 $> ");
					   bzero(cmd, 100);
				   }
				   bzero(tmp, 100);
				   break;
		default: strncat(tmp, &c, 1);
			break;
		}
	}
	free(tmp);
	free(path_str);
	for (i = 0; my_envp[i] != NULL; i++)
		free(my_envp[i]);
	for (i = 0; i<10; i++)
		free(search_path[i]);
	printf("\n");
	return 0;
}

