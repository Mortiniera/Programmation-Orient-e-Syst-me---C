/* ** NOTE: undocumented in Doxygen
 * @file db_utils.c
 * @brief implementation of several tool functions for pictDB
 *
 * @author Mia Primorac
 * @date 2 Nov 2015
 */

#include "pictDB.h"

#include <stdint.h> // for uint8_t
#include <stdio.h> // for sprintf
#include <openssl/sha.h> // for SHA256_DIGEST_LENGTH
#include <inttypes.h>
#include <string.h>

/********************************************************************//**
 * Human-readable SHA
 */
static void
sha_to_string (const unsigned char* SHA,
               char* sha_string)
{
    if (SHA == NULL) {
        return;
    }
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        sprintf(&sha_string[i*2], "%02x", SHA[i]);
    }

    sha_string[2*SHA256_DIGEST_LENGTH] = '\0';
}

/********************************************************************//**
 * pictDB header display.
 */
/* **********************************************************************
 * TODO: WRITE YOUR print_header CODE HERE
 * **********************************************************************
 */
void print_header(const struct pictdb_header* header)
{
    printf("*****************************************\n");
    printf("**********DATABASE HEADER START**********\n");
    printf("DB NAME: %31s\n", header->db_name);
    printf("VERSION: %" PRIu32 "\n", header->db_version);
    printf("IMAGE COUNT: %" PRIu32 "\t\tMAX IMAGES: %" PRIu32 "\n", header->num_files, header->max_files);
    printf("THUMBNAIL: %" PRIu16 " x %" PRIu16 "\tSMALL: %" PRIu16 " x %" PRIu16 "\n", header->res_resized[0], header->res_resized[1], header->res_resized[2], header->res_resized[3]);
    printf("***********DATABASE HEADER END***********\n");
    printf("*****************************************\n");
}
/********************************************************************//**
 * Metadata display.
 */
void
print_metadata (const struct pict_metadata* metadata)
{
    char sha_printable[2*SHA256_DIGEST_LENGTH+1];
    sha_to_string(metadata->SHA, sha_printable);

    /* **********************************************************************
     * TODO: WRITE YOUR CODE HERE
     * **********************************************************************
     */
    printf("PICTURE ID: %s\n", metadata->pict_id);
    printf("SHA: %s\n", sha_printable);
    printf("VALID: %" PRIu16 "\n", metadata->is_valid);
    printf("UNUSED: %" PRIu16 "\n", metadata->unused_16);
    printf("OFFSET ORIG. : %" PRIu64 "\t\tSIZE ORIG. : %" PRIu32 "\n", metadata->offset[RES_ORIG], metadata->size[RES_ORIG]);
    printf("OFFSET THUMB.: %" PRIu64 "\t\tSIZE THUMB.: %" PRIu32 "\n", metadata->offset[RES_THUMB], metadata->size[RES_THUMB]);
    printf("OFFSET SMALL : %" PRIu64 "\t\tSIZE SMALL : %" PRIu32 "\n", metadata->offset[RES_SMALL], metadata->size[RES_SMALL]);
    printf("ORIGINAL: %" PRIu16 " x %" PRIu16 "\n", metadata->res_orig[0], metadata->res_orig[1]);
    printf("*****************************************\n");
}

int do_open(const char* nom_fichier, const char* open_mode, struct pictdb_file* db_file)
{
    if (strlen(nom_fichier) <= 0 || strlen(nom_fichier) > FILENAME_MAX + 1) return ERR_INVALID_FILENAME;
    if (db_file == NULL) return ERR_INVALID_ARGUMENT;

    db_file->fpdb = fopen(nom_fichier, open_mode);
    if (db_file->fpdb == NULL) return ERR_FILE_NOT_FOUND;

    if (fread(&(db_file->header), sizeof(struct pictdb_header), 1, (db_file->fpdb)) != 1) {
        if (fclose(db_file->fpdb) != 0) fprintf(stderr, "Erreur lors de la fermeture du fichier.\n");
        return ERR_IO;
    }

    if (db_file->header.max_files > MAX_MAX_FILES) {
        if (fclose(db_file->fpdb) != 0) fprintf(stderr, "Erreur lors de la fermeture du fichier.\n");
        return ERR_MAX_FILES;
    }
    db_file->metadata = calloc(db_file->header.max_files, sizeof(struct pict_metadata));
    if (db_file->metadata == NULL) {
        if (fclose(db_file->fpdb) != 0) fprintf(stderr, "Erreur lors de la fermeture du fichier.\n");
        return ERR_OUT_OF_MEMORY;
    }

    if (fread(db_file->metadata, sizeof(struct pict_metadata), db_file->header.max_files, (db_file->fpdb)) != db_file->header.max_files) {
        free(db_file->metadata);
        db_file->metadata = NULL;
        if (fclose(db_file->fpdb) != 0) fprintf(stderr, "Erreur lors de la fermeture du fichier.\n");
        return ERR_IO;
    }

    return 0;
}

void do_close(struct pictdb_file* db_file)
{
    if(db_file != NULL) {
        if (db_file->fpdb != NULL) {
            free(db_file->metadata);
            db_file->metadata = NULL;
            if (fclose(db_file->fpdb) != 0) fprintf(stderr, "Erreur lors de la fermeture du fichier.\n");
        }
    }
}

int resolution_atoi(const char* img_res)
{
    if (img_res != NULL) {
        if (strcmp(img_res, "thumbnail") == 0 || strcmp(img_res, "thumb") == 0) return RES_THUMB;
        else if (strcmp(img_res, "small") == 0) return RES_SMALL;
        else if (strcmp(img_res, "original") == 0 || strcmp(img_res, "orig") == 0) return RES_ORIG;
        else return -1;
    }
    return -1;
}
