//Code for Scheduler Dispatch Simulation Project
//Tom Dale, Alex Gonzalez, Nick Little
//
//---------------------------------------------
#include <iostream>
#include <algorithm>
#include "string.h"
#include <queue>
#include <vector>
#include <random>
#include <fstream>
#include "times.h"
#include "process.h"
using namespace std;


vector<process> initializeFile() {//returns a huge list of processes that were read in from the file

	vector<process> process(100);//TODO make a thousand not a hundred processes

	int a;
	int b;
	int c;
	int d;
	int e;

	int PID = 0;

	ifstream inFile;
	inFile.open("times.txt");

	for (int i = 99; i >= 0 ; i--)
	{
		inFile >> a >> b >> c >> d >> e;


		process[i].setcpuTime1(a);
		process[i].setiO1(b);
		process[i].setcpuTime2(c);
		process[i].setiO2(d);
		process[i].setPID(PID);
		process[i].setArrivalTime(e);

		//INCREMENT FOR NEXT PID #
		PID++;
	}

	inFile.close();

	return process;
}


int main() {
	vector<process> inputFile = initializeFile();

	vector<process> STR;//shortest remaining time first
	queue<process> RR1;//round robin 1 time quantum
	queue<process> RR3;//round robin 3
	queue<process> RR5;//round robin 5
	queue<process> RR10;//round robin 10
	queue<process> FCFS;//first come first serve

	vector<int> processorTime = { 0,0,0,0,0,0 };

	//here are dataAnalysis values-------------------
	int totalTurnaround = 0; //how long all processes take
	int totalWaitTime = 0; //times spent in queues
	int totalResponseTime = 0; //time from entrance to queue to first processing
	int totalContextSwitchTime = 0; //time processors take to context switch
	int processorUtilization = 0; //time processor queues are spent empty
	int totalNumProcesses = inputFile.size();
	//------------------------------------------------

	while (!(inputFile.empty())) {//loop until processes are completed
		//find processor that is furthest back in time
		int lowest = 0;
		for (int i = 0; i < 6; i++){
			if (processorTime.at(i) < processorTime.at(lowest)) lowest = i;
			i++;
		}
		//run the processor furthest back in time
		int shortestTimeRemaining;
		int processTimeOverlap;
		switch (lowest)
		{
		case 0:  //process STR queue, this one is really complicated!
			if (STR.empty()) {//if empty jump time to start of next incoming process
				processorUtilization += inputFile.back().startTime() - processorTime.at(0);
				processorTime.at(0) = inputFile.back().startTime();
				STR.push_back(inputFile.back());
				inputFile.pop_back();

			}
			shortestTimeRemaining = 0; //find shortest time remaining in STR queue
			for (unsigned int i = 0; i < STR.size(); i++) {
				if (STR.at(i).actualTimeRemaining() < STR.at(shortestTimeRemaining).actualTimeRemaining()) shortestTimeRemaining = i;
				i++;
			}
			//load in all processes that will enter queue during processing time of the current process
			while (inputFile.back().startTime() < STR.at(shortestTimeRemaining).actualTimeRemaining() + processorTime.at(0)) {
				if (inputFile.empty()) break;
				int firstProcessTime = inputFile.back().startTime() - processorTime.at(0);//amount of time processing before shorter process enters
				STR.push_back(inputFile.back());//load in process from inputFile
				inputFile.pop_back();//remove that process from inputFile
				int timeFirstProcessRemaining = STR.at(shortestTimeRemaining).estimatedTimeRemaining() - firstProcessTime; //how much of the first process remains at new process entering queue time
				if (STR.back().estimatedTimeRemaining() < timeFirstProcessRemaining ) {//the process that entered is shorter than the current process
					processorTime.at(0) += contextSwitch;
					totalContextSwitchTime += contextSwitch;
					processorTime.at(0) += firstProcessTime;//add discrete event time jump for this processor
					if (STR.at(shortestTimeRemaining).processSomeTime(firstProcessTime) <= 0) {//if the process was completed before interruption
						
						cout << "Finished processing PID#" << STR.at(shortestTimeRemaining).getPID() << " in shortest time remaining processor." << endl;
						totalWaitTime += STR.back().waitTime(processorTime.at(0));
						totalTurnaround += STR.back().turnaroundTime(processorTime.at(0));
						STR.erase(STR.begin() + shortestTimeRemaining); // delete old iteration of this process
					}
					if (STR.at(shortestTimeRemaining).isFirstResponse()) {
						totalResponseTime += STR.at(shortestTimeRemaining).age(processorTime.at(0));
					}
					goto label;//process was replaced so end continuation of processing on this step
					}
				}
			processorTime.at(0) += STR.at(shortestTimeRemaining).actualTimeRemaining();//add discrete event time jump for this processor
			cout << "Finished processing PID#" << STR.at(shortestTimeRemaining).getPID() << " in shortest time remaining processor." << endl;
			if (STR.at(shortestTimeRemaining).isFirstResponse()) {
				totalResponseTime += STR.at(shortestTimeRemaining).age(processorTime.at(0));
			}
			totalWaitTime += STR.back().waitTime(processorTime.at(0));
			totalTurnaround += STR.back().turnaroundTime(processorTime.at(0));
			STR.erase(STR.begin() + shortestTimeRemaining); // delete old iteration of this process
			label:
			//check ages in queue
			for (unsigned int i = 0; i < STR.size(); i++) {
				if (STR.at(i).age(processorTime.at(0)) > STRtimeLimit) {
					RR1.push(STR.at(i));//move process to RR1 processor queue
					STR.erase(STR.begin() + i); // delete old iteration of this process
				}
			}
			break;

		
		case 1:  //process 1 second of RR1
			if (RR1.empty()) {
				processorUtilization++;
				processorTime.at(1)++;
				break;
			}
			if (RR1.front().isFirstResponse()) {
				totalResponseTime += RR1.front().age(processorTime.at(1));
			}
			if (RR1.front().processSomeTime(RR10ProcessTime) <= 0 ){//process 1 second and check to see if it was completed
				cout << "Finished processing PID#" << RR1.front().getPID() << " in round robin processor" << endl;

				totalWaitTime += FCFS.back().waitTime(processorTime.at(1));
				totalTurnaround += FCFS.back().turnaroundTime(processorTime.at(1));
				RR1.pop();
				processorTime.at(1) += RR1ProcessTime;

			}
			else {//if process not completed then first check if it is too old for the queue
				processorTime.at(1) += RR1ProcessTime + contextSwitch;
				totalContextSwitchTime += contextSwitch;
				if (RR1.front().age(processorTime.at(1)) > RR1timeLimit) {
					RR3.push(RR1.front());//move process to RR3 processor queue
					RR1.pop(); // delete old iteration of this process
				}
				else {//if not too old put it in the back of the line
					RR1.push(RR1.front());
					RR1.pop();
				}
			}
			
			break;


		case 2:  //Process 3 seconds of RR3
			if (RR3.empty()) {
				processorUtilization++;
				processorTime.at(2)++;
				break;
			}
			if (RR3.front().isFirstResponse()) {
				totalResponseTime += RR3.front().age(processorTime.at(2));
			}
			processTimeOverlap = RR3.front().processSomeTime(RR3ProcessTime);//overlap is the difference of process remaining and the amount that was processed
			if (processTimeOverlap <= 0) {//process 3 second and check to see if it was completed
				cout << "Finished processing PID#" << RR3.front().getPID() << " in round robin processor" << endl;

				totalWaitTime += FCFS.back().waitTime(processorTime.at(2));
				totalTurnaround += FCFS.back().turnaroundTime(processorTime.at(2));
				RR3.pop();
				processorTime.at(2) += RR3ProcessTime + processTimeOverlap; //increment by RR3 time or a smaller number if process finished early
			}
			else {//if process not completed then first check if it is too old for the queue
				processorTime.at(2) += RR3ProcessTime + contextSwitch;
				totalContextSwitchTime += contextSwitch;
				if (RR3.front().age(processorTime.at(2)) > RR3timeLimit) {
					RR5.push(RR3.front());//move process to RR5 processor queue
					RR3.pop(); // delete old iteration of this process
				}
				else {//if not too old put it in the back of the line
					RR3.push(RR3.front());
					RR3.pop();
				}
			}
			break;

		case 3:  //process 5 seconds of RR5
			if (RR5.empty()) {
				processorUtilization++;
				processorTime.at(3)++;
				break;
			}
			if (RR5.front().isFirstResponse()) {
				totalResponseTime += RR5.front().age(processorTime.at(3));
			}
			processTimeOverlap = RR5.front().processSomeTime(RR5ProcessTIme);//overlap is the difference of process remaining and the amount that was processed
			if (processTimeOverlap <= 0) {//process 5 second and check to see if it was completed
				cout << "Finished processing PID#" << RR5.front().getPID() << " in round robin processor" << endl;

				totalWaitTime += FCFS.back().waitTime(processorTime.at(3));
				totalTurnaround += FCFS.back().turnaroundTime(processorTime.at(3));
				RR5.pop();
				processorTime.at(3) += RR5ProcessTIme + processTimeOverlap; //increment by RR5 time or a smaller number if process finished early
			}
			else {//if process not completed then first check if it is too old for the queue
				processorTime.at(3) += RR5ProcessTIme + contextSwitch;
				totalContextSwitchTime += contextSwitch;
				if (RR5.front().age(processorTime.at(3)) > RR5timeLimit) {
					RR10.push(RR5.front());//move process to RR10 processor queue
					RR5.pop(); // delete old iteration of this process
				}
				else {//if not too old put it in the back of the line
					RR5.push(RR5.front());
					RR5.pop();
				}
			}
			break;


		case 4:  //process 10 seconds of RR10
			if (RR10.empty()) {
				processorUtilization++;
				processorTime.at(4)++;
				break;
			}
			if (RR10.front().isFirstResponse()) {
				totalResponseTime += RR10.front().age(processorTime.at(4));
			}
			processTimeOverlap = RR10.front().processSomeTime(RR10ProcessTime);//overlap is the difference of process remaining and the amount that was processed
			if (processTimeOverlap <= 0) {//process 10 second and check to see if it was completed
				cout << "Finished processing PID#" << RR10.front().getPID() << " in round robin processor" << endl;

				totalWaitTime += RR10.back().waitTime(processorTime.at(4));
				totalTurnaround += RR10.back().turnaroundTime(processorTime.at(4));
				RR10.pop();
				processorTime.at(4) += RR10ProcessTime + processTimeOverlap; //increment by RR10 time or a smaller number if process finished early
			}
			else {//if process not completed then first check if it is too old for the queue
				processorTime.at(4) += RR10ProcessTime + contextSwitch;
				totalContextSwitchTime += contextSwitch;
				if (RR10.front().age(processorTime.at(4)) > RR10timeLimit) {
					FCFS.push(RR10.front());//move process to RR5 processor queue
					RR10.pop(); // delete old iteration of this process
				}
				else {//if not too old put it in the back of the line
					RR10.push(RR10.front());
					RR10.pop();
				}
			}
			break;


		case 5:  //finish next process in FCFS
			if (FCFS.empty()) {
				processorUtilization++;
				processorTime.at(5)++;
				break;
			}
			if (FCFS.front().isFirstResponse()) {
				totalResponseTime += FCFS.front().age(processorTime.at(5));
			}
			processorTime.at(5) += FCFS.back().actualTimeRemaining();
			totalWaitTime += FCFS.back().waitTime(processorTime.at(5));
			totalTurnaround += FCFS.back().turnaroundTime(processorTime.at(5));
			FCFS.pop();
			break;
		}
		//cout << " Processor " << lowest << " is now at time " << processorTime.at(lowest) << endl;
		
	}

	cout << "All processes finished.";

	int highest = 0;
	for (int i = 0; i < 6; i++) {
		if (processorTime.at(i) > processorTime.at(highest)) highest = i;
		i++;
	}
	int finalTime = processorTime.at(highest);


	//TODO: output test data to excel. Turn the below totals into averages
	int avgTurnaround = totalTurnaround / totalNumProcesses;
	int avgWaitTime = totalWaitTime / totalNumProcesses;
	int avgResponse = totalResponseTime / totalNumProcesses;
	int totalthroughput = totalNumProcesses; //how many processes
	totalContextSwitchTime;
	processorUtilization; 



	
	return 0;
}