/*
 * AttrifyApp.h - Main application class
 */
 
#ifndef ATTRIFYAPP_H
#define ATTRIFYAPP_H

#include <Application.h>

#define APP_SIGNATURE "application/x-vnd.haikuinsider-attrify"

class AttrifyApp : public BApplication {
public:
	AttrifyApp();
	
	virtual void ReadyToRun();
	virtual void RefsReceived(BMessage* message);
	virtual void ArgvReceived(int32 argc, char** argv);
	virtual void MessageReceived(BMessage* message);
	virtual bool QuitRequested();

private:
	void OpenFile(const entry_ref& ref);
};

#endif // ATTRIFYAPP_H