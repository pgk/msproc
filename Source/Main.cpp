/*
  ==============================================================================

msproc: command line tool for working with Mid-Side audio files

Copyright (C) <2015> <Panos Kountanis>

This program is free software; you can redistribute it and/or modify it under the terms of
the GNU General Public License version 2 as published by the Free Software Foundation;

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program;
if not, write to the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"


int BUFFER_SIZE = 44100;
int CHANNELS = 2;


using namespace juce;


void Usage(String error)
{
	std::cout << error << std::endl;
    std::cout << "usage: msproc decode <file>" << std::endl;
}

void PrintMessage(String msg)
{
	std::cout << msg << std::endl;
}


void decodeMidSide(AudioSampleBuffer *input, AudioSampleBuffer *output, long frameSize)
{
	auto mid = input->getReadPointer(0);
	auto side = input->getReadPointer(1);
	auto outLeft = output->getWritePointer(0);
	auto outRight = output->getWritePointer(1);
	for (int i =0; i < frameSize; i++) {
		outLeft[i] = mid[i] + side[i];
		outRight[i] = mid[1] - side[i];

	}
}

// class MidSideProcessorOptions {
// public:
// 	MidSideProcessorOptions(String &anAction, short aMidChannelIndex,
// 												  String anOutFileName) :
// 	    action(anAction),
// 			midChannelIndex(aMidChannelIndex),
// 			outFileName(anOutFileName)
// 	{
// 	}
//
// 	static MidSideProcessorOptions *parseCommandLineArgs(int argc, const char *argv)
// 	{
// 		return new MidSideProcessorOptions(action, midChannelIndex, outFileName);
// 	}
//
// 	String &action;
// 	short midChannelIndex;
// 	String outFileName;
// };


class MidSideProcessor {

public:
	MidSideProcessor(String &filePath_)
	: filePath(filePath_)
	{
		file = juce::File(File::getCurrentWorkingDirectory().getChildFile(filePath));
		manager.registerBasicFormats();
	}
	~MidSideProcessor()
	{
		manager.clearFormats();
	}

	int process() {
		juce::ScopedPointer<AudioFormatReader> reader = manager.createReaderFor(file);

		if (reader->numChannels != 2) {
			PrintMessage("Exactly 2 channels required");
			return 1;
		}

		auto formatName = reader->getFormatName();
		auto format = manager.findFormatForFileExtension(file.getFileExtension());

		if (format == nullptr) {
			PrintMessage(String("Can not find format") + formatName);
			return 1;
		}

		const auto outFile = juce::File(File::getCurrentWorkingDirectory()
										.getChildFile("./ms_decoded_" + file.getFileName()));
		if (outFile.existsAsFile()) {
			outFile.deleteFile();
		}
		ScopedPointer<FileOutputStream> outStream = outFile.createOutputStream();
		if (outStream == nullptr) {
			PrintMessage("Failed to create output stream for file");
			return 1;
		}

		juce::ScopedPointer<AudioFormatWriter> writer =
			format->createWriterFor(outStream, reader->sampleRate, reader->numChannels,
									24, StringPairArray(), 0);

		if (writer != nullptr) {
			outStream.release();
			auto audioBuffer = juce::AudioSampleBuffer(CHANNELS, BUFFER_SIZE);
			auto outBuffer = juce::AudioSampleBuffer(CHANNELS, BUFFER_SIZE);
			unsigned long j = 0;
			int samplesToRead = BUFFER_SIZE;
			while (j < reader->lengthInSamples) {
				if (j + samplesToRead >= reader->lengthInSamples) {
					int diff = (j + samplesToRead) - reader->lengthInSamples;
					samplesToRead -= diff;
				}
				reader->read(&audioBuffer, 0, samplesToRead, j, true, true);
				decodeMidSide(&audioBuffer, &outBuffer, samplesToRead);
				writer->writeFromAudioSampleBuffer(outBuffer, 0, samplesToRead);
				j += BUFFER_SIZE;
			}

		}

		return 0;
	}

private:
	String &filePath;
	File file;
	AudioFormatManager manager;

};


//==============================================================================
int main (int argc, char* argv[])
{
	PrintMessage("msproc v1.0.0");
	if (argc == 2 && String(argv[1]).compare("--help") != -1) {
		Usage("");
		return 0;
	}

    if (argc < 3) {
        Usage("too few arguments");
        return 1;
    }
	String action, filePath;

	if (argc > 3) {
		Usage("too many arguments");
		return 1;
	} else {
		action = String(argv[1]);
		filePath = String(argv[2]);
	}


	if (action != "decode") {
		Usage("action not recognized. Possible values: (encode)");
		return 1;
	}

	PrintMessage("Perfoming action on file");

	MidSideProcessor processor(filePath);
	auto result = processor.process();
	return result;
}
