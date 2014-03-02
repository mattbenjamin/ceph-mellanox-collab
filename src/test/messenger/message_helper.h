// -*- mode:C++; tab-width:8; c-basic-offset:2; indent-tabs-mode:t -*- 
// vim: ts=8 sw=2 smarttab
/*
 * Ceph - scalable distributed file system
 *
 * Copyright (C) 2004-2006 Sage Weil <sage@newdream.net>
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License version 2.1, as published by the Free Software 
 * Foundation.  See file COPYING.
 * 
 */

#ifndef MESSAGE_HELPER_H_
#define MESSAGE_HELPER_H_

#include "msg/msg_types.h"
#include "messages/MDataPing.h"
#include "msg/XioMessenger.h"

static inline Message* new_ping_monstyle(const char *tag, int mult)
{
  Message *m = new MPing();
  Formatter *f = new JSONFormatter(true);

  string str = "one giant step for ";

  f->open_object_section(tag);
  for (int ix = 0; ix < mult; ++ix) {
    f->dump_string(tag, str);
  }
  f->close_section();

  bufferlist bl;
  stringstream ss;

  f->flush(ss);
  ::encode(ss.str(), bl);
  m->set_payload(bl);

  return m;
}

extern struct xio_rdma_mempool *xio_msgr_mpool;

void xio_hook_func(struct xio_rdma_mp_mem *mp)
{
  xio_rdma_mempool_free(mp);
}

static inline Message* new_ping_with_data(const char *tag, uint32_t size)
{
  static uint32_t counter;

  MDataPing *m = new MDataPing();
  m->counter = counter++;
  m->tag = tag;

  bufferlist bl;
  void *p;

  struct xio_rdma_mp_mem *mp = m->get_mp();
  (void) xio_rdma_mempool_alloc(xio_msgr_mpool, size, mp);
  p = mp->addr;
  m->set_rdma_hook(xio_hook_func);

  strcpy((char*) p, tag);
  uint32_t* t = (uint32_t* ) (((char*) p) + size - 32);
  *t = counter;

  bl.append(buffer::create_static(size, (char*) p));
  m->set_data(bl);

  return static_cast<Message*>(m);
}

static inline Message* new_simple_ping_with_data(const char *tag,
						 uint32_t size)
{
  static size_t pagesize = sysconf(_SC_PAGESIZE);
  static uint32_t counter;

  MDataPing *m = new MDataPing();
  m->counter = counter++;
  m->tag = tag;

  bufferlist bl;
  void *p;

  size = (size + pagesize - 1) & ~(pagesize - 1);
  if (posix_memalign(&p, pagesize, size))
    p = NULL;
  m->free_data = true;

  strcpy((char*) p, tag);
  uint32_t* t = (uint32_t* ) (((char*) p) + size - 32);
  *t = counter;

  bl.append(buffer::create_static(size, (char*) p));
  m->set_data(bl);

  return static_cast<Message*>(m);
}


#endif /* MESSAGE_HELPER_H_ */