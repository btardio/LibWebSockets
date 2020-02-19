/*
 * libwebsockets-test-server - libwebsockets test implementation
 *
 * Copyright (C) 2010-2016 Andy Green <andy@warmcat.com>
 *
 * This file is made available under the Creative Commons CC0 1.0
 * Universal Public Domain Dedication.
 *
 * The person who associated a work with this deed has dedicated
 * the work to the public domain by waiving all of his or her rights
 * to the work worldwide under copyright law, including all related
 * and neighboring rights, to the extent allowed by law. You can copy,
 * modify, distribute and perform the work, even for commercial purposes,
 * all without asking permission.
 *
 * The test apps are intended to be adapted for use in your code, which
 * may be proprietary.  So unlike the library itself, they are licensed
 * Public Domain.
 */

//#define DEBUG_PRINT 0

#include "lwsev.h"
#include <libwebsockets.h>

//#include "/usr/include/hiredis/hiredis.h"
/* dumb_increment protocol */

void rConnect_lws_Callback(const redisAsyncContext *c, int status) {
  
    if (status != REDIS_OK) {
        printf("(HI)Redis Error: %s\n", c->errstr);
        return;
    }
    printf("(HI)Redis Connected...\n");
}


void rDisconnect_lws_Callback( const redisAsyncContext *c, int status ){
  
    if (status != REDIS_OK) {
      printf("(HI)Redis Error: %s\n", c->errstr);
      return;
    }
    printf("(HI)Redis Disconnected...\n");
    
    // TODO: should probably do something here, stop the server, try to reestablish redis connection, etc
  
}


void key_destroy_func( gpointer data ){
  
  free ( data );
  
}

void value_destroy_func( gpointer data ){
  
  free ( data );
  
}

