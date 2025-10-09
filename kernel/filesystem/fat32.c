#include "kernel/filesystem/fat32.h"
#include "kernel/hardware/pio.h"
#include "kernel/memory/heap.h"
#include "kernel/tty.h"


static fat32_fs_t __fs = {};
static fat32_boot_record_t __boot_record = {};
static void* __buffer = (void*)0;


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
    __buffer = halloc.malloc(__boot_record.bpb.bytes_per_sector);
    pio.read(__buffer, __fs.root_dir_sector, 1);
    fat32_directory_entry_t* root_dir = __buffer;
    for (int i = 0; root_dir[i].creation_date; ++i){
        tty.printf("%s : %d %d\n", root_dir[i].filename, root_dir[i].filesize, root_dir[i].first_cluster_low);
    }
    halloc.free(__buffer);
}

// reads entry in directory
static int locate(uint32_t dir_cluster, char* name, char* extension, fat32_directory_entry_t* result){
    int dir_sector = __fs.first_data_sector + cluster2lba(dir_cluster - 2);
    // parse name
    uint8_t filename[11] = {};
    memset(&filename[0], 11, ' ');
    if (strlen(name) + strlen(extension) > 11) return 0;
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
    memset(*buffer, size, 0);
    if (size > __boot_record.bpb.bytes_per_sector){
        // TODO: test it!!!!!!
        uint32_t* fat = halloc.malloc(__boot_record.bpb.bytes_per_sector);
        memset(fat, 0, __boot_record.bpb.bytes_per_sector);
        int offset = 0;
        // one sector of fat for now
        pio.read(fat, __boot_record.bpb.reserved_sectors, 1);
        while (size > __boot_record.bpb.bytes_per_sector){
            pio.read(*buffer + offset, lba, 1);
            offset += __boot_record.bpb.bytes_per_sector;
            size -= __boot_record.bpb.bytes_per_sector;
            cluster = fat[cluster];
            lba = __fs.first_data_sector + cluster2lba(cluster - 2);
        }
        halloc.free(fat);
        return 0;
    }
    // just read
    pio.read(*buffer, lba, 1);
    // maybe return fd?
    return 0;
}

static void unload_file(void* buffer){
    halloc.free(buffer);
    // clear here fd???
}

static int open(char* path){
    return -1;
}

static int read(int fd, void* buffer, int size){
    return 0;
}

static int close(int fd){
    return 0;
}

static void init(void){
    read_boot_record();
    read_root_directory();
    read_fat();
    fat32_directory_entry_t entry;
    tty.printf("Locate: %d\n", locate(__boot_record.extended.root_directory_cluster, "FOLDER", "", &entry));
    tty.printf("Locate: %d\n", locate(4, "HELLO", "TXT", &entry));
    void* buf;
    load_file(&buf, &entry);
    tty.printf("DATA: ~%s~\n", buf);
}


Fat32_t fat32 = {
    .init = &init,
};
