#include "kernel/filesystem/fat32.h"
#include "kernel/filesystem/vfs.h"


static void init(void){
    // Init chosen filesystem
    fat32.init();
    vfs.open = fat32.open;
    vfs.read = fat32.read;
    vfs.close = fat32.close;
}


Vfs_t vfs = {
    .init = &init,
};
