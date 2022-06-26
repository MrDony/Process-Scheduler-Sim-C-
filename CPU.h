#include "PCB.h"
#include <fstream>
enum status { IDLE = 0, WORKING };
const char* CPUStatus[] = { "Idle","Working" };
struct CPU;
struct CPUarr;
void addCPU(CPUarr*, CPU*);
void printCPU(CPU*);
void* CPU_thread(void*);
bool schedule(PCB*, int);
void wakeCPU(CPU*);
//CPU* getFreeCPU();

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int g_schedule;
int g_timeSlice;

bool g_loading;
bool g_end;
int g_time;//time goes brrrrrr
/// <summary>
/// 000
/// 010
/// 020
/// .
/// .
/// .
/// 100
/// 110
/// 120
/// .
/// .
/// .
/// 990
/// 1000
/// 1010
/// 1020
/// .
/// .
/// .
/// </summary>

bool g_pause;
LinkedList* g_PCBs;

Queue** g_Queues;

CPUarr* g_CPUs;


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


struct CPU
{
	int cpuNo{ 0 };
	int status{ IDLE };
	PCB* pcb{ NULL };
	int cpuTime{ 0 };
	int burstTime{ 0 };

	pthread_mutex_t mutex;
	pthread_cond_t cond;
};

CPU* CPU_constructor(int cn=0)
{
	CPU* l_cpu = new CPU;
	l_cpu->cpuNo = cn;
	return l_cpu;
}

void printCPU(CPU* c)
{
	std::cout << "CPU NO : " << c->cpuNo << "  State : " << CPUStatus[c->status];
	if (c->pcb != NULL && c->status == WORKING)
	{
		std::cout << "  CT:" << c->cpuTime << " BT:" << c->burstTime<<"  "; printPCB(c->pcb, 3);
	}
}

struct CPUarr
{
	CPU** arr{ NULL };
	int count{ 0 };
};

void addCPU(CPUarr* Carr, CPU* c)
{
	
	if (Carr->count == 0)
	{
		Carr->arr = new CPU* [1];
		Carr->arr[0] = c;
		Carr->count++;
	}
	else
	{
		int s = Carr->count + 1;
		CPU** l_arr = new CPU * [s];
		for (int i = 0; i < Carr->count; i++)
		{
			l_arr[i] = Carr->arr[i];
		}
		l_arr[s - 1] = c;
		delete [] Carr->arr;
		Carr->arr = l_arr;
		Carr->count = s;
	}
}

void printCPUArr(CPUarr* a)
{
	for (int i = 0; i < a->count; i++)
	{
		std::cout << i << "->";
		printCPU(a->arr[i]);
		std::cout << "\n";
	}
}

CPU* getFreeCPU(int priority = -1)
{
	//printCPUArr(g_CPUs);
	//fflush(stdout);
	int lowestPriority = 10000;
	//std::cout << "x\n"; fflush(stdout);
	int l_cpuNo;
	for (int i = 0; i < g_CPUs->count; i++)//find an idling cpu and return it
	{
		
		if (g_CPUs->arr[i]->status == IDLE)
			return g_CPUs->arr[i];
		if (lowestPriority > g_CPUs->arr[i]->pcb->priority)
		{
			lowestPriority = g_CPUs->arr[i]->pcb->priority;
			l_cpuNo = i;
		}
			
	}
	if (g_schedule != PP)//if FCFS or RR or ZS return NULL
		return NULL;
	//else preempt the process which has the lowest priority in the running currently and has a priority lower than the new one being added
	//std::cout << lowestPriority << " < " << priority << "\n";
	//g_pause = true;
	//sleep(1);
	//g_pause = false;
	if (lowestPriority < priority)
	{
		
		CPU* ret = g_CPUs->arr[l_cpuNo];
		PCB* swappingWith = ret->pcb;
		//std::cout << "swapping " << swappingWith->procName;
		
		if (swappingWith->timeSpentIn[Running] != 0 && swappingWith->timeSpentIn[Running] % swappingWith->ioTime == 0)
		{
			schedule(swappingWith, Blocked);
			swappingWith->timeSpentIn[Blocked] += 10;
			removeQueue(g_Queues[Running], swappingWith);
		}
		else//send to ready
		{
			//std::cout << "sending " << swappingWith->procName << " to Ready\n";
			schedule(swappingWith, Ready);
			swappingWith->timeSpentIn[Ready] += 10;
			removeQueue(g_Queues[Running], swappingWith);
		}
		fflush(stdout);
		return ret;
	}
	return NULL;
}