void hmget_addedit_Callback(redisAsyncContext *c, void *r, void *privdata) {
  
  redisReply *reply = r;
  
  struct callback_data* cbd = privdata;
  
  struct hmgetstruct* gstruct;
  
  unsigned long long int* hashid;
  
  gboolean gocontinue = 0;
  
  JsonBuilder *builder = json_builder_new ();
  
  gstruct = malloc(sizeof(struct hmgetstruct));
  
  hashid = malloc(sizeof(unsigned long long int));
  
  unsigned char strbuf[512];
  
  int n, m;
  
  
  
  printf("cbd->number in hmget_addedit_callback: %d", cbd->pss->number);
  cbd->pss->number ++;
  
  
  // TODO: check to see if the redis entry has len of 5
  if (reply == NULL) {
    printf("Reply is NULL");
    return;
  }
#ifdef DEBUG_PRINT  
  printf("n_elements in hmget: %d\n", reply->elements);
#endif  
  if ( reply->type == REDIS_REPLY_ARRAY ){
    
    // gstruct->id
    gstruct->id = strtoull( &reply->element[0]->str[0], '\0' , 10 );
    
#ifdef DEBUG_PRINT
    printf( "id: %ull\n", gstruct->id );
#endif    
    
    // gstruct->entrytime
    *hashid = gstruct->entrytime = strtoull( &reply->element[1]->str[0], '\0', 10 );

#ifdef DEBUG_PRINT
    printf( "hashid: %ull\n", *hashid );
#endif    
    
#ifdef DEBUG_PRINT    
    printf( "entrytime: %ull\n", gstruct->entrytime );
#endif    
    
    // gstruct->job_id
    snprintf(gstruct->job_id,
              ( reply->element[2]->len+1 > 256 ? 256 : reply->element[2]->len+1 ), 
              "%s", reply->element[2]->str );
    
#ifdef DEBUG_PRINT    
    printf("job_id: %s\n", gstruct->job_id);
#endif
    
    // gstruct->job_entry_date
    snprintf(gstruct->job_entry_date,
              ( reply->element[3]->len+1 > 128 ? 128 : reply->element[3]->len+1 ),
              "%s", reply->element[3]->str );

#ifdef DEBUG_PRINT
    printf("job_entry_date: %s\n", gstruct->job_entry_date);
#endif
    
    // gstruct->job_description
    snprintf(gstruct->job_description,
              ( reply->element[4]->len+1 > 65535 ? 65535 : reply->element[4]->len+1 ),
              "%s", reply->element[4]->str);

#ifdef DEBUG_PRINT
    printf("job_description: %s\n", gstruct->job_description);
#endif
    
    // gstruct->job_status
    gstruct->job_status[0] = reply->element[5]->str[0];
    gstruct->job_status[1] = reply->element[5]->str[1];
    gstruct->job_status[2] = '\0';
    
#ifdef DEBUG_PRINT
    printf("%c%c\n", gstruct->job_status[0], gstruct->job_status[1]);
#endif    
    
    
    // TODO: lock should stay until end of hash_table_add
    
    
    while ( cbd->pss->hash_lock ){
    }
    cbd->pss->hash_lock = 1;
    gocontinue = g_hash_table_contains(cbd->pss->ghasht_sent_entries, hashid );
    cbd->pss->hash_lock = 0;
    
    
    if ( !gocontinue ){    
    
    //if ( !g_hash_table_contains(cbd->pss->ghasht_sent_entries, hashid ) ){
    
      json_builder_begin_object (builder);

      json_builder_set_member_name (builder, "type");
      
      json_builder_add_string_value (builder, "addedit");      
      
      json_builder_set_member_name (builder, "id");
      
      json_builder_add_int_value (builder, gstruct->id);
      
      json_builder_set_member_name (builder, "job_id");
      
      json_builder_add_string_value (builder, gstruct->job_id);
      
      json_builder_set_member_name (builder, "job_entry_date");
      
      json_builder_add_string_value (builder, gstruct->job_entry_date);
      
      json_builder_set_member_name (builder, "job_description");
      
      json_builder_add_string_value (builder, gstruct->job_description);
      
      json_builder_set_member_name (builder, "job_status");
      
      json_builder_add_string_value (builder, gstruct->job_status);    
      
      json_builder_end_object (builder);
      
      JsonGenerator *gen = json_generator_new ();
      
      JsonNode * root = json_builder_get_root (builder);
      
      json_generator_set_root (gen, root);
    
      gchar *jsonstr = json_generator_to_data (gen, NULL);
      
#ifdef DEBUG_PRINT      
      printf("json string: %s\n", jsonstr);
#endif      
      
      //TODO: check the id of the entry entered, 
      //TODO: if a newer entry was consumed / is in hash or the queue has a newer entry dont add this entry
      while ( cbd->pss->queue_lock ){
      }
      cbd->pss->queue_lock = 1;
      g_queue_push_head ( cbd->pss->gqueue_entries, jsonstr );
      cbd->pss->queue_lock = 0;
      
//       unsigned char buf[LWS_PRE + 512];
//       unsigned char *p = &buf[LWS_PRE];     
//       
//       n = sprintf((char *)p, "%s", jsonstr);
//       m = lws_write(cbd->wsi, p, n, LWS_WRITE_TEXT);
//       if (m < n) {
//              lwsl_err("ERROR %d writing to di socket\n", n);
//       }
      
      
      json_node_free (root);
      g_object_unref (gen);
      g_object_unref (builder);
      
      // TODO: g_hash_table will eat up all memory if a socket connection is left open and user keeps making
      //       edits, check if the entries in g_hash_table have expired and remove from the table
      
      while ( cbd->pss->hash_lock ){
      }
      cbd->pss->hash_lock = 1;
      g_hash_table_insert (cbd->pss->ghasht_sent_entries, hashid, gstruct);
      cbd->pss->hash_lock = 0;
    }

    
//       snprintf( gstruct.job_id, reply->element[1]      
//       
//       for ( int i = 0; i < reply->elements; i++ ){
//         printf("%s\n", reply->element[i]->str);
//         //redisReply* replyitr = reply->element[i];
//         
//         //printf( "%d\n", replyitr->len );
//         redisAsyncCommand(redis_async_context, hmgetCallback, (void*) wsi, 
//                           "hmget app_acrylic_process_entry_addedit_165 id job_id job_entry_date job_description job_status");
//         //snprintf( strbuf, replyitr->len + 2, "%s\n\0", replyitr->str );
//         //printf( "%s", strbuf );
//         //printf("%s\n", );
//         
//       }
    
    //freeReplyObject(reply);
    
  }
  
  //unsigned char buf[LWS_PRE + 512];
  //unsigned char *p = &buf[LWS_PRE];     
  
  //n = sprintf((char *)p, "%s", reply->str);
  //m = lws_write(wsi, p, n, LWS_WRITE_TEXT);
  //if (m < n) {
  //        lwsl_err("ERROR %d writing to di socket\n", n);
  //}
    
  
  //unsigned char buf[LWS_PRE + 512];
  //unsigned char *p = &buf[LWS_PRE];     
  
  //n = sprintf((char *)p, "%s", reply->str);
  //m = lws_write(wsi, p, n, LWS_WRITE_TEXT);
  //if (m < n) {
  //        lwsl_err("ERROR %d writing to di socket\n", n);
  //}

  free ( cbd );
}


