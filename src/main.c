/**
 ***********************************************************************************
 * @file main.c
 * @author Matěj Křenek <xkrenem00@stud.fit.vutbr.cz>
 * @brief -
 * @date 2023-17-10
 *
 * @copyright Copyright (c) 2023
 ***********************************************************************************
 */
#include <stdio.h>
#include <string.h>
#include "logger/logger.h"

#define len(arr) sizeof(arr) / sizeof(arr[0])

typedef struct
{
    int rows;
    int cols;
    unsigned char *cells;
} Map;

typedef struct
{
    char *name;
    int (*callback)(int argc, char **argv);
} Command;

int cmd_call_help(int argc, char **argv);
int cmd_call_test(int argc, char **argv);
int cmd_call_rpath(int argc, char **argv);
int cmd_call_lpath(int argc, char **argv);
int cmd_call_shortest(int argc, char **argv);
int cmd_exists(char *name, Command *commands, int length);

int main(int argc, char **argv)
{
    logger_init(LOG_DEBUG);

    Command commands[] = {
        {
            .name = "--help",
            .callback = cmd_call_help,
        },
        {
            .name = "--test",
            .callback = cmd_call_test,
        },
        {
            .name = "--rpath",
            .callback = cmd_call_rpath,
        },
        {
            .name = "--lpath",
            .callback = cmd_call_lpath,
        },
        {
            .name = "--shortest",
            .callback = cmd_call_shortest,
        },
    };

    if (argc > 1)
    {
        int cmd_index = cmd_exists(argv[1], commands, len(commands));

        if (cmd_index >= 0)
        {
            return commands[cmd_index].callback(argc, argv);
        }
        else
        {
            fprintf(stderr, "command doesnt exists");
            return 1;
        }
    }
    else
    {
        fprintf(stderr, "Please provide valid number of arguments");
        return 1;
    }

    return 0;
}

int cmd_call_help(int argc, char **argv)
{
    printf("%s\n", "--helpcmd");
    printf("%i", argc);
    printf("%s", *argv);

    return 0;
}

int cmd_call_test(int argc, char **argv)
{
    printf("%s\n", "--testcmd");
    printf("%i", argc);
    printf("%s", *argv);

    return 0;
}

int cmd_call_rpath(int argc, char **argv)
{
    printf("%s\n", "--rpathcmd");
    printf("%i", argc);
    printf("%s", *argv);

    return 0;
}

int cmd_call_lpath(int argc, char **argv)
{
    printf("%s\n", "--lpathcmd");
    printf("%i", argc);
    printf("%s", *argv);

    return 0;
}

int cmd_call_shortest(int argc, char **argv)
{
    printf("%s\n", "--shotestcmd");
    printf("%i", argc);
    printf("%s", *argv);

    return 0;
}

int cmd_exists(char *name, Command *commands, int length)
{
    int index = -1;

    for (int i = 0; i < length; i++)
    {
        if (!strcmp(commands[i].name, name))
        {
            index = i;
        }
    }

    return index;
}