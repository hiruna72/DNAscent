//----------------------------------------------------------
// Copyright 2019-2020 University of Oxford
// Written by Michael A. Boemo (mb915@cam.ac.uk)
// This software is licensed under GPL-3.0.  You should have
// received a copy of the license with this software.  If
// not, please Email the author.
//----------------------------------------------------------

#include <fstream>
#include <omp.h>
#include <stdio.h>
#include <cmath>
#include <omp.h>
#include "../tinydir/tinydir.h"
#include "../fast5/include/fast5.hpp"
#include "common.h"
#include "error_handling.h"
#include "fast5.h"
#include "pod5.h"

#define _USE_MATH_DEFINES

 static const char *help=
"index: DNAscent executable that builds an index file for DNAscent detect.\n"
"To run DNAscent index, do:\n"
"   DNAscent index -f /path/to/directory\n"
"Required arguments are:\n"
"  -f,--files                full path to fast5 or pod5 files.\n"
"Optional arguments are:\n"
"  -s,--sequencing-summary   (legacy) path to sequencing summary file from using Guppy on fast5 files,\n"
"  -o,--output               output file name (default is index.dnascent).\n"
"DNAscent is under active development by the Boemo Group, Department of Pathology, University of Cambridge (https://www.boemogroup.org/).\n"
"Please submit bug reports to GitHub Issues (https://github.com/MBoemo/DNAscent/issues).";

 struct Arguments_index {
	std::string sigfilesPath;
	std::string seqssumPath;
	std::string outfile;
	bool hasSeqSum = false;
};


Arguments_index parseIndexArguments_index( int argc, char** argv ){

 	if( argc < 2 ){
 		std::cout << "Exiting with error.  Insufficient arguments passed to DNAscent index." << std::endl << help << std::endl;
		exit(EXIT_FAILURE);
	}
 	if ( std::string( argv[ 1 ] ) == "-h" or std::string( argv[ 1 ] ) == "--help" ){
 		std::cout << help << std::endl;
		exit(EXIT_SUCCESS);
	}
	else if( argc < 2 ){
 		std::cout << "Exiting with error.  Insufficient arguments passed to DNAscent index." << std::endl;
		exit(EXIT_FAILURE);
	}
 	Arguments_index args;
	args.outfile = "index.dnascent";

 	/*parse the command line arguments */
	for ( int i = 1; i < argc; ){
 		std::string flag( argv[ i ] );
 		if ( flag == "-f" or flag == "--files" ){
 		
 			if (i == argc-1) throw TrailingFlag(flag);		
 		
 			std::string strArg( argv[ i + 1 ] );
			char trailing = strArg.back();
			if (trailing == '/') strArg.pop_back();
			args.sigfilesPath = strArg;
			i+=2;
		}
		else if ( flag == "-s" or flag == "--sequencing-summary" ){

 			if (i == argc-1) throw TrailingFlag(flag);		

			std::string strArg( argv[ i + 1 ] );
			args.seqssumPath = strArg;
			args.hasSeqSum = true;
			i+=2;
		}
		else if ( flag == "-o" or flag == "--output" ){

			if (i == argc-1) throw TrailingFlag(flag);		

			std::string strArg( argv[ i + 1 ] );
			args.outfile = strArg;
			i+=2;
		}
		else throw InvalidOption( flag );
	}
	return args;
}


std::map<std::string,std::string> parseSequencingSummary(Arguments_index &args){

	std::map<std::string,std::string> readID2fast5;
	std::map<std::string,std::vector<std::string>> fast52readID;
	
 	std::ifstream inFile( args.seqssumPath );
	if ( not inFile.is_open() ) throw IOerror( args.seqssumPath );
	std::string line;
	
	//parse header
	int column_filename = -1;
	int column_readID = -1;
	int cIndex = 0;
	std::string col_header;
	std::getline(inFile,line);
	std::stringstream ssHeader(line);
	while( std::getline( ssHeader, col_header, '\t' ) ){
	
		if (col_header == "filename" or col_header == "filename_fast5") column_filename = cIndex;
		else if (col_header == "read_id") column_readID = cIndex;
		cIndex++;
	}
	
	if (column_filename == -1 or column_readID == -1){
	
		std::cerr << "Failed to parse sequencing summary file." << std::endl;
		std::cerr << "Please raise this as an issue on GitHub (https://github.com/MBoemo/DNAscent/issues) and paste the first few lines of the sequencing summary file." << std::endl;
		throw IOerror( args.seqssumPath );
	}
	
	while ( std::getline( inFile, line ) ){

		std::string readID, fast5, column;
		std::stringstream ssLine(line);
		cIndex = 0;
		
		while( std::getline( ssLine, column, '\t' ) ){
		
			if (cIndex == column_filename) fast5 = column;
			else if (cIndex == column_readID) readID = column;
			cIndex++;
		}
		readID2fast5[readID] = fast5;
		fast52readID[fast5].push_back(readID);
	}

	return readID2fast5;
}


