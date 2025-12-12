/*
 * AttrifyApp.cpp - Application implementation
 */

#include "AttrifyApp.h"
#include "AttrifyWindow.h"
#include <Catalog.h>
#include <Entry.h>
#include <Path.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "AttrifyApp"

AttrifyApp::AttrifyApp()
	: BApplication(APP_SIGNATURE)
{
}

void AttrifyApp::ReadyToRun()
{
	// If no window is open, create an empty window
	if (CountWindows() == 0) {
		AttrifyWindow* window = new AttrifyWindow();
		window->Show();
	}
}

void AttrifyApp::RefsReceived(BMessage* message)
{
	entry_ref ref;
	int32 index = 0;
	
	// Open a window for each received file
	while (message->FindRef("refs", index++, &ref) == B_OK) {
		OpenFile(ref);
	}
}

void AttrifyApp::ArgvReceived(int32 argc, char** argv)
{
	// Process command line arguments
	for (int32 i = 1; i < argc; i++) {
		BEntry entry(argv[i]);
		entry_ref ref;
		
		if (entry.InitCheck() == B_OK && entry.GetRef(&ref) == B_OK) {
			OpenFile(ref);
		}
	}
}

void AttrifyApp::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case MSG_QUIT:
			PostMessage(B_QUIT_REQUESTED);
			break;
		
		default:
			BApplication::MessageReceived(message);
			break;
	}
}

void AttrifyApp::OpenFile(const entry_ref& ref)
{
	// Create a new window for this file
	AttrifyWindow* window = new AttrifyWindow(ref);
	window->Show();
}

bool AttrifyApp::QuitRequested()
{
	// Close all windows gracefully
	for (int32 i = CountWindows() - 1; i >= 0; i--) {
		BWindow* window = WindowAt(i);
		if (window) {
			// Lock window to safely check if it can quit
			if (window->Lock()) {
				// Ask if window can close (important for unsaved changes)
				if (!window->QuitRequested()) {
					window->Unlock();
					return false;  // User canceled quit
				}
				// Close the window
				window->Quit();
			}
		}
	}
	
	return true;  // All windows closed, app can quit
}

int main()
{
	AttrifyApp app;
	app.Run();
	return 0;
}