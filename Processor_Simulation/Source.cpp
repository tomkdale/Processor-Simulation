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
using namespace std;

class process //TODO create fully functioning process object
{
public:
	process();
	~process();
	int actualTimeRemaining();
	int estimatedTimeRemaining();
	int startTime();
	int getPID();
	int processSomeTime(int timeProcessed);
	void completed();
	int timeWaiting(int currentTime);
private:
	int PID, arrivalTime, cpuTime1, iO1, cpuTime2, iO2; // initialized data
};

process::process()
{
	//TODO initialize data 
}
int process::actualTimeRemaining() {//returns time remaining to be processed
	return cpuTime1 + iO1 + cpuTime2 + iO2;

}
int process::estimatedTimeRemaining() {

}
int process::startTime() {//returns the time that the process first enters the queue
	return rand() % 100;
}
int process::getPID() {
	return PID;
}
int process::processSomeTime(int timeProcessed)//should decrement the timeRemaining because it was half processed, return 0 if finished
{
	cout << "Process " << PID << " processed for " << timeProcessed;
	if(this->actualTimeRemaining()-timeProcessed <= 0) cout << ". Completed!" << endl;
	else cout << ". Did not complete, " << this->actualTimeRemaining() - timeProcessed << " remaining." << endl;
	return this->actualTimeRemaining-timeProcessed;
}
void process::completed() {
	cout << "Process " << PID << " completed!" << endl;
}
int process::timeWaiting(int currentTime)
{
	return 0;
}
process::~process()
{
}

vector<process> initializeFile() {//returns a huge list of processes that were read in from the file
	vector<process> nothing;
	return nothing;
}


int main() {

	//TODO: insert code to import processors and load them into imput file

	vector<process> inputFile = initializeFile();

	vector<process> STR;//shortest remaining time first
	queue<process> RR1;//round robin 1 time quantum
	queue<process> RR3;//round robin 3
	queue<process> RR5;//round robin 5
	queue<process> RR10;//round robin 10
	queue<process> FCFS;//first come first serve

	//waterfalling processor priority need a time limit for when processes move into the next queue
	int STRtimeLimit = 100;
	int RR1timeLimit = 200;
	int RR3timeLimit = 400;
	int RR5timeLimit = 500;
	int RR10timeLimit = 1000;

	vector<int> processorTime = { 0,0,0,0,0,0 };

	//here are dataAnalysis values-------------------
	int totalthroughput = 0; //how many processes
	int totalTurnaround = 0; //how long all processes take
	int totalWaitTime = 0; //times spent in queues
	int totalResponseTime = 0; //time from entrance to queue to first processing
	int totalContextSwitchTime = 0; //time processors take to context switch
	int processorUtilization = 0; //time processor queues are spent empty
	//------------------------------------------------

	while (!(inputFile.empty() && STR.empty() && RR1.empty() && RR3.empty() && RR5.empty() && RR10.empty())) {//loop until processes are completed
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
				int firstProcessTime = inputFile.back().startTime() - processorTime.at(0);
				STR.push_back(inputFile.back());//load in process from inputFile
				inputFile.pop_back();//remove that process from inputFile
				int timeFirstProcessRemaining = STR.at(shortestTimeRemaining).actualTimeRemaining() - firstProcessTime; //how much of the first process remains at new process entering queue time
				if (STR.back().actualTimeRemaining() < timeFirstProcessRemaining ) {//the process that entered is shorter than the current process
					if (STR.at(shortestTimeRemaining).processSomeTime(firstProcessTime)) {
						cout << "Finished processing PID#" << STR.at(shortestTimeRemaining).getPID() << " in shortest time remaining processor." << endl;
					}
					processorTime.at(0) += firstProcessTime;//add discrete event time jump for this processor
					goto label;//process was replaced so end continuation of processing on this step
					}
				}
			processorTime.at(0) += STR.at(shortestTimeRemaining).actualTimeRemaining();//add discrete event time jump for this processor
			cout << "Finished processing PID#" << STR.at(shortestTimeRemaining).getPID() << " in shortest time remaining processor." << endl;
			STR.at(shortestTimeRemaining).completed();//finish processing first process
			label:
			//check ages in queue
			for (unsigned int i = 0; i < STR.size(); i++) {
				if (STR.at(i).timeWaiting(processorTime.at(0)) > STRtimeLimit) {
					RR1.push(STR.at(i));//move process to RR1 processor queue
					STR.erase(STR.begin() + i); // delete old iteration of this process
				}
			}
			break;

		
		case 1:  //process 1 second of RR1
			if (RR1.front().processSomeTime(1) == 0 ){//process 1 second and check to see if it was completed
				cout << "Finished processing PID#" << RR1.front().getPID() << " in round robin processor" << endl;
				RR1.pop();
				processorTime.at(1) += 1;

			}
			else {//if process not completed then first check if it is too old for the queue
				processorTime.at(1) += 1;
				if (RR1.front().timeWaiting(processorTime.at(1)) > RR1timeLimit) {
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
			processTimeOverlap = RR3.front().processSomeTime(3);//overlap is the difference of process remaining and the amount that was processed
			if (processTimeOverlap <= 0) {//process 3 second and check to see if it was completed
				cout << "Finished processing PID#" << RR3.front().getPID() << " in round robin processor" << endl;
				RR3.pop();
				processorTime.at(2) += 3 + processTimeOverlap; //increment by 3 or a smaller number if process finished early
			}
			else {//if process not completed then first check if it is too old for the queue
				processorTime.at(2) += 3;
				if (RR3.front().timeWaiting(processorTime.at(2)) > RR3timeLimit) {
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
			processTimeOverlap = RR5.front().processSomeTime(5);//overlap is the difference of process remaining and the amount that was processed
			if (processTimeOverlap <= 0) {//process 5 second and check to see if it was completed
				cout << "Finished processing PID#" << RR5.front().getPID() << " in round robin processor" << endl;
				RR5.pop();
				processorTime.at(3) += 5 + processTimeOverlap; //increment by 3 or a smaller number if process finished early
			}
			else {//if process not completed then first check if it is too old for the queue
				processorTime.at(3) += 5;
				if (RR5.front().timeWaiting(processorTime.at(3)) > RR5timeLimit) {
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
			processTimeOverlap = RR10.front().processSomeTime(10);//overlap is the difference of process remaining and the amount that was processed
			if (processTimeOverlap <= 0) {//process 10 second and check to see if it was completed
				cout << "Finished processing PID#" << RR10.front().getPID() << " in round robin processor" << endl;
				RR10.pop();
				processorTime.at(4) += 10 + processTimeOverlap; //increment by 3 or a smaller number if process finished early
			}
			else {//if process not completed then first check if it is too old for the queue
				processorTime.at(4) += 10;
				if (RR10.front().timeWaiting(processorTime.at(4)) > RR10timeLimit) {
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
			processorTime.at(5) += FCFS.back().actualTimeRemaining();
			FCFS.back().completed();
			FCFS.pop();
			break;
		}
		cout << " Processor " << lowest << " is now at time " << processorTime.at(lowest) << endl;
		
	}

	cout << "All processes finished.";

	//TODO: output test data to excel. Turn the below totals into averages
	totalthroughput;
	totalTurnaround;
	totalWaitTime;
	totalResponseTime;
	totalContextSwitchTime;
	processorUtilization;



	
	return 0;
}