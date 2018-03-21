#include "pictDB.h"
#include "image_content.h"
#include "dedup.h"
#include "libmongoose/mongoose.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <vips/vips.h>

static int new_image(struct pictdb_file* db_file, struct pictdb_file* new_db_file, size_t i) {
	size_t j;
	for (j = 0; j < 2; j++) {
		if (db_file->metadata[i].size[j] != 0) {
			int res;
			if (j == 0) res = RES_THUMB;
			else res = RES_SMALL;
			size_t k = 0;
			while (strcmp(db_file->metadata[i].pict_id, new_db_file->metadata[k].pict_id) != 0) k++;	
			int check = lazily_resize(res, new_db_file, k);
			if (check != 0) return check;
		}
	}
	return 0;
}

static int insert_pict(struct pictdb_file* db_file, struct pictdb_file* new_db_file, size_t i) {	
	if (db_file->metadata[i].size[RES_ORIG] != 0 && db_file->metadata[i].is_valid == NON_EMPTY) {
		char* tmp_img = NULL;
		uint32_t size = 0;

		int first_check = do_read(db_file->metadata[i].pict_id, RES_ORIG, &tmp_img, &size, db_file);
		if (first_check != 0) return first_check;

		int second_check = do_insert(tmp_img, size, db_file->metadata[i].pict_id, new_db_file);
		if (second_check != 0) {
			free(tmp_img);
			tmp_img = NULL;
			return second_check;
		}

		free(tmp_img);
		tmp_img = NULL;
		
		int third_check = new_image(db_file, new_db_file, i);
		if (third_check != 0) return third_check;
	}
	
	return 0;
}

int do_gbcollect(struct pictdb_file* db_file, char* orig_file_name, char* temp_file_name) {
	if (db_file == NULL) {
		do_close(db_file);
		return ERR_INVALID_ARGUMENT;
	}
	if (orig_file_name == NULL) {
		do_close(db_file);
		return ERR_INVALID_ARGUMENT;
	}
	if (temp_file_name == NULL) {
		do_close(db_file);
		return ERR_INVALID_ARGUMENT;
	}
	if (strcmp(orig_file_name, temp_file_name) == 0) {
		do_close(db_file);
		return ERR_INVALID_ARGUMENT;
	}
	
	struct pictdb_file new_db_file;
	
	new_db_file.header.max_files = db_file->header.max_files;
    new_db_file.header.res_resized[0] = db_file->header.res_resized[0];
    new_db_file.header.res_resized[1] = db_file->header.res_resized[1];
    new_db_file.header.res_resized[2] = db_file->header.res_resized[2];
    new_db_file.header.res_resized[3] = db_file->header.res_resized[3];
	
	int first_check = do_create(temp_file_name, &new_db_file);
	if (first_check != 0) {
		if ((&new_db_file)->metadata != NULL) {
			free((&new_db_file)->metadata);
			(&new_db_file)->metadata = NULL;
		}
		if (fclose(new_db_file.fpdb) != 0) fprintf(stderr, "Erreur lors de la fermeture du fichier.\n");
		do_close(db_file);
		return first_check;
	}
	
	int second_check;
	size_t i;
	for(i = 0; i < db_file->header.max_files; i++) {
		if ((second_check = insert_pict(db_file, &new_db_file, i)) != 0) {
			free((&new_db_file)->metadata);
			(&new_db_file)->metadata = NULL;
			if (fclose(new_db_file.fpdb) != 0) fprintf(stderr, "Erreur lors de la fermeture du fichier.\n");
			do_close(db_file);
			return second_check;
		}
	}
	
	free((&new_db_file)->metadata);
	(&new_db_file)->metadata = NULL;
	if (fclose(new_db_file.fpdb) != 0) fprintf(stderr, "Erreur lors de la fermeture du fichier.\n");
	do_close(db_file);
	if (remove(orig_file_name) != 0) return ERR_IO;
	if (rename(temp_file_name, orig_file_name) != 0) return ERR_IO;	
	
	return 0;
}
