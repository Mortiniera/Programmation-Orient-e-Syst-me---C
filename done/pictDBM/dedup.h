/**
 * @file dedup.h
 * @brief prototype of a function used to avoid duplication of images
 */

#ifndef PICTDBPRJ_DEDUP_H
#define PICTDBPRJ_DEDUP_H

#include "pictDB.h"

/**
 * @brief Check first if the file does not contain two differents images with the same ID.
 * 		  Then if we found the same SHA for another image with a different name, then
 * 		  we can avoid duplication of this picture at the given position, for all the resolutions.
 *
 * @param pict_db The file in which we want to deduplicate the picture.
 * @param index The position of the picture in the metadata array.
 */
int do_name_and_content_dedup(struct pictdb_file* pict_db, uint32_t index);

#endif
