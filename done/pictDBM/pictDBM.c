/**
 * @file pictDBM.c
 * @brief pictDB Manager: command line interpretor for pictDB core commands.
 *
 * Picture Database Management Tool
 *
 * @author Mia Primorac
 * @date 2 Nov 2015
 */

#include "pictDB.h"
#include "pictDBM_tools.h"
#include "db_gbcollect.h"

#include <stdlib.h>
#include <string.h>
#include <vips/vips.h>

/**
 * @brief Pointer function type.
 */
typedef int (*command)(int, char*[]);

/**
 * @brief a datastructure to associate the command name to its corresponding function.
 */
struct command_mapping {
    const char* myString;
    command myCommand;
};

/********************************************************************//**
 * Opens pictDB file and calls do_list command.
 ********************************************************************** */
int do_list_cmd(int args, char *argv[])
{
    /* This is a quick and dirty way of reading the file.
     * It's provided here as such to avoid solution leak.
     * You shall NOT proceed as such in your future open function
     * (in week 6).
     */
    /* **********************************************************************
     * TODO WEEK 06: REPLACE THE PROVIDED CODE BY YOUR OWN CODE HERE
     * **********************************************************************
     */
    if (args < 2) return ERR_NOT_ENOUGH_ARGUMENTS;
    if (args > 2) return ERR_INVALID_ARGUMENT;

    if (strlen(argv[1]) <= 0 || strlen(argv[1]) > FILENAME_MAX + 1) return ERR_INVALID_FILENAME;

    struct pictdb_file myfile;

    if (do_open(argv[1], "rb", &myfile) == 0) {
        do_list(&myfile, STDOUT);
        do_close(&myfile);
    }

    return 0;
}


/********************************************************************//**
 * Prepares and calls do_create command.
********************************************************************** */
int do_create_cmd(int args, char *argv[])
{
    if (args < 2) return ERR_NOT_ENOUGH_ARGUMENTS;

    if (strlen(argv[1]) <= 0 || strlen(argv[1]) > FILENAME_MAX + 1) return ERR_INVALID_FILENAME;

    uint32_t max_files_temp = DEFAULT_MAX_FILES;
    uint16_t thumb_res1_temp = DEFAULT_THUMB_RES;
    uint16_t thumb_res2_temp = DEFAULT_THUMB_RES;
    uint16_t small_res1_temp = DEFAULT_SMALL_RES;
    uint16_t small_res2_temp = DEFAULT_SMALL_RES;

    if (args > 2) {
        int i = 2;
        while (i < args) {
            if (strcmp(argv[i], "-max_files") == 0) {
                if (i+1 >= args) return ERR_NOT_ENOUGH_ARGUMENTS;
                max_files_temp = atouint32(argv[i + 1]);
                if (max_files_temp == 0 || max_files_temp > MAX_MAX_FILES) return ERR_MAX_FILES;
                i = i+2;
            } else if (strcmp(argv[i], "-thumb_res") == 0) {
                if (i+2 >= args) return ERR_NOT_ENOUGH_ARGUMENTS;
                thumb_res1_temp = atouint16(argv[i + 1]);
                thumb_res2_temp = atouint16(argv[i + 2]);
                if (thumb_res1_temp == 0 || thumb_res2_temp == 0 || thumb_res1_temp > MAX_THUMB_RES || thumb_res2_temp > MAX_THUMB_RES) return ERR_RESOLUTIONS;
                i = i+3;
            } else if (strcmp(argv[i], "-small_res") == 0) {
                if (i+2 >= args) return ERR_NOT_ENOUGH_ARGUMENTS;
                small_res1_temp = atouint16(argv[i + 1]);
                small_res2_temp = atouint16(argv[i + 2]);
                if (small_res1_temp == 0 || small_res2_temp == 0 || small_res1_temp > MAX_SMALL_RES || small_res2_temp > MAX_SMALL_RES) return ERR_RESOLUTIONS;
                i = i+3;
            } else return ERR_INVALID_ARGUMENT;
        }
        if (thumb_res1_temp >= small_res1_temp || thumb_res2_temp >= small_res2_temp) return ERR_RESOLUTIONS;
    }

    const uint32_t max_files = max_files_temp;
    const uint16_t thumb_res1 = thumb_res1_temp;
    const uint16_t thumb_res2 = thumb_res2_temp;
    const uint16_t small_res1 = small_res1_temp;
    const uint16_t small_res2 = small_res2_temp;

    puts("Create");
    /* **********************************************************************
     * TODO WEEK 05: WRITE YOUR CODE HERE (and change the return if needed).
     * **********************************************************************
     */
    struct pictdb_file db_file;

    db_file.header.max_files = max_files;
    db_file.header.res_resized[0] = thumb_res1;
    db_file.header.res_resized[1] = thumb_res2;
    db_file.header.res_resized[2] = small_res1;
    db_file.header.res_resized[3] = small_res2;
    
    int check = do_create(argv[1], &db_file);
    if ((&db_file)->metadata != NULL) {
		free((&db_file)->metadata);
		(&db_file)->metadata = NULL;
	}
	if (check == 0) print_header(&(db_file.header));
    if (fclose(db_file.fpdb) != 0) fprintf(stderr, "Erreur lors de la fermeture du fichier.\n");
	
    return check;
}


