/*
 * AttrifyWindow.cpp - Main window implementation
 */

#include "AttrifyApp.h"
#include "AttrifyWindow.h"
#include "AttributeControl.h"
#include <AboutWindow.h>
#include <Application.h>
#include <Button.h>
#include <ControlLook.h>
#include <LayoutBuilder.h>
#include <GroupLayout.h>
#include <GridLayout.h>
#include <GridLayoutBuilder.h>
#include <MenuBar.h>
#include <MenuItem.h>
#include <Alert.h>
#include <Node.h>
#include <NodeInfo.h>
#include <MimeType.h>
#include <Path.h>
#include <Catalog.h>
#include <Locale.h>
#include <String.h>
#include <StringView.h>
#include <ScrollView.h>
#include <SymLink.h>
#include <fs_attr.h>
#include <Roster.h>

#include <vector>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "Attrify"


AttrifyWindow::AttrifyWindow()
	: BWindow(BRect(100, 100, 600, 500), B_TRANSLATE("Attrify"),
		B_TITLED_WINDOW, B_ASYNCHRONOUS_CONTROLS)
	, fFileLoaded(false)
	, fModified(false)
	, fMenuBar(NULL)
	, fPictureView(NULL)
	, fAttributeView(NULL)
	, fOpenPanel(NULL)
	, fSavePanel(NULL)
#if B_HAIKU_VERSION <= B_HAIKU_VERSION_1_BETA_5
	, fAttributes(20, true)
#else
	, fAttributes(20)
#endif
{
	BuildMenu();
	
	fMimeType = "application/x-person";
	SetTitle(B_TRANSLATE("Attrify - New Person"));
	
	fPictureView = new PictureView();
	fPictureView->SetMimeType(fMimeType);
	
	// Add BEOS:TYPE attribute first
	Attribute* typeAttr = new Attribute("BEOS:TYPE", B_MIME_STRING_TYPE);
	typeAttr->SetValue(fMimeType);
	fAttributes.AddItem(typeAttr);
	
	LoadAttributesFromMime();
	LoadGenericMimeAttributes();
	
	fAttributeView = new BView("AttributeView", B_WILL_DRAW);
	fAttributeView->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));

	// Create a ScrollView for attributes
	BScrollView* scrollView = new BScrollView("ScrollView", fAttributeView, 
		B_WILL_DRAW | B_FRAME_EVENTS, false, true);
	
	// Force size limits on the ScrollView itself
	scrollView->SetExplicitMinSize(BSize(B_SIZE_UNSET, 100));  // Min height 100px
	scrollView->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNLIMITED));  // No max limit

	//scrollView->SetExplicitMinSize(BSize(scrollView->MinSize().width, 0));
	
	BGridLayout* grid = new BGridLayout();
	float spacing = be_control_look->DefaultItemSpacing();
	grid->SetInsets(spacing, spacing, spacing, spacing);
	grid->SetHorizontalSpacing(spacing);
	grid->SetVerticalSpacing(spacing);
	fAttributeView->SetLayout(grid);

	// Create a view for the banner + icon
	BView* leftPanel = new BView("LeftPanel", B_WILL_DRAW);
	leftPanel->SetViewColor(tint_color(ui_color(B_PANEL_BACKGROUND_COLOR), B_DARKEN_1_TINT));

	leftPanel->SetExplicitMinSize(BSize(100, B_SIZE_UNSET));
	leftPanel->SetExplicitMaxSize(BSize(100, B_SIZE_UNSET));

	BGroupLayout* leftLayout = new BGroupLayout(B_VERTICAL);
	//leftLayout->SetInsets(spacing, spacing, 0, 0);
	leftPanel->SetLayout(leftLayout);
	leftPanel->AddChild(fPictureView);
	
	BLayoutBuilder::Group<>(this, B_VERTICAL, 0)
		.SetInsets(0)
		.Add(fMenuBar)
		.AddGroup(B_HORIZONTAL, 0)
			.Add(leftPanel)
			.AddGroup(B_VERTICAL, 0)
				.Add(scrollView, 1)
			.End()
		.End()
	.End();
	
	SetSizeLimits(400, 10000, 300, 10000);
	
	BuildAttributeView();
	
	CenterOnScreen();
}


AttrifyWindow::AttrifyWindow(const entry_ref& ref)
	: AttrifyWindow()
{
	LoadFile(ref);
}

