/**
 ***********************************************************************************
 * @file maze.c
 * @author Matěj Křenek <xkrenem00@stud.fit.vutbr.cz>
 * @brief Program that finds a way out in a given maze
 * @date 2023-01-12
 *
 * @copyright Copyright (c) 2023
 ***********************************************************************************
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define len(arr) sizeof(arr) / sizeof(arr[0])
#define MAP_FILE_LINE_BUFFER_SIZE 100

typedef enum
{
    LEFT = 0b001,
    RIGHT = 0b010,
    BOTTOM_TOP = 0b100,
} MapBorder;

typedef enum
{
    LEFT_HAND,
    RIGHT_HAND
} MapRule;

typedef struct
{
    int rows;
    int cols;
    unsigned char *cells;
} Map;

typedef struct
{
    char *name;
    int argc;
    int (*callback)(char **argv);
} Command;

int cmd_help(char **argv);
int cmd_test(char **argv);
int cmd_rpath(char **argv);
int cmd_lpath(char **argv);
int cmd_exists(char *name, Command *commands, int length);
Map *map_ctor(char *filename);
void map_dtor(Map *map);
int map_test(Map *map);
unsigned char map_cell(Map *map, int row, int col);
bool map_isborder(Map *map, int row, int col, MapBorder border);
MapBorder map_rotateborder(int row, int col, MapBorder border, MapRule rule);
MapBorder map_startborder(Map *map, int row, int col, MapRule rule);
void map_find_path(Map *map, int row, int col, MapBorder border, MapRule rule);

int main(int argc, char **argv)
{
    Command commands[] = {
        {
            .name = "--help",
            .argc = 0,
            .callback = cmd_help,
        },
        {
            .name = "--test",
            .argc = 1,
            .callback = cmd_test,
        },
        {
            .name = "--rpath",
            .argc = 3,
            .callback = cmd_rpath,
        },
        {
            .name = "--lpath",
            .argc = 3,
            .callback = cmd_lpath,
        }};

    if (argc > 1)
    {
        int cmd_index = cmd_exists(argv[1], commands, len(commands));

        if (cmd_index >= 0)
        {
            if (commands[cmd_index].argc == argc - 2)
            {
                return commands[cmd_index].callback(argv);
            }

            fprintf(stderr, "`%s` requires %d arguments but %d given\n", commands[cmd_index].name, commands[cmd_index].argc, argc - 2);
            return 1;
        }
    }

    return commands[0].callback(argv);
}

/**
 * @brief Command to print help message
 * @param argv Pointer to list of program argumenents
 * @return Error code
 */
int cmd_help(char **argv)
{
    if (argv)
    {
        printf("\nUsage: ./maze [arguments]\n\n");
        printf("Arguments:\n\n");
        printf("  --help\t\t\tShow this message.\n\n");
        printf("  --test <FILE>\t\t\tCheck if passed FILE contains valid map format of maze.\n\n");
        printf("  --rpath <R> <C> <FILE>\tFinds a path from the maze stored in the FILE\n\t\t\t\tfrom row R of column C using the right hand rule.\n\n");
        printf("  --lpath <R> <C> <FILE>\tFinds a path from the maze stored in the FILE\n\t\t\t\tfrom row R of column C using the left hand rule.\n\n");
    }

    return 0;
}

/**
 * @brief Command to test map stored in filepath provided via arguments
 * @param argv Pointer to list of program argumenents
 * @return Error code
 */
int cmd_test(char **argv)
{
    Map *map = map_ctor(argv[2]);

    if (map == NULL)
    {
        printf("Invalid\n");
        return 0;
    }

    if (!map_test(map))
    {
        printf("Invalid\n");
    }
    else
    {
        printf("Valid\n");
    }

    map_dtor(map);

    return 0;
}

/**
 * @brief Command to find path out of provided maze using right hand rule
 * @param argv Pointer to list of program argumenents
 * @return Error code
 */
int cmd_rpath(char **argv)
{
    Map *map = map_ctor(argv[4]);

    if (map == NULL)
    {
        printf("Invalid map, please try to run `--test` command\n");

        return 0;
    }

    if (!map_test(map))
    {
        printf("Invalid map, please try to run `--test` command\n");
    }

    MapBorder start_border = map_startborder(map, atoi(argv[2]), atoi(argv[3]), RIGHT_HAND);

    if (!start_border)
    {
        printf("Can't enter the maze from cell on row `%d` and column `%d`\n", atoi(argv[2]), atoi(argv[3]));
        return 0;
    }

    map_find_path(map, atoi(argv[2]), atoi(argv[3]), start_border, RIGHT_HAND);

    map_dtor(map);

    return 0;
}

