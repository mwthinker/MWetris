#include "flags.h"
#include "flagsexception.h"

#include <sstream>
#include <limits>
#include <iostream>

#include <fmt/format.h>

using namespace std::chrono_literals;
using namespace tetris;

namespace {
	
	int extractArgumentPositiveInteger(std::string str) {
		try {
			std::size_t size;
			int positiveNbr = std::stoi(str, &size);
			if (positiveNbr < 0 || size != str.size()) {
				throw FlagsException{fmt::format("Argument {} expects a positive integer\n", str)};
			}
			return positiveNbr;
		} catch (std::invalid_argument) {
			throw FlagsException{fmt::format("Argument {} expects a positive integer\n", str)};
		}
	}
}

Flags::Flags()
	: Flags{0, nullptr} {
}

Flags::Flags(const int argc, const char* const argv[])
	: programName_("TetrisEngine")
	, maxNbrBlocks_{std::numeric_limits<int>::max()} {

	for (int i = 0; i < argc; ++i) {
		std::string arg = argv[i];
		if (arg == "-h" || arg == "--help") {
			printHelp_ = true;
			return;
		} else if (arg == "-d" || arg == "--delay") {
			if (i + 1 < argc) {
				delay_ = std::chrono::milliseconds{extractArgumentPositiveInteger(argv[i + 1])};
				++i;
			} else {
				throw FlagsException{fmt::format("Missing argument after {} {} flag\n", arg)};
			}
		} else if (arg == "-D" || arg == "--depth") {
			if (i + 1 < argc) {
				depth_ = extractArgumentPositiveInteger(argv[i + 1]);
				++i;
			} else {
				throw FlagsException{fmt::format("Missing argument after {} flag\n", arg)};
			}
		} else if (arg == "-a" || arg == "--ai") {
			if (i + 1 < argc) {
				std::string valueFunction = argv[i + 1];
				try {
					ai_ = Ai{"AI", valueFunction, true};
				} catch (const calc::CalculatorException& exception) {
					throw FlagsException{fmt::format("Value function error. {}\n", exception.what())};
				}
				++i;
			} else {
				throw FlagsException{fmt::format("Missing argument after {} flag\n", arg)};
			}
		} else if (arg == "-m" || arg == "--max-nbr-blocks") {
			if (i + 1 < argc) {
				maxNbrBlocks_ = extractArgumentPositiveInteger(argv[i + 1]);
				++i;
			} else {
				throw FlagsException{fmt::format("Missing argument after {} flag\n", arg)};
			}
		} else if (arg == "-f" || arg == "--file-data") {
			if (i + 1 < argc) {
				randomFilePath_ = argv[i + 1];
				useRandomFile_ = true;
				++i;
			} else {
				throw FlagsException{fmt::format("Missing argument after {} flag\n", arg)};
			}
		} else if (arg == "-s" || arg == "--board-size") {
			if (i + 2 < argc) {
				std::stringstream stream;
				stream << argv[i + 1];
				stream << argv[i + 2];
				stream >> width_;
				stream >> height_;
				i += 2;
				if (width_ < 5 || width_ > 99) {
					throw FlagsException{fmt::format("Argument with flag {}, width {} must be within [5, 99]\n", arg, width_)};
				}
				if (height_ < 5 || height_ > 99) {
					throw FlagsException{fmt::format("Argument with flag {}, height {} must be within [5, 99]\n", arg, width_)};
				}
			} else {
				throw FlagsException{fmt::format("Missing argument after {} flag\n", arg)};
			}
		} else if (arg == "-v" || arg == "--verbose") {
			verbose_ = true;
		} else if (arg == "-p" || arg == "--play") {
			play_ = true;
		} else if (arg == "-T" || arg == "--time-output") {
			outputOrder_.push("-T");
		} else if (arg == "-t" || arg == "--turn-output") {
			outputOrder_.push("-t");
		} else if (arg == "-c" || arg == "--cleared-rows") {
			outputOrder_.push("-c");
		} else if (arg == "-c1" || arg == "--cleared-rows-1") {
			outputOrder_.push("-c1");
		} else if (arg == "-c2" || arg == "--cleared-rows-2") {
			outputOrder_.push("-c2");
		} else if (arg == "-c3" || arg == "--cleared-rows-3") {
			outputOrder_.push("-c3");
		} else if (arg == "-c4" || arg == "--cleared-rows-4") {
			outputOrder_.push("-c4");
		}
	}
	// Add all output options if no option i choosen.
	if (outputOrder_.empty()) {
		outputOrder_.push("-T");
		outputOrder_.push("-t");
		outputOrder_.push("-c1");
		outputOrder_.push("-c2");
		outputOrder_.push("-c3");
		outputOrder_.push("-c4");
	}
}

