#include <stdio.h>
#include <string.h>
#include "JSB_General.h"

///////////////////////////////////////////////////////////////////////////////
// Min/Max:

uint32_t Min_uint32(uint32_t A, uint32_t B)
{
    return A < B ? A : B;
}

int32_t Min_int32(int32_t A, int32_t B)
{
    return A < B ? A : B;
}

uint32_t Max_uint32(uint32_t A, uint32_t B)
{
    return A > B ? A : B;
}

int32_t Max_int32(int32_t A, int32_t B)
{
    return A > B ? A : B;
}

///////////////////////////////////////////////////////////////////////////////
// Clamp:

uint32_t Clamp_uint32(uint32_t Value, uint32_t Min, uint32_t Max)
{
  if (Value < Min)
    Value = Min;
  else if (Value > Max)
    Value = Max;

  return Value;
}

int32_t Clamp_int32(int32_t Value, int32_t Min, int32_t Max)
{
  if (Value < Min)
    Value = Min;
  else if (Value > Max)
    Value = Max;

  return Value;
}

float Clamp_float(float Value, float Min, float Max)
{
  if (Value < Min)
    Value = Min;
  else if (Value > Max)
    Value = Max;

  return Value;
}

///////////////////////////////////////////////////////////////////////////////
// Wrap:

uint32_t wrap_uint32(uint32_t Value, uint32_t NumItems)
{
  if (Value >= NumItems)
    Value -= NumItems;

  return Value;
}

int32_t wrap_int32(int32_t Value, int32_t NumItems)
{
  if (Value >= NumItems)
    Value -= NumItems;
  else if (Value < 0)
    Value += NumItems;

  return Value;
}

///////////////////////////////////////////////////////////////////////////////
// GUIDs:

uint8_t StringToGUID(char *Value, GUID_t *o_pGUID)
{
  GUID_t Result;
  unsigned int X[11];
  int NumFields, NumChars;

  NumFields = sscanf(Value, "{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}%n",
      &X[0], &X[1], &X[2], &X[3], &X[4], &X[5], &X[6], &X[7], &X[8], &X[9], &X[10], &NumChars);

  if ((NumFields != 11) || (NumChars != 38))
    return 0;

  Result.Field0 = X[0];
  Result.Field1 = X[1];
  Result.Field2 = X[2];
  Result.Field3[0] = X[3];
  Result.Field3[1] = X[4];
  Result.Field3[2] = X[5];
  Result.Field3[3] = X[6];
  Result.Field3[4] = X[7];
  Result.Field3[5] = X[8];
  Result.Field3[6] = X[9];
  Result.Field3[7] = X[10];

  *o_pGUID = Result;

  return 1;
}

void GUIDToString(GUID_t *i_Value, char *o_Value)
{
  sprintf(o_Value, "{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
    (unsigned int)i_Value->Field0,
    i_Value->Field1,
    i_Value->Field2,
    i_Value->Field3[0], i_Value->Field3[1], i_Value->Field3[2], i_Value->Field3[3], i_Value->Field3[4], i_Value->Field3[5], i_Value->Field3[6], i_Value->Field3[7]);
}

uint8_t GUID_IsEqual(GUID_t *pA, GUID_t *pB)
{
  return memcmp(pA, pB, sizeof(GUID_t)) == 0 ? 1 : 0;
}

///////////////////////////////////////////////////////////////////////////////
