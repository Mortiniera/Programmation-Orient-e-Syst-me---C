#include "pictDB.h"
#include "dedup.h"
#include "image_content.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int do_read(char* pictID, int res_code, char** image_buffer, uint32_t* img_size, struct pictdb_file* db_file)
{
    if(db_file == NULL) return ERR_INVALID_ARGUMENT;
    if (res_code != 0 && res_code != 1 && res_code != 2) return ERR_INVALID_ARGUMENT;
    if (strlen(pictID) <= 0 || strlen(pictID) > MAX_PIC_ID + 1) return ERR_INVALID_PICID;

    int check;
    size_t i;
    for (i = 0; i < db_file->header.max_files; i++) {
        if (db_file->metadata[i].is_valid == NON_EMPTY && strcmp(db_file->metadata[i].pict_id, pictID) == 0) {
            if (db_file->metadata[i].size[res_code] == 0) check = lazily_resize(res_code, db_file, i);
            else {
                *image_buffer = calloc(db_file->metadata[i].size[res_code], sizeof(char));
                if (*image_buffer == NULL) return ERR_OUT_OF_MEMORY;
                if (fseek(db_file->fpdb, db_file->metadata[i].offset[res_code], SEEK_SET) != 0) {
                    free(*image_buffer);
                    *image_buffer = NULL;
                    return ERR_IO;
                }
                if (fread(*image_buffer, db_file->metadata[i].size[res_code], 1, db_file->fpdb) != 1) {
                    free(*image_buffer);
                    *image_buffer = NULL;
                    return ERR_IO;
                }
                *img_size = db_file->metadata[i].size[res_code];
                return 0;
            }

            if (check == 0) {
                *image_buffer = calloc(db_file->metadata[i].size[res_code], sizeof(char));
                if (*image_buffer == NULL) return ERR_OUT_OF_MEMORY;
                if (fseek(db_file->fpdb, db_file->metadata[i].offset[res_code], SEEK_SET) != 0) {
                    free(*image_buffer);
                    *image_buffer = NULL;
                    return ERR_IO;
                }
                if (fread(*image_buffer, db_file->metadata[i].size[res_code], 1, db_file->fpdb) != 1) {
                    free(*image_buffer);
                    *image_buffer = NULL;
                    return ERR_IO;
                }
                *img_size = db_file->metadata[i].size[res_code];
            }
            return check;
        }
    }
    return ERR_FILE_NOT_FOUND;
}
