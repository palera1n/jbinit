//
//  overwrite.c
//  dummy
//
//  Created by Nick Chan on 30/1/2024.
//

#include <payload/payload.h>
#include <xpc/xpc.h>
#include <xpc/connection.h>
#include <libjailbreak/libjailbreak.h>
#include <sys/mman.h>
#include <unistd.h>

#ifdef DEV_BUILD

int overwrite_main(int argc, char* argv[]) {
    mode_t mode = 0;
    bool has_mode = false;
    int ch;
    while ((ch = getopt(argc, argv, "m:")) != -1) {
        switch(ch) {
            case 'm':
                {
                    has_mode = true;
                    char* endptr;
                    mode = strtoul(optarg, &endptr, 0);
                    if (*endptr != '\0' || optarg[0] == '\0') {
                        fprintf(stderr, "Error: invalid file mode supplied");
                        return -1;
                    }
                }
                
                break;
            default:
                return -1;
        }
    }
    
    argc -= optind;
    argv += optind;
    
    if (argc < 1) {
        fprintf(stderr, "Error: No source file specified\n");
        return -1;
    }
    
    if (argc < 2) {
        fprintf(stderr, "Error: No destination file specified\n");
        return -1;
    }
    
    char source[PATH_MAX], destination[PATH_MAX];
    realpath(argv[0], source);
    realpath(argv[1], destination);
    int fd = open(source, O_RDONLY);
    if (fd == -1) {
        fprintf(stderr, "Error: open source file failed: %d (%s)\n", errno, strerror(errno));
        return errno;
    }
    ssize_t size = (ssize_t)lseek(fd, 0, SEEK_END);
    if (size == -1) {
        fprintf(stderr, "Error: lseek failed: %d (%s)\n", errno, strerror(errno));
        close(fd);
        return errno;
    }
    uint8_t* data = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (data == MAP_FAILED) {
        fprintf(stderr, "Error: mmap failed: %d (%s)\n", errno, strerror(errno));
        close(fd);
        return errno;
    }
    
    xpc_object_t xdict = xpc_dictionary_create(NULL, NULL, 0);
    if (has_mode) xpc_dictionary_set_uint64(xdict, "mode", mode);
    xpc_dictionary_set_data(xdict, "data", data, size);
    xpc_dictionary_set_string(xdict, "path", destination);
    xpc_dictionary_set_uint64(xdict, "cmd", JBD_CMD_OVERWRITE_FILE_WITH_CONTENT);
    
    xpc_object_t xreply = jailbreak_send_jailbreakd_message_with_reply_sync(xdict);
    print_jailbreakd_reply(xreply);
    
    xpc_release(xreply);
    xpc_release(xdict);
    munmap(data, size);
    close(fd);
    
    return 0;
}

#endif