AttrifyWindow::~AttrifyWindow()
{
	delete fOpenPanel;
	delete fSavePanel;
}

void AttrifyWindow::BuildMenu()
{
	fMenuBar = new BMenuBar("MenuBar");
	
	// File Menu
	BMenu* fileMenu = new BMenu(B_TRANSLATE("File"));
	fileMenu->AddItem(new BMenuItem(B_TRANSLATE("New"), 
		new BMessage(MSG_FILE_NEW), 'N'));
	fileMenu->AddSeparatorItem();	
	fileMenu->AddItem(new BMenuItem(B_TRANSLATE("Open" B_UTF8_ELLIPSIS), 
		new BMessage(MSG_FILE_OPEN), 'O'));
	fileMenu->AddSeparatorItem();
	fileMenu->AddItem(new BMenuItem(B_TRANSLATE("Save"), 
		new BMessage(MSG_FILE_SAVE), 'S'));
	fileMenu->AddItem(new BMenuItem(B_TRANSLATE("Save as" B_UTF8_ELLIPSIS), 
		new BMessage(MSG_FILE_SAVE_AS), 'S', B_SHIFT_KEY));
	fileMenu->AddSeparatorItem();
	fileMenu->AddItem(new BMenuItem(B_TRANSLATE("About"), 
		new BMessage(MSG_ABOUT)));
	fileMenu->AddSeparatorItem();
	fileMenu->AddItem(new BMenuItem(B_TRANSLATE("Close"), 
		new BMessage(MSG_FILE_CLOSE), 'W'));
	fileMenu->AddItem(new BMenuItem(B_TRANSLATE("Close others"), 
		new BMessage(MSG_FILE_CLOSE_OTHERS), 'W', B_SHIFT_KEY));
	fileMenu->AddItem(new BMenuItem(B_TRANSLATE("Quit"), 
		new BMessage(MSG_QUIT), 'Q'));
	
	fMenuBar->AddItem(fileMenu);
	
	// Edit Menu 
	BMenu* editMenu = new BMenu(B_TRANSLATE("Edit"));
	editMenu->AddItem(new BMenuItem(B_TRANSLATE("Configure mime attributes" B_UTF8_ELLIPSIS), 
		new BMessage(MSG_CONFIGURE_ATTRIBUTES)));
	editMenu->AddItem(new BMenuItem(B_TRANSLATE("Configure super mime" B_UTF8_ELLIPSIS), 
		new BMessage(MSG_CONFIGURE_SATTRIBUTES)));
	
	fMenuBar->AddItem(editMenu);

}

bool AttrifyWindow::QuitRequested()
{
	if (fModified) {
		BAlert* alert = new BAlert(B_TRANSLATE("Save changes?"),
			B_TRANSLATE("The file has been modified. Save changes?"),
			B_TRANSLATE("Cancel"), B_TRANSLATE("Don't save"), 
			B_TRANSLATE("Save"));
		
		int32 result = alert->Go();
		if (result == 0)
			return false;
		if (result == 2)
			SaveFile();
	}

	// If Last window, close the application
	if (be_app->CountWindows() == 1) {
		be_app->PostMessage(B_QUIT_REQUESTED);
	}
	
	// Only close this window, not the entire application
	return true;
}

void AttrifyWindow::HandleRefsReceived(BMessage* message)
{
	entry_ref ref;
	int32 index = 0;
	
	// Open each file
	while (message->FindRef("refs", index++, &ref) == B_OK) {
		if (fFileLoaded) {
			// If this window already has a file, create a new one
			AttrifyWindow* window = new AttrifyWindow(ref);
			window->Show();
		} else {
			// Otherwise use this window
			LoadFile(ref);
		}
	}
}

