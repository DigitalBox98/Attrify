/*
 * Attribute.cpp - Attribute implementation
 */

#include "Attribute.h"
#include <Catalog.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "Attribute"

Attribute::Attribute(const BString& name, type_code type)
	: fName(name)
	, fType(type)
{
	memset(&fValue, 0, sizeof(fValue));
}

Attribute::~Attribute()
{
}

bool Attribute::Editable() const
{
	// B_RAW_TYPE is not editable
	if (fType == B_RAW_TYPE)
		return false;
	
	// BEOS:TYPE is never editable (system attribute)
	if (fName == "BEOS:TYPE")
		return false;
	
	return true;
}

BString Attribute::ToString() const
{
	BString result;
	char buffer[256];
	
	switch (fType) {
		case B_BOOL_TYPE:
			result = fValue.fBool ? B_TRANSLATE("true") : B_TRANSLATE("false");
			break;
		case B_INT8_TYPE:
			snprintf(buffer, sizeof(buffer), "%d", fValue.fInt8);
			result = buffer;
			break;
		case B_INT16_TYPE:
			snprintf(buffer, sizeof(buffer), "%d", fValue.fInt16);
			result = buffer;
			break;
		case B_INT32_TYPE:
			snprintf(buffer, sizeof(buffer), "%d", fValue.fInt32);
			result = buffer;
			break;
		case B_INT64_TYPE:
			snprintf(buffer, sizeof(buffer), "%" PRId64, fValue.fInt64);
			result = buffer;
			break;
		case B_FLOAT_TYPE:
			snprintf(buffer, sizeof(buffer), "%g", fValue.fFloat);
			result = buffer;
			break;
		case B_DOUBLE_TYPE:
			snprintf(buffer, sizeof(buffer), "%g", fValue.fDouble);
			result = buffer;
			break;
		case B_TIME_TYPE: {
			struct tm* timeinfo = localtime(&fValue.fTime);
			strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
			result = buffer;
			break;
		}
		case B_STRING_TYPE:
		case B_MIME_STRING_TYPE:
			result = fString;
			break;
		case B_RAW_TYPE:
			snprintf(buffer, sizeof(buffer), 
				B_TRANSLATE("[RAW data: %zu bytes]"), fRawData.size());
			result = buffer;
			break;
		default:
			result = B_TRANSLATE("[Unknown type]");
			break;
	}
	
	return result;
}

status_t Attribute::SetValueFromString(const BString& value)
{
	if (!Editable())
		return B_NOT_ALLOWED;
	
	switch (fType) {
		case B_BOOL_TYPE:
			fValue.fBool = (value.ICompare("true") == 0 || value == "1");
			break;
		case B_INT8_TYPE:
			fValue.fInt8 = (int8)atoi(value.String());
			break;
		case B_INT16_TYPE:
			fValue.fInt16 = (int16)atoi(value.String());
			break;
		case B_INT32_TYPE:
			fValue.fInt32 = atol(value.String());
			break;
		case B_INT64_TYPE:
			fValue.fInt64 = atoll(value.String());
			break;
		case B_FLOAT_TYPE:
			fValue.fFloat = atof(value.String());
			break;
		case B_DOUBLE_TYPE:
			fValue.fDouble = atof(value.String());
			break;
		case B_TIME_TYPE:
			// Simplification: accept a numeric timestamp
			fValue.fTime = (time_t)atoll(value.String());
			break;
		case B_STRING_TYPE:
		case B_MIME_STRING_TYPE:
			fString = value;
			break;
		default:
			return B_BAD_TYPE;
	}
	
	return B_OK;
}

// Typed setters
status_t Attribute::SetValue(bool value)
{
	if (fType != B_BOOL_TYPE) return B_BAD_TYPE;
	fValue.fBool = value;
	return B_OK;
}

status_t Attribute::SetValue(int8 value)
{
	if (fType != B_INT8_TYPE) return B_BAD_TYPE;
	fValue.fInt8 = value;
	return B_OK;
}

status_t Attribute::SetValue(int16 value)
{
	if (fType != B_INT16_TYPE) return B_BAD_TYPE;
	fValue.fInt16 = value;
	return B_OK;
}

status_t Attribute::SetValue(int32 value)
{
	if (fType != B_INT32_TYPE) return B_BAD_TYPE;
	fValue.fInt32 = value;
	return B_OK;
}

