/*
 * Copyright (c) 1997-2005 Erez Zadok <ezk@cs.stonybrook.edu>
 * Copyright (c) 2001-2005 Stony Brook University
 *
 * For specific licensing information, see the COPYING file distributed with
 * this package, or get one from ftp://ftp.filesystems.org/pub/fistgen/COPYING.
 *
 * This Copyright notice must be kept intact and distributed with all
 * fistgen sources INCLUDING sources generated by fistgen.
 */
/*
 * Copyright (c) 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * John Heidemann of the UCLA Ficus project.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)snoopfs_vnops.c	8.6 (Berkeley) 5/27/95
 *
 * Ancestors:
 *	@(#)lofs_vnops.c	1.2 (Berkeley) 6/18/92
 *	...and...
 *	@(#)snoopfs_vnodeops.c 1.20 92/07/07 UCLA Ficus project
 *
 * $FreeBSD: src/sys/fs/snoopfs/snoopfs_vnops.c,v 1.60 2002/10/16 08:00:32 phk Exp $
 */

/*
 * Wrapfs Layer
 *
 * (See mount_snoopfs(8) for more information.)
 *
 * The snoopfs layer duplicates a portion of the filesystem
 * name space under a new name.  In this respect, it is
 * similar to the loopback filesystem.  It differs from
 * the loopback fs in two respects:  it is implemented using
 * a stackable layers techniques, and its "snoopfs-node"s stack above
 * all lower-layer vnodes, not just over directory vnodes.
 *
 * The snoopfs layer has two purposes.  First, it serves as a demonstration
 * of layering by proving a layer which does nothing.  (It actually
 * does everything the loopback filesystem does, which is slightly
 * more than nothing.)  Second, the snoopfs layer can serve as a prototype
 * layer.  Since it provides all necessary layer framework,
 * new filesystem layers can be created very easily be starting
 * with a snoopfs layer.
 *
 * The remainder of this man page examines the snoopfs layer as a basis
 * for constructing new layers.
 *
 *
 * INSTANTIATING NEW SNOOPFS LAYERS
 *
 * New snoopfs layers are created with mount_wra[fsfs(8).
 * Mount_snoopfs(8) takes two arguments, the pathname
 * of the lower vfs (target-pn) and the pathname where the snoopfs
 * layer will appear in the namespace (alias-pn).  After
 * the snoopfs layer is put into place, the contents
 * of target-pn subtree will be aliased under alias-pn.
 *
 *
 * OPERATION OF A SNOOPFS LAYER
 *
 * The snoopfs layer is the minimum filesystem layer,
 * simply bypassing all possible operations to the lower layer
 * for processing there.  The majority of its activity centers
 * on the bypass routine, through which nearly all vnode operations
 * pass.
 *
 * The bypass routine accepts arbitrary vnode operations for
 * handling by the lower layer.  It begins by examing vnode
 * operation arguments and replacing any snoopfs-nodes by their
 * lower-layer equivlants.  It then invokes the operation
 * on the lower layer.  Finally, it replaces the snoopfs-nodes
 * in the arguments and, if a vnode is return by the operation,
 * stacks a snoopfs-node on top of the returned vnode.
 *
 * Although bypass handles most operations, vop_getattr, vop_lock,
 * vop_unlock, vop_inactive, vop_reclaim, and vop_print are not
 * bypassed. Vop_getattr must change the fsid being returned.
 * Vop_lock and vop_unlock must handle any locking for the
 * current vnode as well as pass the lock request down.
 * Vop_inactive and vop_reclaim are not bypassed so that
 * they can handle freeing snoopfs-layer specific data. Vop_print
 * is not bypassed to avoid excessive debugging information.
 * Also, certain vnode operations change the locking state within
 * the operation (create, mknod, remove, link, rename, mkdir, rmdir,
 * and symlink). Ideally these operations should not change the
 * lock state, but should be changed to let the caller of the
 * function unlock them. Otherwise all intermediate vnode layers
 * (such as union, umapfs, etc) must catch these functions to do
 * the necessary locking at their layer.
 *
 *
 * INSTANTIATING VNODE STACKS
 *
 * Mounting associates the snoopfs layer with a lower layer,
 * effect stacking two VFSes.  Vnode stacks are instead
 * created on demand as files are accessed.
 *
 * The initial mount creates a single vnode stack for the
 * root of the new snoopfs layer.  All other vnode stacks
 * are created as a result of vnode operations on
 * this or other snoopfs vnode stacks.
 *
 * New vnode stacks come into existance as a result of
 * an operation which returns a vnode.
 * The bypass routine stacks a snoopfs-node above the new
 * vnode before returning it to the caller.
 *
 * For example, imagine mounting a snoopfs layer with
 * "mount_snoopfs /usr/include /dev/layer/snoopfs".
 * Changing directory to /dev/layer/snoopfs will assign
 * the root snoopfs-node (which was created when the snoopfs layer was mounted).
 * Now consider opening "sys".  A vop_lookup would be
 * done on the root snoopfs-node.  This operation would bypass through
 * to the lower layer which would return a vnode representing
 * the UFS "sys".  Wrapfs_bypass then builds a snoopfs-node
 * aliasing the UFS "sys" and returns this to the caller.
 * Later operations on the snoopfs-node "sys" will repeat this
 * process when constructing other vnode stacks.
 *
 *
 * CREATING OTHER FILE SYSTEM LAYERS
 *
 * One of the easiest ways to construct new filesystem layers is to make
 * a copy of the snoopfs layer, rename all files and variables, and
 * then begin modifing the copy.  Sed can be used to easily rename
 * all variables.
 *
 * The umap layer is an example of a layer descended from the
 * snoopfs layer.
 *
 *
 * INVOKING OPERATIONS ON LOWER LAYERS
 *
 * There are two techniques to invoke operations on a lower layer
 * when the operation cannot be completely bypassed.  Each method
 * is appropriate in different situations.  In both cases,
 * it is the responsibility of the aliasing layer to make
 * the operation arguments "correct" for the lower layer
 * by mapping an vnode arguments to the lower layer.
 *
 * The first approach is to call the aliasing layer's bypass routine.
 * This method is most suitable when you wish to invoke the operation
 * currently being handled on the lower layer.  It has the advantage
 * that the bypass routine already must do argument mapping.
 * An example of this is snoopfs_getattrs in the snoopfs layer.
 *
 * A second approach is to directly invoke vnode operations on
 * the lower layer with the VOP_OPERATIONNAME interface.
 * The advantage of this method is that it is easy to invoke
 * arbitrary operations on the lower layer.  The disadvantage
 * is that vnode arguments must be manualy mapped.
 *
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* HAVE_CONFIG_H */
#ifdef FISTGEN
# include "fist_snoopfs.h"
#endif /* FISTGEN */
#include <fist.h>
#include <snoopfs.h>
//needed to get HW3 to work. We are using them to 
//gain access to all of the data structures that we will need 
#include <ufs/ufs/quota.h>
#include <ufs/ufs/inode.h>
#include <sys/types.h>
#include <sys/uio.h>

