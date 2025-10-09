#pragma once

#include <stdint.h>


// size = 1 sector, lba = 0
typedef struct {
    uint8_t start_code[3];
    uint8_t identifier[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t fats_count;
    uint16_t root_dir_entries_count;
    uint16_t total_sectors;
    uint8_t mdt;
    uint16_t sectors_per_fat;
    uint16_t sectors_per_track;
    uint16_t heads_count;
    uint32_t hidden_sectors;
    uint32_t large_sector_count;
}__attribute__((packed)) fat32_bios_parameter_block_t;

typedef struct {
    uint32_t sectors_per_fat;
    uint16_t flags;
    uint16_t version;
    uint32_t root_directory_cluster;
    uint16_t fsinfo_sector;
    uint16_t backup_boot_sector;
    uint8_t __reserved[12];
    uint8_t drive_number;
    uint8_t nt_flags;
    uint8_t signature;
    uint32_t serial_id;
    uint8_t label[11];
    uint8_t system_id[8];
    uint8_t boot_code[420];
    uint16_t boot_signature;
}__attribute__((packed)) fat32_extended_boot_record_t;

typedef struct {
    fat32_bios_parameter_block_t bpb;
    fat32_extended_boot_record_t extended;
}__attribute__((packed)) fat32_boot_record_t;

typedef struct {
    uint32_t signature_major;
    uint8_t __reserved0[480];
    uint32_t signature_minor;
    uint32_t free_clusters;
    uint32_t lookup_cluster;
    uint8_t __reserved1[12];
    uint32_t signature_trail;
}__attribute__((packed)) fat32_fsinfo_t;

enum {
    FAT32_ATTRIBUTE_RO=0x01,
    FAT32_ATTRIBUTE_HIDDEN=0x02,
    FAT32_ATTRIBUTE_SYSTEM=0x04,
    FAT32_ATTRIBUTE_VOLUME_ID=0x08,
    FAT32_ATTRIBUTE_DIRECTORY=0x10,
    FAT32_ATTRIBUTE_ARCHIVE=0x20,
    FAT32_ATTRIBUTE_LFN=0xf,
};

typedef struct {
    uint8_t filename[11];
    uint8_t attributes;
    uint8_t __reserved;
    uint8_t __wtf;
    uint16_t creation_time;
    uint16_t creation_date;
    uint16_t access_date;
    uint16_t first_cluster_high;
    uint16_t modify_time;
    uint16_t modify_date;
    uint16_t first_cluster_low;
    uint32_t filesize;
}__attribute__((packed)) fat32_directory_entry_t;

typedef struct {
    int first_data_sector;
    int root_dir_sector;
} fat32_fs_t;

typedef struct {
    void* buffer;
    int size;
    int pos;
    int fd;
    // int mode?
} fat32_file_t;

typedef struct {
    void (*init)(void);
} Fat32_t;


extern Fat32_t fat32;