void Flags::printHelpFunction() const {
	std::cout << "Usage: " << programName_ << "\n";
	std::cout << "\t" << "Simulate a tetris game, using a ai value-funtion.\n";
	std::cout << "\t" << programName_ << " -d <DELAY>\n";
	std::cout << "\t" << programName_ << " -a <VALUE_FUNCTION>\n";
	std::cout << "\t" << programName_ << " -D <DEPTH>\n";
	std::cout << "\t" << programName_ << " -m <MAX_TURNS>\n";
	std::cout << "\t" << programName_ << " -f <FILE>\n";
	std::cout << "\t" << programName_ << " -s <WIDTH> <HEIGHT>\n\n";

	Ai ai{};
	std::cout << "\t" << "Default ai value function is \"" << ai.getValueFunction() << "\".\n";
	std::cout << "\t" << "Variables available in the value function:\n";
	for (std::string var : ai.getCalculator().getVariables()) {
		std::cout << "\t\t" << var << "\n";
	}

	std::cout << "\n";
	std::cout << "\tIf using the -f flag, the file provided must be a text file and be filled with\n";
	std::cout << "\tnumber, space, number etc. Other characters will be ignored.\n";
	std::cout << "\tBlocktypes: \n";
	std::cout << "\tI = 0\n";
	std::cout << "\tJ = 1\n";
	std::cout << "\tL = 2\n";
	std::cout << "\tO = 3\n";
	std::cout << "\tS = 4\n";
	std::cout << "\tT = 5\n";
	std::cout << "\tZ = 6\n";
	std::cout << "\tExample of data in file is \"0 2 0 7 1 3\".\n";
	std::cout << "\tWhen the simulation has used the whole file, it start to read from the beginning again, and so on.\n\n";

	std::cout << "Options: " << "\n";
	std::cout << "\t-h --help                show this help\n";
	std::cout << "\t-d --delay               delay in milliseconds between each turn\n";
	std::cout << "\t-a --ai-function         define the value function to be use by the ai\n";
	std::cout << "\t-D --depth               the search depth for the ai (0, 1 or 2), default is 1\n";
	std::cout << "\t-m --max-turns           define the max number of turns\n";
	std::cout << "\t-f --file-data           use random data from a file\n";
	std::cout << "\t-s --board-size          define the size of the board\n";
	std::cout << "\t-v --verbose             show additional info\n";
	std::cout << "\t-p --play                show board each turn\n";
	std::cout << "\n";
	std::cout << "\tOutput order:\n";
	std::cout << "\t-T --time                print the time lapsed\n";
	std::cout << "\t-t --turns               print the number of turns\n";
	std::cout << "\t-c --cleared-rows        print cleared rows\n";
	std::cout << "\t-c1 --cleared-rows-1     print cleared one rows\n";
	std::cout << "\t-c2 --cleared-rows-2     print cleared two rows\n";
	std::cout << "\t-c3 --cleared-rows-3     print cleared three rows\n";
	std::cout << "\t-c4 --cleared-rows-4     print cleared four rows\n\n";

	std::cout << "Example: " << "\n";
	std::cout << "\tShow the default result for the default ai, in the same order as the abowe output order.\n";
	std::cout << "\t" << programName_ << "\n\n";

	std::cout << "\tShow the tetrisboard each turn and delay 250 ms between each turn.\n";
	std::cout << "\t" << programName_ << " --play -d 250\n\n";

	std::cout << "\tList the time and number of four-rows cleared.\n";
	std::cout << "\t" << programName_ << " -T --cleared-row-4\n";
}
