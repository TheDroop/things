#include "stdio.h"
#include "stdlib.h"
#include "time.h"
#include "math.h"

#define MEM_SIZE 64
#define HANDLER_TIMEOUT 5

void sleep(int ms);
int getMemAdress(int s, int size);
void occupyMemory(int start, int size);

int memory[MEM_SIZE] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int cpu = 0;
int lpid = 1;

int cpu_strategy = 1; //0 = FCFS, 1 = Round Robin, 2 = SRTN, 3 = BFN, 4 = FFN

int pids = 1;
time_t start;
int handler_flag = 1;

int GLOBAL_FUKUMETRO;

// --- Random number generator ---

static unsigned long x=123456789, y=362436069, z=521288629;

unsigned long rng() {          //period 2^96-1

	unsigned long t;
	    x ^= x << 16;
	    x ^= x >> 5;
	    x ^= x << 1;

	   t = x;
	   x = y;
	   y = z;
	   z = t ^ x ^ y;

	  return z;

}	

// ---

typedef struct process PROCESS;

struct process {

	int pid;
	float starttime;
	int mem_need;
	int ctime_need;
	int tspent;
	int mem_alloc;

	PROCESS *next;

};

	PROCESS *handler;
	PROCESS *last_item;

void append(PROCESS *tprocess) {

	pids++;

	if(last_item != NULL) {

		last_item->next = tprocess;
		last_item = last_item->next;

	} else {

		handler = tprocess;
		last_item = tprocess;

	}

}

void tick() {

	sleep(1);

}

void plist() {

	PROCESS *ptr = handler;
	int i = 1;

	printf("\n- Program running for %f seconds -----------------------------\n\n", difftime(time(NULL), start));

	while(ptr != NULL) {

		printf("%i) Prozess %i - gestartet %f - %i Speichereinheiten benötigt - %i Rechenzeit bis Abschluss - %i turns spent on this process!\n", 
		i, ptr->pid, ptr->starttime, ptr->mem_need, ptr->ctime_need, ptr->tspent);

		i++;
		ptr = ptr->next;

	}

	printf("\n---------- MEMORY USAGE: ");	
	for(int j = 0; j < MEM_SIZE; j++) {
		printf("%i ", memory[j]);
	}
	
	printf(" --- RANDOM NUMBER: %i --- Processes generated: %i \n", GLOBAL_FUKUMETRO, pids);
	

}

void init() {

	PROCESS *init = (PROCESS*) malloc(sizeof(PROCESS));
		init->pid = pids;
		init->starttime = difftime(time(NULL), start);
		init->mem_need = 1;
		init->ctime_need = -1;
		init->next = NULL;
		init->tspent = 0;
		init->mem_alloc = 0;
	occupyMemory(0, 1);
	
	append(init);

}

void generate_process() {

	PROCESS *temp = (PROCESS*) malloc(sizeof(PROCESS));
		temp->pid = pids;
		temp->starttime = difftime(time(NULL), start);
		temp->mem_need = (rand() % MEM_SIZE/2)+1;
		temp->ctime_need = (rand() % 10)+1;
		temp->next = NULL;
		temp->tspent = 0;
		temp->mem_alloc = -1;

	//last_item->next = temp;
	//last_item = last_item->next;

	append(temp);

}

void ling(PROCESS *tprocess) { // deletes tprocess

	PROCESS *tmp = handler;
	while(tmp->next != tprocess) {
	
		tmp = tmp->next;

	}

	tmp->next = tprocess->next;
	free(tprocess);

}

void occupyMemory(int s, int size) {

	for(int i = 0; i < size; i++) {

		memory[i+s] = 1;

	}

}

void freeMemory(int s, int size) {

	for(int i = 0; i < size; i++) {

		memory[i+s] = 0;

	}

}

int getMemAddress(int size) { // returns -1 if no chunk of the requested size is found, starting address otherwise

	int i = 1, k = 0;

	while(i < MEM_SIZE-size) {

		for(int j = 0; j < size; j++) {
		
			if(memory[i+j]) {

				k = 0;
				if(i+j+1 < MEM_SIZE) i = i+j+1;
				else i = MEM_SIZE;
				break;

			} else {

				k++;
				if(k == size) return i;

			}

		}
	
	}

	return -1;

}

void fcfs() {	

	if(handler_flag == HANDLER_TIMEOUT) {

		handler_flag = 1;
		handler->tspent++;

	} else {

		if(handler->next != NULL) {

			PROCESS *tmp = handler->next;
			tmp->ctime_need--;
			tmp->tspent++;

			if(!tmp->ctime_need){

				ling(tmp);		

			}

			handler_flag++;
		
		} else {
		
			handler_flag = 1;
			handler->tspent++;

		}

	}

}

void round_robin() {

	if(handler_flag == HANDLER_TIMEOUT) {

		handler_flag = 1;
		handler->tspent++;

	} else {

		PROCESS *tmp = handler;
		//RRbegin:
		int i = 0;

		while((tmp->next != NULL) && (i < lpid)) {

			tmp = tmp->next;
			i++;

		}

		int j = getMemAddress(tmp->mem_need);

		if(tmp->mem_alloc == -1) {

			if(j != -1) {
		
				occupyMemory(j, tmp->mem_need);
				tmp->mem_alloc = j;
				tmp->ctime_need--;
				tmp->tspent++;

				if(tmp->ctime_need == 0){

					freeMemory(tmp->mem_alloc, tmp->mem_need);
					ling(tmp);	

				}

				if(tmp->next != NULL) lpid++;
				else lpid = 1;

			} else {

				if(tmp->next != NULL) {

					lpid++;
					
				} else {
	
					lpid = 1;

				}

				round_robin();

			}

		} else {

			tmp->ctime_need--;
			tmp->tspent++;

			if(tmp->ctime_need == 0){

				freeMemory(tmp->mem_alloc, tmp->mem_need);
				ling(tmp);		

			}

			if(tmp->next != NULL) lpid++;
			else lpid = 1;

		}

	}

	handler_flag++;

}

void handle() {

	// FCFS/FIFO

	switch(cpu_strategy) {

		case 0: fcfs(); break;
		case 1: round_robin(); break;
		//case 3: printf("a ist drei\n"); break;
		default: printf("Ungültige Ressourcenstrategie!"); break;
	
	}

}

void run() {

	while(1) {

		GLOBAL_FUKUMETRO = rand()%10;

		if(GLOBAL_FUKUMETRO < 3) generate_process();
		tick();
		handle();
		system("clear");
		plist();

	}

}

int main() {

	start = time(NULL);
	srand(start);
	init();
	run();

	return 1;

}