/********************************************************************//**
 * Displays some explanations.
 ********************************************************************** */
int help(int args, char *argv[])
{
    /* **********************************************************************
     * TODO WEEK 05: WRITE YOUR CODE HERE (and change the return if needed).
     * **********************************************************************
     */
    if (args >= 1 && argv[0] != NULL) {
        printf("pictDBM [COMMAND] [ARGUMENTS]\n");
        printf("  help: displays this help.\n");
        printf("  list <dbfilename>: list pictDB content.\n");
        printf("  create <dbfilename> [options]: create a new pictDB.\n");
        printf("      options are:\n");
        printf("          -max_files <MAX_FILES>: maximum number of files.\n");
        printf("                                  default value is 10\n");
        printf("                                  maximum value is 100000\n");
        printf("          -thumb_res <X_RES> <Y_RES>: resolution for thumbnail images.\n");
        printf("                                  default value is 64x64\n");
        printf("                                  maximum value is 128x128\n");
        printf("          -small_res <X_RES> <Y_RES>: resolution for small images.\n");
        printf("                                  default value is 256x256\n");
        printf("                                  maximum value is 512x512\n");
        printf("  read <dbfilename> <pictID> [original|orig|thumbnail|thumb|small]:\n");
        printf("      read an image from the pictDB and save it to a file.\n");
        printf("      default resolution is \"original\".\n");
        printf("  insert <dbfilename> <pictID> <filename>: insert a new image in the pictDB.\n");
        printf("  delete <dbfilename> <pictID>: delete picture pictID from pictDB.\n");
        printf("  gc <dbfilename> <tmp dbfilename>: performs garbage collecting on pictDB. Requires a temporary filename for copying the pictDB.\n");
    }

    return 0;
}


/********************************************************************//**
 * Deletes a picture from the database.
 ********************************************************************** */
int do_delete_cmd(int args, char *argv[])
{
    /* **********************************************************************
     * TODO WEEK 06: WRITE YOUR CODE HERE (and change the return if needed).
     * **********************************************************************
     */
    if (args < 3) return ERR_NOT_ENOUGH_ARGUMENTS;
    if (args > 3) return ERR_INVALID_ARGUMENT;

    struct pictdb_file myfile;
    int check;
	
    if (strlen(argv[1]) <= 0 || strlen(argv[1]) > FILENAME_MAX + 1) return ERR_INVALID_FILENAME;
    if (strlen(argv[2]) <= 0 || strlen(argv[2]) > MAX_PIC_ID + 1) return ERR_INVALID_PICID;

    if (do_open(argv[1], "rb+", &myfile) == 0) {
        check = do_delete(argv[2], &myfile);
        do_close(&myfile);
    }

    return check;
}


/********************************************************************//**
 * Reads a picture from the database.
 ********************************************************************** */
