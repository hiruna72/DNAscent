//----------------------------------------------------------
// Copyright 2019 University of Oxford
// Written by Michael A. Boemo (mb915@cam.ac.uk)
// This software is licensed under GPL-2.0.  You should have
// received a copy of the license with this software.  If
// not, please Email the author.
//----------------------------------------------------------

#include <fstream>
#include <omp.h>
#include <stdio.h>
#include "../tinydir/tinydir.h"
#include "../fast5/include/fast5.hpp"
#include "common.h"
#include "data_IO.h"
#include "error_handling.h"
#include <cmath>
#define _USE_MATH_DEFINES

 static const char *help=
"index: DNAscent executable that builds an index file for DNAscent detect.\n"
"To run DNAscent index, do:\n"
"  ./DNAscent index -f /path/to/fast5Directory\n"
"Required arguments are:\n"
"  -d,--detect               path to fast5 files.\n"
"Optional arguments are:\n"
"  -o,--output               output file name (default is index.dnascent),\n"
"  -s,--sequencing-summary   path to sequencing summary file Guppy (optional but strongly recommended).\n"
"Written by Michael Boemo, Department of Pathology, University of Cambridge.\n"
"Please submit bug reports to GitHub Issues (https://github.com/MBoemo/DNAscent/issues).";

 struct Arguments {
	std::string fast5path;
	std::string ssPath;
	std::string outfile;
	bool gaveSeqSummary = false;
};


Arguments parseIndexArguments( int argc, char** argv ){

 	if( argc < 2 ){
 		std::cout << "Exiting with error.  Insufficient arguments passed to DNAscent index." << std::endl << help << std::endl;
		exit(EXIT_FAILURE);
	}
 	if ( std::string( argv[ 1 ] ) == "-h" or std::string( argv[ 1 ] ) == "--help" ){
 		std::cout << help << std::endl;
		exit(EXIT_SUCCESS);
	}
	else if( argc < 3 ){
 		std::cout << "Exiting with error.  Insufficient arguments passed to DNAscent index." << std::endl;
		exit(EXIT_FAILURE);
	}
 	Arguments args;
	args.outfile = "index.dnascent";

 	/*parse the command line arguments */
	for ( int i = 1; i < argc; ){
 		std::string flag( argv[ i ] );
 		if ( flag == "-d" or flag == "--detect" ){
 			std::string strArg( argv[ i + 1 ] );
			args.fast5path = strArg;
			i+=2;
		}
		else if ( flag == "-s" or flag == "--sequencing-summary" ){

			std::string strArg( argv[ i + 1 ] );
			args.ssPath = strArg;
			args.gaveSeqSummary = true;
			i+=2;
		}
		else if ( flag == "-o" or flag == "--output" ){

			std::string strArg( argv[ i + 1 ] );
			args.outfile = strArg;
			i+=2;
		}
		else throw InvalidOption( flag );
	}
	return args;
}


void countFast5(std::string path, int &count){

	tinydir_dir dir;
	unsigned int i;
	if (tinydir_open_sorted(&dir, path.c_str()) == -1){
		std::string error = "Error opening directory "+path;
		perror(error.c_str());
		goto fail;
	}

	for (i = 0; i < dir.n_files; i++){

		tinydir_file file;
		if (tinydir_readfile_n(&dir, &file, i) == -1){
			std::string error = "Error opening file in "+path;
			perror(error.c_str());
			goto fail;
		}

		if (file.is_dir){

			if (strcmp(file.name,".") != 0 and strcmp(file.name,"..") != 0){

				std::string newPath = path + "/" + file.name;
				countFast5(newPath, count);
			}
		}
		else count++;
	}

	fail:
	tinydir_close(&dir);
}

const char *get_ext(const char *filename){

	const char *ext = strrchr(filename, '.');
	if(!ext || ext == filename) return "";
	return ext + 1;
}


void readDirectory(std::string path, std::map<std::string,std::string> &allfast5paths){

	tinydir_dir dir;
	unsigned int i;
	if (tinydir_open_sorted(&dir, path.c_str()) == -1){
		std::string error = "Error opening directory "+path;
		perror(error.c_str());
		goto fail;
	}

	for (i = 0; i < dir.n_files; i++){

		tinydir_file file;
		if (tinydir_readfile_n(&dir, &file, i) == -1){
			std::string error = "Error opening file in "+path;
			perror(error.c_str());
			goto fail;
		}

		if (file.is_dir){

			if (strcmp(file.name,".") != 0 and strcmp(file.name,"..") != 0){

				std::string newPath = path + "/" + file.name;
				readDirectory(newPath, allfast5paths);
			}
		}
		else allfast5paths[file.name] = path + "/" + file.name;
	}

	fail:
	tinydir_close(&dir);
}


