#include <stdint.h>
#include <stddef.h>
#include "stdlib.h"
#include "stdio.h"
#include "../../../SharedLibraries/sharedStructs.h"
#include "color.h"

#define MAX_PARTICIPANTS 6
#define SEM_NAME_LEN 32

typedef struct {
    char value;
    char mutex_name[SEM_NAME_LEN];
    char slots_name[SEM_NAME_LEN];
    char items_name[SEM_NAME_LEN];
} mvar_shared_t;

static const Color reader_palette[MAX_PARTICIPANTS] = {
    {0, 0, 255},      // red
    {0, 255, 0},      // green
    {255, 0, 0},      // blue
    {0, 255, 255},    // yellow
    {255, 0, 255},    // magenta
    {255, 255, 0}     // cyan
};

static const uint32_t writer_delays[MAX_PARTICIPANTS] = {8, 11, 14, 17, 20, 23};
static const uint32_t reader_delays[MAX_PARTICIPANTS] = {9, 12, 15, 18, 21, 24};

static void zero_memory(void *ptr, uint64_t size);
static void build_name(char *dest, const char *prefix, const char *suffix);
static int init_shared(mvar_shared_t *shared, const char *suffix);
static void cleanup_shared(mvar_shared_t *shared);
static uint32_t next_random(uint32_t *state);
static void pseudo_delay(uint32_t base, uint32_t *state);
static void busy_delay(uint32_t iterations);
static uint64_t hex_to_uint64(const char *hex);
static void wait_for_children(int64_t *pids, int count);
static void kill_spawned(int64_t *pids, int count);

static int writer_main(int argc, char **argv);
static int reader_main(int argc, char **argv);
static int manager_main(int argc, char **argv);

static void zero_memory(void *ptr, uint64_t size) {
    uint8_t *cursor = (uint8_t *)ptr;
    while (size--) {
        *cursor++ = 0;
    }
}

static void build_name(char *dest, const char *prefix, const char *suffix) {
    uint32_t i = 0;
    while (prefix[i] && i < SEM_NAME_LEN - 1) {
        dest[i] = prefix[i];
        i++;
    }
    uint32_t j = 0;
    while (suffix[j] && i < SEM_NAME_LEN - 1) {
        dest[i++] = suffix[j++];
    }
    dest[i] = '\0';
}

static int init_shared(mvar_shared_t *shared, const char *suffix) {
    if (shared == NULL || suffix == NULL) {
        return -1;
    }

    shared->value = '\0';
    build_name(shared->mutex_name, "mvar_mutex_", suffix);
    build_name(shared->slots_name, "mvar_slots_", suffix);
    build_name(shared->items_name, "mvar_items_", suffix);

    if (my_sem_open(shared->mutex_name, 1) == -1) {
        return -1;
    }
    if (my_sem_open(shared->slots_name, 1) == -1) {
        my_sem_close(shared->mutex_name);
        return -1;
    }
    if (my_sem_open(shared->items_name, 0) == -1) {
        my_sem_close(shared->mutex_name);
        my_sem_close(shared->slots_name);
        return -1;
    }
    return 0;
}

static void cleanup_shared(mvar_shared_t *shared) {
    if (shared == NULL) {
        return;
    }
    my_sem_close(shared->mutex_name);
    my_sem_close(shared->slots_name);
    my_sem_close(shared->items_name);
}

static uint32_t next_random(uint32_t *state) {
    *state = (*state) * 1664525u + 1013904223u;
    return *state;
}

#define SPIN_FACTOR 60000U

static void pseudo_delay(uint32_t base, uint32_t *state) {
    if (base == 0) {
        base = 1;
    }

    uint32_t random_extra = next_random(state) % (base + 1);
    uint64_t limit = (uint64_t)(base + random_extra) * SPIN_FACTOR;

    volatile uint64_t counter = 0;
    while (counter++ < limit) {
        __asm__ volatile("nop");
    }
}

static uint64_t hex_to_uint64(const char *hex) {
    if (hex == NULL) {
        return 0;
    }
    uint64_t value = 0;
    char c;
    while ((c = *hex++) != '\0') {
        value <<= 4;
        if (c >= '0' && c <= '9') {
            value |= (uint64_t)(c - '0');
        } else if (c >= 'a' && c <= 'f') {
            value |= (uint64_t)(c - 'a' + 10);
        } else if (c >= 'A' && c <= 'F') {
            value |= (uint64_t)(c - 'A' + 10);
        }
    }
    return value;
}

static void wait_for_children(int64_t *pids, int count) {
    if (pids == NULL) {
        return;
    }
    for (int i = 0; i < count; i++) {
        if (pids[i] > 0) {
            my_wait_pid(pids[i], NULL);
        }
    }
}

static void kill_spawned(int64_t *pids, int count) {
    if (pids == NULL) {
        return;
    }
    for (int i = 0; i < count; i++) {
        if (pids[i] > 0) {
            my_kill(pids[i]);
        }
    }
    wait_for_children(pids, count);
}

