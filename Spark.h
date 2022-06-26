#include "CPU.h"
#include <iostream>
#include <string>

using namespace std;


ofstream g_out;
void writeInFile()
{
	string s;
	//time
	//running count
	//ready count
	//waiting count
	//cpu statuses
	//IO queue
	
	g_out.open("output.txt", std::ios_base::app);
	float ti = ((float)g_time) / 100;
	s += to_string(ti);
	s += "\t";
	s += to_string(g_Queues[Running]->count);
	s += "\t";
	s += to_string(g_Queues[Ready]->count);
	s += "\t";
	s += to_string(g_Queues[Blocked]->count);
	s += "\t";
	
	for (int i = 0; i < g_CPUs->count; i++)
	{
		if (g_CPUs->arr[i]->status == IDLE)
			s += "Idle";
		else
		{
			s += g_CPUs->arr[i]->pcb->procName[2];
			s += g_CPUs->arr[i]->pcb->procName[3];
			s += g_CPUs->arr[i]->pcb->procName[4];
		}
			s += "\t";
	}

	s += getQStr(g_Queues[Blocked]) + "\n";
	//cout << s;
	fflush(stdout);
	g_out << s;
	g_out.close();
}

inline string* processDetailsList(string& line, int size)
{
	
	string* ret = new string[size]{ "" };
	int x = 0;
	int i = 0;
	for (int j = 0; j < line.length(); j++)
	{
		if (line[j] == '\t' || line[j] == '\0' || line[j] == '\n')
		{
			x++;
			continue;
		}
		ret[x] += line[j];
	}
	cout << "details -> ";
	for (int i = 0; i < size; i++)
		cout << ret[i] << "\t";// << "\t" << ret[1] << endl;
	cout << endl;
	fflush(stdout);
	return ret;
}

inline void printSimulation()
{
	cout << "Time : " << g_time << "\n";
	cout << "Scheduling : " << SchedulingNames[g_schedule] << "\n";
	for (int i = 0; i < 5; i++)
	{
		cout << QueueNames[i] << " Queue\n";
		printQueue(g_Queues[i]);
		cout << "\n";
	}
	cout << "\nCPUs\n";
	printCPUArr(g_CPUs);
	cout << endl;
}



bool setupSpark(string fileName, int scheduleType, float TimeSlice=-1, int cpuCount=1)
{
	if (cpuCount < 1)
	{
		cout << "ERROR: cant create " << cpuCount << " CPUs \n";
		return false;
	}
	ifstream in(fileName);
	string line;
	if (in.is_open())
	{
		g_loading = true;
		string s;
		ofstream out("output.txt");
		out.open("output.txt");
		s+="t\tRu\tRe\tWi\t";
		for (int i = 0; i < cpuCount; i++)
		{
			s += "CPU" + to_string(i)+"\t";
		}
		s += "IO Q\n";
		out<<s;
		out.close();

		//return false;


		g_schedule = scheduleType;
		g_timeSlice = TimeSlice;
		g_time = 0;
		g_PCBs = new LinkedList;
		g_Queues = new Queue* [5];
		for (int i = 0; i < 5; i++)
			g_Queues[i] = new Queue;
		g_CPUs = new CPUarr;

		
		getline(in, line);//headings
		int argus = 1;
		for (int i = 0; i < line.length(); i++)
		{
			if (line[i] == '\t' || line[i] == '\0' || line[i] == '\n')
				argus++;
		}

		string* details = NULL;
		while (!in.eof())
		{
			getline(in, line);
			delete[] details;
			details = processDetailsList(line, argus);
			PCB* p;
			//cout << "fetched " << details[0]<endl;

			if (argus == 6)
				p=PCB_constructor(details[0], stoi(details[1]), stof(details[2]), details[3][0], stof(details[4]), stof(details[5]));

			else
				p=PCB_constructor(details[0], stoi(details[1]), stof(details[2]), details[3][0]);
			//cout << "inserting " << p->m_Process_name << endl;
			insert(g_PCBs, p);
		}

		int PCBCount = countLL(g_PCBs);
		for (int i = 0; i < PCBCount; i++)
		{
			Enque(g_Queues[New], getLL(g_PCBs, i));
			cout << g_Queues[New]->tail->val->procName << " added to new\n";
			fflush(stdout);
		}

		for (int i = 0; i < cpuCount; i++)
		{
			addCPU(g_CPUs, CPU_constructor(i));
			pthread_t tid;
			//cout << "firing CPU no : " << g_CPUs->arr[i]->cpuNo << "\t";
			pthread_create(&tid, NULL, CPU_thread, g_CPUs->arr[i]);
		}

		sleep(1);
		cout << "\n\n\n-----------SPARK KERNEL LOADED-----------\n\n";
		cout << "PCBs\n";
		printLL(g_PCBs,4);
		cout << "\nNew\n";
		printQueue(g_Queues[New]);
		cout << "\nCPUs\n";
		printCPUArr(g_CPUs);
		cout << endl;
		//sleep(20);
		return true;
	}
	else 
	{
		cout << "CANT OPEN FILE E100\n"; return false;
	}
}