#if 0
#include "opt_debug.h"

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/conf.h>
#include <sys/kernel.h>
#include <sys/lock.h>
#include <sys/malloc.h>
#include <sys/mount.h>
#include <sys/mutex.h>
#include <sys/namei.h>
#include <sys/sysctl.h>
#include <sys/vnode.h>

#include <fs/snoopfs/snoopfs.h>

#include <vm/vm.h>
#include <vm/vm_extern.h>
#include <vm/vm_object.h>/l
#include <vm/vnode_pager.h>
#include <snoopfs.h>
#endif

#define FIST_COHERENCY 1

static int snoopfs_bug_bypass = 0;   /* for debugging: enables bypass printf'ing */
SYSCTL_INT(_debug, OID_AUTO, snoopfs_bug_bypass, CTLFLAG_RW,
	&snoopfs_bug_bypass, 0, "");

static int	snoopfs_access(struct vop_access_args *ap);
static int	snoopfs_createvobject(struct vop_createvobject_args *ap);
static int	snoopfs_destroyvobject(struct vop_destroyvobject_args *ap);
static int	snoopfs_getattr(struct vop_getattr_args *ap);
static int	snoopfs_getvobject(struct vop_getvobject_args *ap);
static int	snoopfs_inactive(struct vop_inactive_args *ap);
static int	snoopfs_islocked(struct vop_islocked_args *ap);
static int	snoopfs_lock(struct vop_lock_args *ap);
static int	snoopfs_lookup(struct vop_lookup_args *ap);
static int	snoopfs_open(struct vop_open_args *ap);
static int	snoopfs_print(struct vop_print_args *ap);
static int	snoopfs_reclaim(struct vop_reclaim_args *ap);
static int	snoopfs_rename(struct vop_rename_args *ap);
static int	snoopfs_setattr(struct vop_setattr_args *ap);
static int	snoopfs_unlock(struct vop_unlock_args *ap);
/* adding system call to snoopfs*/
static int  snoopfs_read(struct vop_read_args *ap);
static int  snoopfs_write(struct vop_write_args *ap);

