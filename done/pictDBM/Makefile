CFLAGS += -Wall -Wextra -std=c99
CFLAGS += $$(pkg-config vips --cflags)
LDLIBS += $$(pkg-config vips --libs) -lm -lcrypto -lmongoose -ljson-c
LDFLAGS += -Llibmongoose

all : pictDBM pictDB_server

pictDBM : pictDBM.c db_list.c db_utils.c error.c db_create.c db_delete.c image_content.c pictDBM_tools.c dedup.c db_insert.c db_read.c db_gbcollect.c
	gcc $(CFLAGS) $(LDFLAGS) -o pictDBM pictDBM.c db_list.c db_utils.c error.c db_create.c db_delete.c image_content.c pictDBM_tools.c dedup.c db_insert.c db_read.c db_gbcollect.c $(LDLIBS)
	
pictDB_server : pictDB_server.c db_list.c db_utils.c error.c db_read.c image_content.c db_insert.c dedup.c db_delete.c
	gcc $(CFLAGS) $(LDFLAGS) -o pictDB_server pictDB_server.c db_list.c db_utils.c error.c db_read.c image_content.c db_insert.c dedup.c db_delete.c $(LDLIBS)