void countSignalFiles(std::string path, int &count){

	tinydir_dir dir;
	unsigned int i;
	if (tinydir_open_sorted(&dir, path.c_str()) == -1){
		std::string error = "Error opening directory: "+path;
		perror(error.c_str());
		goto fail;
	}

	for (i = 0; i < dir.n_files; i++){

		tinydir_file file;
		if (tinydir_readfile_n(&dir, &file, i) == -1){
			std::string error = "Error opening file in: "+path;
			perror(error.c_str());
			goto fail;
		}

		if (file.is_dir){

			if (strcmp(file.name,".") != 0 and strcmp(file.name,"..") != 0){

				std::string newPath = path + "/" + file.name;
				countSignalFiles(newPath, count);
			}
		}
		else{
		
			const char *ext = get_ext(file.name);
			if ( strcmp(ext,"fast5") == 0 or strcmp(ext,"pod5") == 0 ) count++;
		}
	}

	fail:
	tinydir_close(&dir);
}


void readDirectory(std::string path, std::vector<std::string> &signalFilePaths){

	tinydir_dir dir;
	unsigned int i;
	if (tinydir_open_sorted(&dir, path.c_str()) == -1){
		std::string error = "Error opening directory: "+path;
		perror(error.c_str());
		goto fail;
	}

	for (i = 0; i < dir.n_files; i++){

		tinydir_file file;
		if (tinydir_readfile_n(&dir, &file, i) == -1){
			std::string error = "Error opening file in: "+path;
			perror(error.c_str());
			goto fail;
		}

		if (file.is_dir){

			if (strcmp(file.name,".") != 0 and strcmp(file.name,"..") != 0){

				char &trail = path.back();
				if (trail == '/') path.pop_back();

				std::string newPath = path + "/" + file.name;
				readDirectory(newPath, signalFilePaths);
			}
		}
		else{
			const char *ext = get_ext(file.name);
			if ( strcmp(ext,"fast5") == 0 or strcmp(ext,"pod5") == 0 ){

				char &trail = path.back();
				if (trail == '/') path.pop_back();

				signalFilePaths.push_back(path + "/" + file.name);
			}
		}
	}

	fail:
	tinydir_close(&dir);
}


bool isFileNameInPaths(const std::string& path, const std::string& fileNameToCheck) {

	size_t lastSlashPos = path.find_last_of("/\\");

	std::string extractedFileName = (lastSlashPos != std::string::npos) 
		? path.substr(lastSlashPos + 1) 
		: path;

	return extractedFileName == fileNameToCheck;
}


int index_main( int argc, char** argv ){

 	Arguments_index args = parseIndexArguments_index( argc, argv );

	int totalSignalFiles = 0;
	countSignalFiles(args.sigfilesPath.c_str(), totalSignalFiles);

	int progress = 0;
	progressBar pb(totalSignalFiles,false);

	std::ofstream outFile( args.outfile );
	if ( not outFile.is_open() ) throw IOerror( args.outfile );

	//iterate on the filesystem to find the full path for each signal file
	std::vector<std::string> signalFilePaths;
	readDirectory(args.sigfilesPath.c_str(), signalFilePaths);

	//if a user specified a sequencing summary for Guppy/fast5, use it instead of crawling through files
	if (args.hasSeqSum){
	
		std::map<std::string,std::string> readID2fast5 = parseSequencingSummary(args);
		
		for (auto idpair = readID2fast5.begin(); idpair != readID2fast5.end(); idpair++){

			std::string fn = idpair->second;

			auto it = std::find_if(signalFilePaths.begin(), signalFilePaths.end(), [&fn](const std::string& path) {
				return isFileNameInPaths(path, fn);
			});
			
			//check that we have the file we need and exit gracefully if not
			if ( it == signalFilePaths.end() ){
			
				const char *ext = get_ext((idpair->second).c_str());
				if (strcmp(ext,"fast5") != 0){
			
					std::cerr << "This isn't a fast5 file: " << idpair->second << std::endl;
				}
			
				throw MissingFast5(idpair->second);
			}
			
			progress++;
			pb.displayProgress( progress, 0, 0 );

			outFile << idpair->first << "\t-1\t-1\t" << *it << std::endl;
		}
	}
	else{

		for (size_t fi = 0; fi < signalFilePaths.size(); fi++){

			const char *ext = get_ext((signalFilePaths[fi]).c_str());
			if (strcmp(ext,"fast5") == 0){
			
				std::vector<std::string> IDs_in_file = fast5_extract_readIDs(signalFilePaths[fi]);
				for (size_t i = 0; i < IDs_in_file.size(); i++){
					outFile << IDs_in_file[i] << "\t-1\t-1\t" << signalFilePaths[fi] << std::endl;
				}
			}
			else if (strcmp(ext,"pod5") == 0){

				std::vector<std::string> IDs_in_file = pod5_extract_readIDs(signalFilePaths[fi]);
				for (size_t i = 0; i < IDs_in_file.size(); i++){
					outFile << IDs_in_file[i] << "\t" << signalFilePaths[fi] << std::endl;
				}		
			}
			else{
				std::cerr << "This doesn't look like a fast5 or pod5 file: " << signalFilePaths[fi] << std::endl;
				throw MissingFast5(signalFilePaths[fi]);
			}
			progress++;
			pb.displayProgress( progress, 0, 0 );
		}
	}

	outFile.close();
	std::cout << std::endl;
 	return 0;
}
