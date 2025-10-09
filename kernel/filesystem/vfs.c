#include "kernel/filesystem/fat32.h"
#include "kernel/filesystem/vfs.h"


static void init(void){
    // Init chosen filesystem
    fat32.init();
}


Vfs_t vfs = {
    .init = &init,
};
