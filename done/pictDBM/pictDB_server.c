#include "pictDB.h"
#include "libmongoose/mongoose.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include <vips/vips.h>

#define MAX_QUERY_PARAM 5

static const char *s_http_port = "8000";
static struct mg_serve_http_opts http_opts;

struct pictdb_file db_file;
void split(char* result[], char* tmp, const char* src, const char* delim, size_t len);
void mg_error(struct mg_connection* nc, int error);

/**
 * @brief Calls the list function on the webserver
 *
 * @param nc the connection
 */
static void handle_list_call(struct mg_connection* nc)
{
    if (nc != NULL) {
        char* buf = do_list(&db_file, JSON);

        mg_printf(nc, "HTTP/1.1 200 OK\r\n"
                  "Content-Type: application/json\r\n"
                  "Content-Length: %d\r\n\r\n"
                  "%s", (int) strlen(buf), buf);
        mg_send_http_chunk(nc, "", 0);
    }

    nc->flags |= MG_F_SEND_AND_CLOSE;
}

/**
 * @brief Calls the read function on the webserver
 *
 * @param nc the connection
 * @param hm the URI
 */
static void handle_read_call(struct mg_connection* nc, struct http_message* hm)
{
    if((nc != NULL) && (hm != NULL)) {
        const char* query = hm->query_string.p;
        size_t length = hm->query_string.len;
        char* result[MAX_QUERY_PARAM];
        const char delim[] = "&= ";

        char* tmp = malloc(length);
        if(tmp == NULL) mg_error(nc, ERR_OUT_OF_MEMORY);
        else {
            split(result, tmp, query, delim, length);

            if(((strcmp(result[0], "res") == 0) || (strcmp(result[2], "res") == 0)) && ((strcmp(result[0], "pict_id") == 0) || (strcmp(result[2], "pict_id") == 0))) {
                int r;
                char* buffer = NULL;
                uint32_t size = 0;

                if(strcmp(result[0], "res") == 0) {
                    r = do_read(result[3], resolution_atoi(result[1]), &buffer, &size, &db_file);
                    if(r != 0) mg_error(nc, r);
                    else {
                        mg_printf(nc, "HTTP/1.1 200 OK\r\n"
                                  "Content-Type: image/jpeg\r\n"
                                  "Content-Length: <%" PRIu32 ">\r\n\r\n", size);
                        mg_send(nc, buffer, size);

                        free(buffer);
                        buffer = NULL;
                    }
                } else {
                    r = do_read(result[1], resolution_atoi(result[3]), &buffer, &size, &db_file);
                    if(r != 0) mg_error(nc, r);
                    else {
                        mg_printf(nc, "HTTP/1.1 200 OK\r\n"
                                  "Content-Type: image/jpeg\r\n"
                                  "Content-Length:<%" PRIu32 ">\r\n\r\n", size);
                        mg_send(nc, buffer, size);

                        free(buffer);
                        buffer = NULL;
                    }
                }

            } else mg_error(nc, ERR_INVALID_ARGUMENT);

            size_t i;
            for (i = 0; i < MAX_QUERY_PARAM; i++) {
                free(result[i]);
                result[i] = NULL;
            }
            free(tmp);
            tmp = NULL;
        }
    }

    nc->flags |= MG_F_SEND_AND_CLOSE;
}

/**
 * @brief Calls the insert function on the webserver
 *
 * @param nc the connection
 * @param hm the URI
 */
static void handle_insert_call(struct mg_connection* nc, struct http_message* hm)
{
    if((nc != NULL) && (hm != NULL)) {
        char var_name[100], file_name[100];
        const char *chunk;
        size_t chunk_len, n1, n2;
        n1 = n2 = 0;

        while ((n2 = mg_parse_multipart(hm->body.p + n1, hm->body.len - n1,var_name, sizeof(var_name), file_name, sizeof(file_name), &chunk, &chunk_len)) > 0) n1 += n2;

        int r;
        r = do_insert(chunk, chunk_len, file_name, &db_file);
        if (r != 0) mg_error(nc, r);
        else mg_printf(nc, "HTTP/1.1 302 Found\r\n"
                           "Location: http://localhost:%s/index.html\r\n\r\n", s_http_port);
    }

    nc->flags |= MG_F_SEND_AND_CLOSE;
}

/**
 * @brief Calls the delete function on the webserver
 *
 * @param nc the connection
 * @param hm the URI
 */