void hmget_delete_Callback(redisAsyncContext *c, void *r, void *privdata) {
  
  redisReply *reply = r;
  
  struct callback_data* cbd = privdata;
  
  struct hmgetstruct* gstruct;
  
  gboolean gocontinue = 0;
  
  unsigned long long int* hashid;
  
  JsonBuilder *builder = json_builder_new ();
  
  gstruct = malloc(sizeof(struct hmgetstruct));
  
  hashid = malloc(sizeof(unsigned long long int));
  
  unsigned char strbuf[512];
  
  int n, m;
  
  if (reply == NULL) {
    printf("Reply is NULL");
    return;
  }
#ifdef DEBUG_PRINT  
  printf("n_elements in hmget: %d\n", reply->elements);
#endif  
  if ( reply->type == REDIS_REPLY_ARRAY ){
    
    // gstruct->id
    gstruct->id = strtoull( &reply->element[0]->str[0], '\0' , 10 );
    
#ifdef DEBUG_PRINT
    printf( "id: %ull\n", gstruct->id );
#endif    
    
    // gstruct->entrytime
    *hashid = gstruct->entrytime = strtoull( &reply->element[1]->str[0], '\0', 10 );
    
#ifdef DEBUG_PRINT
    printf( "hashid: %ull\n", *hashid );
#endif        
    
#ifdef DEBUG_PRINT    
    printf( "entrytime: %ull\n", gstruct->entrytime );
#endif    
    
    // gstruct->job_id
    snprintf(gstruct->job_id,
              ( reply->element[2]->len+1 > 256 ? 256 : reply->element[2]->len+1 ), 
              "%s", reply->element[2]->str );
    
#ifdef DEBUG_PRINT    
    printf("job_id: %s\n", gstruct->job_id);
#endif
    
    // gstruct->job_status
    gstruct->job_status[0] = reply->element[3]->str[0];
    gstruct->job_status[1] = reply->element[3]->str[1];
    gstruct->job_status[2] = '\0';
    
#ifdef DEBUG_PRINT
    printf("%c%c\n", gstruct->job_status[0], gstruct->job_status[1]);
#endif    
    
    
    while ( cbd->pss->hash_lock ){
    }
    cbd->pss->hash_lock = 1;
    gocontinue = g_hash_table_contains(cbd->pss->ghasht_sent_entries, hashid );
    cbd->pss->hash_lock = 0;
    
    
    if ( !gocontinue ){
    
      json_builder_begin_object (builder);
      
      json_builder_set_member_name (builder, "type");
      
      json_builder_add_string_value (builder, "delete");            
      
      json_builder_set_member_name (builder, "id");
      
      json_builder_add_int_value (builder, gstruct->id);
      
      json_builder_set_member_name (builder, "job_id");
      
      json_builder_add_string_value (builder, gstruct->job_id);
            
      json_builder_set_member_name (builder, "job_status");
      
      json_builder_add_string_value (builder, gstruct->job_status);    
      
      json_builder_end_object (builder);
      
      JsonGenerator *gen = json_generator_new ();
      
      JsonNode * root = json_builder_get_root (builder);
      
      json_generator_set_root (gen, root);
    
      gchar *jsonstr = json_generator_to_data (gen, NULL);
      
#ifdef DEBUG_PRINT      
      printf("json string: %s\n", jsonstr);
#endif      
      
      
      while ( cbd->pss->queue_lock ){
      }
      cbd->pss->queue_lock = 1;
      g_queue_push_head ( cbd->pss->gqueue_entries, jsonstr );
      cbd->pss->queue_lock = 0;
            
      json_node_free (root);
      g_object_unref (gen);
      g_object_unref (builder);
      
      // TODO: g_hash_table will eat up all memory if a socket connection is left open and user keeps making
      //       edits, check if the entries in g_hash_table have expired and remove from the table
      
      while ( cbd->pss->hash_lock ){
      }
      cbd->pss->hash_lock = 1;
      g_hash_table_insert (cbd->pss->ghasht_sent_entries, hashid, gstruct);
      cbd->pss->hash_lock = 0;
    }
    
  }

  free ( cbd );
}

