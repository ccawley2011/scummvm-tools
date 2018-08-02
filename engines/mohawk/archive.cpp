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

#include "engines/mohawk/archive.h"
#include "common/textconsole.h"

MohawkArchive::MohawkArchive() {
	_mhk = NULL;
	_curFile.clear();
	_types = NULL;
	_fileTable = NULL;
	_curExType = 0;
	_curExTypeIndex = 0;
}

void MohawkArchive::close() {
	_mhk = NULL;
	delete[] _types; _types = NULL;
	delete[] _fileTable; _fileTable = NULL;

	_curFile.clear();
	_curExType = 0;
	_curExTypeIndex = 0;
}

void MohawkArchive::open(Common::File *stream) {
	// Make sure no other file is open...
	close();
	_mhk = stream;

	if (_mhk->readUint32BE() != ID_MHWK)
		error ("Could not find tag \'MHWK\'");

	_fileSize = _mhk->readUint32BE();

	if (_mhk->readUint32BE() != ID_RSRC)
		error ("Could not find tag \'RSRC\'");

	_rsrc.version = _mhk->readUint16BE();

	if (_rsrc.version != 0x100)
		error("Unsupported Mohawk resource version %d.%d", (_rsrc.version >> 8) & 0xff, _rsrc.version & 0xff);

	_rsrc.compaction = _mhk->readUint16BE(); // Only used in creation, not in reading
	_rsrc.filesize = _mhk->readUint32BE();
	_rsrc.abs_offset = _mhk->readUint32BE();
	_rsrc.file_table_offset = _mhk->readUint16BE();
	_rsrc.file_table_size = _mhk->readUint16BE();

	debug (3, "Absolute Offset = %08x", _rsrc.abs_offset);

	/////////////////////////////////
	//Resource Dir
	/////////////////////////////////

	// Type Table
	_mhk->seek(_rsrc.abs_offset, SEEK_SET);
	_typeTable.name_offset = _mhk->readUint16BE();
	_typeTable.resource_types = _mhk->readUint16BE();

	debug (0, "Name List Offset = %04x  Number of Resource Types = %04x", _typeTable.name_offset, _typeTable.resource_types);

	_types = new Type[_typeTable.resource_types];

	for (uint16 i = 0; i < _typeTable.resource_types; i++) {
		_types[i].tag = _mhk->readUint32BE();
		_types[i].resource_table_offset = _mhk->readUint16BE();
		_types[i].name_table_offset = _mhk->readUint16BE();

		// HACK: Zoombini's SND resource starts will a NULL.
		if (_types[i].tag == ID_SND)
			debug (3, "Type[%02d]: Tag = \'SND\' ResTable Offset = %04x  NameTable Offset = %04x", i, _types[i].resource_table_offset, _types[i].name_table_offset);
		else
			debug (3, "Type[%02d]: Tag = \'%s\' ResTable Offset = %04x  NameTable Offset = %04x", i, tag2str(_types[i].tag), _types[i].resource_table_offset, _types[i].name_table_offset);

		//Resource Table
		_mhk->seek(_rsrc.abs_offset + _types[i].resource_table_offset, SEEK_SET);
		_types[i].resTable.resources = _mhk->readUint16BE();

		debug (3, "Resources = %04x", _types[i].resTable.resources);

		_types[i].resTable.entries = new Type::ResourceTable::Entries[_types[i].resTable.resources];

		for (uint16 j = 0; j < _types[i].resTable.resources; j++) {
			_types[i].resTable.entries[j].id = _mhk->readUint16BE();
			_types[i].resTable.entries[j].index = _mhk->readUint16BE();

			debug (4, "Entry[%02x]: ID = %04x (%d) Index = %04x", j, _types[i].resTable.entries[j].id, _types[i].resTable.entries[j].id, _types[i].resTable.entries[j].index);
		}

		// Name Table
		_mhk->seek(_rsrc.abs_offset + _types[i].name_table_offset, SEEK_SET);
		_types[i].nameTable.num = _mhk->readUint16BE();

		debug (3, "Names = %04x", _types[i].nameTable.num);

		_types[i].nameTable.entries = new Type::NameTable::Entries[_types[i].nameTable.num];

		for (uint16 j = 0; j < _types[i].nameTable.num; j++) {
			_types[i].nameTable.entries[j].offset = _mhk->readUint16BE();
			_types[i].nameTable.entries[j].index = _mhk->readUint16BE();

			debug (4, "Entry[%02x]: Name List Offset = %04x  Index = %04x", j, _types[i].nameTable.entries[j].offset, _types[i].nameTable.entries[j].index);

			// Name List
			uint32 pos = _mhk->pos();
			_mhk->seek(_rsrc.abs_offset + _typeTable.name_offset + _types[i].nameTable.entries[j].offset, SEEK_SET);
			char c = (char)_mhk->readByte();
			while (c != 0) {
				_types[i].nameTable.entries[j].name += c;
				c = (char)_mhk->readByte();
			}

			debug (3, "Name = \'%s\'", _types[i].nameTable.entries[j].name.c_str());

			// Get back to next entry
			_mhk->seek(pos, SEEK_SET);
		}

		// Return to next TypeTable entry
		_mhk->seek(_rsrc.abs_offset + (i + 1) * 8 + 4, SEEK_SET);

		debug (3, "\n");
	}

	_mhk->seek(_rsrc.abs_offset + _rsrc.file_table_offset, SEEK_SET);
	_fileTableAmount = _mhk->readUint32BE();
	_fileTable = new FileTable[_fileTableAmount];

	for (uint32 i = 0; i < _fileTableAmount; i++) {
		_fileTable[i].offset = _mhk->readUint32BE();
		_fileTable[i].dataSize = _mhk->readUint16BE();
		_fileTable[i].dataSize += _mhk->readByte() << 16; // Get bits 15-24 of dataSize too
		_fileTable[i].flags = _mhk->readByte();
		_fileTable[i].unk = _mhk->readUint16BE();

		// Add in another 3 bits for file size from the flags.
		// The flags are useless to us except for doing this ;)
		_fileTable[i].dataSize += (_fileTable[i].flags & 7) << 24;

		debug (4, "File[%02x]: Offset = %08x  DataSize = %07x  Flags = %02x  Unk = %04x", i, _fileTable[i].offset, _fileTable[i].dataSize, _fileTable[i].flags, _fileTable[i].unk);
	}
}

