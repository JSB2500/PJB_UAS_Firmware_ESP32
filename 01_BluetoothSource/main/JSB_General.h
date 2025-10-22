#ifndef __JSB_GENERAL_H
#define __JSB_GENERAL_H

#include "stdint.h"

///////////////////////////////////////////////////////////////////////////////
// Min/Max:

uint32_t Min_uint32(uint32_t A, uint32_t B);
int32_t Min_int32(int32_t A, int32_t B);
uint32_t Max_uint32(uint32_t A, uint32_t B);
int32_t Max_int32(int32_t A, int32_t B);

///////////////////////////////////////////////////////////////////////////////
// Clamp:

uint32_t Clamp_uint32(uint32_t Value, uint32_t Min, uint32_t Max);
int32_t Clamp_int32(int32_t Value, int32_t Min, int32_t Max);
float Clamp_float(float Value, float Min, float Max);

///////////////////////////////////////////////////////////////////////////////
// Wrap:

uint32_t wrap_uint32(uint32_t Value, uint32_t NumItems);
int32_t wrap_int32(int32_t Value, int32_t NumItems);

///////////////////////////////////////////////////////////////////////////////
// GUIDs:

typedef struct
{
  uint32_t Field0;
  uint16_t Field1;
  uint16_t Field2;
  uint8_t Field3[8];
} GUID_t;

uint8_t StringToGUID(char *Value, GUID_t *o_pGUID);
void GUIDToString(GUID_t *i_Value, char *o_Value);
uint8_t GUID_IsEqual(GUID_t *pA, GUID_t *pB);

///////////////////////////////////////////////////////////////////////////////

#endif