static int writer_main(int argc, char **argv) {
    if (argc != 3) {
        return -1;
    }

    mvar_shared_t *shared = (mvar_shared_t *)(uintptr_t)hex_to_uint64(argv[0]);
    if (shared == NULL) {
        return -1;
    }

    char token = argv[1][0];
    int delay = atoi(argv[2]);
    if (delay <= 0) {
        delay = 5;
    }

    uint32_t rng = (uint32_t)my_getpid();

    while (1) {
        pseudo_delay((uint32_t)delay, &rng);

        if (my_sem_wait(shared->slots_name) == -1) {
            break;
        }
        if (my_sem_wait(shared->mutex_name) == -1) {
            my_sem_post(shared->slots_name);
            break;
        }

        shared->value = token;

        my_sem_post(shared->mutex_name);
        my_sem_post(shared->items_name);
    }

    return 0;
}

static int reader_main(int argc, char **argv) {
    if (argc != 3) {
        return -1;
    }

    mvar_shared_t *shared = (mvar_shared_t *)(uintptr_t)hex_to_uint64(argv[0]);
    if (shared == NULL) {
        return -1;
    }

    int index = atoi(argv[1]);
    int delay = atoi(argv[2]);
    if (delay <= 0) {
        delay = 6;
    }

    Color color = reader_palette[index % MAX_PARTICIPANTS];
    uint32_t rng = (uint32_t)(my_getpid() ^ (uint32_t)index);

    while (1) {
        pseudo_delay((uint32_t)delay, &rng);

        if (my_sem_wait(shared->items_name) == -1) {
            break;
        }
        if (my_sem_wait(shared->mutex_name) == -1) {
            my_sem_post(shared->items_name);
            break;
        }

        char token = shared->value;

        my_sem_post(shared->mutex_name);
        my_sem_post(shared->slots_name);

        printfc(color, "%c", token);
    }

    return 0;
}

static int manager_main(int argc, char **argv) {
    if (argc != 3) {
        return -1;
    }

    mvar_shared_t *shared = (mvar_shared_t *)(uintptr_t)hex_to_uint64(argv[0]);
    int writers = atoi(argv[1]);
    int readers = atoi(argv[2]);

    if (shared == NULL || writers <= 0 || readers <= 0) {
        return -1;
    }

    char suffix[SEM_NAME_LEN];
    itoa((uint64_t)my_getpid(), suffix, 16);

    if (init_shared(shared, suffix) != 0) {
        my_free(shared);
        return -1;
    }

    int total = writers + readers;
    int64_t *pids = (int64_t *)my_malloc(sizeof(int64_t) * total);

    if (pids == NULL) {
        cleanup_shared(shared);
        my_free(shared);
        return -1;
    }

    int created = 0;
    int spawn_failed = 0;

    for (int i = 0; i < writers && created < total; i++) {
        char token_buf[2] = { (char)('A' + (i % 26)), '\0' };
        char delay_buf[12];
        itoa(writer_delays[i % MAX_PARTICIPANTS], delay_buf, 10);

        char *wargv[] = {
            argv[0],
            token_buf,
            delay_buf,
            NULL
        };

        int wfds[2] = {STDIN, STDOUT};
        int64_t pid = my_create_process((entry_point_t)writer_main, wargv, "mvar_writer", wfds);
        if (pid < 0) {
            spawn_failed = 1;
            break;
        }
        pids[created++] = pid;
    }

    for (int j = 0; j < readers && created < total && !spawn_failed; j++) {
        char index_buf[12];
        char delay_buf[12];
        itoa(j, index_buf, 10);
        itoa(reader_delays[j % MAX_PARTICIPANTS], delay_buf, 10);

        char *rargv[] = {
            argv[0],
            index_buf,
            delay_buf,
            NULL
        };

        int rfds[2] = {STDIN, STDOUT};
        int64_t pid = my_create_process((entry_point_t)reader_main, rargv, "mvar_reader", rfds);
        if (pid < 0) {
            spawn_failed = 1;
            break;
        }
        pids[created++] = pid;
    }

    if (spawn_failed) {
        kill_spawned(pids, created);
        my_free(pids);
        cleanup_shared(shared);
        my_free(shared);
        return -1;
    }

    wait_for_children(pids, created);
    my_free(pids);
    cleanup_shared(shared);
    my_free(shared);

    printf("\n");
    return 0;
}

int cmd_mvar(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Uso: mvar <writers> <readers>\n");
        return -1;
    }

    int writers = atoi(argv[1]);
    int readers = atoi(argv[2]);

    if (writers <= 0 || readers <= 0 || writers > MAX_PARTICIPANTS || readers > MAX_PARTICIPANTS) {
        printf("mvar: cantidades invalidas (maximo %d por tipo)\n", MAX_PARTICIPANTS);
        return -1;
    }

    mvar_shared_t *shared = (mvar_shared_t *)my_malloc(sizeof(mvar_shared_t));
    if (shared == NULL) {
        printf("mvar: sin memoria\n");
        return -1;
    }

    zero_memory(shared, sizeof(mvar_shared_t));

    char shared_hex[17];
    itoa((uint64_t)(uintptr_t)shared, shared_hex, 16);

    char writers_buf[12];
    char readers_buf[12];
    itoa(writers, writers_buf, 10);
    itoa(readers, readers_buf, 10);

    char *manager_argv[] = {
        shared_hex,
        writers_buf,
        readers_buf,
        NULL
    };

    int fds[2] = {STDIN, STDOUT};
    int64_t manager_pid = my_create_process((entry_point_t)manager_main, manager_argv, "mvar_manager", fds);
    if (manager_pid < 0) {
        printf("mvar: no pude iniciar el administrador\n");
        my_free(shared);
        return -1;
    }

    my_wait_pid(manager_pid, NULL);
    return 0;
}
