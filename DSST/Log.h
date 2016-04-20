#ifndef LOG_H
	void EventLog(int type,int instance, int state);
	void EventLog(int type,int instance, int state, char* str);
	int InitLog(char* file);
	void AddToLog(char* Line);
	void CloseLog();
	int LogStatus();
	
#endif