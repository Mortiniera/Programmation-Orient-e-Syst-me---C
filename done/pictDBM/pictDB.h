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
#include <stdlib.h>

#define CAT_TXT "EPFL PictDB binary"

/* constraints */
#define MAX_DB_NAME 31  // max. size of a PictDB name
#define MAX_PIC_ID 127  // max. size of a picture id
#define MAX_MAX_FILES 100000  // will be increased later in the project

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

#define DEFAULT_MAX_FILES 10
#define DEFAULT_THUMB_RES 64
#define DEFAULT_SMALL_RES 256
#define MAX_THUMB_RES 128
#define MAX_SMALL_RES 512

enum do_list_mode{STDOUT, JSON};


/* **********************************************************************
 * TODO WEEK 04: DEFINE YOUR STRUCTS HERE.
 * **********************************************************************
 */
/**
 * @brief datastructure to store the configuration elements of the
 * 		  system.
 */
struct pictdb_header {

    char db_name[MAX_DB_NAME + 1];
    uint32_t db_version;
    uint32_t num_files;
    uint32_t max_files;
    uint16_t res_resized[2*(NB_RES - 1)];
    uint32_t unused_32;
    uint64_t unused_64;

};

/**
 * @brief datastructure that represents the metadatas of a picture,
 * 		  where each element of the structure is part of the metadatas
 * 		  of this picture.
 */
struct pict_metadata {

    char pict_id[MAX_PIC_ID + 1];
    unsigned char SHA[SHA256_DIGEST_LENGTH];
    uint32_t res_orig[2];
    uint32_t size[NB_RES];
    uint64_t offset[NB_RES];
    uint16_t is_valid;
    uint16_t unused_16;

};

/**
 * @brief datastructure that contains all the metadatas of all the
 * 		  pictures, and the header of the database, into a file.
 */
struct pictdb_file {

    FILE* fpdb;
    struct pictdb_header header;
    struct pict_metadata* metadata;

};




/**
 * @brief Prints database header informations.
 *
 * @param header The header to be displayed.
 */

/* **********************************************************************
* TODO WEEK 04: ADD THE PROTOTYPE OF print_header HERE.
* **********************************************************************
*/
void print_header(const struct pictdb_header* header);
/**
 * @brief Prints picture metadata informations.
 *
 * @param metadata The metadata of one picture.
 */
void print_metadata(const struct pict_metadata* metadata);

/**
 * @brief Displays (on stdout) pictDB metadata.
 *
 * @param db_file In memory structure with header and metadata.
 */
/* **********************************************************************
 * TODO WEEK 04: ADD THE PROTOTYPE OF do_list HERE.
 * **********************************************************************
 */
char* do_list(const struct pictdb_file* db_file, enum do_list_mode enum_mode);
/**
 * @brief Creates the database, and writes the header and the
 *        preallocated empty metadata array to database file.
 *
 * @param db_file In memory structure with header and metadata.
 */
/* **********************************************************************
 * TODO WEEK 05: ADD THE PROTOTYPE OF do_create HERE.
 * **********************************************************************
 */
int do_create(const char* nom_fichier, struct pictdb_file* db_file);
/* **********************************************************************
 * TODO WEEK 06: ADD THE PROTOTYPE OF do_delete HERE.
 * **********************************************************************
 */
/**
 * @brief Opens the file of the given datastructure and reads the
 * 		  content of the header and the metadatas.
 *
 * @param nom_fichier The name of the file we work on.
 * @param open_mode The open mode for the file.
 * @param db_file The datastructure.
 */
int do_open(const char* nom_fichier, const char* open_mode, struct pictdb_file* db_file);

/**
 * @brief Closes the file of the given datastructure.
 *
 * @param db_file The datastructure.
 */
void do_close(struct pictdb_file* db_file);

/**
 * @brief Deletes the picture specified by its name in the database.
 *
 * @param pictID The reference of the picture we want to delete.
 * @param db_file The datastructure containing the picture.
 */
int do_delete(const char* pictID, struct pictdb_file* db_file);
/* **********************************************************************
 * TODO WEEK 09: ADD THE PROTOTYPE OF resolution_atoi HERE.
 * **********************************************************************
 */
/**
 * @brief Returns the corresponding resolution code depending of what
 * 		  entered the user.
 *
 * @param img_res The sequence of characters entered by the user.
 */
int resolution_atoi(const char* img_res);
/* **********************************************************************
 * TODO WEEK 09: ADD THE PROTOTYPE OF do_read HERE.
 * **********************************************************************
 */
/**
 * @brief Reads an image in the file and displays it in the directory
 * 		  of the project.
 *
 * @param pictID The name of the image that we want to read in the database.
 * @param res_code The resolution code.
 * @param image_buffer The buffer that will contain the image.
 * @param img_size The variable that will contain the size of the image.
 * @param db_file The datastructure containing the picture.
 */
int do_read(char* pictID, int res_code, char** image_buffer, uint32_t* img_size, struct pictdb_file* db_file);
/* **********************************************************************
 * TODO WEEK 09: ADD THE PROTOTYPE OF do_insert HERE.
 * **********************************************************************
 */
/**
 * @brief Inserts an image in the the file from the directory of the
 * 		  project.
 *
 * @param img The binary representation of the image.
 * @param img_size The size of the image.
 * @param pictID The name that we give to the inserted image.
 * @param db_file The datastructure that will contain the image.
 */
int do_insert(const char* img, size_t img_size, char* pictID, struct pictdb_file* db_file);
#ifdef __cplusplus
}
#endif
#endif