/**
 * @brief Command to find path out of provided maze using left hand rule
 * @param argv Pointer to list of program argumenents
 * @return Error code
 */
int cmd_lpath(char **argv)
{
    Map *map = map_ctor(argv[4]);

    if (map == NULL)
    {
        printf("Invalid map, please try to run `--test` command\n");

        return 0;
    }

    if (!map_test(map))
    {
        printf("Invalid map, please try to run `--test` command\n");
    }

    MapBorder start_border = map_startborder(map, atoi(argv[2]), atoi(argv[3]), LEFT_HAND);

    if (!start_border)
    {
        printf("Can't enter the maze from cell on row `%d` and column `%d`\n", atoi(argv[2]), atoi(argv[3]));
        return 0;
    }

    map_find_path(map, atoi(argv[2]), atoi(argv[3]), start_border, LEFT_HAND);

    map_dtor(map);

    return 0;
}

/**
 * @brief Command utility function to find index of command in provided array of commands
 * @param name Name of command to be found
 * @param commands Array of commands
 * @param length Length of provided array of commands
 * @return Index of found command (-1 if not found)
 */
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

/**
 * @brief Load and parse map structure from provided filepath
 * @param filepath Path to file with map
 * @return Pointer to created Map struct (NULL if failed)
 */
Map *map_ctor(char *filepath)
{
    Map *map = malloc(sizeof(Map));
    FILE *file = fopen(filepath, "r");

    if (file == NULL || map == NULL)
    {
        return NULL;
    }

    char buffer[MAP_FILE_LINE_BUFFER_SIZE];
    int file_row = 0;
    int file_col = 0;

    while (fgets(buffer, len(buffer), file))
    {
        // Parse map dimensions from first row
        if (file_row == 0)
        {
            sscanf(buffer, "%d %d", &map->rows, &map->cols);
            map->cells = malloc(map->rows * map->cols * sizeof(unsigned char));
        }
        else
        {
            file_col = 0;

            for (int i = 0; buffer[i]; i++)
            {
                if (buffer[i] >= '0' && buffer[i] <= '7')
                {
                    map->cells[(file_row - 1) * map->cols + file_col] = atoi(&buffer[i]);
                    file_col++;
                }
            }

            if (file_col == 0)
            {
                continue;
            }

            // Check number of cols is valid
            if (file_col != map->cols)
            {
                map_dtor(map);
                fclose(file);

                return NULL;
            }
        }

        file_row++;
    }

    // Check number of rows is valid
    if (file_row - 1 != map->rows)
    {
        map_dtor(map);
        fclose(file);

        return NULL;
    }

    fclose(file);

    return map;
}

/**
 * @brief Free allocated memory of provided Map
 * @param map Pointer to Map structure
 */
void map_dtor(Map *map)
{
    free(map->cells);
    free(map);
}

/**
 * @brief Test provided map is valid maze
 * @param map Pointer to Map structure
 * @return Valid state (1: is valid, 0: isn't valid)
 */
int map_test(Map *map)
{
    int valid = 1;

    for (int row = 0; row < map->rows; row++)
    {
        for (int col = 0; col < map->cols; col++)
        {
            unsigned char current_cell = map_cell(map, row, col);

            if (col < map->cols - 1)
            {
                unsigned char right_cell = map_cell(map, row, col + 1);

                if (((current_cell >> 1) ^ right_cell) & 0b001)
                {
                    valid = 0;
                }
            }

            if ((row + col) % 2 != 0 && row < map->rows - 1)
            {
                unsigned char bottom_border = map_cell(map, row + 1, col);

                if (((current_cell) ^ bottom_border) & 0b100)
                {
                    valid = 0;
                }
            }

            if (!valid)
            {
                break;
            }
        }

        if (!valid)
        {
            break;
        }
    }

    return valid;
}

/**
 * @brief Get value of cell in map on specified row and col
 * @param map Pointer to Map structure
 * @param row Row of cell (starting from 0)
 * @param col Column of cell (starting from 0)
 * @return Value of borders in cell
 */
unsigned char map_cell(Map *map, int row, int col)
{
    return map->cells[row * map->cols + col];
}