void AttrifyWindow::MessageReceived(BMessage* message)
{
    if (message->WasDropped()) {
    	HandleRefsReceived(message);
        return;
    }
	
	
	switch (message->what) {
	
		case MSG_FILE_NEW: {
			if (fModified) {
				BAlert* alert = new BAlert(B_TRANSLATE("Save changes?"),
					B_TRANSLATE("The file has been modified. Save changes before creating a new file?"),
					B_TRANSLATE("Cancel"), B_TRANSLATE("Don't save"), 
					B_TRANSLATE("Save"));
				
				int32 result = alert->Go();
				if (result == 0)  // Cancel
					break;
				if (result == 2)  // Save
					SaveFile();
			}
			
			// Reinit window 
			NewFile();
			break;
		}
		
		case MSG_FILE_OPEN: {
			if (fOpenPanel == nullptr) {
				fOpenPanel = new BFilePanel(B_OPEN_PANEL, new BMessenger(this));
			}
			fOpenPanel->Show();
			break;
		}
		
		case B_REFS_RECEIVED: {
			HandleRefsReceived(message);
			break;
		}
		
		case MSG_FILE_SAVE:
			SaveFile();
			break;
		
		case MSG_FILE_SAVE_AS:
			SaveFileAs();
			break;
	
		case MSG_FILE_CLOSE_OTHERS: {
			// Close all windows except this one
			for (int32 i = be_app->CountWindows() - 1; i >= 0; i--) {
				BWindow* window = be_app->WindowAt(i);
				if (window && window != this) {
					window->PostMessage(B_QUIT_REQUESTED);
				}
			}
			break;
		}

		case MSG_FILE_CLOSE:
			PostMessage(B_QUIT_REQUESTED);
			break;
		
		case MSG_QUIT:
			be_app->PostMessage(MSG_QUIT);
			break;
		
		case MSG_ABOUT: {
			ShowAbout();
			break;
		}
		
		case MSG_ATTRIBUTE_CHANGED:
			fModified = true;
			break;

		case MSG_CONFIGURE_ATTRIBUTES:
			ConfigureAttributes(&fMimeType);
			break;

		case MSG_CONFIGURE_SATTRIBUTES: {
			// Extract the supertype (e.g., "text/plain" -> "text")
			int32 slashPos = fMimeType.FindFirst('/');
			if (slashPos > 0) {
				BString superType;
				fMimeType.CopyInto(superType, 0, slashPos);
				ConfigureAttributes(&superType);
			}
			break;
		}
		
		case MSG_SHOW_IN_TRACKER: {
			// Resolve the entry (handles both symlinks and regular files)
			BEntry entry(&fFileRef, true); // true = traverse symlinks
			
			if (entry.InitCheck() != B_OK || !entry.Exists()) {
				// If traversal fails, try without resolving
				entry.SetTo(&fFileRef, false);
			}
			
			entry_ref resolvedRef;
			if (entry.GetRef(&resolvedRef) != B_OK)
				break;
			
			BMessenger tracker("application/x-vnd.Be-TRAK");
			if (!tracker.IsValid())
				break;
			
			// Check if it's a directory
			if (entry.IsDirectory()) {
				// Open the directory directly
				BMessage openMsg(B_REFS_RECEIVED);
				openMsg.AddRef("refs", &resolvedRef);
				tracker.SendMessage(&openMsg);
			} else {
				// It's a file: we need to open parent folder first, then select
				BEntry parent;
				entry_ref parentRef;
				
				if (entry.GetParent(&parent) != B_OK || parent.GetRef(&parentRef) != B_OK)
					break;
				
				// Step 1: Open the parent folder
				BMessage openMsg(B_REFS_RECEIVED);
				openMsg.AddRef("refs", &parentRef);
				tracker.SendMessage(&openMsg);
				
				// Step 2: Wait a bit for the window to open, then select the file
				snooze(200000); // 200ms - as in Tracker's code
				
				BMessage selectMsg('Tsel'); // kSelect
				selectMsg.AddRef("refs", &resolvedRef);
				tracker.SendMessage(&selectMsg);
			}
			break;
		}
		
		case MSG_FOLLOW_SYMLINK: {
			BSymLink symlink(&fFileRef);
			char linkPath[B_PATH_NAME_LENGTH];
			
			if (symlink.ReadLink(linkPath, sizeof(linkPath)) > 0) {
				// Resolve the path (handle relative paths)
				BPath basePath(&fFileRef);
				basePath.GetParent(&basePath);
				
				BPath targetPath;
				if (linkPath[0] == '/') {
					// Absolute path
					targetPath.SetTo(linkPath);
				} else {
					// Relative path
					targetPath.SetTo(basePath.Path(), linkPath);
				}
				
				BEntry entry(targetPath.Path());
				entry_ref targetRef;
				if (entry.GetRef(&targetRef) == B_OK) {
					// Launch with Tracker (default application)
					BMessenger tracker("application/x-vnd.Be-TRAK");
					BMessage openMsg(B_REFS_RECEIVED);
					openMsg.AddRef("refs", &targetRef);
					tracker.SendMessage(&openMsg);
				} else {
					BAlert* alert = new BAlert(B_TRANSLATE("Error"),
						B_TRANSLATE("Cannot open target file."),
						B_TRANSLATE("OK"));
					alert->Go();
				}
			}
			break;
		}
		
		case B_SAVE_REQUESTED: {
			entry_ref dirRef;
			const char* name;
			
			if (message->FindRef("directory", &dirRef) == B_OK &&
			    message->FindString("name", &name) == B_OK) {
				
				// Build the full path 
				BPath path(&dirRef);
				path.Append(name);
				
				// Create or open the file
				BFile file(path.Path(), B_READ_WRITE | B_CREATE_FILE | B_ERASE_FILE);
				if (file.InitCheck() != B_OK) {
					BAlert* alert = new BAlert(B_TRANSLATE("Error"),
						B_TRANSLATE("Cannot create file."),
						B_TRANSLATE("OK"));
					alert->Go();
					break;
				}
				
				// Get entry_ref of the new file 
				BEntry entry(path.Path());
				if (entry.GetRef(&fFileRef) == B_OK) {
					fFileLoaded = true;
					
					// Update the title
					BString title = B_TRANSLATE("Attrify - ");
					title << name;
					SetTitle(title.String());
					
					// Define the MIME type of the file
					BNodeInfo nodeInfo(&file);
					nodeInfo.SetType(fMimeType.String());
					
					// Save the attributes
					WriteAttributesToFile();
					fModified = false;
				}
			}
			break;
		}
		
		default:
			BWindow::MessageReceived(message);
			break;
	}
}

