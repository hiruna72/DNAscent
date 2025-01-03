//----------------------------------------------------------
// Copyright 2019 University of Oxford
// This software is licensed under GPL-3.0.  You should have
// received a copy of the license with this software.  If
// not, please Email the author.
//----------------------------------------------------------

#ifndef ERROR_HANDLING_H
#define ERROR_HANDLING_H

#include <math.h>
#include <cmath>
#include <stdlib.h>
#include <assert.h>
#include <exception>
#include <string.h>
#include <string>

struct IOerror : public std::exception {
	std::string badFilename;	
	IOerror( std::string s ){

		badFilename = s;
	}
	const char* what () const throw () {
		const char* message = "Could not open file: ";
		const char* specifier = badFilename.c_str();
		char* result;
		result = static_cast<char*>(calloc(strlen(message)+strlen(specifier)+1, sizeof(char)));
		strcpy( result, message);
		strcat( result, specifier );

		return result;
	}	
};

struct VBZError : public std::exception {
	std::string filename;	
	VBZError( std::string s ){

		filename = s;
	}
	const char* what () const throw () {
		const char* message = "This fast5 file is compressed with vbz: ";
		const char* message2 = "\nSee documentation (https://dnascent.readthedocs.io/en/latest/installation.html#building-from-source) for how to load vbz plugin.";
		const char* specifier = filename.c_str();
		char* result;
		result = static_cast<char*>(calloc(strlen(message)+strlen(message2)+strlen(specifier)+1, sizeof(char)));
		strcpy( result, message);
		strcat( result, specifier );
		strcat( result, message2 );

		return result;
	}	
};


struct InvalidExtension : public std::exception {
	std::string badFilename;	
	InvalidExtension( std::string s ){

		badFilename = s;
	}
	const char* what () const throw () {
		const char* message = "Output extension should be detect or bam. Received: ";
		const char* specifier = badFilename.c_str();
		char* result;
		result = static_cast<char*>(calloc(strlen(message)+strlen(specifier)+1, sizeof(char)));
		strcpy( result, message);
		strcat( result, specifier );

		return result;
	}	
};


struct InvalidOption : public std::exception {
	std::string badOption;	
	InvalidOption( std::string s ){

		badOption = s;
	}
	const char* what () const throw () {
		const char* message = "Invalid option passed: ";
		const char* specifier = badOption.c_str();
		char* result;
		result = static_cast<char*>(calloc(strlen(message)+strlen(specifier)+1, sizeof(char)));
		strcpy( result, message);
		strcat( result, specifier );

		return result;
	}	
};


struct MissingFast5 : public std::exception {
	std::string badFast5;	
	MissingFast5( std::string s ){

		badFast5 = s;
	}
	const char* what () const throw () {
		const char* message = "In specified directory, couldn't find fast5 or pod5 file: ";
		const char* specifier = badFast5.c_str();
		char* result;
		result = static_cast<char*>(calloc(strlen(message)+strlen(specifier)+1, sizeof(char)));
		strcpy( result, message);
		strcat( result, specifier );

		return result;
	}	
};


struct BadBamField : public std::exception {
	std::string msg;
	BadBamField( std::string s ){

		msg = s;
	}
	const char* what () const throw () {
		const char* message = "Invalid bam field on readID: ";
		const char* specifier = msg.c_str();
		char* result;
		result = static_cast<char*>(calloc(strlen(message)+strlen(specifier)+1, sizeof(char)));
		strcpy( result, message);
		strcat( result, specifier );

		return result;
	}
};


struct BamWriteError : public std::exception {
	std::string filename;	
	BamWriteError( std::string s ){

		filename = s;
	}
	const char* what () const throw () {
		const char* message = "Write error on filename: ";
		const char* specifier = filename.c_str();
		char* result;
		result = static_cast<char*>(calloc(strlen(message)+strlen(specifier)+1, sizeof(char)));
		strcpy( result, message);
		strcat( result, specifier );

		return result;
	}	
};


struct InvalidDevice : public std::exception {
	std::string badDeviceID;
	InvalidDevice( std::string s ){

		badDeviceID = s;
	}
	const char* what () const throw () {
		const char* message = "Invalid GPU device ID (expected single int): ";
		const char* specifier = badDeviceID.c_str();
		char* result;
		result = static_cast<char*>(calloc(strlen(message)+strlen(specifier)+1, sizeof(char)));
		strcpy( result, message);
		strcat( result, specifier );

		return result;
	}
};


struct TrailingFlag : public std::exception {
	std::string msg;
	TrailingFlag( std::string s ){

		msg = s;
	}
	const char* what () const throw () {
		const char* message = "Expected input for flag: ";
		const char* specifier = msg.c_str();
		char* result;
		result = static_cast<char*>(calloc(strlen(message)+strlen(specifier)+1, sizeof(char)));
		strcpy( result, message);
		strcat( result, specifier );

		return result;
	}
};


struct InsufficientArguments : public std::exception {
	const char * what () const throw () {
		return "Insufficient number of arguments passed to executable.";
	}
};


struct BadStrandDirection : public std::exception {
	const char * what () const throw () {
		return "Unrecognised strand direction.";
	}
};


struct FastaFormatting : public std::exception {
	const char * what () const throw () {
		return "Reference file is not in the correct format - fasta format required.";
	}
};


struct BadFast5Field : public std::exception {
	const char * what () const throw () {
		return "fast5 field could not be opened.";
	}
};


struct BadPod5Field : public std::exception {
	const char * what () const throw () {
		return "pod5 field could not be opened.";
	}
};


struct MismatchedDimensions : public std::exception {
	const char * what () const throw () {
		return "Gaussian elimination on A*x=b.  Rows in A must equal length of b.";
	}
};


struct ForkSenseData : public std::exception {
	const char * what () const throw () {
		return "Not enough data was passed to forkSense to reliably segment analogue regions. Data passed in bam or detect format and should include at least several hundred reads.";
	}
};


struct ParsingError : public std::exception {
	const char * what () const throw () {
		return "Parsing error reading BAM file.";
	}
};


struct DetectParsing : public std::exception {
	const char * what () const throw () {
		return "Parsing error on DNAscent detect output.";
	}
};

struct IndexFormatting : public std::exception {
	const char * what () const throw () {
		return "Index should specify whether fast5 is bulk or individual.  Please contact the author.";
	}
};

struct MissingModelPath : public std::exception {
	const char * what () const throw () {
		return "Missing path to model file.  Please contact the author.";
	}
};

struct OverwriteFailure : public std::exception {
	const char * what () const throw () {
		return "Output filename would overwrite one of the input files.";
	}
};

struct UnrecognisedBase : public std::exception {
	const char * what () const throw () {
		return "Input sequence contains an unrecognised base - must be A, T, G, C, or N.";
	}
};

struct InvalidMappingThreshold : public std::exception {
	const char * what () const throw () {
		return "Mapping quality passed with -q must be an integer >= 0.";
	}
};

struct InvalidLengthThreshold : public std::exception {
	const char * what () const throw () {
		return "Minimum read mapping length passed with -l must be an integer >= 100.";
	}
};

#endif

