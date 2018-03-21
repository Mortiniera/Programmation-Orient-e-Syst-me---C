#include "pictDB.h"
#include "image_content.h"

#include <stdint.h>
#include <stdio.h>
#include <vips/vips.h>
#include <stdlib.h>
#include <openssl/sha.h>
#include <string.h>

int lazily_resize(int res_code, struct pictdb_file* db_file, size_t index)
{
    if (db_file == NULL) return ERR_INVALID_ARGUMENT;
    if (res_code != RES_THUMB && res_code != RES_SMALL && res_code != RES_ORIG) return ERR_INVALID_ARGUMENT;
    if (index >= db_file->header.max_files || db_file->metadata[index].is_valid == EMPTY) return ERR_INVALID_ARGUMENT;

    VipsObject* process = VIPS_OBJECT(vips_image_new());
    VipsImage** t1 = (VipsImage**) vips_object_local_array(process, 1);
    VipsImage** t2 = (VipsImage**) vips_object_local_array(process, 1);
    void* buf_load = calloc(db_file->metadata[index].size[RES_ORIG], sizeof(char));
    if (buf_load == NULL) {
        g_object_unref(process);
        process = NULL;
        return ERR_OUT_OF_MEMORY;
    }
    void* buf_save = NULL;

    if (res_code == RES_THUMB || res_code == RES_SMALL) {
        if (db_file->metadata[index].size[res_code] == 0) {

            double ratioX = (double) (db_file->header).res_resized[res_code*2] / (double) (db_file->metadata)[index].res_orig[0];
            double ratioY = (double) (db_file->header).res_resized[res_code*2 + 1] / (double) (db_file->metadata)[index].res_orig[1];
            double ratio = ratioX > ratioY ? ratioX : ratioY;
            size_t newSize = 0;
            long int savePos_temp;
            uint64_t savePos;

            if (fseek(db_file->fpdb, (db_file->metadata)[index].offset[RES_ORIG], SEEK_SET) != 0) {
                free(buf_load);
                buf_load = NULL;
                g_object_unref(process);
                process = NULL;
                return ERR_IO;
            }
            if (fread(buf_load, (db_file->metadata)[index].size[RES_ORIG], 1, db_file->fpdb) != 1) {
                free(buf_load);
                buf_load = NULL;
                g_object_unref(process);
                process = NULL;
                return ERR_IO;
            }
            if (vips_jpegload_buffer(buf_load, (db_file->metadata)[index].size[RES_ORIG], t1, NULL) == -1) {
                free(buf_load);
                buf_load = NULL;
                g_object_unref(process);
                process = NULL;
                return ERR_VIPS;
            }
#if VIPS_MAJOR_VERSION > 7 || (VIPS_MAJOR_VERSION == 7 && MINOR_VERSION > 40)
            if (vips_resize(*t1, t2, ratio, NULL) == -1) {
                free(buf_load);
                buf_load = NULL;
                g_object_unref(process);
                process = NULL;
                return ERR_VIPS;
            }
#else
            if (ratio < 1.0) {
                ratio = (int) (1.0/ratio) + 1.0;
                if (vips_shrink(*t1, t2, ratio, ratio, NULL) == -1) {
                    free(buf_load);
                    buf_load = NULL;
                    g_object_unref(process);
                    process = NULL;
                    return ERR_VIPS;
                }
            }
#endif
            if (vips_jpegsave_buffer(*t2, &buf_save, &newSize, NULL) == -1) {
                free(buf_load);
                buf_load = NULL;
                g_object_unref(process);
                process = NULL;
                return ERR_VIPS;
            }
            if (buf_save == NULL) {
                free(buf_load);
                buf_load = NULL;
                g_object_unref(process);
                process = NULL;
                return ERR_OUT_OF_MEMORY;
            }
            if (fseek(db_file->fpdb, 0, SEEK_END) != 0) {
                g_free(buf_save);
                buf_save = NULL;
                free(buf_load);
                buf_load = NULL;
                g_object_unref(process);
                process = NULL;
                return ERR_IO;
            }
            savePos_temp = ftell(db_file->fpdb);
            if (savePos_temp == -1L) {
                g_free(buf_save);
                buf_save = NULL;
                free(buf_load);
                buf_load = NULL;
                g_object_unref(process);
                process = NULL;
                return ERR_IO;
            }
            savePos = (uint64_t) savePos_temp;
            if (fwrite(buf_save, newSize, 1, db_file->fpdb) != 1) {
                g_free(buf_save);
                buf_save = NULL;
                free(buf_load);
                buf_load = NULL;
                g_object_unref(process);
                process = NULL;
                return ERR_IO;
            }

            g_free(buf_save);
            buf_save = NULL;
            free(buf_load);
            buf_load = NULL;
            g_object_unref(process);
            process = NULL;

            db_file->metadata[index].size[res_code] = newSize;
            db_file->metadata[index].offset[res_code] = savePos;

            if (fseek(db_file->fpdb, sizeof(struct pictdb_header) + index*sizeof(struct pict_metadata), SEEK_SET) != 0) return ERR_IO;
            if (fwrite(&(db_file->metadata[index]), sizeof(struct pict_metadata), 1, db_file->fpdb) != 1) return ERR_IO;
            
            size_t i;
            for (i = 0; i < db_file->header.max_files; i++) {
				if (db_file->metadata[i].is_valid == NON_EMPTY && i != index && memcmp(db_file->metadata[i].SHA, db_file->metadata[index].SHA, SHA256_DIGEST_LENGTH) == 0) {
					db_file->metadata[i].size[res_code] = newSize;
					db_file->metadata[i].offset[res_code] = savePos;
				}
				if (fseek(db_file->fpdb, sizeof(struct pictdb_header) + i*sizeof(struct pict_metadata), SEEK_SET) != 0) return ERR_IO;
				if (fwrite(&(db_file->metadata[i]), sizeof(struct pict_metadata), 1, db_file->fpdb) != 1) return ERR_IO;
			}
			
        }
    }

    return 0;
}

int get_resolution(uint32_t* height, uint32_t* width, const char* image_buffer, size_t image_size)
{
    VipsObject* process = VIPS_OBJECT(vips_image_new());
    VipsImage** t = (VipsImage**) vips_object_local_array(process, 1);
    if (vips_jpegload_buffer((char*) image_buffer, image_size, t, NULL) == -1) {
        g_object_unref(process);
        process = NULL;
        return ERR_VIPS;
    }
    *height = (uint32_t) (*t)->Ysize;
    *width = (uint32_t) (*t)->Xsize;
    g_object_unref(process);
    process = NULL;
    return 0;
}
