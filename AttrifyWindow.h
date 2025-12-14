/*
 * AttrifyWindow.h - Main attribute editing window
 */
#ifndef ATTRIFYWINDOW_H
#define ATTRIFYWINDOW_H

#include <kernel/fs_attr.h>
#include <Window.h>
#include <Entry.h>
#include <MenuBar.h>
#include <FilePanel.h>
#include <ObjectList.h>
#include "Attribute.h"
#include "PictureView.h"

// Messages
const uint32 MSG_FILE_OPEN = 'FOpn';
const uint32 MSG_FILE_SAVE = 'FSav';
const uint32 MSG_FILE_SAVE_AS = 'FSvA';
const uint32 MSG_FILE_CLOSE = 'FClo';
const uint32 MSG_QUIT = 'Quit';
const uint32 MSG_ABOUT = 'Abou';
const uint32 MSG_FOLLOW_SYMLINK = 'FlSy';
const uint32 MSG_CONFIGURE_ATTRIBUTES = 'CfAt';
const uint32 MSG_CONFIGURE_SATTRIBUTES = 'CfSA';   
const uint32 MSG_SHOW_IN_TRACKER = 'ShTr'; 
const uint32 MSG_FILE_NEW = 'fnew';
const uint32 MSG_FILE_CLOSE_OTHERS = 'ClOt';

class AttrifyWindow : public BWindow {
public:
	AttrifyWindow();
	AttrifyWindow(const entry_ref& ref);
	~AttrifyWindow();
	
	virtual bool QuitRequested();
	virtual void MessageReceived(BMessage* message);
	
	void LoadFile(const entry_ref& ref);
	void SaveFile();
	void SaveFileAs();
	void NewFile();
	void ShowAbout();
	
private:
	void BuildMenu();
	void LoadAttributesFromFile();
	void LoadAttributesFromMime();
	void LoadGenericMimeAttributes();
	void BuildAttributeView();
	void WriteAttributesToFile();
	void SortAttributes();
	void ConfigureAttributes(const BString* mimetype);

	void HandleRefsReceived(BMessage* message);
	
	entry_ref fFileRef;
	bool fFileLoaded;
	bool fModified;
	
	BMenuBar* fMenuBar;
	PictureView* fPictureView;
	BView* fAttributeView;
	
	BFilePanel* fOpenPanel;
	BFilePanel* fSavePanel;
	
#if B_HAIKU_VERSION <= B_HAIKU_VERSION_1_BETA_5
	BObjectList<Attribute> fAttributes;
#else
	BObjectList<Attribute, true> fAttributes;
#endif
	BString fMimeType;
};

#endif // ATTRIFYWINDOW_H
