/**
 * @file db_gbcollect.h
 * @brief prototype of a function used to clear a file
 */

#ifndef PICTDBPRJ_GBCOLLECT_H
#define PICTDBPRJ_GBCOLLECT_H

#include "pictDB.h"

/**
 * @brief Reduces the size of the file if there are unused pictures and may remove duplicates of small images.
 * 
 * @param db_file The file we want to clear.
 * @param orig_name The name of the original file already open.
 * @param new_file_name The name of the new temporary file.
 */ 
int do_gbcollect(struct pictdb_file* db_file, char* orig_name, char* new_file_name);

#endif
