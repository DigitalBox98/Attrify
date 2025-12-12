/*
 * PictureView.h - Display of MIME type icon
 */
 
#ifndef PICTUREVIEW_H
#define PICTUREVIEW_H

#include <View.h>
#include <Bitmap.h>
#include <String.h>

class PictureView : public BView {
public:
	PictureView();
	~PictureView();
	
	void SetMimeType(const BString& mimeType);
	void SetClickable(bool clickable); 
	void SetClickableToolTip(const char* tooltip);
	
	virtual bool GetToolTipAt(BPoint point, BToolTip** _tip);
	virtual void Draw(BRect updateRect);
	virtual void GetPreferredSize(float* width, float* height);
	virtual void MouseDown(BPoint where);
	virtual void MouseMoved(BPoint where, uint32 code, const BMessage* dragMessage);

private:
	void LoadIcon();
	BRect GetIconRect() const;
	
	BBitmap* fIcon;
	BString fMimeType;
	bool fClickable; 
	BString fTooltipText;
};

#endif // PICTUREVIEW_H