void
AttrifyWindow::ShowAbout()
{
	BAboutWindow* aboutwindow
		= new BAboutWindow(B_TRANSLATE_SYSTEM_NAME("Attrify"), APP_SIGNATURE);

	const char* authors[] = {
		"DigitalBox",
		NULL
	};

	const char* thanks[] = {
		"ZuMi for the Icon",
		NULL
	};

	aboutwindow->AddCopyright(2025, "Haiku Insider");
	aboutwindow->AddAuthors(authors);
	aboutwindow->AddSpecialThanks(thanks);

	aboutwindow->ResizeTo(400, 320);

	aboutwindow->Show();
}	

void AttrifyWindow::LoadFile(const entry_ref& ref)
{
	fFileRef = ref;
	fFileLoaded = true;
	fModified = false;
	
	BPath path(&ref);
	BString title = B_TRANSLATE("Attrify - ");
	title << path.Leaf();
	SetTitle(title.String());
	
	// Determine the MIME type
	BNode node(&ref);
	BNodeInfo nodeInfo(&node);
	char mimeString[B_MIME_TYPE_LENGTH];
	if (nodeInfo.GetType(mimeString) == B_OK) {
		fMimeType = mimeString;
	} else {
		// No MIME type found
		fMimeType = "N/A";
	}
	
	fPictureView->SetMimeType(fMimeType);
	
	bool isSymlink = (fMimeType == "application/x-vnd.Be-symlink");
	fPictureView->SetClickable(isSymlink);  
	
	if (isSymlink) {
		fPictureView->SetClickableToolTip(B_TRANSLATE("Show link"));
	} else {
		fPictureView->SetClickableToolTip(NULL);  // No tooltip
	}

	// Load attributes
	fAttributes.MakeEmpty();
	// Add BEOS:TYPE attribute first (with actual or N/A value)
	Attribute* typeAttr = new Attribute("BEOS:TYPE", B_MIME_STRING_TYPE);
	typeAttr->SetValue(fMimeType);
	fAttributes.AddItem(typeAttr);
	// Only load MIME-based attributes if we have a valid MIME type
	if (fMimeType != "N/A") {
		LoadAttributesFromMime();
		LoadGenericMimeAttributes();
	}	
	LoadAttributesFromFile();
	
	SortAttributes(); 
	BuildAttributeView();

}

void AttrifyWindow::NewFile()
{
	fFileLoaded = false;
	fModified = false;
	fMimeType = "application/x-person";
	
	SetTitle(B_TRANSLATE("Attrify - New Person"));
	
	fPictureView->SetMimeType(fMimeType);
	fPictureView->SetClickable(false);
	fPictureView->SetClickableToolTip(NULL);
	
	fAttributes.MakeEmpty();
	
	// Add BEOS:TYPE attribute first
	Attribute* typeAttr = new Attribute("BEOS:TYPE", B_MIME_STRING_TYPE);
	typeAttr->SetValue(fMimeType);
	fAttributes.AddItem(typeAttr);
	
	LoadAttributesFromMime();
	LoadGenericMimeAttributes();
	
	BuildAttributeView();
}

