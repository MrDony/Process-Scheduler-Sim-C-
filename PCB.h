#include <iostream>
#include <string>
#include <ctime>
#include <unistd.h>


struct PCB;
struct QNode;
struct Queue;
struct LNode;
struct LinkedList;
void Enque(Queue*, PCB*);
PCB* Dequeue(Queue*);
void printQueue(Queue*);

enum QNames { New = 0, Ready, Running, Blocked, Exited };
const char* QueueNames[] = { "New", "Ready", "Running", "Blocked", "Exited" };

enum sch { FCFS = 0, RR, PP, ZS };
const char* SchedulingNames[] = { "First Come First Serve","Round Robin","Preemptive Priority","Zeno's Scheduler" };

struct PCB
{
	std::string procName{"N/A"};
	int priority{ 0 };
	int arrivalTime{ 0 };
	char type{ 'C' };
	int cpuTime{ 0 };
	int ioTime{ 0 };
	int timeSpentIn[5]{ 0 };//0 new	1 ready		2 running		3 blocked		4 exited
	bool blockingServed{ false };
	int status{ 0 };//0 new		1 ready		2 running		3 blocked		4 exited
	int cpuNum{ -1 };
};

//NAME		PRIORITY	ARRIVALTIME		TYPE		CPUTIME		IOTIME
inline PCB* PCB_constructor(std::string n, int p, float at, char t, float ct=0, float iot=0)
{
	PCB* l_pcb = new PCB;
	l_pcb->procName = n;
	l_pcb->priority = p;
	l_pcb->arrivalTime = at*100;
	l_pcb->type = t;
	if (ct == 0)
		l_pcb->cpuTime = (rand() % 5 + 3)*100;
	else
		l_pcb->cpuTime = ct*100;
	if (iot == 0)
		l_pcb->ioTime = (rand() % 2 + 1)*100;
	else
		l_pcb->ioTime = iot*100;
	return l_pcb;
}

inline void printPCB(PCB* pcb, int i=0)
{
	std::cout << pcb->procName;
	if (i > 0)
		std::cout <<"(" << pcb->priority << ") " << pcb->timeSpentIn[Running];
	if (i > 1)
		std::cout << "-" << pcb->timeSpentIn[Ready];
	if (i > 2)
		std::cout << "-" << pcb->timeSpentIn[Blocked];
	if (i > 3)
		std::cout << " >> "<< " " << pcb->arrivalTime << " " << pcb->type << " " << pcb->cpuTime << " " << pcb->ioTime;
	
}

struct QNode
{
	PCB* val{ NULL };
	QNode* next{ NULL };
};

struct Queue
{
	QNode* head{ NULL };
	QNode* tail{ NULL };
	int count{ 0 };
};

void Enque(Queue* Q, PCB* pcb)
{
	//std::cout << "Enqueu into "; fflush(stdout);
	if (Q->head == NULL)
	{
		//std::cout << "First entry "; fflush(stdout);
		Q->head = new QNode;
		Q->head->val = pcb;
		Q->tail = Q->head;
		Q->count++;
		//std::cout << "Made\t"<<Q->head->val->procName<<"\t"; fflush(stdout);
	}
	else
	{
		Q->tail->next = new QNode;
		Q->tail->next->val = pcb;
		Q->tail = Q->tail->next;
		Q->count++;
	}
}

PCB* Dequeue(Queue* Q)
{
	if (Q->head == NULL)
		return NULL;
	PCB* ret = Q->head->val;
	QNode* l_head = Q->head;
	Q->head = Q->head->next;
	if (Q->head == NULL)
	{
		Q->tail = Q->head;
	}
	
	delete l_head;
	Q->count--;
	if (Q->count == 0)
	{
		Q->head = NULL;
		Q->tail == NULL;
	}
	//std::cout << "returning from dequeue : " << ret->procName << "\t"; fflush(stdout);
	//printQueue(Q); fflush(stdout);
	return ret;
}

void removeQueue(Queue* Q, PCB* p)
{
	QNode* l_head = Q->head;
	QNode* prev = l_head;
	while (Q->head!=NULL && Q->head->val == p)
	{
		Dequeue(Q);
	}
	l_head = Q->head;
	while (l_head)
	{
		if (l_head->val == p)
		{
			prev->next = l_head->next;
			delete l_head;
			Q->count--;
			l_head = prev;
			if (Q->count == 0)
			{
				Q->head = NULL;
				Q->tail = NULL;
				l_head = NULL;
			}
		}
		prev = l_head;
		if (l_head)
			l_head = l_head->next;
	}
}

inline void printQueue(Queue* Q)
{
	//if (Q->head == NULL)
		//return;
	QNode* l_head = Q->head;
	std::cout << "--------------------------------------------------------------------------------------------------------------------------------\n";
	while (l_head)
	{
		printPCB(l_head->val,3);
		std::cout << "  ";
		l_head = l_head->next;
	}
	std::cout << "\n--------------------------------------------------------------------------------------------------------------------------------\n";

}

std::string getQStr(Queue* Q)
{
	std::string str;
	str += "<";
	QNode* l_head = Q->head;
	while (l_head)
	{
		printPCB(l_head->val, 3);
		str += (l_head->val->procName + " ");
		l_head = l_head->next;
	}
	str += ">";
	return str;

}
struct LNode
{
	PCB* val{ NULL };
	LNode* next{ NULL };
};

struct LinkedList
{
	LNode* head{ NULL };
};

void insert(LinkedList* LL, PCB* pcb)
{
	LNode* l_head = LL->head;
	if (l_head == NULL)
	{
		LL->head = new LNode;
		LL->head->val = pcb;
		return;
	}
	while (l_head->next)
	{
		l_head = l_head->next;
	}
	l_head->next = new LNode;
	l_head->next->val = pcb;
	return;
}

int countLL(LinkedList* LL)
{
	int j = 0;
	LNode* l_head = LL->head;
	while (l_head)
	{
		l_head = l_head->next;
		j++;
	}
	return j;
}

PCB* getLL(LinkedList* LL, int i)
{
	int j = 0;
	LNode* l_head = LL->head;
	while (l_head)
	{
		if (i == j)
			return l_head->val;
		l_head = l_head->next;
		j++;
	}
	return NULL;
}

void printLL(LinkedList* LL,int j=0)
{
	LNode* l_head = LL->head;
	while (l_head)
	{
		printPCB(l_head->val,j);
		if (j != 4)
			std::cout << "---";
		else
			std::cout << "\n--------------------------------------------\n";
		l_head = l_head->next;
	}
}