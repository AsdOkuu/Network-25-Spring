#include "common.h"
#include <stdio.h>
#include <string.h>

// Global variable to store name
char device_name[100];

// Function declarations
void Hub();
void Switch();
void Router();

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <name> <type(hub/switch/router)>\n", argv[0]);
        return 1;
    }

    // Store name in global variable
    strncpy(device_name, argv[1], sizeof(device_name)-1);
    device_name[sizeof(device_name)-1] = '\0';

    // Call corresponding function based on type
    if (strcmp(argv[2], "hub") == 0) {
        Hub();
    } else if (strcmp(argv[2], "switch") == 0) {
        Switch();
    } else if (strcmp(argv[2], "router") == 0) {
        Router();
    } else {
        printf("Invalid device type: %s\n", argv[2]);
        printf("Valid types are: hub, switch, router\n");
        return 1;
    }

    printf("Device '%s' initialized as %s\n", device_name, argv[2]);
    return 0;
}