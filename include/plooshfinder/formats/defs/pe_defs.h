#ifndef _PE_DEFS_H
#define _PE_DEFS_H
#include <stdint.h>

struct DOS_Header {
    char e_magic[0x2];
    uint16_t e_cblp;
    uint16_t e_cp;
    uint16_t e_crlc;
    uint16_t e_cparhdr;
    uint16_t e_minalloc;
    uint16_t e_maxalloc;
    uint16_t e_ss;
    uint16_t e_sp;
    uint16_t e_csum;
    uint16_t e_ip;
    uint16_t e_cs;
    uint16_t e_lfarlc;
    uint16_t e_ovno;
    char e_res1[0x8];
    uint16_t e_oemid;
    uint16_t e_oeminfo;
    char e_res2[0x14];
    uint32_t e_lfanew;
};

struct COFF_Header {
    char magic[0x4];
    uint16_t machine;
    uint16_t numberOfSections;
    uint32_t timeDateStamp;
    uint32_t pointerToSymbolTable;
    uint32_t numberOfSymbols;
    uint16_t sizeOfOptionalHeader;
    uint16_t characteristics;
};

struct Section_Header {
    char name[0x8];
    uint32_t virtualSize;
    uint32_t virtualAddress;
    uint32_t sizeOfRawData;
    uint32_t pointerToRawData;
    uint32_t pointerToRelocations;
    uint32_t pointerToLineNumbers;
    uint16_t numberOfRelocations;
    uint16_t numberOfLineNumbers;
    uint32_t characteristics;
};

struct Symbol_Header {
    union {
        char name[0x8];
        struct {
            uint32_t zeros;
            uint32_t strtab_off;
        } over_8b;
    } name;
    uint32_t value;
    int16_t nsect;
    uint16_t type;
    uint8_t s_class;
    uint8_t n_aux;
};

struct PE_Data_Directory_Entry {
    uint32_t virtualAddress;
    uint32_t size;
};

struct PE64_Optional_Header {
    uint16_t magic;
    uint8_t majorLinkerVersion;
    uint8_t minorLinkerVersion;
    uint32_t sizeOfCode;
    uint32_t sizeOfInitializedData;
    uint32_t sizeOfUninitializedData;
    uint32_t addressOfEntryPoint;
    uint32_t baseOfCode;
    uint64_t imageBase;
    uint32_t sectionAlignment;
    uint32_t fileAlignment;
    uint16_t majorOperatingSystemVersion;
    uint16_t minorOperatingSystemVersion;
    uint16_t majorImageVersion;
    uint16_t minorImageVersion;
    uint16_t majorSubsystemVersion;
    uint16_t minorSubsystemVersion;
    uint32_t win32VersionValue;
    uint32_t sizeOfImage;
    uint32_t sizeOfHeaders;
    uint32_t checkSum;
    uint16_t subsystem;
    uint16_t dllCharacteristics;
    uint64_t sizeOfStackReserve;
    uint64_t sizeOfStackCommit;
    uint64_t sizeOfHeapReserve;
    uint64_t sizeOfHeapCommit;
    uint32_t loaderFlags;
    uint32_t numberOfRvaAndSizes;
    struct PE_Data_Directory_Entry exportTableEntry;
    struct PE_Data_Directory_Entry importTableEntry;
    struct PE_Data_Directory_Entry resourceTableEntry;
    struct PE_Data_Directory_Entry exceptionTableEntry;
    struct PE_Data_Directory_Entry certificateTableEntry;
    struct PE_Data_Directory_Entry baseRelocationTableEntry;
    struct PE_Data_Directory_Entry debugEntry;
    struct PE_Data_Directory_Entry architectureEntry;
    struct PE_Data_Directory_Entry globalPtrEntry;
    struct PE_Data_Directory_Entry tlsTableEntry;
    struct PE_Data_Directory_Entry loadConfigTableEntry;
    struct PE_Data_Directory_Entry boundImportEntry;
    struct PE_Data_Directory_Entry iatEntry;
    struct PE_Data_Directory_Entry delayImportDescriptorEntry;
    struct PE_Data_Directory_Entry clrRuntimeHeaderEntry;
    struct PE_Data_Directory_Entry reservedEntry;
};

#endif