/*
 * This is the 10-Apr-92 bypass routine.
 *    This version has been optimized for speed, throwing away some
 * safety checks.  It should still always work, but it's not as
 * robust to programmer errors.
 *
 * In general, we map all vnodes going down and unmap them on the way back.
 * As an exception to this, vnodes can be marked "unmapped" by setting
 * the Nth bit in operation's vdesc_flags.
 *
 * Also, some BSD vnode operations have the side effect of vrele'ing
 * their arguments.  With stacking, the reference counts are held
 * by the upper node, not the lower one, so we must handle these
 * side-effects here.  This is not of concern in Sun-derived systems
 * since there are no such side-effects.
 *
 * This makes the following assumptions:
 * - only one returned vpp
 * - no INOUT vpp's (Sun's vop_open has one of these)
 * - the vnode operation vector of the first vnode should be used
 *   to determine what implementation of the op should be invoked
 * - all mapped vnodes are of our vnode-type (NEEDSWORK:
 *   problems on rmdir'ing mount points and renaming?)
 */
int
snoopfs_bypass(ap)
	struct vop_generic_args /* {
		struct vnodeop_desc *a_desc;
		<other random data follows, presumably>
	} */ *ap;
{
  register struct vnode **this_vp_p;
  int error;
  struct vnode *old_vps[VDESC_MAX_VPS];
  struct vnode **vps_p[VDESC_MAX_VPS];
  struct vnode ***vppp;
  struct vnodeop_desc *descp = ap->a_desc;
  int reles, i;

  fist_dprint(4, "FXN=%s FILE=%s LINE=%d FOR=%s\n",
	      __FUNCTION__,__FILE__,__LINE__,descp->vdesc_name);

  if (snoopfs_bug_bypass)
    fist_dprint(5, "snoopfs_bypass: %s\n", descp->vdesc_name);

#ifdef DIAGNOSTIC
  /*
   * We require at least one vp.
   */
  if (descp->vdesc_vp_offsets == NULL ||
      descp->vdesc_vp_offsets[0] == VDESC_NO_OFFSET)
    panic ("snoopfs_bypass: no vp's in map");
#endif

  /*
   * Map the vnodes going in.
   * Later, we'll invoke the operation based on
   * the first mapped vnode's operation vector.
   */
  reles = descp->vdesc_flags;
  for (i = 0; i < VDESC_MAX_VPS; reles >>= 1, i++) {
    if (descp->vdesc_vp_offsets[i] == VDESC_NO_OFFSET)
      break;   /* bail out at end of list */
    vps_p[i] = this_vp_p =
      VOPARG_OFFSETTO(struct vnode**,descp->vdesc_vp_offsets[i],ap);
    /*
     * We're not guaranteed that any but the first vnode
     * are of our type.  Check for and don't map any
     * that aren't.  (We must always map first vp or vclean fails.)
     */
    if (i && (*this_vp_p == NULLVP ||
		    (*this_vp_p)->v_op != snoopfs_vnodeop_p)) {
      old_vps[i] = NULLVP;
    } else {
      old_vps[i] = *this_vp_p;
      *(vps_p[i]) = SNOOPFS_VP_TO_LOWERVP(*this_vp_p);
      /*
       * XXX - Several operations have the side effect
       * of vrele'ing their vp's.  We must account for
       * that.  (This should go away in the future.)
       */
      if (reles & VDESC_VP0_WILLRELE)
	VREF(*this_vp_p);
    }

  }

  /*
   * Call the operation on the lower layer
   * with the modified argument structure.
   */
  if (vps_p[0] && *vps_p[0])
    error = VCALL(*(vps_p[0]), descp->vdesc_offset, ap);
	else {
	  printf("snoopfs_bypass: no map for %s\n", descp->vdesc_name);
	  error = EINVAL;
	}

  /*
   * Maintain the illusion of call-by-value
   * by restoring vnodes in the argument structure
   * to their original value.
   */
  reles = descp->vdesc_flags;
  for (i = 0; i < VDESC_MAX_VPS; reles >>= 1, i++) {
    if (descp->vdesc_vp_offsets[i] == VDESC_NO_OFFSET)
      break;   /* bail out at end of list */
    if (old_vps[i]) {
      *(vps_p[i]) = old_vps[i];
#if 0
      if (reles & VDESC_VP0_WILLUNLOCK)
	VOP_UNLOCK(*(vps_p[i]), LK_THISLAYER, curthread);
#endif
      if (reles & VDESC_VP0_WILLRELE)
	vrele(*(vps_p[i]));
    }
  }

  /*
   * Map the possible out-going vpp
   * (Assumes that the lower layer always returns
   * a VREF'ed vpp unless it gets an error.)
   */
  if (descp->vdesc_vpp_offset != VDESC_NO_OFFSET &&
      !(descp->vdesc_flags & VDESC_NOMAP_VPP) &&
      !error) {
    /*
     * XXX - even though some ops have vpp returned vp's,
     * several ops actually vrele this before returning.
     * We must avoid these ops.
     * (This should go away when these ops are regularized.)
     */
    if (descp->vdesc_flags & VDESC_VPP_WILLRELE)
      goto out;
    vppp = VOPARG_OFFSETTO(struct vnode***,
			   descp->vdesc_vpp_offset,ap);
    if (*vppp)
      error = snoopfs_nodeget(old_vps[0]->v_mount, **vppp, *vppp);
  }

 out:
  print_location();
  return (error);
}

