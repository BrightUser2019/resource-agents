/******************************************************************************
*******************************************************************************
**
**  Copyright (C) 2004 Red Hat, Inc.  All rights reserved.
**
**  This copyrighted material is made available to anyone wishing to use,
**  modify, copy, or redistribute it subject to the terms and conditions
**  of the GNU General Public License v.2.
**
*******************************************************************************
******************************************************************************/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <libxml/parser.h>
#include <errno.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

#include "comm_headers.h"
#include "log.h"
#include "debug.h"
#include "misc.h"

int quorate = 0;

int update_required = 0;
pthread_mutex_t update_lock;

open_doc_t *master_doc = NULL;

int get_doc_version(xmlDocPtr ldoc){
  int error = 0;
  xmlXPathObjectPtr  obj = NULL;
  xmlXPathContextPtr ctx = NULL;
  xmlNodePtr        node = NULL;

  ENTER("get_doc_version");

  ctx = xmlXPathNewContext(ldoc);
  if(!ctx){
    log_err("Error: unable to create new XPath context.\n");
    error = -EIO;  /* ATTENTION -- what should this be? */
    goto fail;
  }

  obj = xmlXPathEvalExpression("//cluster/@config_version", ctx);
  if(!obj || !obj->nodesetval || (obj->nodesetval->nodeNr != 1)){
    log_err("Error while retrieving config_version.\n");
    error = -ENODATA;
    goto fail;
  }

  node = obj->nodesetval->nodeTab[0];
  if(node->type != XML_ATTRIBUTE_NODE){
    log_err("Object returned is not of attribute type.\n");
    error = -ENODATA;
    goto fail;
  }

  if(!node->children->content || !strlen(node->children->content)){
    log_dbg("No content found.\n");
    error = -ENODATA;
    goto fail;
  }

  error = atoi(node->children->content);

 fail:
  if(ctx){
    xmlXPathFreeContext(ctx);
  }
  if(obj){
    xmlXPathFreeObject(obj);
  }
  EXIT("get_doc_version");
  return error;
}


/**
 * get_cluster_name
 * @ldoc:
 *
 * The caller must remember to free the string that is returned.
 *
 * Returns: NULL on failure, (char *) otherwise
 */
char *get_cluster_name(xmlDocPtr ldoc){
  int error = 0;
  char *rtn = NULL;
  xmlXPathObjectPtr  obj = NULL;
  xmlXPathContextPtr ctx = NULL;
  xmlNodePtr        node = NULL;

  ENTER("get_cluster_name");

  ctx = xmlXPathNewContext(ldoc);
  if(!ctx){
    log_err("Error: unable to create new XPath context.\n");
    error = -EIO;  /* ATTENTION -- what should this be? */
    goto fail;
  }

  obj = xmlXPathEvalExpression("//cluster/@name", ctx);
  if(!obj || !obj->nodesetval || (obj->nodesetval->nodeNr != 1)){
    log_err("Error while retrieving config_version.\n");
    error = -ENODATA;
    goto fail;
  }

  node = obj->nodesetval->nodeTab[0];
  if(node->type != XML_ATTRIBUTE_NODE){
    log_err("Object returned is not of attribute type.\n");
    error = -ENODATA;
    goto fail;
  }

  if(!node->children->content || !strlen(node->children->content)){
    log_dbg("No content found.\n");
    error = -ENODATA;
    goto fail;
  }

  rtn = strdup(node->children->content);

 fail:
  if(ctx){
    xmlXPathFreeContext(ctx);
  }
  if(obj){
    xmlXPathFreeObject(obj);
  }
  EXIT("get_cluster_name");
  return rtn;
}



