/**
 ***********************************************************************************
 * @file maze.c
 * @author Matěj Křenek <xkrenem00@stud.fit.vutbr.cz>
 * @brief Program that finds a way out in a given maze
 * @date 2023-25-11
 *
 * @copyright Copyright (c) 2023
 * @todo
 *   - [x] implement map allocations using ctor and dtor function
 *   - []  reimplement start border to be more ef
 *   - []  replace odd/even calc for cell with row + cell
 ***********************************************************************************
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define len(arr) sizeof(arr) / sizeof(arr[0])
#define MAP_FILE_LINE_BUFFER_SIZE 100

typedef struct
{
    char *name;
    int argc;
    int (*callback)(char **argv);
} Command;

typedef struct
{
    int rows;
    int cols;
    unsigned char *cells;
} Map;

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

int cmd_help(char **argv);
int cmd_test(char **argv);
int cmd_rpath(char **argv);
int cmd_lpath(char **argv);
int cmd_shortest(char **argv);
int cmd_exists(char *name, Command *commands, int length);
Map *map_ctor(char *filename);
void map_dtor(Map *map);
int map_test(Map *map);
void map_print(Map *map);
unsigned char map_cell(Map *map, int row, int col);
int map_isborder(Map *map, int row, int col, MapBorder border);
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
        },
        {
            .name = "--shortest",
            .argc = 3,
            .callback = cmd_shortest,
        },
    };

    if (argc > 1)
    {
        int cmd_index = cmd_exists(argv[1], commands, len(commands));

        if (cmd_index >= 0)
        {
            if (commands[cmd_index].argc == argc - 2)
            {
                return commands[cmd_index].callback(argv);
            }

            fprintf(stderr, "Please provide valid number of arguments\n");
            return 1;
        }
    }

    return commands[0].callback(argv);
}

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
        printf("  --shortest <R> <C> <FILE>\tFinds the shortest path from the maze\n\t\t\t\tstored in the FILE from row R of column C.\n\n");
    }

    return 0;
}

int cmd_test(char **argv)
{
    Map *map = map_ctor(argv[2]);

    if (map == NULL)
    {
        printf("Invalid\n");
        // fprintf(stderr, "Passed `%s` couldnt be loaded", argv[2]);

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

int cmd_rpath(char **argv)
{
    Map *map = map_ctor(argv[4]);

    if (map == NULL)
    {
        fprintf(stderr, "Passed `%s` couldnt be loaded", argv[4]);

        return 1;
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

int cmd_lpath(char **argv)
{
    Map *map = map_ctor(argv[4]);

    if (map == NULL)
    {
        fprintf(stderr, "Passed `%s` couldnt be loaded", argv[4]);

        return 1;
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

int cmd_shortest(char **argv)
{
    printf("%s\n", "--helpcmd");
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

/**
 * @brief Load and parse map structure from passed file
 * @param filepath path to file with map
 * @return int
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

            // Check number of columns is valid
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
 * @brief Free allocated memory of cells from passed map
 * @param map pointer to Map structure
 */
void map_dtor(Map *map)
{
    free(map->cells);
    free(map);
}

/**
 * @brief Test map is valid maze
 * @param map pointer to Map structure
 * @return is valid state
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

            if ((row * map->cols + col) % 2 != 0 && row < map->rows - 1)
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
 * @brief Print passed map as matrix
 * @param map pointer to Map structure
 */
unsigned char map_cell(Map *map, int row, int col)
{
    return map->cells[row * map->cols + col];
}

/**
 * @brief Finds out if there is border on the given cell of passed map
 * @param map pointer to Map structure
 * @param row row position of cell (starting from 1)
 * @param col column position of cell (starting from 1)
 * @param border border position to be analyzed
 * @return int
 */
int map_isborder(Map *map, int row, int col, MapBorder border)
{
    return (int)(map_cell(map, row - 1, col - 1) & border);
}

/**
 * @brief Finds out starting border of map
 */
