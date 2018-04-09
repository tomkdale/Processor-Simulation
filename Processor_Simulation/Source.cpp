//Code for Scheduler Dispatch Simulation Project
//Tom Dale, Alex Gonzalez, Nick Little
//
//---------------------------------------------
#include <iostream>
#include <algorithm>
#include <string>
#include <queue>
#include <vector>
#include <random>
#include <fstream>
#include "process.h"
using namespace std;


vector<process> initializeFile() {//returns a huge list of processes that were read in from the file

	vector<process> process(1000);
	int a, b, c, d, e;
	int PID = 0;
	ifstream inFile;
	inFile.open("times.txt");

	for (int i = 999; i >= 0 ; i--)
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

void printToExcel(ofstream &outdata, int testNum, int turnAround, int wait, int response)
{
	outdata << to_string(testNum) + "," + to_string(turnAround) + "," + to_string(wait) + "," + to_string(response) << endl; //Write data to corrasponding columns
}

int main() {

	ofstream outdata;//object to write to .csv file
	outdata.open("DataCollection.csv", ios::app);//loacte/create file

	int contextSwitch = 10; // time taken in a processor to context switch
	int RRtimeFactor = 5;//factor that Round robin time quantums change
	int timeLimitFactor = 170;//factor that time limit factors change
	for (int q = 0; q < 20; q++) {
		//values for length of RoundRobin processors 
		int RR1ProcessTime = 1 * RRtimeFactor;
		int RR3ProcessTime = 1 * RRtimeFactor;
		int RR5ProcessTime = 1 * RRtimeFactor;
		int RR10ProcessTime = 1 * RRtimeFactor;

		//Time limit before process is moved down into next processor
		int STRtimeLimit = timeLimitFactor;
		int RR1timeLimit = timeLimitFactor * 2;
		int RR3timeLimit = timeLimitFactor * 3;
		int RR5timeLimit = timeLimitFactor * 4;
		int RR10timeLimit = timeLimitFactor * 5;

		vector<process> inputFile = initializeFile();

		vector<process> STR;//shortest remaining time first
		vector<process> RR1;//round robin 1 time quantum
		vector<process> RR3;//round robin 3
		vector<process> RR5;//round robin 5
		vector<process> RR10;//round robin 10
		vector<process> FCFS;//first come first serve

		vector<int> processorTime = { 0,0,0,0,0,0 };

		//here are dataAnalysis values-------------------
		int totalTurnaround = 0; //how long all processes take
		int totalWaitTime = 0; //times spent in queues
		int totalResponseTime = 0; //time from entrance to queue to first processing
		int totalContextSwitchTime = 0; //time processors take to context switch
		int processorUtilization = 0; //time processor queues are spent empty
		int totalNumProcesses = inputFile.size();
		//------------------------------------------------

		while (!(inputFile.empty() && STR.empty())) {//loop until FCFS process has gone for full processing time
			int shortestTimeRemaining;
			int processTimeOverlap;
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
			while (!inputFile.empty() && inputFile.back().startTime() < STR.at(shortestTimeRemaining).actualTimeRemaining() + processorTime.at(0)) {
				if (inputFile.empty()) break;
				int firstProcessTime = inputFile.back().startTime() - processorTime.at(0);//amount of time processing before shorter process enters
				STR.push_back(inputFile.back());//load in process from inputFile
				inputFile.pop_back();//remove that process from inputFile
				int timeFirstProcessRemaining = STR.at(shortestTimeRemaining).estimatedTimeRemaining() - firstProcessTime; //how much of the first process remains at new process entering queue time
				if (STR.front().estimatedTimeRemaining() < timeFirstProcessRemaining) {//the process that entered is shorter than the current process
					processorTime.at(0) += contextSwitch;
					totalContextSwitchTime += contextSwitch;
					processorTime.at(0) += firstProcessTime;//add discrete event time jump for this processor
					if (STR.at(shortestTimeRemaining).processSomeTime(firstProcessTime) <= 0) {//if the process was completed before interruption

						//cout << "Finished processing PID#" << STR.at(shortestTimeRemaining).getPID() << " in shortest time remaining processor." << endl;
						totalWaitTime += STR.front().waitTime(processorTime.at(0));
						totalTurnaround += STR.front().turnaroundTime(processorTime.at(0));
						STR.erase(STR.begin() + shortestTimeRemaining); // delete old iteration of this process
					}
					if (STR.at(shortestTimeRemaining).isFirstResponse()) {
						totalResponseTime += STR.at(shortestTimeRemaining).age(processorTime.at(0));
					}
					goto label;//process was replaced so end continuation of processing on this step
				}
			}
			processorTime.at(0) += STR.at(shortestTimeRemaining).actualTimeRemaining();//add discrete event time jump for this processor
			//cout << "Finished processing PID#" << STR.at(shortestTimeRemaining).getPID() << " in shortest time remaining processor." << endl;
			if (STR.at(shortestTimeRemaining).isFirstResponse()) {
				totalResponseTime += STR.at(shortestTimeRemaining).age(processorTime.at(0));
			}
			totalWaitTime += STR.front().waitTime(processorTime.at(0));
			totalTurnaround += STR.front().turnaroundTime(processorTime.at(0));
			STR.erase(STR.begin() + shortestTimeRemaining); // delete old iteration of this process
		label:
			//check ages in queue
			for (unsigned int i = 0; i < STR.size(); i++) {
				if (STR.at(i).age(processorTime.at(0)) > STRtimeLimit) {
					STR.at(i).setTransferTime(processorTime.at(0));//set tranfer time
					RR1.push_back(STR.at(i));//move process to RR1 processor queue
					STR.erase(STR.begin() + i); // delete old iteration of this process
				}
			}
		}
		cout << "FINISHED STR stage" << endl;



		int processTimeOverlap;
		int index = 0;
		while (!RR1.empty()) {//process RR1 from time 0 to final
			if (index + 1 >= RR1.size()) {
				index = 0;
			}
			if (RR1.at(index).getTransferTime() <= processorTime.at(1)) {//index process is in this queue at this time. Start processing!
				processTimeOverlap = RR1.at(index).processSomeTime(RR1ProcessTime);//processes time, returns time needed to complete, 0 means completed, negative means completed early
				if (RR1.at(index).isFirstResponse()) {
					totalResponseTime += RR1.at(index).age(processorTime.at(1));
				}
				if (processTimeOverlap <= 0) {//process 1 second and check to see if it was completed
					processorTime.at(1) += RR1ProcessTime + processTimeOverlap;
					//cout << "Finished processing PID#" << RR1.at(index).getPID() << " in round robin 1 processor" << endl;
					totalWaitTime += RR1.at(index).waitTime(processorTime.at(1));
					totalTurnaround += RR1.at(index).turnaroundTime(processorTime.at(1));
					RR1.erase(RR1.begin() + index); // delete process from queue
				}
				else {//if process not completed then first check if it is too old for the queue
					processorTime.at(1) += RR1ProcessTime + contextSwitch;
					totalContextSwitchTime += contextSwitch;
					if (RR1.at(index).age(processorTime.at(1)) > RR1timeLimit) {
						RR3.push_back(RR1.at(index));//move process to RR3 processor queue
						RR1.erase(RR1.begin() + index); // delete old iteration of this process
					}

				}
				index++;
			}
			else {
				if (RR1.at(0).getTransferTime() >= processorTime.at(1)) {//if no processes are in fast forward to time when next process enters queue
					processorUtilization += RR1.at(0).getTransferTime() - processorTime.at(1);
					processorTime.at(1) = RR1.at(0).getTransferTime();
				}

				index = 0;
			}

		}
		cout << "Finished RR1 stage." << endl;
		index = 0;
		while (!RR3.empty()) {//process RR3 from time 0 to final
			if (index + 1 >= RR3.size()) {
				index = 0;
			}
			if (RR3.at(index).getTransferTime() <= processorTime.at(2)) {//index process is in this queue at this time. Start processing!
				processTimeOverlap = RR3.at(index).processSomeTime(RR3ProcessTime);//processes time, returns time needed to complete, 0 means completed, negative means completed early
				if (RR3.at(index).isFirstResponse()) {
					totalResponseTime += RR3.at(index).age(processorTime.at(2));
				}
				if (processTimeOverlap <= 0) {//process 1 second and check to see if it was completed
					processorTime.at(2) += RR3ProcessTime + processTimeOverlap;
				//	cout << "Finished processing PID#" << RR3.at(index).getPID() << " in round robin 3 processor" << endl;
					totalWaitTime += RR3.at(index).waitTime(processorTime.at(2));
					totalTurnaround += RR3.at(index).turnaroundTime(processorTime.at(2));
					RR3.erase(RR3.begin() + index); // delete process from queue
				}
				else {//if process not completed then first check if it is too old for the queue
					processorTime.at(2) += RR3ProcessTime + contextSwitch;
					totalContextSwitchTime += contextSwitch;
					if (RR3.at(index).age(processorTime.at(2)) > RR3timeLimit) {
						RR5.push_back(RR3.at(index));//move process to RR3 processor queue
						RR3.erase(RR3.begin() + index); // delete old iteration of this process
					}

				}
				index++;
			}
			else {
				if (RR3.at(0).getTransferTime() >= processorTime.at(2)) {//if no processes are in fast forward to time when next process enters queue
					processorUtilization += RR3.at(0).getTransferTime() - processorTime.at(2);
					processorTime.at(2) = RR3.at(0).getTransferTime();
				}

				index = 0;
			}

		}
		cout << "Finished RR3 stage." << endl;
		index = 0;

		while (!RR5.empty()) {//process RR5 from time 0 to final
			if (index + 1 >= RR5.size()) {
				index = 0;
			}
			if (RR5.at(index).getTransferTime() <= processorTime.at(3)) {//index process is in this queue at this time. Start processing!
				processTimeOverlap = RR5.at(index).processSomeTime(RR5ProcessTime);//processes time, returns time needed to complete, 0 means completed, negative means completed early
				if (RR5.at(index).isFirstResponse()) {
					totalResponseTime += RR5.at(index).age(processorTime.at(3));
				}
				if (processTimeOverlap <= 0) {//process 1 second and check to see if it was completed
					processorTime.at(3) += RR5ProcessTime + processTimeOverlap;
					//cout << "Finished processing PID#" << RR5.at(index).getPID() << " in round robin 5 processor" << endl;
					totalWaitTime += RR5.at(index).waitTime(processorTime.at(3));
					totalTurnaround += RR5.at(index).turnaroundTime(processorTime.at(3));
					RR5.erase(RR5.begin() + index); // delete process from queue
				}
				else {//if process not completed then first check if it is too old for the queue
					processorTime.at(3) += RR5ProcessTime + contextSwitch;
					totalContextSwitchTime += contextSwitch;
					if (RR5.at(index).age(processorTime.at(3)) > RR5timeLimit) {
						RR10.push_back(RR5.at(index));//move process to RR5 processor queue
						RR5.erase(RR5.begin() + index); // delete old iteration of this process
					}

				}
				index++;
			}
			else {
				if (RR5.at(0).getTransferTime() >= processorTime.at(3)) {//if no processes are in fast forward to time when next process enters queue
					processorUtilization += RR5.at(0).getTransferTime() - processorTime.at(3);
					processorTime.at(3) = RR5.at(0).getTransferTime();
				}

				index = 0;
			}

		}
		cout << "Finished RR5 stage." << endl;
		index = 0;
		while (!RR10.empty()) {//process RR10 from time 0 to final
			if (index + 1 >= RR10.size()) {
				index = 0;
			}
			if (RR10.at(index).getTransferTime() <= processorTime.at(4)) {//index process is in this queue at this time. Start processing!
				processTimeOverlap = RR10.at(index).processSomeTime(RR10ProcessTime);//processes time, returns time needed to complete, 0 means completed, negative means completed early
				if (RR10.at(index).isFirstResponse()) {
					totalResponseTime += RR10.at(index).age(processorTime.at(4));
				}
				if (processTimeOverlap <= 0) {//process 1 second and check to see if it was completed
					processorTime.at(4) += RR10ProcessTime + processTimeOverlap;
					//cout << "Finished processing PID#" << RR10.at(index).getPID() << " in round robin 10 processor" << endl;
					totalWaitTime += RR10.at(index).waitTime(processorTime.at(4));
					totalTurnaround += RR10.at(index).turnaroundTime(processorTime.at(4));
					RR10.erase(RR10.begin() + index); // delete process from queue
				}
				else {//if process not completed then first check if it is too old for the queue
					processorTime.at(4) += RR10ProcessTime + contextSwitch;
					totalContextSwitchTime += contextSwitch;
					if (RR10.at(index).age(processorTime.at(4)) > RR10timeLimit) {
						FCFS.push_back(RR10.at(index));//move process to RR10 processor queue
						RR10.erase(RR10.begin() + index); // delete old iteration of this process
					}

				}
				index++;
			}
			else {
				if (RR10.at(0).getTransferTime() >= processorTime.at(4)) {//if no processes are in fast forward to time when next process enters queue
					processorUtilization += RR10.at(0).getTransferTime() - processorTime.at(4);
					processorTime.at(4) = RR10.at(0).getTransferTime();
				}

				index = 0;
			}

		}
		cout << "Finished RR10 stage." << endl;
		while (!FCFS.empty()) {
			if (FCFS.at(0).getTransferTime() <= processorTime.at(5)) {//first process is in this queue at this time. Start processing!
				processorTime.at(5) += FCFS.at(0).actualTimeRemaining();
				//cout << "Finished processing PID#" << FCFS.at(0).getPID() << " in first come first serve processor" << endl;
				totalWaitTime += FCFS.at(0).waitTime(processorTime.at(5));
				totalTurnaround += FCFS.at(0).turnaroundTime(processorTime.at(5));
				if (FCFS.at(0).isFirstResponse()) {
					totalResponseTime += FCFS.at(0).age(processorTime.at(5));
				}
				FCFS.erase(FCFS.begin()); // delete process from queue
			}
			else {
				if (FCFS.at(0).getTransferTime() >= processorTime.at(5)) {//if no processes are in, fast forward to time when next process enters queue
					processorUtilization += FCFS.at(0).getTransferTime() - processorTime.at(5);
					processorTime.at(5) = FCFS.at(0).getTransferTime();
				}
			}

		}
		cout << "Finished FCFS stage." << endl << "Finished all processes!" << endl;
		//FINISHED SIMULATION CALCULATIONS

	//now analysis data is calculated and output to file
		int avgTurnaround = totalTurnaround / totalNumProcesses;
		int avgWaitTime = totalWaitTime / totalNumProcesses;
		int avgResponse = totalResponseTime / totalNumProcesses;
		int totalthroughput = totalNumProcesses; //how many processes
		totalContextSwitchTime;
		processorUtilization;

		//OUTPUT DATA HERE
		printToExcel(outdata, RRtimeFactor, avgTurnaround, avgWaitTime, avgResponse);


		RRtimeFactor += 5;
	}

	
	return 0;
}