void simulate()
{
	int pcbCount;
	pcbCount = countLL(g_PCBs);
	PCB* l_pcb;
	while (!g_end)
	{
		//NEW TO READY
		while (g_Queues[New]->count > 0 && g_Queues[New]->head->val->arrivalTime <= g_time)		
		{
			//if (!(schedule(g_Queues[New]->head->val, Ready)));
			l_pcb = Dequeue(g_Queues[New]);
			l_pcb->status = Ready;
			Enque(g_Queues[Ready], l_pcb);
		}
		//READY TO RUNNING
		while (g_Queues[Ready]->count > 0)		
		{
			if (g_schedule == PP)
			{
				QNode* l_head = g_Queues[Ready]->head;
				while (l_head)
				{
					//printQueue(g_Queues[Running]);
					
					//cout << "\n" << l_head->val->procName << "  ";
					//cout << "ready to running\n"; fflush(stdout);
					if (l_head)
					{
						l_pcb = l_head->val;

						if (schedule(l_pcb, Running))
						{
							//printCPUArr(g_CPUs); fflush(stdout);
							Enque(g_Queues[Running], l_pcb);
							l_pcb->status = Running;
							removeQueue(g_Queues[Ready], l_pcb);
							l_head = g_Queues[Ready]->head;
						}
						else if (l_head)
							l_head = l_head->next;
					}
				}
				break;

			}
			else
			{
				if (schedule(g_Queues[Ready]->head->val, Running))
				{
					l_pcb = Dequeue(g_Queues[Ready]);
					l_pcb->status = Running;
					Enque(g_Queues[Running], l_pcb);
				}
				else                  //if cant schedule for running then that meas that there are no CPUs available
				{
					break;
				}
			}
	
		}
		//cout << "xx\n"; fflush(stdout);
		//BLOCKED TO READY
		while (g_Queues[Blocked]->count > 0 && g_Queues[Blocked]->head->val->timeSpentIn[Blocked] != 0 && (g_Queues[Blocked]->head->val->timeSpentIn[Blocked] % g_Queues[Blocked]->head->val->ioTime == 0))
		{
			l_pcb = Dequeue(g_Queues[Blocked]);
			l_pcb->blockingServed = true;
			l_pcb->status = Ready;
			Enque(g_Queues[Ready], l_pcb);

		}
		//increment times of process according to where they are accept in running
		for (int i = 0; i < pcbCount; i++)
		{
			l_pcb = getLL(g_PCBs, i);
			if(l_pcb->status!=Running)
				l_pcb->timeSpentIn[l_pcb->status] += 10;
		}
		while (g_pause);
		g_time += 10;
		while (g_pause);
			//sleep(1);
			//system("clear");
			//cout << "\n\n------------------------------------------\n"; fflush(stdout);// sleep(10);
			//printSimulation();
			writeInFile();
		g_end = true;
		for (int i = 0; i < 4; i++)
		{
			if (i == Running)
			{
				if (!allcpufree())
				{
					g_end = false;
					break;
				}
			}
			else if (g_Queues[i]->count > 0)
			{
				//cout << "\n" << i << "\n";
				g_end = false;
				break;
			}
		}
        //for output.txt only, change to 0
        //for a simulation on command line, change to 1
        if(0)
		  sleep(1);
		
		
		
		g_loading = false;
		
	}
}