/*
 * We have to carry on the locking protocol on the snoopfs layer vnodes
 * as we progress through the tree. We also have to enforce read-only
 * if this layer is mounted read-only.
 */
static int
snoopfs_lookup(ap)
	struct vop_lookup_args /* {
		struct vnode * a_dvp;
		struct vnode ** a_vpp;
		struct componentname * a_cnp;
	} */ *ap;
{
  struct componentname *cnp = ap->a_cnp;
  struct vnode *dvp = ap->a_dvp;
  struct thread *td = cnp->cn_thread;
  int flags = cnp->cn_flags;
  struct vnode *vp, *ldvp, *lvp;
  int error;
  CNP_VARS;

  if ((flags & ISLASTCN) && (dvp->v_mount->mnt_flag & MNT_RDONLY) &&
      (cnp->cn_nameiop == DELETE || cnp->cn_nameiop == RENAME))
    return (EROFS);
  /*
   * Although it is possible to call snoopfs_bypass(), we'll do
   * a direct call to reduce overhead
   */
  ldvp = SNOOPFS_VP_TO_LOWERVP(dvp);
  vp = lvp = NULL;

  CNP_BEFORE( ap->a_dvp);

  error = VOP_LOOKUP(ldvp, &lvp, cnp);

  CNP_AFTER(ap->a_dvp);


  if (error == EJUSTRETURN && (flags & ISLASTCN) &&
      (dvp->v_mount->mnt_flag & MNT_RDONLY) &&
      (cnp->cn_nameiop == CREATE || cnp->cn_nameiop == RENAME))
    error = EROFS;

  /*
   * Rely only on the PDIRUNLOCK flag which should be carefully
   * tracked by underlying filesystem.
   */
  if (cnp->cn_flags & PDIRUNLOCK)
    VOP_UNLOCK(dvp, LK_THISLAYER, td);
  if ((error == 0 || error == EJUSTRETURN) && lvp != NULL) {
    if (ldvp == lvp) {
      *ap->a_vpp = dvp;
      VREF(dvp);
      vrele(lvp);
    } else {
      error = snoopfs_nodeget(dvp->v_mount, lvp, &vp);
      if (error) {
	/* XXX Cleanup needed... */
	printf("snoopfs_nodeget failed");
      }
      *ap->a_vpp = vp;
    }
  }

  print_location();
  return (error);
}

/*
 * Setattr call. Disallow write attempts if the layer is mounted read-only.
 */
static int
snoopfs_setattr(ap)
	struct vop_setattr_args /* {
		struct vnodeop_desc *a_desc;
		struct vnode *a_vp;
		struct vattr *a_vap;
		struct ucred *a_cred;
		struct thread *a_td;
	} */ *ap;
{
  struct vnode *vp = ap->a_vp;
  struct vattr *vap = ap->a_vap;
  int error = 0;

  fist_dprint(4, "FXN=%s FILE=%s LINE=%d\n",__FUNCTION__,__FILE__,__LINE__);

  if ((vap->va_flags != VNOVAL || vap->va_uid != (uid_t)VNOVAL ||
       vap->va_gid != (gid_t)VNOVAL || vap->va_atime.tv_sec != VNOVAL ||
       vap->va_mtime.tv_sec != VNOVAL || vap->va_mode != (mode_t)VNOVAL) &&
      (vp->v_mount->mnt_flag & MNT_RDONLY))
    return (EROFS);
  if (vap->va_size != VNOVAL) {
    switch (vp->v_type) {
    case VDIR:
      return (EISDIR);
    case VCHR:
    case VBLK:
    case VSOCK:
    case VFIFO:
      if (vap->va_flags != VNOVAL)
	return (EOPNOTSUPP);
      return (0);
    case VREG:
    case VLNK:
    default:
      /*
       * Disallow write attempts if the filesystem is
       * mounted read-only.
       */
      if (vp->v_mount->mnt_flag & MNT_RDONLY)
	return (EROFS);
    }
  }
  print_location();
  return (snoopfs_bypass((struct vop_generic_args *)ap));
}

/*
 *  We handle getattr only to change the fsid.
 */
static int
snoopfs_getattr(ap)
	struct vop_getattr_args /* {
		struct vnode *a_vp;
		struct vattr *a_vap;
		struct ucred *a_cred;
		struct thread *a_td;
	} */ *ap;
{
  int error;
  vnode_t *vp = ap->a_vp;

  fist_dprint(4, "FXN=%s FILE=%s LINE=%d\n",__FUNCTION__,__FILE__,__LINE__);

  if ((error = snoopfs_bypass((struct vop_generic_args *)ap)) != 0)
    return (error);

  ap->a_vap->va_fsid = ap->a_vp->v_mount->mnt_stat.f_fsid.val[0];
  print_location();
  return (0);
}