int do_read_cmd(int args, char *argv[])
{
    if (args < 3) return ERR_NOT_ENOUGH_ARGUMENTS;
    if (args > 4) return ERR_INVALID_ARGUMENT;

    if (strlen(argv[1]) <= 0 || strlen(argv[1]) > FILENAME_MAX + 1) return ERR_INVALID_FILENAME;
    if (strlen(argv[2]) <= 0 || strlen(argv[2]) > MAX_PIC_ID + 1) return ERR_INVALID_PICID;

    struct pictdb_file myfile;
    char* buffer = NULL;
    uint32_t size = 0;
    int res_code;
    int check;

    char* my_resolution_suffix = NULL;
    char* orig_suffix = "_orig";
    char* small_suffix = "_small";
    char* thumb_suffix = "_thumb";
    char* format = ".jpg";

    if (args == 3) {
        res_code = RES_ORIG;
        my_resolution_suffix = calloc(strlen(orig_suffix) + 1, sizeof(char));
        if (my_resolution_suffix == NULL) return ERR_OUT_OF_MEMORY;
        strncpy(my_resolution_suffix, orig_suffix, strlen(orig_suffix) + 1);
    } else {
        if ((res_code = resolution_atoi(argv[3])) == -1) return ERR_IO;
        if (res_code == RES_ORIG) {
            my_resolution_suffix = calloc(strlen(orig_suffix) + 1, sizeof(char));
            if (my_resolution_suffix == NULL) return ERR_OUT_OF_MEMORY;
            strncpy(my_resolution_suffix, orig_suffix, strlen(orig_suffix) + 1);
        } else if (res_code == RES_SMALL) {
            my_resolution_suffix = calloc(strlen(small_suffix) + 1, sizeof(char));
            if (my_resolution_suffix == NULL) return ERR_OUT_OF_MEMORY;
            strncpy(my_resolution_suffix, small_suffix, strlen(small_suffix) + 1);
        } else {
            my_resolution_suffix = calloc(strlen(thumb_suffix) + 1, sizeof(char));
            if (my_resolution_suffix == NULL) return ERR_OUT_OF_MEMORY;
            strncpy(my_resolution_suffix, thumb_suffix, strlen(thumb_suffix) + 1);
        }
    }

    FILE* myimage = NULL;
    char* myname = calloc(strlen(argv[2]) + strlen(my_resolution_suffix) + strlen(format) + 1, sizeof(char));
    if (myname == NULL) {
        free(my_resolution_suffix);
        my_resolution_suffix = NULL;
        return ERR_OUT_OF_MEMORY;
    }
    myname[0] = 0;
    strncat(myname, argv[2], strlen(argv[2]));
    strncat(myname, my_resolution_suffix, strlen(my_resolution_suffix));
    strncat(myname, format, strlen(format) + 1);
    myimage = fopen(myname, "wb");
    if (myimage == NULL) {
        free(myname);
        myname = NULL;
        free(my_resolution_suffix);
        my_resolution_suffix = NULL;
        return ERR_FILE_NOT_FOUND;
    }

    if (do_open(argv[1], "rb+", &myfile) == 0) {
        check = do_read(argv[2], res_code, &buffer, &size, &myfile);
        do_close(&myfile);
    }

    if (buffer != NULL) {
        if (fwrite(buffer, size, 1, myimage) != 1) {
            free(buffer);
            buffer = NULL;
            if (fclose(myimage) != 0) fprintf(stderr, "Erreur lors de la fermeture du fichier.\n");
            free(myname);
            myname = NULL;
            free(my_resolution_suffix);
            my_resolution_suffix = NULL;
            return ERR_IO;
        }
        free(buffer);
        buffer = NULL;
    }
    if (fclose(myimage) != 0) fprintf(stderr, "Erreur lors de la fermeture du fichier.\n");
    free(myname);
    myname = NULL;
    free(my_resolution_suffix);
    my_resolution_suffix = NULL;

    return check;
}


/********************************************************************//**
 * Inserts a picture to the database.
 ********************************************************************** */
