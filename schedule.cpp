#include <string>
#include <iostream>
#include <ctime>
#include <vector>

using namespace std;

/* program overview:

main data structure: a boolean array, true = available
array size: 7*28 representing 7 days & 28 time blocks (800-830, 830-900...2130-2200)


1. get current time & 22:00 at 7 days from today 
(represented in seconds elapsed from 1900, c++ time module default representation)

2. parse input line by line, ignore user id (doesn't matter who is busy)
create c++ tm structure (with year, month, day, hour .ect ) based on input

3. use "difftime" method from ctime module, check if the input time is within the week
if so calculate the day difference & the time block and set its corresponding index to schedule
if not set the index as 0 or 7*28-1 based on if it's too early or too late

4. with the start time index & end time index as range boundary, mark the boolean array as false
optimization: 
 4.1 whenever a cell is marked true->false, total avail (initially 7*28) will decrease one
     thus if total avail = 0, no need to read more from the file since the entire week is busy

 4.2 whenever a cell is marked true->false, the corresponding day's avail decrease
     thus if the day's avail is 0, no need to check the longest block of avail time later

5. after marking the boolean array, 
   loop through the 1D array of 7*28 elements (7 groups of 28 continuous elements)
   find the longest continuous "true" within among all groups & return the start index of the continuous block

6. with this index i: 
i/28 is how many days from today, i%28 indicate time block => decode the index to day& time 
the find max function will also return the length of the time block as an int, and each time block is half an hour

disclaimer: 
the algorithm & the implementation was designed entirely by me
I was not familar with the ctime module (how to get current time, how to compare time ect) and referred to cplusplus.com/reference

*/



int stringToInt(string s){
  istringstream is{s};
  int sInt;
  is >> sInt;
  return sInt;
}

//time in "2017-04-03 13:30:00 form"
//create a tm structure
void fillTime(tm& toFill, string time){
  toFill.tm_year = stringToInt(time.substr(0,4)) - 1900;
  toFill.tm_mon = stringToInt(time.substr(5,2)) -1;
  toFill.tm_mday = stringToInt(time.substr(8,2));
  toFill.tm_hour = stringToInt(time.substr(11,2)) -1;
  toFill.tm_min = stringToInt(time.substr(14,2));
}

//diff in second between time & current program time
double diffBegin(tm& time){
  time_t now = time(NULL);
  return difftime(mktime(&time), now);
}

//diff in second between 7 days from today & time
double diffFinish(tm& time){
  time_t now = time(NULL);
  //finish time is 22:00 7 days later than now
  tm finish = *localtime(&now);
  finish.tm_mday += 7; 
  finish.tm_hour = 22;
  finish.tm_min = 0;
  finish.tm_sec = 0;
  return difftime(mktime(&finish), mktime(&time));
}

//return the index correspond to schedule of busy start time
int timeToIndex(tm& time){
  
  //period finish - busy time
  if(diffFinish(time) <= 0) return 7*28-1;
  //busy time - now
  if(diffBegin(time) <= 0) return 0;

  //within the one week period
  time_t now = time(NULL);
  tm today = *localtime(&now);
  int day = time.tm_mday - today.tm_mday;
  int time = (time.tm_hour)*100 + time.tm_min;
  if(time <= 800) time = 800;
  if(time >= 2200) time = 2200;
  time = time%800;
  int block = 2*time/100 + (time%100)%30;
  return day*7 + block;    
}


//line: 1001, 2017-04-03 13:30:00, 2017-04-05 13:30:00
//parse input -> create time structure -> get corresponding index of start & end
void getIndex(int& startIndex, int& endIndex, string line){
  //read in input
  string sTime, eTime;
  istringstream iline{line};
  getline(iline, sTime, ","); //user id
  getline(iline, sTime, ","); //start time
  getline(iline, eTime, ","); //end time
  
  time_t now = time(NULL);
  tm start = *localtime(&now);
  tm end = *localtime(&now);
  //fill tm strcture of busy start & end
  fillTime(start, sTime);
  fillTime(end, eTime);

  //get corresponding index
  startIndex = timeToIndex(start);
  endIndex = timeToIndex(end);
}





//chop schedule into groups of 28(0-27, 28-55), check the longest continuous true
int getMax(vector<int>& schedule, vector<int>& sumDay, int maxAvail){
  if(schedule.size() != 7*28) throw "schedule bit map has wrong size\n";
   //longest continuous avail block
  int maxIndex = -1; //index of the start 

  for(int i = 0; i < 7; i++){
    if(sumDay[i] == 0) continue; //this day has no availability
    //find longest continues time block
    int maxCurDay = 0; //max block in this day so far
    int maxInd = 0; //start index of that max block
    bool found = false; 
    int count = 0; //continuous block count
    for(int j = 0; j < 28; j++){
       found = schedule[i*7 + j]; 
       if(schedule[i*7 + j]) count ++;//if this block available
          
       else{ //if this block not available, updat curMax with count
          maxCurDay = count > maxCurDay? count : maxCurDay;
          maxInd = count > maxCurDay? i*7 + j - count : maxInd;
       }
    }
      //compare max of this day with overall max so far
      if(maxCurDay > maxAvail){
        maxAvail = maxCurDay;
        maxIndex = maxInd;
      }
  }

  return maxIndex;
}



int main() {
  try{
    
    int sumAvail = 7*28; //to optimize, if no availability this week, no need to read all input return 
    vector<int> sumDay = (7, 28); //to optimize, if no availability in that day, no need to check for longest avail time block
    //today: 0*28 + (0 - 27), 0: 800-830, 1: 830-900
    //day 1: 1*28 + (0 - 27)
    //day 2: 2*28 + (0 - 27)
    //bit map of all the blocks
    vector<bool> schedule (7*28, true);
    ifstream fin{"schedule.csv"};
    string line;
    //finish program when no input or all week is busy 
    while(getline(fin, line) && sumAvail != 0){
      int startIndex, endIndex;
      getIndex(startIndex, endIndex, line);
      //mark all the busy time as false, decrease sumAvail
      for(int i = startIndex; i <= endIndex; i++){
        if(schedule[i]){
          schedule[i] = false;
          sumAvail --;
          sumDay[i/28] --;
        } //do nothing if time was marked
      }
    }

    if(sumAvail == 0) cout << "there is no available time for meet up this week\n";

    //get the day with longest availability
    //return the index of the start of the longest block
    //chop schedule into groups of 28, check the longest continuous true
    int maxAvail = 0;
    int longestBlock = getMax(schedule, sumDay, maxAvail);
    cout << longestBlock/28 << " days from today has the longest continuous block\n";
    cout << "time block starts from " << (longestBlock%28)/2+8 << " : " << (longestBlock%28)%2 * 30 << endl;
    cout << "for " << maxAvail/2 << "." << maxAvail%2 << "hours long\n";
  
  } catch(const string& msg){
    // If an exception occurs print the message and end the program
    cerr << msg << endl;
  }
    /* Enter your code here. Read input from STDIN. Print output to STDOUT */    
    return 0;
}

