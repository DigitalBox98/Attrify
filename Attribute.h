/*
 * Attribute.h - Represents a Haiku file attribute
 */
#ifndef ATTRIBUTE_H
#define ATTRIBUTE_H

#include <String.h>
#include <SupportDefs.h>
#include <TypeConstants.h>
#include <kernel/fs_attr.h>
#include <vector>

class Attribute {
public:
	Attribute(const BString& name, type_code type);
	~Attribute();
	
	Attribute(const Attribute&) = delete;
	Attribute& operator=(const Attribute&) = delete;
	
	// Accessors
	BString Name() const { return fName; }
	type_code Type() const { return fType; }
	
	// Editing
	bool Editable() const;
	BString ToString() const;
	status_t SetValueFromString(const BString& value);
	
	// Typed values
	status_t SetValue(bool value);
	status_t SetValue(int8 value);
	status_t SetValue(int16 value);
	status_t SetValue(int32 value);
	status_t SetValue(int64 value);
	status_t SetValue(float value);
	status_t SetValue(double value);
	status_t SetValue(const BString& value);
//	status_t SetValue(time_t value);
	status_t SetRawValue(const void* data, size_t size);
	
	status_t GetValue(bool* value) const;
	status_t GetValue(int8* value) const;
	status_t GetValue(int16* value) const;
	status_t GetValue(int32* value) const;
	status_t GetValue(int64* value) const;
	status_t GetValue(float* value) const;
	status_t GetValue(double* value) const;
	status_t GetValue(BString* value) const;
	//status_t GetValue(time_t* value) const;
	const void* GetRawValue(size_t* size) const;
	
	// Static utility
	static BString TypeCodeToName(type_code type);

private:
	BString fName;
	type_code fType;
	
	// Unified storage
	union {
		bool fBool;
		int8 fInt8;
		int16 fInt16;
		int32 fInt32;
		int64 fInt64;
		float fFloat;
		double fDouble;
		time_t fTime;
	} fValue;
	
	BString fString;
	std::vector<uint8_t> fRawData;
};

#endif // ATTRIBUTE_H