/*
 * Handle to disallow write access if mounted read-only.
 */
static int
snoopfs_access(ap)
	struct vop_access_args /* {
		struct vnode *a_vp;
		int  a_mode;
		struct ucred *a_cred;
		struct thread *a_td;
	} */ *ap;
{
  struct vnode *vp = ap->a_vp;
  mode_t mode = ap->a_mode;

  fist_dprint(4, "FXN=%s FILE=%s LINE=%d\n",__FUNCTION__,__FILE__,__LINE__);
  /*
   * Disallow write attempts on read-only layers;
   * unless the file is a socket, fifo, or a block or
   * character device resident on the filesystem.
   */
  if (mode & VWRITE) {
    switch (vp->v_type) {
    case VDIR:
    case VLNK:
    case VREG:
      if (vp->v_mount->mnt_flag & MNT_RDONLY)
	return (EROFS);
      break;
    default:
      break;
    }
  }

  print_location();
  return (snoopfs_bypass((struct vop_generic_args *)ap));
}

/*
 * We must handle open to be able to catch MNT_NODEV and friends.
 */
static int
snoopfs_open(ap)
	struct vop_open_args /* {
		struct vnode *a_vp;
		int  a_mode;
		struct ucred *a_cred;
		struct thread *a_td;
	} */ *ap;
{
  struct vnode *vp = ap->a_vp;
  struct vnode *lvp = SNOOPFS_VP_TO_LOWERVP(ap->a_vp);

  fist_dprint(4, "FXN=%s FILE=%s LINE=%d\n",__FUNCTION__,__FILE__,__LINE__);

  if ((vp->v_mount->mnt_flag & MNT_NODEV) &&
      (lvp->v_type == VBLK || lvp->v_type == VCHR))
    return ENXIO;

  print_location();
  return (snoopfs_bypass((struct vop_generic_args *)ap));
}


/*
 * We need to process our own vnode lock and then clear the
 * interlock flag as it applies only to our vnode, not the
 * vnodes below us on the stack.
 */
static int
snoopfs_lock(ap)
	struct vop_lock_args /* {
		struct vnode *a_vp;
		int a_flags;
		struct thread *a_td;
	} */ *ap;
{
  struct vnode *vp = ap->a_vp;
  int flags = ap->a_flags;
  struct thread *td = ap->a_td;
  struct vnode *lvp;
  int error;

  fist_dprint(4, "FXN=%s FILE=%s LINE=%d\n",__FUNCTION__,__FILE__,__LINE__);

  if (flags & LK_THISLAYER) {
    if (vp->v_vnlock != NULL) {
      /* lock is shared across layers */
      if (flags & LK_INTERLOCK)
	mtx_unlock(&vp->v_interlock);
      return 0;
    }
    error = lockmgr(&vp->v_lock, flags & ~LK_THISLAYER,
		    &vp->v_interlock, td);
    return (error);
  }

  if (vp->v_vnlock != NULL) {
    /*
     * The lower level has exported a struct lock to us. Use
     * it so that all vnodes in the stack lock and unlock
     * simultaneously. Note: we don't DRAIN the lock as DRAIN
     * decommissions the lock - just because our vnode is
     * going away doesn't mean the struct lock below us is.
     * LK_EXCLUSIVE is fine.
     */
    if ((flags & LK_TYPE_MASK) == LK_DRAIN) {
      fist_dprint(2, "snoopfs_lock: avoiding LK_DRAIN\n");
      return(lockmgr(vp->v_vnlock,
		     (flags & ~LK_TYPE_MASK) | LK_EXCLUSIVE,
				&vp->v_interlock, td));
    }
    return(lockmgr(vp->v_vnlock, flags, &vp->v_interlock, td));
  } else {
    /*
     * To prevent race conditions involving doing a lookup
     * on "..", we have to lock the lower node, then lock our
     * node. Most of the time it won't matter that we lock our
     * node (as any locking would need the lower one locked
     * first). But we can LK_DRAIN the upper lock as a step
     * towards decomissioning it.
     */
    lvp = SNOOPFS_VP_TO_LOWERVP(vp);
    if (lvp == NULL)
      return (lockmgr(&vp->v_lock, flags, &vp->v_interlock, td));
    if (flags & LK_INTERLOCK) {
      mtx_unlock(&vp->v_interlock);
      flags &= ~LK_INTERLOCK;
    }
    if ((flags & LK_TYPE_MASK) == LK_DRAIN) {
      error = VOP_LOCK(lvp,
		       (flags & ~LK_TYPE_MASK) | LK_EXCLUSIVE, td);
    } else
      error = VOP_LOCK(lvp, flags, td);
    if (error)
      return (error);
    error = lockmgr(&vp->v_lock, flags, &vp->v_interlock, td);
    if (error)
      VOP_UNLOCK(lvp, 0, td);
   print_location();
    return (error);
  }
}