bool schedule(PCB* p, int qn)
{
	if (qn == Running)//someone wants to schedule this for running so look for a posibility to supply it a cpu
	{
		CPU* freeCPU = getFreeCPU(p->priority);
		
		if (freeCPU)
		{
			if (g_schedule == FCFS || g_schedule == PP)
			{
				freeCPU->burstTime = p->cpuTime;
			}
			if (g_schedule == RR)
			{
				freeCPU->burstTime = g_timeSlice;//time slice for round robin
			}
			if (g_schedule == ZS)
			{
				int timeGive = p->cpuTime - p->timeSpentIn[Running];
				timeGive = timeGive * 0.1;
				if (timeGive < 5)
					timeGive = 10;
				freeCPU->burstTime = timeGive;//time slice for ZS is 50 of the cpu time it needs currently
			}
			freeCPU->pcb = p;
			freeCPU->pcb->cpuNum = freeCPU->cpuNo;
			freeCPU->cpuTime = 0;
			//std::cout << "Scheduling " << p->procName << " to " << freeCPU->cpuNo<<"\n";
			if (freeCPU->status == IDLE)
			{
				wakeCPU(freeCPU);
			}
				
			return true;//scheduled and cpu given
		}
		return false;//couldnt find a cpu
	}
	else
	{
		Enque(g_Queues[qn], p);
		p->timeSpentIn[qn] += 10;
		p->status = qn;
		return true;//added to queue
	}
	return false;
}

void Idle(CPU* c)
{
	pthread_mutex_lock(&c->mutex);
	c->status = IDLE;
	while (c->status == IDLE)
	{
		pthread_cond_wait(&(c->cond), &(c->mutex));
		c->status = WORKING;
	}
	pthread_mutex_unlock(&c->mutex);
}

void wakeCPU(CPU* c)
{
	pthread_mutex_lock(&(c->mutex));
	pthread_mutex_unlock(&(c->mutex));
	pthread_cond_broadcast(&(c->cond));
	c->status = WORKING;
}

void* CPU_thread(void* arg)
{
	CPU* thisCPU = (CPU*)arg;
	std::cout << "\n CPU NO : " << thisCPU->cpuNo << " is alive\n";
	int l_time;
	while (g_loading);
	l_time = g_time;
	while (!g_end)
	{
		if (thisCPU->status == IDLE)
		{
			Idle(thisCPU);
			thisCPU->cpuTime = 0;
		}
		if (thisCPU->status == WORKING && thisCPU->pcb!=NULL)
		{
			//thisCPU->cpuTime += 10;
			l_time=g_time;
			while (l_time == g_time);l_time = g_time;//wait for a global tick

			if (thisCPU->pcb->timeSpentIn[Running] >= thisCPU->pcb->cpuTime)//SEND THE PROCESS TO EXIT
			{
				//std::cout << "check 1\n"; fflush(stdout);
				schedule(thisCPU->pcb, Exited); removeQueue(g_Queues[Running], thisCPU->pcb); thisCPU->pcb->status = Exited;
				thisCPU->status = IDLE; thisCPU->pcb = NULL;
				//std::cout << "1\n"; fflush(stdout);
			}
			else if (thisCPU->pcb->timeSpentIn[Running] != 0 && thisCPU->pcb->ioTime!=-100 && thisCPU->pcb->blockingServed == false && ( thisCPU->pcb->timeSpentIn[Running] % thisCPU->pcb->ioTime) == 0)
			{//SEND THE PROCESS TO BLOCKING
				//std::cout << "check 2\n"; fflush(stdout);
				schedule(thisCPU->pcb, Blocked); removeQueue(g_Queues[Running], thisCPU->pcb);
				thisCPU->pcb->blockingServed = false;
				thisCPU->status = IDLE; thisCPU->pcb = NULL;
				//std::cout << "2\n"; fflush(stdout);
			}
			else if (thisCPU->burstTime <= thisCPU->cpuTime)//SEND THE PROCESS TO READY
			{
				//std::cout << "check 3\n"; fflush(stdout);
				schedule(thisCPU->pcb, Ready); removeQueue(g_Queues[Running], thisCPU->pcb);
				thisCPU->status = IDLE; thisCPU->pcb = NULL;
				//std::cout << "3\n"; fflush(stdout);
			}
			else//WORK ON THE PROCESS
			{
				//std::cout << "worked\n"; fflush(stdout);
				thisCPU->pcb->timeSpentIn[Running] += 10; thisCPU->cpuTime += 10;
				//std::cout << "w\n"; fflush(stdout);
				thisCPU->pcb->blockingServed = false;
			}
			
		}
	}
	std::cout << "Shutting down CPU : " << thisCPU->cpuNo << "\n";
	return NULL;
}

bool allcpufree()
{
	for (int i = 0; i < g_CPUs->count; i++)
	{
		if (g_CPUs->arr[i]->status == WORKING)
			return false;
	}
	return true;
}