MohawkOutputStream MohawkArchive::getRawData(uint32 tag, uint16 id) {
	MohawkOutputStream output = { 0, 0, 0, 0, 0, 0, 0, "" };

	if (!_mhk)
		return output;

	int16 typeIndex = getTypeIndex(tag);

	if (typeIndex < 0)
		return output;

	int16 idIndex = getIdIndex(typeIndex, id);

	if (idIndex < 0)
		return output;

	// Note: the fileTableIndex is based off 1, not 0. So, subtract 1
	uint16 fileTableIndex = _types[typeIndex].resTable.entries[idIndex].index - 1;

	// WORKAROUND: tMOV resources pretty much ignore the size part of the file table,
	// as the original just passed the full Mohawk file to QuickTime and the offset.
	// We set the resource size to the number of bytes till the beginning of the next
	// resource in the archive.
	// We need to do this because of the way Mohawk is set up (this is much more "proper"
	// than passing _stream at the right offset). We may want to do that in the future, though.
	if (_types[_curExType].tag == ID_TMOV) {
		uint16 nextFileIndex = fileTableIndex + 1;
		output.size = 0;
		while (output.size == 0) {
			if (nextFileIndex == _fileTableAmount)
				output.size = _mhk->size() - _fileTable[fileTableIndex].offset;
			else
				output.size = _fileTable[nextFileIndex].offset - _fileTable[fileTableIndex].offset;

			// Loop because two entries in the file table may point to the same data
			// in the archive.
			nextFileIndex++;
		}
	} else
		output.size = _fileTable[fileTableIndex].dataSize;

	output.tag = tag;
	output.id = id;
	output.index = fileTableIndex;
	output.flags = _fileTable[fileTableIndex].flags;
	output.offset = _fileTable[fileTableIndex].offset;

	for (uint16 i = 0; i < _types[typeIndex].nameTable.num; i++) {
		if (_types[typeIndex].nameTable.entries[i].index == fileTableIndex+1) {
			output.name = _types[typeIndex].nameTable.entries[i].name;
			break;
		}
	}

	output.stream = _mhk;
	output.stream->seek(output.offset, SEEK_SET);

	return output;
}

