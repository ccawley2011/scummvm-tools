/* ScummVM Tools
 *
 * ScummVM Tools is the legal property of its developers, whose
 * names are too numerous to list here. Please refer to the
 * COPYRIGHT file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

/* Mohawk file parser */

#ifndef MOHAWK_ARCHIVE_H
#define MOHAWK_ARCHIVE_H

#include "common/str.h"
#include "common/endian.h"
#include "common/util.h"
#include "common/file.h"

// Main FourCC's
#define ID_MHWK MKTAG('M','H','W','K') // Main FourCC
#define ID_RSRC MKTAG('R','S','R','C') // Resource Directory Tag

// Myst Resource FourCC's
#define ID_CLRC MKTAG('C','L','R','C') // Cursor Hotspots
#define ID_EXIT MKTAG('E','X','I','T') // Card Exit Scripts
#define ID_HINT MKTAG('H','I','N','T') // Specifies Cursors in What Area
#define ID_INIT MKTAG('I','N','I','T') // Card Entrance Scripts
#define ID_MSND MKTAG('M','S','N','D') // Standard Mohawk Sound
#define ID_RLST MKTAG('R','L','S','T') // Resource List, Specifies HotSpots
#define ID_RSFL MKTAG('R','S','F','L') // ??? (system.dat only)
#define ID_VIEW MKTAG('V','I','E','W') // Card Details
#define ID_WDIB MKTAG('W','D','I','B') // LZ-Compressed Windows Bitmap

// Myst Masterpiece Edition Resource FourCC's (In addition to Myst FourCC's)
#define ID_HELP MKTAG('H','E','L','P') // Help Chunk
#define ID_MJMP MKTAG('M','J','M','P') // MSND Jumps (To reduce MSND duplication)
#define ID_PICT MKTAG('P','I','C','T') // JPEG/PICT Image

// Riven Resource FourCC's
#define ID_BLST MKTAG('B','L','S','T') // Card Hotspot Enabling Lists
#define ID_CARD MKTAG('C','A','R','D') // Card Scripts
#define ID_FLST MKTAG('F','L','S','T') // Card SFXE Lists
#define ID_HSPT MKTAG('H','S','P','T') // Card Hotspots
#define ID_MLST MKTAG('M','L','S','T') // Card Movie Lists
#define ID_NAME MKTAG('N','A','M','E') // Object Names
#define ID_PLST MKTAG('P','L','S','T') // Card Picture Lists
#define ID_RMAP MKTAG('R','M','A','P') // Card Code
#define ID_SFXE MKTAG('S','F','X','E') // Water Effect Animations
#define ID_SLST MKTAG('S','L','S','T') // Card Ambient Sound Lists
#define ID_TMOV MKTAG('t','M','O','V') // Game Movie

// Riven Saved Game FourCC's
#define ID_VARS MKTAG('V','A','R','S') // Saved Game Variable Values
#define ID_VERS MKTAG('V','E','R','S') // Version Info
#define ID_ZIPS MKTAG('Z','I','P','S') // Zip Mode Status

// Zoombini Resource FourCC's
#define ID_SND  MKTAG('\0','S','N','D') // Standard Mohawk Sound
#define ID_CURS MKTAG('C','U','R','S') // Cursor?
#define ID_SCRB MKTAG('S','C','R','B') // ???
#define ID_SCRS MKTAG('S','C','R','S') // ???
#define ID_NODE MKTAG('N','O','D','E') // ???
#define ID_PATH MKTAG('P','A','T','H') // ???
#define ID_SHPL MKTAG('S','H','P','L') // ???

// Living Books Resource FourCC's
#define ID_TCUR MKTAG('t','C','U','R') // Cursor
#define ID_BITL MKTAG('B','I','T','L') // ???
#define ID_CTBL MKTAG('C','T','B','L') // Color Table?
#define ID_SCRP MKTAG('S','C','R','P') // Script?
#define ID_SPR  MKTAG('S','P','R','#') // Sprites?
#define ID_VRSN MKTAG('V','R','S','N') // Version?
#define ID_ANI  MKTAG('A','N','I',' ') // Animation?
#define ID_SHP  MKTAG('S','H','P','#') // ???

// JamesMath Resource FourCC's
#define ID_TANM MKTAG('t','A','N','M') // Animation?
#define ID_TMFO MKTAG('t','M','F','O') // ???

// Mohawk Wave Tags
#define ID_WAVE MKTAG('W','A','V','E') // Game Sound (Third Tag)
#define ID_ADPC MKTAG('A','D','P','C') // Game Sound Chunk
#define ID_DATA MKTAG('D','a','t','a') // Game Sound Chunk
#define ID_CUE  MKTAG('C','u','e','#') // Game Sound Chunk

// Mohawk MIDI Tags
#define ID_MIDI MKTAG('M','I','D','I') // Game Sound (Third Tag), instead of WAVE
#define ID_PRG  MKTAG('P','r','g','#') // Midi Program?

// Old Mohawk Resource FourCC's
#define ID_WAV  MKTAG('W','A','V',' ') // Old Sound Resource
#define ID_BMAP MKTAG('B','M','A','P') // Standard Mohawk Bitmap

