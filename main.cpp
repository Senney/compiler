/* Sean Heintz - 10053525 */

#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdio>
#include <string>
#include <vector>

#include "Error.h"
#include "JMMLexer.h"
#include "JMMParser.h"
#include "ASTWalker.h"
#include "CodeGenerator.h"

void usage();
void parseArguments(int, char**);

// Globals for the parsing of arguments.
bool ShowHelp = false;
bool ContinueLexerError = false;
bool PrintTokens = false;
bool Verbose = false;

/* Entry point for the program.
Expects a command line argument specifying the filename. */
/* Expected Input Format */
/* Lexer [OPTIONS]* filename */
int main(int argc, char* argv[]) {
	parseArguments(argc, argv);
	
	if (ShowHelp) {
		usage();
		return -1;
	}

	JMMLexer lexer;
	
	/* Open the requested file. */
	if (lexer.open(argv[argc - 1]) != 0) {
		panic("An error occured while opening the file.\n");
		return -1;
	}

	info("Parsing the file...\n");
	JMMParser parser(&lexer);

	if (!parser.parse()) {
		panic("Parsing failed...\n");
	}
	
	ASTWalker walker(parser.getASTRoot(), argv[argc - 1]);
        
        // Add RTS.
        walker.insertSymbol("prints", 0);
        walker.insertSymbol("halt", 0);
        walker.insertSymbol("printc", 0);
        walker.insertSymbol("printb", 0);
        walker.insertSymbol("printi", 0);
        walker.insertSymbol("getchar", 0);
        
	if (!walker.runSemanticCheck()) {
	  std::cerr << "Compilation of file " << argv[argc-1] << " has failed." << std::endl;
          return -1;
	}
	
	CodeGenerator gen("out.s");
	gen.generate(parser.getASTRoot());
        
        std::ofstream of("out.s");
        of << gen.getCode();
        of.close();

        std::cout << "Assembly output saved to out.s" << std::endl;
        
	return 0;
}

/* Parse the arguments that are passed in via argc and argv. Reports to stderr when encountering an undetected character. */
void parseArguments(int argc, char** argv) {
	argc--; argv++;
	if (argc == 0) {
		ShowHelp = true;
		return;
	}
	
	for (int i = 0; i < argc; i++) {
		std::string arg = argv[i];
		if (arg == "-h" || arg == "--help") {
			ShowHelp = true;
			break;
		} else if (arg == "--LexerContinue" || arg == "-lc") {
			ContinueLexerError = true;
		} else if (arg == "-lp" || arg == "--LexerPrint") {
			PrintTokens = true;
		} else if (arg == "-v" || arg == "--verbose") {
			Verbose = true;
		} else {
			// Make sure we don't throw an error for the filename.
			if (i < (argc-1))
				warning("Unexpected argument encountered. Continuing...\n");
		}
	}
}

/* Prints usage information */
void usage() {
	std::cout << "This is Milestone 1 of the project for CPSC411 at the University of Calgary \nwritten by Sean Heintz [10053525]. \nPlease report any bugs to sean.heintz@gmail.com" << std::endl;
	std::cout << "Usage:" << std::endl;
	std::cout << "\tLexer [options] [filename]" << std::endl;
	std::cout << "Options:\n\t-h | --help\t\tShow the help dialogue." << std::endl;
	std::cout << "Parser Options:" << std::endl;
	std::cout << "\t-lc | --LexerContinue\tAsk the lexer to continue when it encounters an error." << std::endl;
	std::cout << "\t-lp | --LexerPrint\tAsk the lexer to print its tokens to the standard output stream." << std::endl;
	std::cout << "\t-v | --verbose\tAsk the lexer to print the tokens that it receives to stdout." << std::endl;
}
