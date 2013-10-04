/*
	Genie/FS/sys/mac/vol/list.hh
	----------------------------
*/

#ifndef GENIE_FS_SYS_MAC_VOL_LIST_HH
#define GENIE_FS_SYS_MAC_VOL_LIST_HH

// Nitrogen
#ifndef MAC_FILES_TYPES_FSVOLUMEREFNUM_HH
#include "Mac/Files/Types/FSVolumeRefNum.hh"
#endif

// plus
#include "plus/string.hh"

// vfs
#include "vfs/node_ptr.hh"


namespace Genie
{
	
	vfs::node_ptr New_FSTree_sys_mac_vol( const vfs::node*     parent,
	                                      const plus::string&  name,
	                                      const void*          args );
	
	vfs::node_ptr Get_sys_mac_vol_N( Mac::FSVolumeRefNum vRefNum );
	
}

#endif

