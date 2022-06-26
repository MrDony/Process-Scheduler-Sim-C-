#include <iostream>
#include <string>
#include <fstream>
#include "Spark.h"
using namespace std;

inline void getCPUCount(int& cpuC)
{
	while (1)
	{
		cout << "Enter how many CPUs : ";
		cin >> cpuC;
		if (cpuC > 0)
			return;
		cout << "PLEASE ENTER A VALID CPU COUNT !\n";
	}
}

inline void getScheduling(int& sc)
{
	while (1)
	{
		cout << "0 for FCFS\n";
		cout << "1 for RR\n";
		cout << "2 for PP\n";
		cout << "3 for ZS\n";
		cout << "Enter Choice = ";
		cin >> sc;
		if (sc >= 0 && sc <= 3)
			return;
		cout << "PLEASE ENTER A VALID SCHEDULING POLICY!\n";
	}
}

inline void getTimeSlice(float& ts)
{
	while (1)
	{
		cout << "Enter time slice <= 0.1 : ";
		cin >> ts;
		if (ts >= 0.1)
			return;
		cout << "Cannot enter such a small timeslice\n";
	}
}

inline void getFile(string& fn)
{
	int c;
	while (1)
	{
		cout << "1 -> Processes1.txt\n";
		cout << "2 -> Processes2.txt\n";
		cout << "Enter choice = ";
		cin >> c;
		if (c == 1)
		{
			fn == "Processes1.txt"; return;
		}
			
		else if (c == 2)
		{
			fn == "Processes2.txt"; return;
		}
		cout << "Please enter a valid file choice\n";

	}
}
int main()
{
	srand(time(0));
	g_end = false;
	g_loading = true;
	int cpuCount = 0;
	int scheduling = FCFS;
	float ts = 50;
	int timeSlice;
	string fn;
	if (1)
	{
		getCPUCount(cpuCount);
		getScheduling(scheduling);
		if (scheduling == RR)
		{
			getTimeSlice(ts);
			timeSlice = ts * 100;
		}
			
		getFile(fn);
	}
	else	//FOR DEBUGGING
	{
		cpuCount = 2;
		scheduling = FCFS;
		timeSlice = 200;
		fn = "Processes1.txt";
	}

	if (setupSpark(fn, scheduling, timeSlice, cpuCount))
	{
		//return 0;
		simulate();

	}
	//g_out.close();
	g_end = true;
	cout << "\nshutting down Spark\n";
	int totalReadyTime = 0;
	for (int i = 0; i < countLL(g_PCBs); i++)
	{
		totalReadyTime += getLL(g_PCBs, i)->timeSpentIn[Ready];
	}
	g_out.open("output.txt",ios::ios_base::app);
	g_out << "\nTotal ready time : " << totalReadyTime;
	g_out << "\nTotal Time Taken : " << g_time;
	g_out << "\n";
	g_out.close();
	sleep(1);
	return 0;
}