void smembers_addedit_Callback(redisAsyncContext *c, void *r, void *privdata) {
  redisReply *reply = r;
  struct callback_data* cbd = privdata;
  
  unsigned char strbuf[512];
  
  char query_string[512];
  
  int n, m;
  
  printf("cbd->number in smembers_addedit_callback: %d", cbd->pss->number);
  cbd->pss->number ++;  
  
  if (reply == NULL) {
    printf("Reply is NULL");
    return;
  }

#ifdef DEBUG_PRINT
  printf("n_elements in smembers: %d\n", reply->elements);
#endif
  
  if ( reply->type == REDIS_REPLY_ARRAY ){
    
    int j = 0;
          
    for ( int i = 0; i < reply->elements; i++ ){
      
      unsigned long long int id = strtoull(&reply->element[i]->str[0], '\0' , 10);
      
      if ( id != 0 ){
      
        struct callback_data* hmget_cbd;
        hmget_cbd = malloc(sizeof(struct callback_data));
        hmget_cbd->pss = cbd->pss;
        hmget_cbd->wsi = cbd->wsi; 
        
        sprintf(query_string, "hmget app_acrylic_process_entry_addedit_%llu id entrytime job_id job_entry_date job_description job_status", 
                id);
        
        redisAsyncCommand(redis_async_context, hmget_addedit_Callback, (void*) hmget_cbd, query_string);
        
      }
      
    }
    
    // freeReplyObject(reply); as documented, async doesn't require freereplyobject, takes care of this for you
    // on return from callback
    
  }
  

  free ( cbd );
  
}




void smembers_delete_Callback(redisAsyncContext *c, void *r, void *privdata) {
  redisReply *reply = r;
  struct callback_data* cbd = privdata;
  
  unsigned char strbuf[512];
  
  char query_string[512];
  
  int n, m;
  
  if (reply == NULL) {
    printf("Reply is NULL");
    return;
  }

#ifdef DEBUG_PRINT
  printf("n_elements in smembers: %d\n", reply->elements);
#endif
  
  if ( reply->type == REDIS_REPLY_ARRAY ){
    
    int j = 0;
          
    for ( int i = 0; i < reply->elements; i++ ){
      
      unsigned long long int id = strtoull(&reply->element[i]->str[0], '\0' , 10);
      
      if ( id != 0 ){
      
        struct callback_data* hmget_cbd;
        hmget_cbd = malloc(sizeof(struct callback_data));
        hmget_cbd->pss = cbd->pss;
        hmget_cbd->wsi = cbd->wsi; 
        
        sprintf(query_string, "hmget app_acrylic_process_entry_delete_%llu id entrytime job_id job_status", 
                id);
        
        redisAsyncCommand(redis_async_context, hmget_delete_Callback, (void*) hmget_cbd, query_string);
        
      }
      
    }
    
    // freeReplyObject(reply); as documented, async doesn't require freereplyobject, takes care of this for you
    // on return from callback
    
  }
  

  free ( cbd );
  
}

// ?? copies header data into wsi ?? should i reject certain header data?
void
dump_handshake_info(struct lws *wsi)
{
        int n = 0, len;
        char buf[256];
        const unsigned char *c;

        do {
                c = lws_token_to_string(n);
                if (!c) {
                        n++;
                        continue;
                }

                len = lws_hdr_total_length(wsi, n);
                if (!len || len > sizeof(buf) - 1) {
                        n++;
                        continue;
                }

                lws_hdr_copy(wsi, buf, sizeof buf, n);
                buf[sizeof(buf) - 1] = '\0';

                fprintf(stderr, "    %s = %s\n", (char *)c, buf);
                n++;
        } while (c);
}

