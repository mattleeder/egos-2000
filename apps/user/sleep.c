#include "app.h"
#include <stdlib.h>

int main(int argc, char** argv) {
    const uint usec_cnt = atoi(argv[1]);
    printf("Start to sleep for %d microseconds\r\n", usec_cnt);
    struct proc_request req;
    req.type = PROC_SLEEP;
    req.argc = usec_cnt;
    grass->sys_send(GPID_PROCESS, (void*)&req, sizeof(req));
    printf("Woke up again after %d microseconds\r\n", usec_cnt);
}