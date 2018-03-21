#include "pictDB.h"
#include "dedup.h"

#include <stdint.h>
#include <stdio.h>
#include <openssl/sha.h>
#include <string.h>
#include <stdlib.h>

static void* sha_convert(const unsigned char* hash, char* result)
{
    if (hash != NULL) {
        size_t i;
        for (i = 0; i < SHA256_DIGEST_LENGTH; i++) sprintf(&result[i*2], "%02x", hash[i]);
        result[2*SHA256_DIGEST_LENGTH] = '\0';
        return result;
    } else return NULL;
}

int do_name_and_content_dedup(struct pictdb_file* db_file, uint32_t index)
{
    if (db_file == NULL || (index >= db_file->header.max_files) || db_file->metadata[index].is_valid == EMPTY) return ERR_INVALID_ARGUMENT;

    char print_format_i[2*SHA256_DIGEST_LENGTH+1];
    char print_format_index[2*SHA256_DIGEST_LENGTH+1];
    size_t i;

    for (i = 0; i < db_file->header.max_files; i++) {
        if (db_file->metadata[i].is_valid == NON_EMPTY && i != index) {

            if (strcmp(db_file->metadata[i].pict_id, db_file->metadata[index].pict_id) == 0) return ERR_DUPLICATE_ID;
            if (db_file->metadata[i].SHA == NULL || db_file->metadata[index].SHA == NULL) return ERR_IO;

            if (strcmp(sha_convert(db_file->metadata[i].SHA, print_format_i), sha_convert(db_file->metadata[index].SHA, print_format_index)) == 0) {
                if (db_file->metadata[index].size[RES_ORIG] != db_file->metadata[i].size[RES_ORIG]) return ERR_IO;

                db_file->metadata[index].offset[RES_THUMB] = db_file->metadata[i].offset[RES_THUMB];
                db_file->metadata[index].offset[RES_SMALL] = db_file->metadata[i].offset[RES_SMALL];
                db_file->metadata[index].offset[RES_ORIG] = db_file->metadata[i].offset[RES_ORIG];
                db_file->metadata[index].size[RES_THUMB] = db_file->metadata[i].size[RES_THUMB];
                db_file->metadata[index].size[RES_SMALL] = db_file->metadata[i].size[RES_SMALL];

                return 0;
            }

        }
    }

    db_file->metadata[index].offset[RES_ORIG] = 0;

    return 0;
}
