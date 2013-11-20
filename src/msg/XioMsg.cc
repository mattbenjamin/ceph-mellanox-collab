// -*- mode:C++; tab-width:8; c-basic-offset:2; indent-tabs-mode:t -*-
// vim: ts=8 sw=2 smarttab
/*
 * Ceph - scalable distributed file system
 *
 * Copyright (C) 2004-2006 Sage Weil <sage@newdream.net>
 * Portions Copyright (C) 2013 CohortFS, LLC
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License version 2.1, as published by the Free Software
 * Foundation.  See file COPYING.
 *
 */

#include "XioMessenger.h"
#include "XioConnection.h"
#include "XioMsg.h"


void XioCompletionHook::finish(int r)
{
  printf("XioCompletionHook::finish called %p (%d)\n", this, r);

  struct xio_msg *msg, *rsp;
  list <struct xio_msg *>::iterator iter;

  for (iter = msg_seq.begin(); iter != msg_seq.end(); ++iter) {
    msg = *iter;
    switch (msg->type) {
    case XIO_MSG_TYPE_REQ:
    {
      ConnectionRef conn = m->get_connection();
      XioConnection *xcon = static_cast<XioConnection*>(conn.get());
      /* XXX ack it (Eyal:  we'd like an xio_ack_response) */
      rsp = (struct xio_msg *) calloc(1, sizeof(struct xio_msg));
      rsp->request = msg;
      pthread_spin_lock(&xcon->sp);
      (void) xio_send_response(rsp); /* XXX can now chain */
      pthread_spin_unlock(&xcon->sp);
    }
      break;
    case XIO_MSG_TYPE_RSP:
    case XIO_MSG_TYPE_ONE_WAY:
    default:
      abort();
      break;
    }
  }
}
