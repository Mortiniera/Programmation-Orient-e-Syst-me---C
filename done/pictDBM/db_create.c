/**
 * @file db_create.c
 * @brief pictDB library: do_create implementation.
 *
 * @author Mia Primorac
 * @date 2 Nov 2015
 */

#include "pictDB.h"

#include <string.h> // for strncpy

/********************************************************************//**
 * Creates the database called db_filename. Writes the header and the
 * preallocated empty metadata array to database file.
 */
/* **********************************************************************
 * TODO: ADD THE PROTOTYPE OF do_create HERE.
 * **********************************************************************
 */
int do_create(const char* nom_fichier, struct pictdb_file* db_file)
{
    if (nom_fichier == NULL || strlen(nom_fichier) <= 0 || strlen(nom_fichier) > FILENAME_MAX + 1) return ERR_INVALID_FILENAME;
    if (db_file == NULL) return ERR_INVALID_ARGUMENT;

    // Sets the DB header name
    strncpy(db_file->header.db_name, CAT_TXT,  MAX_DB_NAME);
    db_file->header.db_name[MAX_DB_NAME] = '\0';

    /* **********************************************************************
     * TODO: WRITE YOUR CODE HERE
     * **********************************************************************
     */
    size_t i;
    size_t k;
    size_t l;

    db_file->header.db_version = 0;
    db_file->header.num_files = 0;

    if (db_file->header.max_files > MAX_MAX_FILES) return ERR_MAX_FILES;
    db_file->metadata = calloc(db_file->header.max_files, sizeof(struct pict_metadata));
    if (db_file->metadata == NULL) return ERR_OUT_OF_MEMORY;

    for (i = 0; i < db_file->header.max_files; i++) db_file->metadata[i].is_valid = EMPTY;
    
	db_file->fpdb = fopen(nom_fichier, "wb+");
    if (db_file->fpdb == NULL) {
        free(db_file->metadata);
        db_file->metadata = NULL;
        return ERR_FILE_NOT_FOUND;
    }

    k = fwrite(&(db_file->header), sizeof(struct pictdb_header), 1, db_file->fpdb);
    if (k != 1) {
        if (fclose(db_file->fpdb) != 0) fprintf(stderr, "Erreur lors de la fermeture du fichier.\n");
        free(db_file->metadata);
        db_file->metadata = NULL;
        return ERR_IO;
    }
    
    l = fwrite(db_file->metadata, sizeof(struct pict_metadata), db_file->header.max_files, db_file->fpdb);
    if (l != db_file->header.max_files) {
        if (fclose(db_file->fpdb) != 0) fprintf(stderr, "Erreur lors de la fermeture du fichier.\n");
        free(db_file->metadata);
        db_file->metadata = NULL;
        return ERR_IO;
    }

    printf("%zu item(s) written\n", k + l);

    return 0;
}
