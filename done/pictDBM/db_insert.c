#include "pictDB.h"
#include "dedup.h"
#include "image_content.h"

#include <stdint.h>
#include <stdio.h>
#include <openssl/sha.h>
#include <string.h>
#include <stdlib.h>

int do_insert(const char* img, size_t img_size, char* pictID, struct pictdb_file* db_file)
{
    if (db_file == NULL) return ERR_INVALID_ARGUMENT;
    if (db_file->header.num_files >= db_file->header.max_files) return ERR_FULL_DATABASE;
    if (strlen(pictID) <= 0 || strlen(pictID) > MAX_PIC_ID + 1) return ERR_INVALID_PICID;

    int check;
    long int savePos_temp;
    uint64_t savePos;

    size_t i = 0;
    while (db_file->metadata[i].is_valid == NON_EMPTY) i++;

    db_file->metadata[i].is_valid = NON_EMPTY;
    (void)SHA256((unsigned char*) img, img_size, db_file->metadata[i].SHA);
    if (db_file->metadata[i].SHA == NULL) return ERR_IO;
    strncpy(db_file->metadata[i].pict_id, pictID, strlen(pictID) + 1);
    db_file->metadata[i].size[RES_ORIG] = (uint32_t) img_size;

    check = do_name_and_content_dedup(db_file, (uint32_t) i);
    if (check != 0) return check;

    if (db_file->metadata[i].offset[RES_ORIG] == 0) {
        if (fseek(db_file->fpdb, 0, SEEK_END) != 0) return ERR_IO;
        savePos_temp = ftell(db_file->fpdb);
        if (savePos_temp == -1L) return ERR_IO;
        savePos = (uint64_t) savePos_temp;
        if (fwrite(img, img_size, 1, db_file->fpdb) != 1) return ERR_IO;

        db_file->metadata[i].offset[RES_ORIG] = savePos;
        db_file->metadata[i].size[RES_THUMB] = 0;
        db_file->metadata[i].size[RES_SMALL] = 0;
        db_file->metadata[i].offset[RES_THUMB] = 0;
        db_file->metadata[i].offset[RES_SMALL] = 0;
    }

    uint32_t height = 0;
    uint32_t width = 0;
    int test = get_resolution(&height, &width, img, img_size);
    if (test != 0) return test;

    db_file->metadata[i].res_orig[0] = width;
    db_file->metadata[i].res_orig[1] = height;
    db_file->header.num_files++;
    db_file->header.db_version++;

    if (fseek(db_file->fpdb, 0, SEEK_SET) != 0) return ERR_IO;
    if (fwrite(&(db_file->header), sizeof(struct pictdb_header), 1, db_file->fpdb) != 1) return ERR_IO;
    if (fseek(db_file->fpdb, sizeof(struct pictdb_header) + i*sizeof(struct pict_metadata), SEEK_SET) != 0) return ERR_IO;
    if (fwrite(&(db_file->metadata[i]), sizeof(struct pict_metadata), 1, db_file->fpdb) != 1) return ERR_IO;

    return 0;
}