void AttrifyWindow::LoadAttributesFromMime()
{
	if (fMimeType.Length() == 0)
		return;
	
	BMimeType mime(fMimeType.String());
	if (!mime.IsValid())
		return;
	
	BMessage attrInfo;
	if (mime.GetAttrInfo(&attrInfo) != B_OK)
		return;
	
	const char* attrName;
	int32 attrType;
	for (int32 i = 0; attrInfo.FindString("attr:name", i, &attrName) == B_OK; i++) {
		if (attrInfo.FindInt32("attr:type", i, &attrType) == B_OK) {
			Attribute* attr = new Attribute(attrName, (type_code)attrType);
			fAttributes.AddItem(attr);
		}
	}
}

void AttrifyWindow::LoadGenericMimeAttributes()
{
	if (fMimeType.Length() == 0)
		return;
	
	// Extract the supertype (e.g., "text/plain" -> "text")
	int32 slashPos = fMimeType.FindFirst('/');
	if (slashPos < 0)
		return;
	
	BString superType;
	fMimeType.CopyInto(superType, 0, slashPos);
	
	BMimeType mime(superType.String());
	if (!mime.IsValid())
		return;
	
	BMessage attrInfo;
	if (mime.GetAttrInfo(&attrInfo) != B_OK)
		return;
	
	const char* attrName;
	int32 attrType;
	for (int32 i = 0; attrInfo.FindString("attr:name", i, &attrName) == B_OK; i++) {
		// Don't add if already present
		bool found = false;
		for (int32 j = 0; j < fAttributes.CountItems(); j++) {
			if (fAttributes.ItemAt(j)->Name() == attrName) {
				found = true;
				break;
			}
		}
		
		if (!found && attrInfo.FindInt32("attr:type", i, &attrType) == B_OK) {
			Attribute* attr = new Attribute(attrName, (type_code)attrType);
			fAttributes.AddItem(attr);
		}
	}
}

void AttrifyWindow::LoadAttributesFromFile()
{
	BNode node(&fFileRef);
	
	// CRITICAL: Check if node was initialized successfully
	if (node.InitCheck() != B_OK) {
		BAlert* alert = new BAlert(B_TRANSLATE("Error"),
			B_TRANSLATE("Cannot open file for reading attributes."),
			B_TRANSLATE("OK"), nullptr, nullptr, B_WIDTH_AS_USUAL, B_STOP_ALERT);
		alert->Go();
		return;
	}
	
	char attrName[B_ATTR_NAME_LENGTH];
	if (node.RewindAttrs() != B_OK) {
		// Unable to start attribute iteration
		return;
	}
	
	while (node.GetNextAttrName(attrName) == B_OK) {
		attr_info info;
		if (node.GetAttrInfo(attrName, &info) != B_OK)
			continue;
		
		// Find or create attribute object
		Attribute* attr = nullptr;
		for (int32 i = 0; i < fAttributes.CountItems(); i++) {
			if (fAttributes.ItemAt(i)->Name() == attrName) {
				attr = fAttributes.ItemAt(i);
				break;
			}
		}
		
		if (!attr) {
			attr = new Attribute(attrName, info.type);
			fAttributes.AddItem(attr);
		}
		
		// Dynamic buffer based on actual attribute size
		// This prevents buffer overflow for large attributes
		std::vector<char> buffer(info.size);
		ssize_t bytesRead = node.ReadAttr(attrName, info.type, 0,
		                                  buffer.data(), buffer.size());
		
		// Check for read errors
		if (bytesRead < 0) {
			// Attribute read failed - skip it
			continue;
		}
		
		if (bytesRead == 0)
			continue;
		
		// Validate buffer size before type casting
		switch (info.type) {
			case B_BOOL_TYPE:
				if (static_cast<size_t>(bytesRead) >= sizeof(bool))
					attr->SetValue(*reinterpret_cast<bool*>(buffer.data()));
				break;
				
			case B_INT8_TYPE:
				if (static_cast<size_t>(bytesRead) >= sizeof(int8))
					attr->SetValue(*reinterpret_cast<int8*>(buffer.data()));
				break;
				
			case B_INT16_TYPE:
				if (static_cast<size_t>(bytesRead) >= sizeof(int16))
					attr->SetValue(*reinterpret_cast<int16*>(buffer.data()));
				break;
				
			case B_INT32_TYPE:
				if (static_cast<size_t>(bytesRead) >= sizeof(int32))
					attr->SetValue(*reinterpret_cast<int32*>(buffer.data()));
				break;
				
			case B_INT64_TYPE:
				if (static_cast<size_t>(bytesRead) >= sizeof(int64))
					attr->SetValue(*reinterpret_cast<int64*>(buffer.data()));
				break;
				
			case B_FLOAT_TYPE:
				if (static_cast<size_t>(bytesRead) >= sizeof(float))
					attr->SetValue(*reinterpret_cast<float*>(buffer.data()));
				break;
				
			case B_DOUBLE_TYPE:
				if (static_cast<size_t>(bytesRead) >= sizeof(double))
					attr->SetValue(*reinterpret_cast<double*>(buffer.data()));
				break;
				
			case B_TIME_TYPE:
				if (static_cast<size_t>(bytesRead) >= sizeof(time_t))
					attr->SetValue(*reinterpret_cast<time_t*>(buffer.data()));
				break;
				
			case B_STRING_TYPE:
			case B_MIME_STRING_TYPE:
				// Ensure null termination for strings
				if (bytesRead > 0 && buffer[bytesRead - 1] != '\0') {
					buffer.push_back('\0');
				}
				attr->SetValue(BString(buffer.data()));
				break;
				
			case B_RAW_TYPE:
				attr->SetRawValue(buffer.data(), bytesRead);
				break;
		}
	}
}

