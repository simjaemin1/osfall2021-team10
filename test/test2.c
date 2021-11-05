#define _GNU_SOURCE
#include <linux/cred.h>
#include <linux/types.h>
#include <stdio.h>

int main() {
    if (current_uid())
        printf("User level\n");
    else
        printf("Root level\n");
    return 0;
}
