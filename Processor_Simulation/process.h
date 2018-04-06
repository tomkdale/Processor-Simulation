
class process //TODO create fully functioning process object
{
public:
	process();
	int actualTimeRemaining();
	int estimatedTimeRemaining();
	int startTime();
	int getPID();
	int processSomeTime(int timeProcessed);
	int age(int currentTime);
	int waitTime(int currentTime);
	int turnaroundTime(int currentTime);
	bool isFirstResponse();
private:
	int PID, arrivalTime, cpuTime1, iO1, cpuTime2, iO2; // initialized data
	int timePreviouslyProcessed = 0;
	bool hasResponded = false;
};

process::process()
{
	//TODO initialize data 
}
int process::actualTimeRemaining() {//returns time remaining to be processed
	return cpuTime1 + iO1 + cpuTime2 + iO2 - timePreviouslyProcessed;
}
int process::estimatedTimeRemaining() {
	return (actualTimeRemaining()) + (rand() % 40 - 20) - timePreviouslyProcessed; //actual Time remaining +/- 20
}
int process::startTime() {//returns the time that the process first enters the queue
	return arrivalTime;
}
int process::getPID() {
	return PID;
}
int process::processSomeTime(int timeProcessed)//should decrement the timeRemaining because it was half processed, return 0 if finished
{
	timePreviouslyProcessed += timeProcessed;
	return actualTimeRemaining();
	//returning 0 means the process finished exacly in that amount of time
	//positive number means process still has time to go
	//negative number means it finished that much before the processor expected
}
int process::age(int currentTime)//return total time since arrival time
{
	return currentTime - arrivalTime;
}

int process::waitTime(int currentTime)//return time spent in queues a.k.a. turnaroundTime - processed time
{
	return currentTime - arrivalTime - (cpuTime1 + iO1 + cpuTime2 + iO2);
}

int process::turnaroundTime(int currentTime)//return total time since arrival (same as age())
{
	return currentTime - arrivalTime;
}

bool process::isFirstResponse()//returns true if not yet responded, every time after will return false
{
	if (hasResponded == false) {
		hasResponded = true;
		return true;
	}
	return false;
}