/**
 * @brief Finds out if there is border on the given cell of provided map
 * @param map Pointer to Map structure
 * @param row Row of cell (starting from 1)
 * @param col Column of cell (starting from 1)
 * @param border Border position to be analyzed
 * @return Bool value
 */
bool map_isborder(Map *map, int row, int col, MapBorder border)
{
    return (bool)(map_cell(map, row - 1, col - 1) & border);
}

/**
 * @brief Rotates border based on provided cell and hand rule
 * @param row Row of cell (starting from 1)
 * @param col Column of cell (starting from 1)
 * @param border Border position to be rotated
 * @param rule Hand rule to be applied
 * @return New rotated border
 */
MapBorder map_rotateborder(int row, int col, MapBorder border, MapRule rule)
{
    if ((rule == RIGHT_HAND && (row + col) % 2 == 0) || (rule == LEFT_HAND && (row + col) % 2 != 0))
    {
        return border == BOTTOM_TOP ? LEFT : (border << 1);
    }

    return border == LEFT ? BOTTOM_TOP : (border >> 1);
}

/**
 * @brief Get starting border of entering cell
 * @param map Pointer to Map structure
 * @param row Row of cell (starting from 1)
 * @param col Column of cell (starting from 1)
 * @param rule Hand rule to be applied
 * @return Start border
 */
MapBorder map_startborder(Map *map, int row, int col, MapRule rule)
{
    // Checking row and col are not out of maze
    if (row > map->rows || row < 1 || col > map->cols || col < 1)
    {
        return (MapBorder)NULL;
    }

    // Coming from left odd row
    if (row % 2 != 0 && col == 1 && !map_isborder(map, row, col, LEFT))
    {
        return rule == RIGHT_HAND ? RIGHT : BOTTOM_TOP;
    }

    // Coming from left even row
    if (row % 2 == 0 && col == 1 && !map_isborder(map, row, col, LEFT))
    {
        return rule == RIGHT_HAND ? BOTTOM_TOP : RIGHT;
    }

    // Coming from right with top
    if (col == map->cols && ((row % 2 != 0 && map->cols % 2 != 0) || (row % 2 == 0 && map->cols % 2 == 0)) && !map_isborder(map, row, col, RIGHT))
    {
        return rule == RIGHT_HAND ? BOTTOM_TOP : LEFT;
    }

    // Coming from right with bottom
    if (col == map->cols && ((row % 2 != 0 && map->cols % 2 == 0) || (row % 2 == 0 && map->cols % 2 != 0)) && !map_isborder(map, row, col, RIGHT))
    {
        return rule == RIGHT_HAND ? LEFT : BOTTOM_TOP;
    }

    // Coming from top
    if (row == 1 && col >= 1 && col <= map->cols && (row + col) % 2 == 0 && !map_isborder(map, row, col, BOTTOM_TOP))
    {
        return rule == RIGHT_HAND ? LEFT : RIGHT;
    }

    // Coming from bottom
    if (row == map->rows && col >= 1 && col <= map->cols && (row + col) % 2 != 0 && !map_isborder(map, row, col, BOTTOM_TOP))
    {
        return rule == RIGHT_HAND ? RIGHT : LEFT;
    }

    return (MapBorder)NULL;
}

/**
 * @brief Find path out of maze from given start
 * @param map Pointer to Map structure
 * @param row Row of cell (starting from 1)
 * @param col Column of cell (starting from 1)
 * @param border Start border of current cell
 * @param rule Hand rule to be applied
 */
void map_find_path(Map *map, int row, int col, MapBorder border, MapRule rule)
{
    printf("%d,%d\n", row, col);

    // Find edge with no border
    while (map_isborder(map, row, col, border))
    {
        border = map_rotateborder(row, col, border, rule);
    }

    // Change cell based on new border
    col = border == BOTTOM_TOP ? col : (border == LEFT ? col - 1 : col + 1);
    row = border != BOTTOM_TOP ? row : ((row + col) % 2 == 0 ? row - 1 : row + 1);

    // Continue if the new cell isn't out of maze
    if (row >= 1 && row <= map->rows && col >= 1 && col <= map->cols)
    {
        // Change starting border
        border = border == BOTTOM_TOP ? border : (border == RIGHT ? LEFT : RIGHT);
        border = map_rotateborder(row, col, border, rule);

        map_find_path(map, row, col, border, rule);
    }

    return;
}