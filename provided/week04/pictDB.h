/**
 * @file pictDB.h
 * @brief Main header file for pictDB core library.
 *
 * Defines the format of the data structures that will be stored on the disk
 * and provides interface functions.
 *
 * The picture database starts with exactly one header structure
 * followed by exactly pictdb_header.max_files metadata
 * structures. The actual content is not defined by these structures
 * because it should be stored as raw bytes appended at the end of the
 * database file and addressed by offsets in the metadata structure.
 *
 * @author Mia Primorac
 * @date 2 Nov 2015
 */

#ifndef PICTDBPRJ_PICTDB_H
#define PICTDBPRJ_PICTDB_H

#include "error.h" /* not needed here, but provides it as required by
                    * all functions of this lib.
                    */
#include <stdio.h> // for FILE
#include <stdint.h> // for uint32_t, uint64_t
#include <openssl/sha.h> // for SHA256_DIGEST_LENGTH

#define CAT_TXT "EPFL PictDB binary"

/* constraints */
#define MAX_DB_NAME 31  // max. size of a PictDB name
#define MAX_PIC_ID 127  // max. size of a picture id
#define MAX_MAX_FILES 10  // will be increased later in the project

/* For is_valid in pictdb_metadata */
#define EMPTY 0
#define NON_EMPTY 1

// pictDB library internal codes for different picture resolutions.
#define RES_THUMB 0
#define RES_SMALL 1
#define RES_ORIG  2
#define NB_RES    3

#ifdef __cplusplus
extern "C" {
#endif


/* **********************************************************************
 * TODO WEEK 04: DEFINE YOUR STRUCTS HERE.
 * **********************************************************************
 */

/**
 * @brief Prints database header informations.
 *
 * @param header The header to be displayed.
 */
/* **********************************************************************
 * TODO WEEK 04: ADD THE PROTOTYPE OF print_header HERE.
 * **********************************************************************
 */

/**
 * @brief Prints picture metadata informations.
 *
 * @param metadata The metadata of one picture.
 */
void print_metadata (const struct pict_metadata metadata);

/**
 * @brief Displays (on stdout) pictDB metadata.
 *
 * @param db_file In memory structure with header and metadata.
 */
/* **********************************************************************
 * TODO WEEK 04: ADD THE PROTOTYPE OF do_list HERE.
 * **********************************************************************
 */

/**
 * @brief Creates the database called db_filename. Writes the header and the
 *        preallocated empty metadata array to database file.
 *
 * @param db_file In memory structure with header and metadata.
 */
/* **********************************************************************
 * TODO WEEK 05: ADD THE PROTOTYPE OF do_create HERE.
 * **********************************************************************
 */

/* **********************************************************************
 * TODO WEEK 06: ADD THE PROTOTYPE OF do_delete HERE.
 * **********************************************************************
 */

/* **********************************************************************
 * TODO WEEK 09: ADD THE PROTOTYPE OF resolution_atoi HERE.
 * **********************************************************************
 */

/* **********************************************************************
 * TODO WEEK 09: ADD THE PROTOTYPE OF do_read HERE.
 * **********************************************************************
 */

/* **********************************************************************
 * TODO WEEK 09: ADD THE PROTOTYPE OF do_insert HERE.
 * **********************************************************************
 */

#ifdef __cplusplus
}
#endif
#endif