/*
 * We need to process our own vnode unlock and then clear the
 * interlock flag as it applies only to our vnode, not the
 * vnodes below us on the stack.
 */
static int
snoopfs_unlock(ap)
	struct vop_unlock_args /* {
		struct vnode *a_vp;
		int a_flags;
		struct thread *a_td;
	} */ *ap;
{
  struct vnode *vp = ap->a_vp;
  int flags = ap->a_flags;
  struct thread *td = ap->a_td;
  struct vnode *lvp;

  fist_dprint(4, "FXN=%s FILE=%s LINE=%d\n",__FUNCTION__,__FILE__,__LINE__);
  if (vp->v_vnlock != NULL) {
    if (flags & LK_THISLAYER)
      return 0;	/* the lock is shared across layers */
    flags &= ~LK_THISLAYER;
    return (lockmgr(vp->v_vnlock, flags | LK_RELEASE,
		    &vp->v_interlock, td));
  }
  lvp = SNOOPFS_VP_TO_LOWERVP(vp);
  if (lvp == NULL)
    return (lockmgr(&vp->v_lock, flags | LK_RELEASE, &vp->v_interlock, td));
  if ((flags & LK_THISLAYER) == 0) {
    if (flags & LK_INTERLOCK) {
      mtx_unlock(&vp->v_interlock);
      flags &= ~LK_INTERLOCK;
    }
    VOP_UNLOCK(lvp, flags & ~LK_INTERLOCK, td);
  } else
    flags &= ~LK_THISLAYER;
  print_location();
  return (lockmgr(&vp->v_lock, flags | LK_RELEASE, &vp->v_interlock, td));
}

static int
snoopfs_islocked(ap)
	struct vop_islocked_args /* {
		struct vnode *a_vp;
		struct thread *a_td;
	} */ *ap;
{
  struct vnode *vp = ap->a_vp;
  struct thread *td = ap->a_td;

  fist_dprint(4, "FXN=%s FILE=%s LINE=%d\n",__FUNCTION__,__FILE__,__LINE__);

  if (vp->v_vnlock != NULL)
    return (lockstatus(vp->v_vnlock, td));
  print_location();
  return (lockstatus(&vp->v_lock, td));
}

/*
 * There is no way to tell that someone issued remove/rmdir operation
 * on the underlying filesystem. For now we just have to release lowevrp
 * as soon as possible.
 *
 * Note, we can't release any resources nor remove vnode from hash before
 * appropriate VXLOCK stuff is is done because other process can find this
 * vnode in hash during inactivation and may be sitting in vget() and waiting
 * for snoopfs_inactive to unlock vnode. Thus we will do all those in VOP_RECLAIM.
 */
static int
snoopfs_inactive(ap)
	struct vop_inactive_args /* {
		struct vnode *a_vp;
		struct thread *a_td;
	} */ *ap;
{
  struct vnode *vp = ap->a_vp;
  struct thread *td = ap->a_td;

  fist_dprint(4, "FXN=%s FILE=%s LINE=%d\n",__FUNCTION__,__FILE__,__LINE__);

  VOP_UNLOCK(vp, 0, td);

  /*
   * If this is the last reference, then free up the vnode
   * so as not to tie up the lower vnodes.
   */
  vrecycle(vp, NULL, td);
  print_location();
  return (0);
}

/*
 * Now, the VXLOCK is in force and we're free to destroy the snoopfs vnode.
 */
static int
snoopfs_reclaim(ap)
	struct vop_reclaim_args /* {
		struct vnode *a_vp;
		struct thread *a_td;
	} */ *ap;
{
  struct vnode *vp = ap->a_vp;
  struct snoopfs_node *xp = VP_TO_SNOOPFS(vp);
  struct vnode *lowervp = xp->snoopfs_lowervp;

  fist_dprint(4, "FXN=%s FILE=%s LINE=%d\n",__FUNCTION__,__FILE__,__LINE__);
  if (lowervp) {
    snoopfs_hashrem(xp);

    vrele(lowervp);
    vrele(lowervp);
  }

  vp->v_data = NULL;
  vp->v_vnlock = &vp->v_lock;
  FREE(xp, M_SNOOPFSNODE);

  print_location();
  return (0);
}

static int
snoopfs_print(ap)
	struct vop_print_args /* {
		struct vnode *a_vp;
	} */ *ap;
{
  register struct vnode *vp = ap->a_vp;
  printf("\ttag %s, vp=%p, lowervp=%p\n", vp->v_tag, vp,
	 SNOOPFS_VP_TO_LOWERVP(vp));
  printf("\n");
  return (0);
}

