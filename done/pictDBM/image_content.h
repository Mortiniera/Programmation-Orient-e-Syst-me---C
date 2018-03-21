/**
 * @file dedup.h
 * @brief prototypes of two functions, one is used to resize an image,
 * 		  and the other one is used to get its resolutions
 */

#ifndef PICTDBPRJ_IMAGE_CONTENT_H
#define PICTDBPRJ_IMAGE_CONTENT_H

#include "pictDB.h"

/**
 * @brief Creates a new version of the specified picture with the corresponding resolution code,
 * 		  then copy the content of this new picture at the end of the file,
 * 		  and update the content of the metadata in the file.
 * 		  We do nothing if the picture already exists with the specified resolution.
 *
 * @param res_code The corresponding code of one of the resolutions.
 * @param db_file The file we are working on.
 * @param index The position of the metadata we want to work on.
 */
int lazily_resize(int res_code, struct pictdb_file* db_file, size_t index);

/**
 * @brief Gets the wight and the height of an image to complete its
 * 		  metatada entry in the database.
 *
 * @param height The variable that will contain the height value.
 * @param width The variable that will contain the width value.
 * @param image_buffer The buffer containing the image under binary representation.
 * @param image_size The size of the image.
 */
int get_resolution(uint32_t* height, uint32_t* width, const char* image_buffer, size_t image_size);

#endif