MohawkOutputStream MohawkArchive::getNextFile() {
	MohawkOutputStream output = { 0, 0, 0, 0, 0, 0, 0, "" };

	if (_curExType >= _typeTable.resource_types) // No more!
		return output;

	if (_curExTypeIndex >= _types[_curExType].resTable.resources) {
		_curExType++;
		_curExTypeIndex = 0;

		if (_curExType >= _typeTable.resource_types) // No more!
			return output;
	}

	uint16 fileTableIndex = _types[_curExType].resTable.entries[_curExTypeIndex].index - 1;

	// WORKAROUND: tMOV resources pretty much ignore the size part of the file table,
	// as the original just passed the full Mohawk file to QuickTime and the offset.
	// We set the resource size to the number of bytes till the beginning of the next
	// resource in the archive.
	// We need to do this because of the way Mohawk is set up (this is much more "proper"
	// than passing _stream at the right offset). We may want to do that in the future, though.
	if (_types[_curExType].tag == ID_TMOV) {
		uint16 nextFileIndex = fileTableIndex + 1;
		output.size = 0;
		while (output.size == 0) {
			if (nextFileIndex == _fileTableAmount)
				output.size = _mhk->size() - _fileTable[fileTableIndex].offset;
			else
				output.size = _fileTable[nextFileIndex].offset - _fileTable[fileTableIndex].offset;

			// Loop because two entries in the file table may point to the same data
			// in the archive.
			nextFileIndex++;
		}
	} else
		output.size = _fileTable[fileTableIndex].dataSize;

	output.tag = _types[_curExType].tag;
	output.id = _types[_curExType].resTable.entries[_curExTypeIndex].id;
	output.index = fileTableIndex;
	output.flags = _fileTable[fileTableIndex].flags;
	output.offset = _fileTable[fileTableIndex].offset;
	output.stream = _mhk;
	output.stream->seek(output.offset, SEEK_SET);

	for (uint16 i = 0; i < _types[_curExType].nameTable.num; i++) {
		if (_types[_curExType].nameTable.entries[i].index == fileTableIndex+1) {
			output.name = _types[_curExType].nameTable.entries[i].name;
			break;
		}
	}

	_curExTypeIndex++;
	return output;
}

