/*
 * PictureView.cpp - PictureView implementation
 */
#include "PictureView.h"
#include "AttrifyWindow.h" 
#include <MimeType.h>
#include <Application.h> 
#include <Cursor.h>

PictureView::PictureView()
	: BView("PictureView", B_WILL_DRAW | B_FULL_UPDATE_ON_RESIZE)
	, fIcon(NULL)
	, fClickable(false)  
{
	SetViewColor(B_TRANSPARENT_COLOR);
	//SetLowColor(ui_color(B_PANEL_BACKGROUND_COLOR));
}

PictureView::~PictureView()
{
	delete fIcon;
}

void PictureView::SetClickable(bool clickable)
{
	fClickable = clickable;
}

void PictureView::MouseDown(BPoint where)
{
	if (fClickable && Window()) {
		BRect clickableRect = GetIconRect();
		
		// Only trigger action if click is within icon bounds
		if (clickableRect.IsValid() && clickableRect.Contains(where)) {
			BMessage msg(MSG_SHOW_IN_TRACKER);
			Window()->PostMessage(&msg);
		} else {
			BView::MouseDown(where);
		}
	} else {
		BView::MouseDown(where);
	}
}

void PictureView::Draw(BRect updateRect)
{
	SetHighColor(ViewColor());
	FillRect(updateRect);
	
	if (fIcon && fIcon->IsValid()) {
		BRect whiteRect = GetIconRect();
		
		// Draw white background
		SetHighColor(255, 255, 255);
		FillRect(whiteRect);
		
		// Draw border
		SetHighColor(0, 0, 0);
		StrokeRect(whiteRect);
		
		// Calculate position for DrawBitmap
		float x = whiteRect.left + 8;
		float y = whiteRect.top + 8;
		
		// Draw icon with alpha blending
		SetDrawingMode(B_OP_ALPHA);
		DrawBitmap(fIcon, BPoint(x, y));
		SetDrawingMode(B_OP_COPY);
	}
}

BRect PictureView::GetIconRect() const
{
	if (!fIcon || !fIcon->IsValid())
		return BRect();
	
	BRect bounds = Bounds();
	BRect iconRect = fIcon->Bounds();
	
	float x = (bounds.Width() - iconRect.Width()) / 2.0f;
	float y = 20;
	
	// Return rectangle with padding (clickable area)
	return BRect(x - 8, y - 8, 
	             x + iconRect.Width() + 8, 
	             y + iconRect.Height() + 8);
}

void PictureView::SetMimeType(const BString& mimeType)
{
	fMimeType = mimeType;
	LoadIcon();
	Invalidate();
}

void PictureView::LoadIcon()
{
	delete fIcon;
	fIcon = NULL;
	
	if (fMimeType.Length() == 0)
		return;
	
	BMimeType mime(fMimeType.String());
	if (!mime.IsValid())
		return;
	
	// Create a bitmap for the large icon (64x64)
	fIcon = new BBitmap(BRect(0, 0, 63, 63), B_RGBA32);
	
	// Try to get the icon from the specific MIME type
	if (mime.GetIcon(fIcon, (icon_size)64) != B_OK) {
		// If that fails, try with the supertype (e.g., "image" for "image/jpeg")
		BMimeType superMime;
		if (mime.GetSupertype(&superMime) == B_OK) {
			if (superMime.GetIcon(fIcon, (icon_size)64) != B_OK) {
				// If even the supertype has no icon, use the generic icon
				delete fIcon;
				fIcon = NULL;
			}
		} else {
			delete fIcon;
			fIcon = NULL;
		}
	}
}

void PictureView::SetClickableToolTip(const char* tooltip)
{
	fTooltipText = tooltip;
}

bool PictureView::GetToolTipAt(BPoint point, BToolTip** _tip)
{
	// Only show tooltip if we're clickable and have tooltip text
	if (!fClickable || fTooltipText.Length() == 0)
		return false;
	
	BRect clickableRect = GetIconRect();
	
	// Only show tooltip if mouse is within icon bounds
	if (clickableRect.IsValid() && clickableRect.Contains(point)) {
		// Set the tooltip text
		SetToolTip(fTooltipText.String());
		return BView::GetToolTipAt(point, _tip);
	}
	
	// No tooltip outside icon area
	SetToolTip((const char*)NULL);
	return false;
}

void PictureView::GetPreferredSize(float* width, float* height)
{
	*width = B_LARGE_ICON + 40;
	*height = B_LARGE_ICON + 40;
}

void PictureView::MouseMoved(BPoint where, uint32 code, const BMessage* dragMessage)
{
	if (fClickable) {
		BRect clickableRect = GetIconRect();
		
		if (clickableRect.IsValid() && clickableRect.Contains(where)) {
			// Use follow link cursor to indicate interactivity
			BCursor cursor(B_CURSOR_ID_FOLLOW_LINK);
			SetViewCursor(&cursor, true);
		} else {
			SetViewCursor(B_CURSOR_SYSTEM_DEFAULT, true);
		}
	}
	
	BView::MouseMoved(where, code, dragMessage);
}