static void handle_delete_call(struct mg_connection* nc, struct http_message* hm)
{
    if((nc != NULL) && (hm != NULL)) {
        const char* query = hm->query_string.p;
        size_t length = hm->query_string.len;
        char* result[MAX_QUERY_PARAM];
        const char delim[] = "&= ";

        char* tmp = malloc(length);
        if (tmp == NULL) mg_error(nc, ERR_OUT_OF_MEMORY);
        else {
            split(result, tmp, query, delim, length);

            if(strcmp(result[0], "pict_id") == 0) {
                int r;

                r = do_delete(result[1], &db_file);
                if (r != 0) mg_error(nc, r);
                else {
                    mg_printf(nc, "HTTP/1.1 302 Found\r\n"
                              "Location: http://localhost:%s/index.html\r\n\r\n", s_http_port);
                }
            } else mg_error(nc, ERR_INVALID_ARGUMENT);

            size_t i;
            for (i = 0; i < MAX_QUERY_PARAM; i++) {
                free(result[i]);
                result[i] = NULL;
            }
            free(tmp);
            tmp = NULL;
        }
    }

    nc->flags |= MG_F_SEND_AND_CLOSE;
}

/**
 * @brief Defines the behaviour of the connection
 *
 * @param nc The connection that has received an event
 * @param ev Event number
 * @param ev_data This pointer points to the event-specific data, and it has different meaning for different events.
 */
static void ev_handler(struct mg_connection* nc, int ev, void* ev_data)
{

    struct http_message* hm = (struct http_message*) ev_data;

    switch (ev) {
    case MG_EV_HTTP_REQUEST:
        if (mg_vcmp(&hm->uri, "/pictDB/list") == 0) handle_list_call(nc);
        else if (mg_vcmp(&hm->uri, "/pictDB/read") == 0) handle_read_call(nc, hm);
        else if (mg_vcmp(&hm->uri, "/pictDB/insert") == 0) handle_insert_call(nc, hm);
        else if (mg_vcmp(&hm->uri, "/pictDB/delete") == 0) handle_delete_call(nc, hm);
        else mg_serve_http(nc, hm, http_opts);
        break;
    default :
        break;
    }
}

/**
 * @brief Splits the query string in chunks
 *
 * @param result array of strings representing the parameters
 * @param tmp string containing the set of all parts of the query string one after another
 * @param src string to split
 * @param src list of delimiters
 * @param length of the query to cut
 */
void split(char* result[], char* tmp, const char* src, const char* delim, size_t len)
{
    if((result != NULL) && (tmp != NULL) && (src != NULL) && (delim != NULL)) {
        char* src_copy = malloc(len + 1);
        strncpy(src_copy, src, len +1);
        tmp = strtok(src_copy, delim);

        size_t i = 0;
        while(i < MAX_QUERY_PARAM) {
            if (tmp != NULL) {
                result[i] = malloc(strlen(tmp) + 1);
                strncpy(result[i], tmp, strlen(tmp) + 1);
            } else result[i] = NULL;
            tmp = strtok(NULL, delim);
            i++;
        }

        free(src_copy);
        src_copy = NULL;
    }
}

/**
 * @brief Reports errors
 *
 * @param nc the connection
 * @param error code of the error
 */
void mg_error(struct mg_connection* nc, int error)
{
    mg_printf(nc, "HTTP/1.1 500 %s\r\n"
              "Content-Length: %d\r\n\r\n", ERROR_MESSAGES[error], 0);
}

/********************************************************************//**
 * MAIN
 */
int main(int argc, char *argv[])
{

    if (argc < 2) {
        return ERR_NOT_ENOUGH_ARGUMENTS;
    } else {
        if (VIPS_INIT(argv[0]) == -1) vips_error_exit("unable to start VIPS");
        argc--;
        argv++;

        struct mg_mgr mgr;
        struct mg_connection* nc;

        mg_mgr_init(&mgr, NULL);
        nc = mg_bind(&mgr, s_http_port, ev_handler);

        if(nc == NULL) {
            fprintf(stderr, "Error, could not start active connection on port %s\n", s_http_port);
            mg_mgr_free(&mgr);
            vips_shutdown();
            return ERR_IO;
        }

        mg_set_protocol_http_websocket(nc);

        if(do_open(argv[0], "rb+", &db_file) == 0) {
            print_header(&(db_file.header));
            for (;;) mg_mgr_poll(&mgr, 1000);
            do_close(&db_file);
        }

        mg_mgr_free(&mgr);
        vips_shutdown();

        return 0;
    }

}