int
callback_acrylic_process(struct lws *wsi, enum lws_callback_reasons reason,
                        void *user, void *in, size_t len)
{

  struct per_session_data__acrylic_process *pss =
                  (struct per_session_data__acrylic_process *)user;

  struct callback_data* cbdaddedit;
  struct callback_data* cbddelete;                  

  char* jsonstr = NULL;
  
  int n,m;
  
  unsigned char buf[LWS_PRE + 512];
  unsigned char *p = &buf[LWS_PRE];     
  
  int condition = 1;
  
  printf("cbd->number in callback acrylic process: %d", pss->number);
  pss->number ++;    
  
  switch (reason) {

  case LWS_CALLBACK_ESTABLISHED:
    
    pss->ghasht_sent_entries = g_hash_table_new_full( g_int64_hash, g_int64_equal, key_destroy_func, value_destroy_func );
    
    pss->gqueue_entries = g_queue_new();
    
    pss->queue_lock = pss->hash_lock = 0;
    
    pss->number = 0;
    break;

  case LWS_CALLBACK_SERVER_WRITEABLE:
    
    
    // check redis, if this was called there is a message,
    // message could have already been sent to client, 
    // determine if message should be sent to client
    // read message, format message, send message to client
    
    
    
    
    while ( condition ){
    
      if ( !pss->queue_lock ){
        pss->queue_lock = 1;
        jsonstr = g_queue_pop_head( pss->gqueue_entries );
        pss->queue_lock = 0;
      }
      
      if ( jsonstr == NULL ) {
        condition = 0;
      }
      else{
        n = sprintf((char *)p, "%s", jsonstr);
        
        
        //
        m = lws_write(wsi, p, n, LWS_WRITE_TEXT);
        //
        
        
        if (m < n) {
                lwsl_err("ERROR %d writing to di socket\n", n);
        }
        
        free ( jsonstr );
      }
    
    }    
    
    cbdaddedit = malloc(sizeof(struct callback_data));
    cbdaddedit->pss = pss;
    cbdaddedit->wsi = wsi;    

    cbddelete = malloc(sizeof(struct callback_data));
    cbddelete->pss = pss;
    cbddelete->wsi = wsi;    

    
    redisAsyncCommand(redis_async_context, smembers_addedit_Callback, (void*) cbdaddedit, "smembers app_acrylic_process_set_add_ids");
    redisAsyncCommand(redis_async_context, smembers_delete_Callback, (void*) cbddelete, "smembers app_acrylic_process_set_delete_ids");
    
    break;

  case LWS_CALLBACK_RECEIVE:
    
          // save message to redis store
    
          if (len < 6)
                  break;
          if (strcmp((const char *)in, "reset\n") == 0)
                  pss->number = 0;
          if (strcmp((const char *)in, "closeme\n") == 0) {
                  lwsl_notice("dumb_inc: closing as requested\n");
                  lws_close_reason(wsi, LWS_CLOSE_STATUS_GOINGAWAY,
                                    (unsigned char *)"seeya", 5);
                  return -1;
          }
          break;
  /*
    * this just demonstrates how to use the protocol filter. If you won't
    * study and reject connections based on header content, you don't need
    * to handle this callback
    */
        case LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION:
                dump_handshake_info(wsi);
          /* you could return non-zero here and kill the connection */
                break;

  /*
    * this just demonstrates how to handle
    * LWS_CALLBACK_WS_PEER_INITIATED_CLOSE and extract the peer's close
    * code and auxiliary data.  You can just not handle it if you don't
    * have a use for this.
    */
  case LWS_CALLBACK_WS_PEER_INITIATED_CLOSE:
          lwsl_notice("LWS_CALLBACK_WS_PEER_INITIATED_CLOSE: len %lu\n",
                      (unsigned long)len);
          for (int n = 0; n < (int)len; n++)
                  lwsl_notice(" %d: 0x%02X\n", n,
                              ((unsigned char *)in)[n]);
          break;

  default:
          break;
  }

  return 0;
}