/* let the underlying filesystem do the work */
static int
snoopfs_createvobject(ap)
     struct vop_createvobject_args /* {
				      struct vnode *vp;
				      struct ucred *cred;
				      struct thread *td;
				      } */ *ap;
{
  struct vnode *vp = ap->a_vp;
  struct vnode *lowervp = VP_TO_SNOOPFS(vp) ? SNOOPFS_VP_TO_LOWERVP(vp) : NULL;
  int error = 0;

  fist_dprint(4, "FXN=%s FILE=%s LINE=%d\n",__FUNCTION__,__FILE__,__LINE__);

  if (vp->v_type == VNON || lowervp == NULL) {
    goto retn; /* returns 0! */
  }
  error = VOP_CREATEVOBJECT(lowervp, ap->a_cred, ap->a_td);
  if (error)
    goto retn;
  vp->v_vflag |= VV_OBJBUF;
retn:
  print_location();
  return (error);
}

/* do nothing */
static int
snoopfs_destroyvobject(ap)
     struct vop_destroyvobject_args /* {
                                       struct vnode *vp;
                                       } */ *ap;
{
  int error = 0;
  struct vnode *vp = ap->a_vp;

  fist_dprint(4, "FXN=%s FILE=%s LINE=%d\n",__FUNCTION__,__FILE__,__LINE__);

  vp->v_vflag &= ~VV_OBJBUF;

  print_location();
  return (error);
}


static int
snoopfs_getvobject(ap)
     struct vop_getvobject_args /* {
				   struct vnode *vp;
				   struct vm_object **objpp;
				   } */ *ap;
{
  struct vnode *lvp = SNOOPFS_VP_TO_LOWERVP(ap->a_vp);

  fist_dprint(4, "FXN=%s FILE=%s LINE=%d\n",__FUNCTION__,__FILE__,__LINE__);
  if (lvp == NULL)
    return EINVAL;
  print_location();
  return (VOP_GETVOBJECT(lvp, ap->a_objpp));
}

/****************************************************************************/


static int
snoopfs_ioctl(ap)
     struct vop_ioctl_args /* {
			      struct vnode *a_vp;
			      u_long  a_command;
			      caddr_t  a_data;
			      int  a_fflag;
			      struct ucred *a_cred;
			      struct thread *a_td;
			      } */ *ap;
{
  int error = 0;
  struct vnode *vp = ap->a_vp;
  vnode_t *this_vnode = ap->a_vp;
  struct vnode *lowervp = SNOOPFS_VP_TO_LOWERVP(vp);
  int val = 0;
  caddr_t arg = ap->a_data;
  cred_t *cr = curthread->td_proc->p_ucred;  /*HARI */
  vfs_t *this_vfs = this_vnode->v_mount;

  fist_dprint(4, "SNOOPFS_IOCTL: vp=0x%x, lowervp=0x%x\n", (int) vp, (int) lowervp);

  switch (ap->a_command) {

    /* add non-debugging fist ioctl's here */


  default:

    /* default to repeating ioctl on lowervp */
    error = VOP_IOCTL(lowervp,
		      ap->a_command,
		      ap->a_data,
		      ap->a_fflag,
		      ap->a_cred,
		      ap->a_td);       /*HARI */

  } /* end of switch statement */

out:
  print_location();
  return (error);
}


#ifdef HAVE_BMAP
static int
snoopfs_bmap(ap)
     struct vop_bmap_args /* {
			     struct vnode *a_vp;
			     daddr_t a_bn;
			     struct vnode **a_vpp;
			     daddr_t *a_bnp;
			     int *a_runp;
			     int *a_runb;
			     } */ *ap;
{
  int error = 0;
  struct vnode *vp = ap->a_vp;
  struct vnode *lowervp = SNOOPFS_VP_TO_LOWERVP(vp);


  fist_dprint(4, "SNOOPFS_BMAP:0x%x:0x%x\n", (int) ap->a_vpp, (int) lowervp);
  printf("SNOOPFS_BMAP:0x%x:0x%x\n", (int) ap->a_vpp, (int) lowervp);


  if (error)
    return error;

#if 0
  ap->a_vp = lowervp;
  error = VCALL(lowervp, VOFFSET(vop_bmap), ap);
  return (error);
#endif

#if 1
  error = snoopfs_bypass((struct vop_generic_args *) ap);
  return (error);
#endif

#if 0
  /* this code also doesn't panic */
  if (ap->a_vpp != NULL)
    *ap->a_vpp = ap->a_vp;
  if (ap->a_bnp != NULL)
    *ap->a_bnp = ap->a_bn;
  if (ap->a_runp != NULL)
    *ap->a_runp = 0;
  if (ap->a_runb != NULL)
    *ap->a_runb = 0;
  return (error);
#endif

#if 0
  error = EINVAL;
  return (error);
#endif
}
#endif /* HAVE_BMAP */

