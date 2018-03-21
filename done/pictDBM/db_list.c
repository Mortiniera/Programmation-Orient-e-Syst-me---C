#include "pictDB.h"

#include <stdio.h>
#include <json-c/json.h>
#include <string.h>

char* do_list(const struct pictdb_file* db_file, enum do_list_mode enum_mode)
{
    char* mychar = NULL;
    if (enum_mode == STDOUT) {
        if(db_file != NULL) {
            print_header(&(db_file->header));
            if (db_file->header.num_files == 0) printf("<< empty database >>\n");
            else {
                size_t i;
                for (i = 0 ; i < db_file->header.max_files ; i++) {
                    if (db_file->metadata[i].is_valid == NON_EMPTY) print_metadata(&(db_file->metadata[i]));
                }
            }
        }
        return mychar;
    } else if (enum_mode == JSON) {
        json_object * jobj = json_object_new_object();
        json_object *jarray = json_object_new_array();
        
        size_t i;
        for (i = 0 ; i < db_file->header.max_files ; i++) {
            if (db_file->metadata[i].is_valid == NON_EMPTY) json_object_array_add(jarray, json_object_new_string(db_file->metadata[i].pict_id));
        }

        json_object_object_add(jobj,"Pictures", jarray);

        mychar = calloc(strlen(json_object_to_json_string(jobj)) + 1, sizeof(char));
        strncpy(mychar, json_object_to_json_string(jobj), strlen(json_object_to_json_string(jobj)) + 1);

        json_object_put(jobj);
        return mychar;
    } else {
        mychar = "unimplemented do_list mode";
        return mychar;
    }
}