void AttrifyWindow::BuildAttributeView()
{
	// Remove all existing controls
	while (fAttributeView->CountChildren() > 0) {
		BView* child = fAttributeView->ChildAt(0);
		fAttributeView->RemoveChild(child);
		delete child;
	}
	
	BGridLayout* grid = dynamic_cast<BGridLayout*>(fAttributeView->GetLayout());
	if (!grid)
		return;

	const float rowHeight = 20.0f;  // Fixed height per row
		
	// Add controls
	for (int32 i = 0; i < fAttributes.CountItems(); i++) {
		Attribute* attr = fAttributes.ItemAt(i);
		
		BString labelText = attr->Name();
		labelText << ":"; // (" << Attribute::TypeCodeToName(attr->Type()) << "):";
		
		AttributeControl* control = AttributeControl::Create(attr);
		control->SetTarget(this);
		
		// Set fixed height for the control
		control->SetExplicitMinSize(BSize(B_SIZE_UNSET, rowHeight));
		control->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, rowHeight));
		
		BStringView* label = new BStringView("label", labelText.String());
		label->SetExplicitMinSize(BSize(B_SIZE_UNSET, rowHeight));
		label->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, rowHeight));
		// Center the label text vertically
		label->SetExplicitAlignment(BAlignment(B_ALIGN_LEFT, B_ALIGN_VERTICAL_CENTER));
		
		
		grid->AddView(label, 0, i);
		grid->AddView(control, 1, i);
	}
	
	grid->SetColumnWeight(0, 0);
	grid->SetColumnWeight(1, 1);
	
	
	// ADD A GLUE AT THE BOTTOM to push everything up
	int32 lastRow = fAttributes.CountItems();
	BView* glue = new BView("glue", 0);
	glue->SetExplicitMinSize(BSize(0, 0));
	glue->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNLIMITED));
	grid->AddView(glue, 0, lastRow, 2, 1);  // Span 2 columns
	grid->SetRowWeight(lastRow, 1.0f);  // Give all weight to the glue
	
	grid->SetColumnWeight(0, 0);
	grid->SetColumnWeight(1, 1);	
}

void AttrifyWindow::SaveFile()
{
	if (!fFileLoaded)
		return;

	WriteAttributesToFile();
	fModified = false;
}

void AttrifyWindow::SaveFileAs()
{
	if (!fSavePanel) {
		fSavePanel = new BFilePanel(B_SAVE_PANEL, new BMessenger(this));
	}
	
	// Propose filename if a file was already loaded
	if (fFileLoaded) {
		BPath path(&fFileRef);
		fSavePanel->SetSaveText(path.Leaf());
	}
	
	fSavePanel->Show();
}

