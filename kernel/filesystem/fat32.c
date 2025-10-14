#include "kernel/filesystem/fat32.h"
#include "kernel/hardware/pio.h"
#include "kernel/memory/heap.h"
#include "kernel/tty.h"


static fat32_fs_t __fs = {};
static fat32_boot_record_t __boot_record = {};
static void* __buffer = (void*)0;
static fat32_file_t* __files[32] = {};


static void memcpy(void* dst, void* src, int size){
    uint8_t* p_dst = dst;
    uint8_t* p_src = src;
    for (int i = 0; i < size; ++i){
        p_dst[i] = p_src[i];
    }
}

static void memset(void* dst, int size, int val){
    uint8_t* p_dst = dst;
    for (int i = 0; i < size; ++i){
        p_dst[i] = (uint8_t)val;
    }
}

static int memcmp(void* a, void* b, int size){
    uint8_t* p_a = a;
    uint8_t* p_b = b;
    for (int i = 0; i < size; ++i){
        if (p_a[i] != p_b[i]) return -1;
    }
    return 0;
}

static int strlen(char* s){
    int result = 0;
    while ('\0' != s[result]){
        ++result;
    }
    return result;
}

static int split(char* s, char delimiter, int start){
    int len = 0;
    while ((start + len) < strlen(s) && s[start + len] && s[start + len] != delimiter){
        len++;
    }
    return len;
}

static int cluster2lba(int cluster){
    return cluster * __boot_record.bpb.sectors_per_cluster * __boot_record.bpb.bytes_per_sector / 512;
}

static void read_boot_record(void){
    fat32_boot_record_t* buffer = halloc.malloc(512);
    pio.read(buffer, 0, 1);
    memcpy(&__boot_record, buffer, sizeof(fat32_boot_record_t));
    halloc.free(buffer);
}

static void read_fat(void){
    int fat_size = __boot_record.bpb.fats_count * __boot_record.extended.sectors_per_fat * __boot_record.bpb.bytes_per_sector / 512;
    tty.printf("FAT size: %d\n", fat_size);
    tty.printf("FAT sector: %d\n", __boot_record.bpb.reserved_sectors);
}

static void read_root_directory(void){
    __fs.first_data_sector = __boot_record.extended.sectors_per_fat * __boot_record.bpb.fats_count + __boot_record.bpb.reserved_sectors;
    __fs.root_dir_sector = __fs.first_data_sector + cluster2lba(__boot_record.extended.root_directory_cluster - 2);
    // __buffer = halloc.malloc(__boot_record.bpb.bytes_per_sector);
    // pio.read(__buffer, __fs.root_dir_sector, 1);
    // fat32_directory_entry_t* root_dir = __buffer;
    // for (int i = 0; root_dir[i].creation_date; ++i){
    //     tty.printf("%s : %d %d\n", root_dir[i].filename, root_dir[i].filesize, root_dir[i].first_cluster_low);
    // }
    // halloc.free(__buffer);
}

// reads entry in directory
static int locate(uint32_t dir_cluster, char* name, char* extension, fat32_directory_entry_t* result){
    int dir_sector = __fs.first_data_sector + cluster2lba(dir_cluster - 2);
    // parse name
    uint8_t filename[11] = {};
    memset(&filename[0], 11, ' ');
    if (strlen(name) + strlen(extension) > 11) return -1;
    memcpy(&filename[0], name, strlen(name));
    int ext_start_index = 11 - strlen(extension);
    memcpy(&filename[ext_start_index], extension, strlen(extension));
    __buffer = halloc.malloc(__boot_record.bpb.bytes_per_sector);
    pio.read(__buffer, dir_sector, 1);
    fat32_directory_entry_t* dir = __buffer;
    int code = -1;
    while (dir->creation_date){
        if (0 == memcmp(&dir->filename[0], &filename[0], 11)){
            memcpy(result, dir, sizeof(fat32_directory_entry_t));
            code = 0;
            break;
        }
        dir++;
    }
    halloc.free(__buffer);
    return code;
}