int do_insert_cmd(int args, char *argv[])
{
    if (args < 4) return ERR_NOT_ENOUGH_ARGUMENTS;
    if (args > 4) return ERR_INVALID_ARGUMENT;

    if (strlen(argv[1]) <= 0 || strlen(argv[1]) > FILENAME_MAX + 1) return ERR_INVALID_FILENAME;
    if (strlen(argv[2]) <= 0 || strlen(argv[2]) > MAX_PIC_ID + 1) return ERR_INVALID_PICID;
    if (strlen(argv[3]) <= 0 || strlen(argv[3]) > FILENAME_MAX + 1) return ERR_INVALID_FILENAME;

    struct pictdb_file myfile;
    char* img = NULL;
    long int size_temp;
    size_t size;
    int check;

    FILE* myimage = NULL;
    myimage = fopen(argv[3], "rb");
    if (myimage == NULL) return ERR_FILE_NOT_FOUND;

    if (fseek(myimage, 0, SEEK_END) != 0) {
        if (fclose(myimage) != 0) fprintf(stderr, "Erreur lors de la fermeture du fichier.\n");
        return ERR_IO;
    }
    size_temp = ftell(myimage);
    if (size_temp == -1L) {
        if (fclose(myimage) != 0) fprintf(stderr, "Erreur lors de la fermeture du fichier.\n");
        return ERR_IO;
    }
    size = (size_t) size_temp;
    if (fseek(myimage, 0, SEEK_SET) != 0) {
        if (fclose(myimage) != 0) fprintf(stderr, "Erreur lors de la fermeture du fichier.\n");
        return ERR_IO;
    }

    img = calloc(size, sizeof(char));
    if (img == NULL) {
        if (fclose(myimage) != 0) fprintf(stderr, "Erreur lors de la fermeture du fichier.\n");
        return ERR_OUT_OF_MEMORY;
    }
    if (fread(img, size, 1, myimage) != 1) {
        free(img);
        img = NULL;
        if (fclose(myimage) != 0) fprintf(stderr, "Erreur lors de la fermeture du fichier.\n");
        return ERR_IO;
    }

    if (do_open(argv[1], "rb+", &myfile) == 0) {
        if (myfile.header.num_files >= myfile.header.max_files) {
            do_close(&myfile);
            free(img);
            img = NULL;
            if (fclose(myimage) != 0) fprintf(stderr, "Erreur lors de la fermeture du fichier.\n");
            return ERR_FULL_DATABASE;
        }
        check = do_insert(img, size, argv[2], &myfile);
        do_close(&myfile);
    }

    free(img);
    img = NULL;
    if (fclose(myimage) != 0) fprintf(stderr, "Erreur lors de la fermeture du fichier.\n");

    return check;
}

int do_gc_cmd(int args, char *argv[]) {
	if (args < 3) return ERR_NOT_ENOUGH_ARGUMENTS;
    if (args > 3) return ERR_INVALID_ARGUMENT;
    
    if (strlen(argv[1]) <= 0 || strlen(argv[1]) > FILENAME_MAX + 1) return ERR_INVALID_FILENAME;
    if (strlen(argv[2]) <= 0 || strlen(argv[2]) > FILENAME_MAX + 1) return ERR_INVALID_FILENAME;
    
    struct pictdb_file myfile;
    int check;
    
    if (do_open(argv[1], "rb+", &myfile) == 0) check = do_gbcollect(&myfile, argv[1], argv[2]);
    
    return check;
}


/********************************************************************//**
 * MAIN
 */
int main (int argc, char* argv[])
{
    int ret = 0;

    if (argc < 2) {
        ret = ERR_NOT_ENOUGH_ARGUMENTS;
    } else {
        /* **********************************************************************
         * TODO WEEK 08: THIS PART SHALL BE REVISED THEN (WEEK 09) EXTENDED.
         * **********************************************************************
         */
        if (VIPS_INIT(argv[0]) == -1) vips_error_exit("unable to start VIPS");
        argc--;
        argv++; // skips command call name

        struct command_mapping listStruct;
        listStruct.myString = "list";
        listStruct.myCommand = &do_list_cmd;
        struct command_mapping createStruct;
        createStruct.myString = "create";
        createStruct.myCommand = &do_create_cmd;
        struct command_mapping helpStruct;
        helpStruct.myString = "help";
        helpStruct.myCommand = &help;
        struct command_mapping readStruct;
        readStruct.myString = "read";
        readStruct.myCommand = &do_read_cmd;
        struct command_mapping insertStruct;
        insertStruct.myString = "insert";
        insertStruct.myCommand = &do_insert_cmd;
        struct command_mapping deleteStruct;
        deleteStruct.myString = "delete";
        deleteStruct.myCommand = &do_delete_cmd;
        struct command_mapping gcStruct;
        gcStruct.myString = "gc";
        gcStruct.myCommand = &do_gc_cmd;

        struct command_mapping tab[7] = {listStruct, createStruct, helpStruct, readStruct, insertStruct, deleteStruct, gcStruct};
        int size_of_tab = sizeof(tab)/sizeof(tab[0]);
        int i = 0;
        int check = 0;

        while(i < size_of_tab && check == 0) {
            if (strcmp(tab[i].myString, argv[0]) == 0) {
                ret = (*tab[i].myCommand)(argc, argv);
                check = 1;
            }
            i++;
        }
        if (check == 0) {
            ret = ERR_INVALID_COMMAND;
        }
        if (ret != 0) {
            fprintf(stderr, "ERROR: %s\n", ERROR_MESSAGES[ret]);
            (void)help(argc, argv);
        }
        vips_shutdown();
    }

    return ret;
}