void AttrifyWindow::WriteAttributesToFile()
{
	BNode node(&fFileRef);
	
	// Check if node was initialized successfully
	if (node.InitCheck() != B_OK) {
		BAlert* alert = new BAlert(B_TRANSLATE("Error"),
			B_TRANSLATE("Cannot write attributes: file access failed"),
			B_TRANSLATE("OK"), NULL, NULL, B_WIDTH_AS_USUAL, B_STOP_ALERT);
		alert->Go();
		return;
	}
	
	for (int32 i = 0; i < fAttributes.CountItems(); i++) {
		Attribute* attr = fAttributes.ItemAt(i);
		
		if (!attr->Editable())
			continue;
		
		const char* name = attr->Name().String();
		type_code type = attr->Type();
		
		switch (type) {
			case B_BOOL_TYPE: {
				bool value;
				if (attr->GetValue(&value) == B_OK)
					node.WriteAttr(name, type, 0, &value, sizeof(value));
				break;
			}
			case B_INT8_TYPE: {
				int8 value;
				if (attr->GetValue(&value) == B_OK)
					node.WriteAttr(name, type, 0, &value, sizeof(value));
				break;
			}
			case B_INT16_TYPE: {
				int16 value;
				if (attr->GetValue(&value) == B_OK)
					node.WriteAttr(name, type, 0, &value, sizeof(value));
				break;
			}
			case B_INT32_TYPE: {
				int32 value;
				if (attr->GetValue(&value) == B_OK)
					node.WriteAttr(name, type, 0, &value, sizeof(value));
				break;
			}
			case B_INT64_TYPE: {
				int64 value;
				if (attr->GetValue(&value) == B_OK)
					node.WriteAttr(name, type, 0, &value, sizeof(value));
				break;
			}
			case B_FLOAT_TYPE: {
				float value;
				if (attr->GetValue(&value) == B_OK)
					node.WriteAttr(name, type, 0, &value, sizeof(value));
				break;
			}
			case B_DOUBLE_TYPE: {
				double value;
				if (attr->GetValue(&value) == B_OK)
					node.WriteAttr(name, type, 0, &value, sizeof(value));
				break;
			}
			case B_TIME_TYPE: {
				time_t value;
				if (attr->GetValue(&value) == B_OK)
					node.WriteAttr(name, type, 0, &value, sizeof(value));
				break;
			}
			case B_STRING_TYPE:
			case B_MIME_STRING_TYPE: {
				BString value;
				if (attr->GetValue(&value) == B_OK)
					node.WriteAttr(name, type, 0, value.String(), value.Length() + 1);
				break;
			}
		}
	}
}

void AttrifyWindow::SortAttributes()
{
	// Find the BEOS:TYPE attribute
	Attribute* typeAttr = NULL;
	int32 typeIndex = -1;
	
	for (int32 i = 0; i < fAttributes.CountItems(); i++) {
		if (fAttributes.ItemAt(i)->Name() == "BEOS:TYPE") {
			typeAttr = fAttributes.ItemAt(i);
			typeIndex = i;
			break;
		}
	}
	
	// If found and not already first, move it
	if (typeAttr && typeIndex > 0) {
		fAttributes.RemoveItemAt(typeIndex);
		fAttributes.AddItem(typeAttr, 0);  // Add at first position
	}
}

void AttrifyWindow::ConfigureAttributes(const BString* mimetype)
{
	const char* arguments[] = { "-type", mimetype->String(), 0 };
	status_t ret = be_roster->Launch(
		"application/x-vnd.Haiku-FileTypes",
		sizeof(arguments) / sizeof(const char*) - 1,
		const_cast<char**>(arguments));
	
	if (ret != B_OK && ret != B_ALREADY_RUNNING) {
		BString errorMsg(B_TRANSLATE("Launching the FileTypes "
			"preflet to configure attributes has failed."
			"\n\nError: "));
		errorMsg << strerror(ret);
		BAlert* alert = new BAlert(B_TRANSLATE("Error"),
			errorMsg.String(), B_TRANSLATE("OK"), NULL, NULL,
			B_WIDTH_AS_USUAL, B_STOP_ALERT);
		alert->SetFlags(alert->Flags() | B_CLOSE_ON_ESCAPE);
		alert->Go(NULL);
	}
}

