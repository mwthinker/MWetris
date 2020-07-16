#ifndef FLAGS_H
#define FLAGS_H

#include <ai.h>
#include <calc/calculatorexception.h>

#include <chrono>
#include <queue>

class Flags {
public:
	Flags();

	Flags(const int argc, const char* const argv[]);

	void printHelpFunction() const;

	std::string programName_;
	
	int maxNbrBlocks_;
	tetris::Ai ai_;
	std::string randomFilePath_;
	std::queue<std::string> outputOrder_;
	
	std::chrono::milliseconds delay_{0};
	bool printHelp_{false};
	bool play_{false};
	bool useRandomFile_{false};
	int width_{10};
	int height_{24};
	bool verbose_{false};
	int depth_{1};
};

#endif	// FLAGS_H
