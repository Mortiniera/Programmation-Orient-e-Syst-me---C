#include "pictDB.h"

#include <stdio.h>
#include <string.h>

int do_delete(const char* pictID, struct pictdb_file* db_file)
{
    if (strlen(pictID) <= 0 || strlen(pictID) > MAX_PIC_ID + 1) return ERR_INVALID_PICID;
    if (db_file == NULL) return ERR_INVALID_ARGUMENT;
    if (db_file->header.num_files == 0) return ERR_IO;

    size_t i;
    int find = 0;

    for (i = 0; i < db_file->header.max_files; i++) {
        if (strcmp(db_file->metadata[i].pict_id, pictID) == 0 && db_file->metadata[i].is_valid == NON_EMPTY) {
            db_file->metadata[i].is_valid = EMPTY;
            if (fseek(db_file->fpdb, sizeof(struct pictdb_header) + i*sizeof(struct pict_metadata), SEEK_SET) != 0) return ERR_IO;
            if (fwrite(&(db_file->metadata[i]), sizeof(struct pict_metadata), 1, db_file->fpdb) != 1) return ERR_IO;
            find = 1;
        }
    }
    if (find == 0) return ERR_FILE_NOT_FOUND;

    db_file->header.db_version += 1;
    db_file->header.num_files -= 1;
    if (fseek(db_file->fpdb, 0, SEEK_SET) != 0) return ERR_IO;
    if (fwrite(&(db_file->header), sizeof(struct pictdb_header), 1, db_file->fpdb) != 1) return ERR_IO;

    return 0;
}