status_t Attribute::SetValue(int64 value)
{
	if (fType != B_INT64_TYPE && fType != B_TIME_TYPE) 
		return B_BAD_TYPE;
	
	if (fType == B_TIME_TYPE)
		fValue.fTime = value;
	else
		fValue.fInt64 = value;
	
	return B_OK;
}

status_t Attribute::SetValue(float value)
{
	if (fType != B_FLOAT_TYPE) return B_BAD_TYPE;
	fValue.fFloat = value;
	return B_OK;
}

status_t Attribute::SetValue(double value)
{
	if (fType != B_DOUBLE_TYPE) return B_BAD_TYPE;
	fValue.fDouble = value;
	return B_OK;
}

status_t Attribute::SetValue(const BString& value)
{
	if (fType != B_STRING_TYPE && fType != B_MIME_STRING_TYPE)
		return B_BAD_TYPE;
	fString = value;
	return B_OK;
}

status_t Attribute::SetRawValue(const void* data, size_t size)
{
	if (fType != B_RAW_TYPE) 
		return B_BAD_TYPE;

	if (data == nullptr || size == 0) {
		fRawData.clear();
		return B_OK;
	}
	
	try {
		const uint8_t* byteData = static_cast<const uint8_t*>(data);
		fRawData.assign(byteData, byteData + size);
	} catch (const std::bad_alloc&) {
		return B_NO_MEMORY;
	}

	return B_OK;
}

// Typed getters
status_t Attribute::GetValue(bool* value) const
{
	if (fType != B_BOOL_TYPE) return B_BAD_TYPE;
	*value = fValue.fBool;
	return B_OK;
}

status_t Attribute::GetValue(int8* value) const
{
	if (fType != B_INT8_TYPE) return B_BAD_TYPE;
	*value = fValue.fInt8;
	return B_OK;
}

status_t Attribute::GetValue(int16* value) const
{
	if (fType != B_INT16_TYPE) return B_BAD_TYPE;
	*value = fValue.fInt16;
	return B_OK;
}

status_t Attribute::GetValue(int32* value) const
{
	if (fType != B_INT32_TYPE) return B_BAD_TYPE;
	*value = fValue.fInt32;
	return B_OK;
}

status_t Attribute::GetValue(int64* value) const
{
	if (fType != B_INT64_TYPE && fType != B_TIME_TYPE) 
		return B_BAD_TYPE;
	
	if (fType == B_TIME_TYPE)
		*value = fValue.fTime;
	else
		*value = fValue.fInt64;
	
	return B_OK;
}

status_t Attribute::GetValue(float* value) const
{
	if (fType != B_FLOAT_TYPE) return B_BAD_TYPE;
	*value = fValue.fFloat;
	return B_OK;
}

status_t Attribute::GetValue(double* value) const
{
	if (fType != B_DOUBLE_TYPE) return B_BAD_TYPE;
	*value = fValue.fDouble;
	return B_OK;
}

status_t Attribute::GetValue(BString* value) const
{
	if (fType != B_STRING_TYPE && fType != B_MIME_STRING_TYPE)
		return B_BAD_TYPE;
	*value = fString;
	return B_OK;
}

const void* Attribute::GetRawValue(size_t* size) const
{
	if (fType != B_RAW_TYPE) 
		return nullptr;
	if (size == nullptr)
		return nullptr;

	*size = fRawData.size();
	return fRawData.empty() ? nullptr : fRawData.data();
}

BString Attribute::TypeCodeToName(type_code type)
{
	switch (type) {
		case B_BOOL_TYPE:
			return B_TRANSLATE("Boolean");
		case B_INT8_TYPE:
			return B_TRANSLATE("Int8");
		case B_INT16_TYPE:
			return B_TRANSLATE("Int16");
		case B_INT32_TYPE:
			return B_TRANSLATE("Int32");
		case B_INT64_TYPE:
			return B_TRANSLATE("Int64");
		case B_FLOAT_TYPE:
			return B_TRANSLATE("Float");
		case B_DOUBLE_TYPE:
			return B_TRANSLATE("Double");
		case B_TIME_TYPE:
			return B_TRANSLATE("Time");
		case B_STRING_TYPE:
			return B_TRANSLATE("String");
		case B_MIME_STRING_TYPE:
			return B_TRANSLATE("MIME String");
		case B_RAW_TYPE:
			return B_TRANSLATE("Raw");
		default:
			return B_TRANSLATE("Unknown");
	}
}