void LivingBooksArchive_v1::open(Common::File *stream) {
	close();
	_mhk = stream;

	// This is for the "old" Mohawk resource format used in some older
	// Living Books. It is very similar, just missing the MHWK tag and
	// some other minor differences, especially with the file table
	// being merged into the resource table.

	uint32 headerSize = _mhk->readUint32BE();

	// NOTE: There are differences besides endianness! (Subtle changes,
	// but different).

	if (headerSize == 6) { // We're in Big Endian mode (Macintosh)
		_mhk->readUint16BE(); // Resource Table Size
		_typeTable.resource_types = _mhk->readUint16BE();
		_types = new OldType[_typeTable.resource_types];

		debug (0, "Old Mohawk File (Macintosh): Number of Resource Types = %04x", _typeTable.resource_types);

		for (uint16 i = 0; i < _typeTable.resource_types; i++) {
			_types[i].tag = _mhk->readUint32BE();
			_types[i].resource_table_offset = (uint16)_mhk->readUint32BE() + 6;
			_mhk->readUint32BE(); // Unknown (always 0?)

			debug (3, "Type[%02d]: Tag = \'%s\'  ResTable Offset = %04x", i, tag2str(_types[i].tag), _types[i].resource_table_offset);

			uint32 oldPos = _mhk->pos();

			// Resource Table/File Table
			_mhk->seek(_types[i].resource_table_offset, SEEK_SET);
			_types[i].resTable.resources = _mhk->readUint16BE();
			_types[i].resTable.entries = new OldType::ResourceTable::Entries[_types[i].resTable.resources];

			for (uint16 j = 0; j < _types[i].resTable.resources; j++) {
				_types[i].resTable.entries[j].id = _mhk->readUint16BE();
				_types[i].resTable.entries[j].offset = _mhk->readUint32BE();
				_types[i].resTable.entries[j].size = _mhk->readByte() << 16;
				_types[i].resTable.entries[j].size += _mhk->readUint16BE();
				_mhk->seek(5, SEEK_CUR); // Unknown (always 0?)

				debug (4, "Entry[%02x]: ID = %04x (%d)\tOffset = %08x, Size = %08x", j, _types[i].resTable.entries[j].id, _types[i].resTable.entries[j].id, _types[i].resTable.entries[j].offset, _types[i].resTable.entries[j].size);
			}

			_mhk->seek(oldPos, SEEK_SET);
			debug (3, "\n");
		}
	} else if (SWAP_BYTES_32(headerSize) == 6) { // We're in Little Endian mode (Windows)
		_mhk->readUint16LE(); // Resource Table Size
		_typeTable.resource_types = _mhk->readUint16LE();
		_types = new OldType[_typeTable.resource_types];

		debug (0, "Old Mohawk File (Windows): Number of Resource Types = %04x", _typeTable.resource_types);

		for (uint16 i = 0; i < _typeTable.resource_types; i++) {
			_types[i].tag = _mhk->readUint32LE();
			_types[i].resource_table_offset = _mhk->readUint16LE() + 6;
			_mhk->readUint16LE(); // Unknown (always 0?)

			debug (3, "Type[%02d]: Tag = \'%s\'  ResTable Offset = %04x", i, tag2str(_types[i].tag), _types[i].resource_table_offset);

			uint32 oldPos = _mhk->pos();

			// Resource Table/File Table
			_mhk->seek(_types[i].resource_table_offset, SEEK_SET);
			_types[i].resTable.resources = _mhk->readUint16LE();
			_types[i].resTable.entries = new OldType::ResourceTable::Entries[_types[i].resTable.resources];

			for (uint16 j = 0; j < _types[i].resTable.resources; j++) {
				_types[i].resTable.entries[j].id = _mhk->readUint16LE();
				_types[i].resTable.entries[j].offset = _mhk->readUint32LE();
				_types[i].resTable.entries[j].size = _mhk->readUint32LE();
				_mhk->readUint16LE(); // Unknown (always 0?)

				debug (4, "Entry[%02x]: ID = %04x (%d)\tOffset = %08x, Size = %08x", j, _types[i].resTable.entries[j].id, _types[i].resTable.entries[j].id, _types[i].resTable.entries[j].offset, _types[i].resTable.entries[j].size);
			}

			_mhk->seek(oldPos, SEEK_SET);
			debug (3, "\n");
		}
	} else
		error("Could not determine type of Old Mohawk resource");

}

MohawkOutputStream LivingBooksArchive_v1::getRawData(uint32 tag, uint16 id) {
	MohawkOutputStream output = { 0, 0, 0, 0, 0, 0, 0, "" };

	if (!_mhk)
		return output;

	int16 typeIndex = getTypeIndex(tag);

	if (typeIndex < 0)
		return output;

	int16 idIndex = getIdIndex(typeIndex, id);

	if (idIndex < 0)
		return output;

	output.offset = _types[typeIndex].resTable.entries[idIndex].offset;
	output.size   = _types[typeIndex].resTable.entries[idIndex].size;
	output.tag = tag;
	output.id = id;
	output.index = idIndex;
	output.stream = _mhk;
	output.stream->seek(output.offset, SEEK_SET);

	return output;
}

MohawkOutputStream LivingBooksArchive_v1::getNextFile() {
	MohawkOutputStream output = { 0, 0, 0, 0, 0, 0, 0, "" };

	if (_curExType >= _typeTable.resource_types) // No more!
		return output;

	if (_curExTypeIndex >= _types[_curExType].resTable.resources) {
		_curExType++;
		_curExTypeIndex = 0;

		if (_curExType >= _typeTable.resource_types) // No more!
			return output;
	}

	output.offset = _types[_curExType].resTable.entries[_curExTypeIndex].offset;
	output.size   = _types[_curExType].resTable.entries[_curExTypeIndex].size;
	output.tag = _types[_curExType].tag;
	output.id = _types[_curExType].resTable.entries[_curExTypeIndex].id;
	output.index = _curExType;
	output.stream = _mhk;
	output.stream->seek(output.offset, SEEK_SET);

	_curExTypeIndex++;
	return output;
}

