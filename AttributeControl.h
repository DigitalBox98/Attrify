/*
 * AttributeControl.h - Edit control for attributes
 */
#ifndef ATTRIBUTECONTROL_H
#define ATTRIBUTECONTROL_H

#include <TextControl.h>
#include "Attribute.h"

// Message sent when the value changes
const uint32 MSG_ATTRIBUTE_CHANGED = 'AtCh';

class AttributeControl : public BTextControl {
public:
	AttributeControl(Attribute* attribute);
	~AttributeControl();
	
	void SetAttribute(Attribute* attribute);
	Attribute* GetAttribute() const { return fAttribute; }
	
	void UpdateFromAttribute();
	status_t UpdateAttribute();
	
	virtual void AttachedToWindow();
	virtual void MessageReceived(BMessage* message);
	
	// Factory to create the right type of control
	static AttributeControl* Create(Attribute* attribute);

private:
	Attribute* fAttribute;
	bool fClickable;
};

#endif // ATTRIBUTECONTROL_H