static int load_file(void** buffer, fat32_directory_entry_t* entry){
    int cluster = entry->first_cluster_low | ((uint32_t)entry->first_cluster_high << 16);
    int lba = __fs.first_data_sector + cluster2lba(cluster - 2);
    int size = entry->filesize;
    *buffer = halloc.malloc(size);
    int offset = 0;
    memset(*buffer, size, 0);
    while (size > 0){
        if (size > __boot_record.bpb.bytes_per_sector){
            pio.read(*buffer + offset, lba, 1);
        } else {
            void* tmp = halloc.malloc(__boot_record.bpb.bytes_per_sector);
            pio.read(tmp, lba, 1);
            memcpy(*buffer + offset, tmp, size);
            halloc.free(tmp);
            return 0;
        }
        uint32_t* fat = halloc.malloc(__boot_record.bpb.bytes_per_sector);
        memset(fat, __boot_record.bpb.bytes_per_sector, 0);
        int fat_sector = cluster / (__boot_record.bpb.bytes_per_sector / 4);
        int fat_offset = cluster % (__boot_record.bpb.bytes_per_sector / 4);
        
        pio.read(fat, __boot_record.bpb.reserved_sectors + fat_sector, 1);
        cluster = fat[fat_offset];
        lba = __fs.first_data_sector + cluster2lba(cluster - 2);
        offset += __boot_record.bpb.bytes_per_sector;
        size -= __boot_record.bpb.bytes_per_sector;
        halloc.free(fat);
    }
    return -1;
}

static void unload_file(void* buffer){
    halloc.free(buffer);
    // clear here fd???
}

static int open(char* path, int flags, int mode){
    // path starts from "/", trim it
    path++;
    for (int i = 3; i < 32; ++i){
        if ((void*)0 != __files[i]) continue;
        // found empty fd
        // split path
        fat32_directory_entry_t entry = {};
        int start = 0;
        int len = 0;
        int code = -1;
        int dir_cluster = __boot_record.extended.root_directory_cluster;
        char name[9] = {};
        char ext[4] = {};
        do {
            memset(&name[0], 9, 0);
            memset(&ext[0], 4, 0);
            int len = split(path, '/', start);
            if (start + len >= strlen(path)){
                // last entry -- file
                int dot = split(path, '.', start);
                memcpy(&name[0], &path[start], dot);
                memcpy(&ext[0], &path[start + dot + 1], len - dot - 1);
                code = locate(dir_cluster, &name[0], &ext[0], &entry);
                code -= (entry.attributes == FAT32_ATTRIBUTE_DIRECTORY);
                break;
            } else {
                // dir
                memcpy(&name[0], &path[start], len);
                code = locate(dir_cluster, name, "", &entry);
                start += len;
                start++;
                dir_cluster = entry.first_cluster_low | ((uint32_t)entry.first_cluster_high << 16);
            }
        } while (code == 0);
        if (0 != code){
            // cant find
            return -1;
        }
        // succesful found
        fat32_file_t* file = halloc.malloc(sizeof(fat32_file_t));
        memset(file, sizeof(fat32_file_t), 0);
        load_file(&file->buffer, &entry);
        file->fd = i;
        file->pos = 0;
        file->size = entry.filesize;
        __files[i] = file;
        // add to current process?
        return i;
    }
    return -1;
}

static int read(int fd, void* buffer, int size){
    if (fd < 0 || fd >= 32) return -1;
    fat32_file_t* file = __files[fd];
    if ((void*)0 == file) return -1;

    int len = file->size - file->pos;
    if (size > len){
        memcpy(buffer, &file->buffer[file->pos], len);
        file->pos = file->size;
        return len;
    }
    memcpy(buffer, &file->buffer[file->pos], size);
    file->pos += size;
    return size;
}

static int close(int fd){
    if (fd < 0 || fd >= 32) return -1;
    fat32_file_t* file = __files[fd];
    if ((void*)0 == file) return -1;

    unload_file(file->buffer);
    halloc.free(file);
    __files[fd] = (void*)0;
    return 0;
}

static void init(void){
    read_boot_record();
    read_root_directory();
}

static int lseek(int fd, int pos, int whence){
    if (fd < 0 || fd >= 32) return -1;
    fat32_file_t* file = __files[fd];
    if ((void*)0 == file) return -1;

    // TODO: security check!!!
    file->pos = pos;
    return 0;
}

Fat32_t fat32 = {
    .init = &init,
    .open = &open,
    .read = &read,
    .close = &close,
    .lseek = &lseek,
};
