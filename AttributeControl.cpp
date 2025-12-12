/*
 * AttributeControl.cpp - AttributeControl implementation
 */
 
#include "AttributeControl.h"
#include <Catalog.h>
#include <Window.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "AttributeControl"

AttributeControl::AttributeControl(Attribute* attribute)
	: BTextControl("", "", "", new BMessage(MSG_ATTRIBUTE_CHANGED))
	, fAttribute(attribute)
	, fClickable(false)
{
	if (fAttribute) {
		BString displayText = fAttribute->ToString();
		
		// Translate "N/A" for display
		if (displayText == "N/A") {
			displayText = B_TRANSLATE("Not available");
		}
		
		SetText(displayText.String());
		
		// Disable for raw types AND unknown types
		bool isKnownType = false;
		switch (fAttribute->Type()) {
			case B_BOOL_TYPE:
			case B_INT8_TYPE:
			case B_INT16_TYPE:
			case B_INT32_TYPE:
			case B_INT64_TYPE:
			case B_FLOAT_TYPE:
			case B_DOUBLE_TYPE:
			case B_STRING_TYPE:
			case B_MIME_STRING_TYPE:
			case B_TIME_TYPE:
				isKnownType = true;
				break;
			case B_RAW_TYPE:
			default:
				isKnownType = false;
				break;
		}
		
		// Enable only if editable AND known type
		SetEnabled(fAttribute->Editable() && isKnownType);
	}
	
	// Enable mouse tracking for hover
	SetEventMask(B_POINTER_EVENTS, 0);
}

AttributeControl::~AttributeControl()
{
	// Don't delete fAttribute, it belongs to the window
}

void AttributeControl::SetAttribute(Attribute* attribute)
{
	fAttribute = attribute;
	UpdateFromAttribute();
}

void AttributeControl::UpdateFromAttribute()
{
	if (!fAttribute)
		return;
	
	SetText(fAttribute->ToString().String());
	SetEnabled(fAttribute->Editable());
}

status_t AttributeControl::UpdateAttribute()
{
	if (!fAttribute || !fAttribute->Editable())
		return B_NOT_ALLOWED;
	
	BString value = Text();
	status_t status = fAttribute->SetValueFromString(value);
	
	if (status != B_OK) {
		// Visual error indication (red border could be added here)
		return status;
	}
	
	return B_OK;
}

void AttributeControl::AttachedToWindow()
{
	BTextControl::AttachedToWindow();
	SetTarget(this);
}

void AttributeControl::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case MSG_ATTRIBUTE_CHANGED: {
			status_t status = UpdateAttribute();
			if (status == B_OK && Window()) {
				// Notify the window that the attribute has changed
				BMessage notify(MSG_ATTRIBUTE_CHANGED);
				notify.AddPointer("attribute", fAttribute);
				Window()->PostMessage(&notify);
			}
			break;
		}
		default:
			BTextControl::MessageReceived(message);
			break;
	}
}

AttributeControl* AttributeControl::Create(Attribute* attribute)
{
	if (!attribute)
		return NULL;
	
	return new AttributeControl(attribute);
}