MapBorder map_startborder(Map *map, int row, int col, MapRule rule)
{
    // Invalid border if row/col is out of maze or is not border of maze
    if (
        (row > 1 && row < map->rows && col > 1 && col < map->cols) ||
        (row < 1 || row > map->rows || col < 1 || col > map->cols) ||
        ((row == 1 || row == map->rows) && col % 2 == 0))
    {
        return (MapBorder)NULL;
    }

    // Invalid border if cell is not available from outside
    // odd top first col -> top or left [x]
    // odd top last col odd -> top or right [x]
    // odd top last col even -> right [x]
    // top non first and non last -> top [x]
    // even bottom first col -> bottom or left [x]
    // odd bottom first col -> left [x]
    // even bottom last odd col -> right or bottom [x]
    // even bottom last even col -> right [x]
    // odd bottom last odd col -> right [x]
    // odd bottom last even col -> right or bottom [x]
    // left non first and non last -> left [x]
    // right non first and non last -> right [x]
    // bottom non first and non last -> bottom [x]
    if (
        (row == 1 && col == 1 && map_isborder(map, row, col, BOTTOM_TOP) && map_isborder(map, row, col, LEFT)) ||
        (row == 1 && col == map->cols && col % 2 != 0 && map_isborder(map, row, col, BOTTOM_TOP) && map_isborder(map, row, col, RIGHT)) ||
        (row == 1 && col == map->cols && col % 2 == 0 && map_isborder(map, row, col, RIGHT)) ||
        (row == 1 && col > 1 && col < map->cols && col % 2 != 0 && map_isborder(map, row, col, BOTTOM_TOP)) ||
        (row == map->rows && col > 1 && col < map->cols && col % 2 != 0 && map_isborder(map, row, col, BOTTOM_TOP)) ||
        (row > 1 && row < map->rows && col == 1 && map_isborder(map, row, col, LEFT)) ||
        (row > 1 && row < map->rows && col == map->cols && map_isborder(map, row, col, RIGHT)) ||
        (row == map->rows && row % 2 == 0 && col == 1 && map_isborder(map, row, col, BOTTOM_TOP) && map_isborder(map, row, col, LEFT)) ||
        (row == map->rows && row % 2 != 0 && col == 1 && map_isborder(map, row, col, LEFT)) ||
        (row == map->rows && row % 2 == 0 && col == map->cols && col % 2 == 0 && map_isborder(map, row, col, RIGHT)) ||
        (row == map->rows && row % 2 == 0 && col == map->cols && col % 2 != 0 && map_isborder(map, row, col, BOTTOM_TOP) && map_isborder(map, row, col, RIGHT)) ||
        (row == map->rows && row % 2 != 0 && col == map->cols && col % 2 != 0 && map_isborder(map, row, col, RIGHT)) ||
        (row == map->rows && row % 2 != 0 && col == map->cols && col % 2 == 0 && map_isborder(map, row, col, BOTTOM_TOP) && map_isborder(map, row, col, RIGHT)))
    {
        return (MapBorder)NULL;
    }

    // Coming from left odd row
    if (row % 2 != 0 && col == 1)
    {
        if (row == 1 && map_isborder(map, row, col, LEFT))
        {
            return rule == RIGHT_HAND ? LEFT : RIGHT;
        }

        return rule == RIGHT_HAND ? RIGHT : BOTTOM_TOP;
    }

    // Coming from left even row
    if (row % 2 == 0 && col == 1)
    {
        if (row == map->rows && map_isborder(map, row, col, LEFT))
        {
            return rule == RIGHT_HAND ? RIGHT : LEFT;
        }

        return rule == RIGHT_HAND ? BOTTOM_TOP : RIGHT;
    }

    // Coming from right with top
    if (col == map->cols && ((row % 2 != 0 && map->cols % 2 != 0) || (row % 2 == 0 && map->cols % 2 == 0)))
    {
        if (row == 1 && col == map->cols && map_isborder(map, row, col, RIGHT))
        {
            return rule == RIGHT_HAND ? LEFT : RIGHT;
        }

        return rule == RIGHT_HAND ? BOTTOM_TOP : LEFT;
    }

    // Coming from right with bottom
    if (col == map->cols && ((row % 2 != 0 && map->cols % 2 == 0) || (row % 2 == 0 && map->cols % 2 != 0)))
    {
        if (row == map->rows && col == map->cols && map_isborder(map, row, col, RIGHT))
        {
            return rule == RIGHT_HAND ? RIGHT : LEFT;
        }

        return rule == RIGHT_HAND ? LEFT : BOTTOM_TOP;
    }

    // Coming from top
    if (row == 1 && col > 1 && col < map->cols)
    {
        return rule == RIGHT_HAND ? LEFT : RIGHT;
    }

    // Coming from bottom
    if (row == map->rows && col > 1 && col < map->cols)
    {
        return rule == RIGHT_HAND ? RIGHT : LEFT;
    }

    return (MapBorder)NULL;
}

void map_find_path(Map *map, int row, int col, MapBorder border, MapRule rule)
{
    printf("%d,%d\n", row, col);

    // Find wall with no border
    while (map_isborder(map, row, col, border))
    {
        if ((rule == RIGHT_HAND && (row + col) % 2 != 0) || (rule == LEFT_HAND && (row + col) % 2 == 0))
        {
            border = border == LEFT ? BOTTOM_TOP : (border >> 1);
        }

        if ((rule == RIGHT_HAND && (row + col) % 2 == 0) || (rule == LEFT_HAND && (row + col) % 2 != 0))
        {
            border = border == BOTTOM_TOP ? LEFT : (border << 1);
        }
    }

    // Change cell based on new border
    if (border == LEFT)
    {
        col -= 1;
    }
    else if (border == RIGHT)
    {
        col += 1;
    }
    else if (border == BOTTOM_TOP && (row + col) % 2 == 0)
    {
        row -= 1;
    }
    else if (border == BOTTOM_TOP && (row + col) % 2 != 0)
    {
        row += 1;
    }
    // Are we out of maze?
    if (row >= 1 && row <= map->rows && col >= 1 && col <= map->cols)
    {
        // Change starting border
        if (border == RIGHT)
        {
            border = LEFT;
        }
        else if (border == LEFT)
        {
            border = RIGHT;
        }

        if ((row + col) % 2 == 0)
        {
            if (rule == RIGHT_HAND)
            {
                border = border == LEFT ? RIGHT : (border == RIGHT ? BOTTOM_TOP : LEFT);
            }
            else
            {
                border = border == LEFT ? BOTTOM_TOP : (border == RIGHT ? LEFT : RIGHT);
            }
        }
        else
        {
            if (rule == RIGHT_HAND)
            {
                border = border == LEFT ? BOTTOM_TOP : (border == RIGHT ? LEFT : RIGHT);
            }
            else
            {
                border = border == LEFT ? RIGHT : (border == RIGHT ? BOTTOM_TOP : LEFT);
            }
        }

        map_find_path(map, row, col, border, rule);
    }

    return;
}

/**
 * @brief Print passed map as matrix
 * @param map pointer to Map structure
 */
void map_print(Map *map)
{
    for (int row = 0; row < map->rows; row++)
    {
        for (int col = 0; col < map->cols; col++)
        {
            printf("%d ", map_cell(map, row, col));
        }

        printf("\n");
    }
}