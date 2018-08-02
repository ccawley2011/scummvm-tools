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

/* Compress Broken Sword I sound clusters into MP3/Ogg Vorbis */

#ifndef COMPRESS_SWORD1_H
#define COMPRESS_SWORD1_H

#include "common/str.h"
#include "compress.h"

class CompressSword1 : public CompressionTool {
public:
	CompressSword1(const std::string &name = "compress_sword1");

	virtual void execute();

	virtual InspectionMatch inspectInput(const Common::Filename &filename);

	bool _compSpeech;
	bool _compMusic;

protected:
	void parseExtraArguments();

	std::string _audioOuputFilename;

	int16 *uncompressSpeech(Common::File &clu, uint32 idx, uint32 cSize, uint32 *returnSize, bool* ok = 0);
	uint8 *convertData(uint8 *rawData, uint32 rawSize, uint32 *resSize);
	void convertClu(Common::File &clu, Common::File &cl3);
	void compressSpeech(const Common::Filename *inpath, const Common::Filename *outpath);
	void compressMusic(const Common::Filename *inpath, const Common::Filename *outpath);
	void checkFilesExist(bool checkSpeech, bool checkMusic, const Common::Filename *inpath);

	enum Endianness { BigEndian , LittleEndian , UnknownEndian } ;
	Endianness guessEndianness(int16 *leData, uint32 leSize, bool leOk, int16 *beData, uint32 beSize, bool beOk);
	double endiannessHeuristicValue(int16* data, uint32 dataSize, uint32 maxSamples, Endianness dataEndianness);

private:
	bool _useOutputMusicSubdir;

	bool _macVersion;

	Endianness _speechEndianness;
};

#endif