void CSWorldDeluxeArchive::open(Common::File *stream) {
	close();
	_mhk = stream;

	// CSWorld Deluxe uses another similar format, but with less features
	// then the next archive version. There is no possibility for a name
	// table here and is the simplest of the three formats.

	uint32 typeTableOffset = _mhk->readUint32LE();

	_mhk->seek(typeTableOffset, SEEK_SET);

	_typeTable.resource_types = _mhk->readUint16LE();
	_types = new OldType[_typeTable.resource_types];

	debug (0, "CSWorld Deluxe File: Number of Resource Types = %04x", _typeTable.resource_types);

	for (uint16 i = 0; i < _typeTable.resource_types; i++) {
		_types[i].tag = _mhk->readUint32LE();
		_types[i].resource_table_offset = _mhk->readUint16LE();

		debug (3, "Type[%02d]: Tag = \'%s\'  ResTable Offset = %04x", i, tag2str(_types[i].tag), _types[i].resource_table_offset);

		uint32 oldPos = _mhk->pos();

		// Resource Table/File Table
		_mhk->seek(_types[i].resource_table_offset + typeTableOffset, SEEK_SET);
		_types[i].resTable.resources = _mhk->readUint16LE();
		_types[i].resTable.entries = new OldType::ResourceTable::Entries[_types[i].resTable.resources];

		for (uint16 j = 0; j < _types[i].resTable.resources; j++) {
			_types[i].resTable.entries[j].id = _mhk->readUint16LE();
			_types[i].resTable.entries[j].offset = _mhk->readUint32LE() + 1; // Need to add one to the offset!
			_types[i].resTable.entries[j].size = (_mhk->readUint32LE() & 0xfffff); // Seems only the bottom 20 bits are valid (top two bytes might be flags?)
			_mhk->readByte(); // Unknown (always 0?)

			debug (4, "Entry[%02x]: ID = %04x (%d)\tOffset = %08x, Size = %08x", j, _types[i].resTable.entries[j].id, _types[i].resTable.entries[j].id, _types[i].resTable.entries[j].offset, _types[i].resTable.entries[j].size);
		}

		_mhk->seek(oldPos, SEEK_SET);
		debug (3, "\n");
	}
}

MohawkArchive *MohawkArchive::createMohawkArchive(Common::File *stream) {
	uint32 headerTag = stream->readUint32BE();

	MohawkArchive *mohawkArchive = 0;

	if (headerTag == ID_MHWK) {
		stream->readUint32BE(); // File size, ignore
		headerTag = stream->readUint32BE();
		if (headerTag == ID_RSRC)
			mohawkArchive = new MohawkArchive();
	} else if (headerTag == 6 || SWAP_BYTES_32(headerTag) == 6) {
		// Assume the Living Books v1 archive format
		mohawkArchive = new LivingBooksArchive_v1();
	} else {
		headerTag = SWAP_BYTES_32(headerTag);
		// Use a simple heuristic for testing if it's a CSWorld Deluxe file
		if (headerTag + 2 < stream->size()) {
			stream->seek(headerTag, SEEK_SET);
			uint16 typeCount = stream->readUint16LE();

			if (typeCount * 6 + (uint32)stream->pos() < stream->size()) {
				bool isDeluxeArchive = true;

				for (uint16 i = 0; i < typeCount; i++) {
					stream->readUint32LE(); // Ignore tag
					if (stream->readUint16LE() + headerTag >= stream->size()) {
						isDeluxeArchive = false;
						break;
					}
				}
	
				if (isDeluxeArchive)
					mohawkArchive = new CSWorldDeluxeArchive();
			}
		}
	}

	stream->seek(0, SEEK_SET);

	if (mohawkArchive)
		mohawkArchive->open(stream);

	return mohawkArchive;
}