// Common Resource FourCC's
#define ID_TBMP MKTAG('t','B','M','P') // Standard Mohawk Bitmap
#define ID_TWAV MKTAG('t','W','A','V') // Standard Mohawk Sound
#define ID_TPAL MKTAG('t','P','A','L') // Standard Mohawk Palette
#define ID_TCNT MKTAG('t','C','N','T') // ??? (CSWorld, CSAmtrak, JamesMath)
#define ID_TSCR MKTAG('t','S','C','R') // Script? Screen? (CSWorld, CSAmtrak, Treehouse)
#define ID_STRL MKTAG('S','T','R','L') // String List (Zoombini, CSWorld, CSAmtrak)
#define ID_TBMH MKTAG('t','B','M','H') // Standard Mohawk Bitmap
#define ID_TMID MKTAG('t','M','I','D') // Standard Mohawk MIDI
#define ID_REGS MKTAG('R','E','G','S') // ??? (Zoombini, Treehouse)
#define ID_BYTS MKTAG('B','Y','T','S') // Database Entry (CSWorld, CSAmtrak)
#define ID_INTS MKTAG('I','N','T','S') // ??? (CSWorld, CSAmtrak)
#define ID_BBOX MKTAG('B','B','O','X') // Boxes? (CSWorld, CSAmtrak)
#define ID_SYSX MKTAG('S','Y','S','X') // MIDI Sysex

struct MohawkOutputStream {
	Common::File *stream;
	uint32 tag;
	uint32 id;
	uint32 index;
	uint32 offset;
	uint32 size;
	byte flags;
	Common::String name;
};

struct FileTable {
	uint32 offset;
	uint32 dataSize; // Really 27 bits
	byte flags; // Mostly useless except for the bottom 3 bits which are part of the size
	uint16 unk; // Always 0
};

struct Type {
	Type() { resTable.entries = NULL; nameTable.entries = NULL; }
	~Type() { delete[] resTable.entries; delete[] nameTable.entries; }

	//Type Table
	uint32 tag;
	uint16 resource_table_offset;
	uint16 name_table_offset;

	struct ResourceTable {
		uint16 resources;
		struct Entries {
			uint16 id;
			uint16 index;
		} *entries;
	} resTable;

	struct NameTable {
		uint16 num;
		struct Entries {
			uint16 offset;
			uint16 index;
			// Name List
			Common::String name;
		} *entries;
	} nameTable;
};

struct TypeTable {
	uint16 name_offset;
	uint16 resource_types;
};

struct RSRC_Header {
	uint16 version;
	uint16 compaction;
	uint32 filesize;
	uint32 abs_offset;
	uint16 file_table_offset;
	uint16 file_table_size;
};

class MohawkArchive {
public:
	MohawkArchive();
	virtual ~MohawkArchive() { close(); }

	// Detect new/old Mohawk archive format. Return NULL if the file is neither.
	static MohawkArchive *createMohawkArchive(Common::File *stream);

	virtual void open(Common::File *stream);
	void close();

	virtual MohawkOutputStream getRawData(uint32 tag, uint16 id);
	virtual MohawkOutputStream getNextFile();

protected:
	Common::File *_mhk;
	TypeTable _typeTable;
	Common::String _curFile;

	// Extraction Variables
	uint32 _curExType;
	uint32 _curExTypeIndex;

	FileTable *_fileTable;

private:
	bool _hasData;
	uint32 _fileSize;
	RSRC_Header _rsrc;
	Type *_types;
	uint16 _nameTableAmount;
	uint16 _resourceTableAmount;
	uint16 _fileTableAmount;

	virtual int16 getTypeIndex(uint32 tag) {
		for (uint16 i = 0; i < _typeTable.resource_types; i++)
			if (_types[i].tag == tag)
				return i;
		return -1;	// not found
	}

	virtual int16 getIdIndex(int16 typeIndex, uint16 id) {
		for (uint16 i = 0; i < _types[typeIndex].resTable.resources; i++)
			if (_types[typeIndex].resTable.entries[i].id == id)
				return i;
		return -1;	// not found
	}
};

class LivingBooksArchive_v1 : public MohawkArchive {
public:
	LivingBooksArchive_v1() : MohawkArchive() {}
	~LivingBooksArchive_v1() {}

	virtual void open(Common::File *stream);
	MohawkOutputStream getRawData(uint32 tag, uint16 id);
	MohawkOutputStream getNextFile();

protected:
	struct OldType {
		uint32 tag;
		uint16 resource_table_offset;
		struct ResourceTable {
			uint16 resources;
			struct Entries {
				uint16 id;
				uint32 offset;
				uint32 size;
			} *entries;
		} resTable;
	} *_types;

	int16 getTypeIndex(uint32 tag) {
		for (uint16 i = 0; i < _typeTable.resource_types; i++)
			if (_types[i].tag == tag)
				return i;
		return -1;	// not found
	}

	int16 getIdIndex(int16 typeIndex, uint16 id) {
		for (uint16 i = 0; i < _types[typeIndex].resTable.resources; i++)
			if (_types[typeIndex].resTable.entries[i].id == id)
				return i;
		return -1;	// not found
	}
};

class CSWorldDeluxeArchive : public LivingBooksArchive_v1 {
public:
	CSWorldDeluxeArchive() : LivingBooksArchive_v1() {}
	~CSWorldDeluxeArchive() {}
	void open(Common::File *stream);
};

#endif
