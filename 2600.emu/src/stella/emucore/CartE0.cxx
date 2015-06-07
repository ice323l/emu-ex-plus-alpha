//============================================================================
//
//   SSSS    tt          lll  lll       
//  SS  SS   tt           ll   ll        
//  SS     tttttt  eeee   ll   ll   aaaa 
//   SSSS    tt   ee  ee  ll   ll      aa
//      SS   tt   eeeeee  ll   ll   aaaaa  --  "An Atari 2600 VCS Emulator"
//  SS  SS   tt   ee      ll   ll  aa  aa
//   SSSS     ttt  eeeee llll llll  aaaaa
//
// Copyright (c) 1995-2015 by Bradford W. Mott, Stephen Anthony
// and the Stella Team
//
// See the file "License.txt" for information on usage and redistribution of
// this file, and for a DISCLAIMER OF ALL WARRANTIES.
//
// $Id: CartE0.cxx 3131 2015-01-01 03:49:32Z stephena $
//============================================================================

#include <cstring>

#include "System.hxx"
#include "CartE0.hxx"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
CartridgeE0::CartridgeE0(const uInt8* image, uInt32 size, const Settings& settings)
  : Cartridge(settings)
{
  // Copy the ROM image into my buffer
  memcpy(myImage, image, BSPF_min(8192u, size));
  createCodeAccessBase(8192);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
CartridgeE0::~CartridgeE0()
{
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeE0::reset()
{
  // Setup segments to some default slices
  segmentZero(4);
  segmentOne(5);
  segmentTwo(6);

  myBankChanged = true;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeE0::install(System& system)
{
  mySystem = &system;

  System::PageAccess access(this, System::PA_READ);

  // Set the page acessing methods for the first part of the last segment
  for(uInt32 i = 0x1C00; i < (0x1FE0U & ~System::PAGE_MASK);
      i += (1 << System::PAGE_SHIFT))
  {
    access.directPeekBase = &myImage[7168 + (i & 0x03FF)];
    access.codeAccessBase = &myCodeAccessBase[7168 + (i & 0x03FF)];
    mySystem->setPageAccess(i >> System::PAGE_SHIFT, access);
  }
  myCurrentSlice[3] = 7;

  // Set the page accessing methods for the hot spots in the last segment
  access.directPeekBase = 0;
  access.codeAccessBase = &myCodeAccessBase[8128];
  access.type = System::PA_READ;
  for(uInt32 j = (0x1FE0 & ~System::PAGE_MASK); j < 0x2000;
      j += (1 << System::PAGE_SHIFT))
    mySystem->setPageAccess(j >> System::PAGE_SHIFT, access);

  // Install some default slices for the other segments
  segmentZero(4);
  segmentOne(5);
  segmentTwo(6);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt8 CartridgeE0::peek(uInt16 address)
{
  address &= 0x0FFF;

  // Switch banks if necessary
  if((address >= 0x0FE0) && (address <= 0x0FE7))
  {
    segmentZero(address & 0x0007);
  }
  else if((address >= 0x0FE8) && (address <= 0x0FEF))
  {
    segmentOne(address & 0x0007);
  }
  else if((address >= 0x0FF0) && (address <= 0x0FF7))
  {
    segmentTwo(address & 0x0007);
  }

  return myImage[(myCurrentSlice[address >> 10] << 10) + (address & 0x03FF)];
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool CartridgeE0::poke(uInt16 address, uInt8)
{
  address &= 0x0FFF;

  // Switch banks if necessary
  if((address >= 0x0FE0) && (address <= 0x0FE7))
  {
    segmentZero(address & 0x0007);
  }
  else if((address >= 0x0FE8) && (address <= 0x0FEF))
  {
    segmentOne(address & 0x0007);
  }
  else if((address >= 0x0FF0) && (address <= 0x0FF7))
  {
    segmentTwo(address & 0x0007);
  }
  return false;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeE0::segmentZero(uInt16 slice)
{ 
  if(bankLocked()) return;

  // Remember the new slice
  myCurrentSlice[0] = slice;
  uInt16 offset = slice << 10;

  // Setup the page access methods for the current bank
  System::PageAccess access(this, System::PA_READ);

  for(uInt32 address = 0x1000; address < 0x1400;
      address += (1 << System::PAGE_SHIFT))
  {
    access.directPeekBase = &myImage[offset + (address & 0x03FF)];
    access.codeAccessBase = &myCodeAccessBase[offset + (address & 0x03FF)];
    mySystem->setPageAccess(address >> System::PAGE_SHIFT, access);
  }
  myBankChanged = true;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeE0::segmentOne(uInt16 slice)
{ 
  if(bankLocked()) return;

  // Remember the new slice
  myCurrentSlice[1] = slice;
  uInt16 offset = slice << 10;

  // Setup the page access methods for the current bank
  System::PageAccess access(this, System::PA_READ);

  for(uInt32 address = 0x1400; address < 0x1800; address += (1 << System::PAGE_SHIFT))
  {
    access.directPeekBase = &myImage[offset + (address & 0x03FF)];
    access.codeAccessBase = &myCodeAccessBase[offset + (address & 0x03FF)];
    mySystem->setPageAccess(address >> System::PAGE_SHIFT, access);
  }
  myBankChanged = true;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeE0::segmentTwo(uInt16 slice)
{ 
  if(bankLocked()) return;

  // Remember the new slice
  myCurrentSlice[2] = slice;
  uInt16 offset = slice << 10;

  // Setup the page access methods for the current bank
  System::PageAccess access(this, System::PA_READ);

  for(uInt32 address = 0x1800; address < 0x1C00;
      address += (1 << System::PAGE_SHIFT))
  {
    access.directPeekBase = &myImage[offset + (address & 0x03FF)];
    access.codeAccessBase = &myCodeAccessBase[offset + (address & 0x03FF)];
    mySystem->setPageAccess(address >> System::PAGE_SHIFT, access);
  }
  myBankChanged = true;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool CartridgeE0::patch(uInt16 address, uInt8 value)
{
  address &= 0x0FFF;
  myImage[(myCurrentSlice[address >> 10] << 10) + (address & 0x03FF)] = value;
  return true;
} 

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const uInt8* CartridgeE0::getImage(int& size) const
{
  size = 8192;
  return myImage;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool CartridgeE0::save(Serializer& out) const
{
  try
  {
    out.putString(name());
    out.putShortArray(myCurrentSlice, 4);
  }
  catch(...)
  {
    cerr << "ERROR: CartridgeE0::save" << endl;
    return false;
  }

  return true;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool CartridgeE0::load(Serializer& in)
{
  try
  {
    if(in.getString() != name())
      return false;

    in.getShortArray(myCurrentSlice, 4);
  }
  catch(...)
  {
    cerr << "ERROR: CartridgeE0::load" << endl;
    return false;
  }

  return true;
}