std::vector<std::string> fast5_get_multi_read_groups(hid_t &hdf5_file){
    std::vector<std::string> out;
    ssize_t buffer_size = 0;
    char* buffer = NULL;

    // get the number of groups in the root group
    H5G_info_t group_info;
    int ret = H5Gget_info_by_name(hdf5_file, "/", &group_info, H5P_DEFAULT);
    if(ret < 0) {
        fprintf(stderr, "error getting group info\n");
        exit(EXIT_FAILURE);
    }

    for(size_t group_idx = 0; group_idx < group_info.nlinks; ++group_idx) {

        // retrieve the size of this group name
        ssize_t size = H5Lget_name_by_idx(hdf5_file, "/", H5_INDEX_NAME, H5_ITER_INC, group_idx, NULL, 0, H5P_DEFAULT);
	
        if(size < 0) {
            fprintf(stderr, "error getting group name size\n");
            exit(EXIT_FAILURE);
        }
        size += 1; // for null terminator
           
        if(size > buffer_size) {
            buffer = (char*)realloc(buffer, size);
            buffer_size = size;
        }
    
        // copy the group name
        H5Lget_name_by_idx(hdf5_file, "/", H5_INDEX_NAME, H5_ITER_INC, group_idx, buffer, buffer_size, H5P_DEFAULT);
        buffer[size] = '\0';
        out.push_back(buffer);
	
    }

    free(buffer);
    buffer = NULL;
    buffer_size = 0;
    return out;
}


std::map<std::string,std::string> parseSequencingSummary(std::string path, bool &bulk){

	std::map<std::string,std::string> readID2fast5;
	std::map<std::string,std::vector<std::string>> fast52readID;
	
 	std::ifstream inFile( path );
	if ( not inFile.is_open() ) throw IOerror( path );
	std::string line;
	std::getline(inFile,line);//header

	bulk = false;

	while ( std::getline( inFile, line ) ){

		std::string readID, fast5, column;
		std::stringstream ssLine(line);
		int cIndex = 0;
		while( std::getline( ssLine, column, '\t' ) ){

			if (cIndex == 0) fast5 = column;
			else if (cIndex == 1) readID = column;
			else if (cIndex > 1) break;
			cIndex++;
		}
		readID2fast5[readID] = fast5;
		fast52readID[fast5].push_back(readID);
	}

	for ( auto idpair = fast52readID.begin(); idpair != fast52readID.end(); idpair++ ){

		if ( (idpair->second).size() > 1 ){

			bulk = true;
			break;
		}
	}

	return readID2fast5;
}


int index_main( int argc, char** argv ){

 	Arguments args = parseIndexArguments( argc, argv );

	int totalFast5 = 0;
	countFast5(args.fast5path.c_str(), totalFast5);

	int progress = 0;
	progressBar pb(totalFast5,false);

	std::ofstream outFile( args.outfile );
	if ( not outFile.is_open() ) throw IOerror( args.outfile );

	//iterate on the filesystem to find the full path for each fast5 file
	std::map<std::string,std::string> fast52fullpath;
	readDirectory(args.fast5path.c_str(), fast52fullpath);

	//if we have a sequencing summary file, use it to build a readID to fast5 map
	if (args.gaveSeqSummary){

		bool isBulkFast5;
		std::map<std::string,std::string> readID2fast5 = parseSequencingSummary(args.ssPath, isBulkFast5);

		if (isBulkFast5) outFile << "#bulk" << std::endl;
		else outFile << "#individual" << std::endl;

		for (auto idpair = readID2fast5.begin(); idpair != readID2fast5.end(); idpair++){

			//check that the path we need is in the map and exit gracefully if not
			if ( fast52fullpath.count(idpair->second) == 0 ) throw MissingFast5(idpair->second);

			outFile << idpair->first << "\t" << fast52fullpath.at(idpair->second) << std::endl;
			progress++;
			pb.displayProgress( progress, 0, 0 );
		}
	}
	else{ //otherwise we're going to have to brute force open each fast5 file and pull out the readID

		std::cout << "Warning: No sequencing summary file specified.  Specifying a sequencing summary will make indexing much faster." << std::endl;

		outFile << "#bulk" << std::endl;

		for ( auto pathpair = fast52fullpath.begin(); pathpair != fast52fullpath.end(); pathpair++ ){

			std::string path = pathpair -> second;

			hid_t hdf5_file = H5Fopen(path.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);
			std::vector<std::string> readIDs = fast5_get_multi_read_groups(hdf5_file);
		
			std::string prefix = "read_";
			for ( auto rawid = readIDs.begin(); rawid < readIDs.end(); rawid++ ){

				std::string id = *rawid;
				id.erase(0, id.find(prefix) + prefix.length());
				outFile << id << "\t" << path << std::endl;
			}
		
			progress++;
			pb.displayProgress( progress, 0, 0 );
		}
	}
	outFile.close();
	std::cout << std::endl;
 	return 0;
}