static int
snoopfs_mmap(ap)
     struct vop_mmap_args /* {
			     struct vnode *a_vp;
			     int  a_fflags;
			     struct ucred *a_cred;
			     struct thread *a_td;
			     } */ *ap;
{
  return EINVAL;
}

int snoopfs_read(ap) struct vop_read_args *ap;
{
     /*Get stuctures that we will be using*/
    struct vnode *v_ptr     = ap->a_vp;
    struct vnode  *lowervp  = VP_TO_SNOOPFS(v_ptr)->snoopfs_lowervp;
    struct inode *i_ptr     = (struct inode*)(lowervp->v_data);

    /* Getting all the values we need to display*/
    int mode            = ap->a_uio->uio_rw;
    int i_node_number   = i_ptr->i_number;
    int block_number    = (int)(v_ptr->v_mount->mnt_stat).f_bsize;
    int number_of_bytes = (int)ap->a_uio->uio_offset;
    
    /*Determins if anything was read, if so we print*/
    if((int)ap->a_uio->uio_offset!=0)
    {
        /*Print out data*/
        printf("%d::%d::%d::%d\n",mode,i_node_number,block_number,number_of_bytes);
    }

    return (snoopfs_bypass((struct vop_generic_args*)ap));
}


static int snoopfs_write(ap) struct vop_write_args *ap;
{
    /*Get structures that we will be using*/
    struct vnode *v_ptr     = ap->a_vp;
    struct vnode *lowervp   = VP_TO_SNOOPFS(v_ptr)->snoopfs_lowervp;
    struct inode *i_ptr     = (struct inode *)(lowervp->v_data);

     /* Getting all the values we need to display*/
    int mode            = ap->a_uio->uio_rw;
    int i_node_number   = i_ptr->i_number;
    int block_number    = (int)(v_ptr->v_mount->mnt_stat).f_bsize;
    int number_of_bytes = (int)ap->a_uio->uio_offset  + (int)ap->a_uio->uio_iov->iov_len;

    /*Determins if anything was written, if so we display*/
    if ((int)ap->a_uio->uio_iovcnt != 0) 
    {
        /*Print out data*/
        printf("%d::%d::%d::%d\n",mode,i_node_number,block_number,number_of_bytes);
    }
    return (snoopfs_bypass((struct vop_generic_args *)ap));
}
/*
 * Global vfs data structures
 */
vop_t **snoopfs_vnodeop_p;
static struct vnodeopv_entry_desc snoopfs_vnodeop_entries[] =
{
  {&vop_default_desc,		(vop_t *) snoopfs_bypass},
  {&vop_access_desc,		(vop_t *) snoopfs_access},
  {&vop_createvobject_desc,	(vop_t *) snoopfs_createvobject },
  {&vop_destroyvobject_desc,	(vop_t *) snoopfs_destroyvobject },
  {&vop_getattr_desc,		(vop_t *) snoopfs_getattr},
  {&vop_getvobject_desc,	(vop_t *) snoopfs_getvobject },
  {&vop_inactive_desc,		(vop_t *) snoopfs_inactive},
  {&vop_lock_desc,		(vop_t *) snoopfs_lock},
  {&vop_islocked_desc,          (vop_t *) snoopfs_islocked},
  {&vop_lookup_desc,		(vop_t *) vfs_cache_lookup}, /* FIST_FILTER_NAME */
  {&vop_print_desc,		(vop_t *) snoopfs_print},
  {&vop_reclaim_desc,		(vop_t *) snoopfs_reclaim},
  {&vop_setattr_desc,		(vop_t *) snoopfs_setattr},
  {&vop_unlock_desc,		(vop_t *) snoopfs_unlock},

  /*HW 3 addions for system calls*/
  /* Added for the read function*/
  
  /*Added for the write function*/

  /* EZK added these */
  { &vop_ioctl_desc,		(vop_t *) snoopfs_ioctl },
#ifdef HAVE_BMAP
  { &vop_bmap_desc,		(vop_t *) snoopfs_bmap },
#endif /* HAVE_BMAP */
  /* KIRAN added this */
  { &vop_open_desc,		(vop_t *) snoopfs_open },
  { &vop_read_desc, (vop_t *) snoopfs_read },
  { &vop_write_desc, (vop_t *) snoopfs_write },
  {NULL, NULL}
};



static struct vnodeopv_desc snoopfs_vnodeop_opv_desc =
	{ &snoopfs_vnodeop_p, snoopfs_vnodeop_entries };

VNODEOP_SET(snoopfs_vnodeop_opv_desc);
