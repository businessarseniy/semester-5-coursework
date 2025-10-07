#include "kernel/filesystem/vfs.h"


static void init(void){
    // Init chosen filesystem
}


Vfs_t vfs = {
    .init = &init,
};
