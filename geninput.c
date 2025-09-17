/* geninput.c
 * MP3
 * Harlan Russell
 * ECE 2230, Fall 2025
 *
 * Compile with 
 *     gcc -g -Wall geninput.c -o geninput 
 *
 * Purpose: Generate an input file for MP3 
 * Assumptions: Input file is for testing sort.  Creates
 *              an unsorted list, uses APPENDREAR to insert items, and
 *              calls SORTGEN or SORTIP to sort the list
 *
 * Command line arguments:
 *    1st -- number of records to create
 *    2nd -- type of list to create
 *           1: random addresses [0, 3/4 number records)
 *           2: assending and sequential
 *           3: descending and sequential 
 *    3rd -- type of sort
 *           1: Insertion
 *           2: Recursive Selection
 *           3: Iterative Selection
 *           4: Merge
 *    4th -- sort field
 *           gen: generator_id
 *           ip:  dest_ip_addr
 *    optional 5th argument to change the seed
 *
 * Pipe the output of this program into lab3. For example
 *     ./geninput 10000 1 1 gen | ./lab3
 *
 * See also mp3test.sh 
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#define TRUE  1
#define FALSE 0

int main(int argc, char *argv[])
{
    int records = 0, list_type = -1, sort_type = -1, field_type = -1;
    int include_duplicates = TRUE;
    int addr_range;
    int gen_id, ip;
    int seed = 1473633460;

    if (argc != 5 && argc != 6) {
        printf("Usage: ./geninput listsize listype sorttype field [seed]\n");
        printf("\tlistsize: number of records to create\n");
        printf("\tlisttype: {1, 2, 3} for random, ascending or descending\n");
        printf("\tsorttype: 1: Insertion\n");
        printf("\t          2: Recursive Selection\n");
        printf("\t          3: Iterative Selection\n");
        printf("\t          4: Merge\n");
        printf("\t          5: qsort\n");
        printf("\tfield: gen or ip\n");
        printf("\tseed: optional seed for random number generator\n");
        exit(1);
    }
    records = atoi(argv[1]);
    if (records < 2) {
        printf("genniput has invalid number records: %d\n", records);
        exit(2);
    }
    list_type = atoi(argv[2]);
    sort_type = atoi(argv[3]);
    if (sort_type < 1 || sort_type > 5) {
        printf("genniput has invalid type of sort: %d\n", sort_type);
        exit(2);
    }
    if (strcmp(argv[4], "gen") == 0) {
        field_type = 1;
    } else if (strcmp(argv[4], "ip") == 0) {
        include_duplicates = FALSE;
        field_type = 2;
    } else {
        printf("genniput has invalid field : %s\n", argv[4]);
        exit(2);
    }

    if (argc == 6) {
        int temp_seed = -1;
        temp_seed = atoi(argv[5]);
        if (temp_seed > 0)
            seed = temp_seed;
    }
    if (records <= 20)
        printf("Seed %d\n", seed);
    addr_range = records * 0.5;
    srand48(seed);

    int i;
    if (list_type == 1) {
        if (include_duplicates == TRUE) {
            // random addresses, 
            for (i = 0; i < records; i++) {
                if ((records >= 8) && (i == records/4 || i == records/2 || i == 3*records/4)) {
                    gen_id = records;
                    ip = records;
                } else if ((records >= 9) && (i == records/3 || i == 2*records/3)) {
                    gen_id = 0;
                    ip = 0;
                } else {
                    gen_id = (int) (addr_range * drand48()) + 1;
                    ip = (int) (10*records * drand48()) + records + 1;
                }
                printf("APPENDREAR %d %d\n", gen_id, ip);
            }

        } else {
            // random permutation, no duplicates 
            int * narray = (int *) malloc(records*sizeof(int));
            for (i = 0; i<records; i++)
                narray[i] = i;
            for (i = 0; i<records; i++) {
                int key = (int) (drand48() * (records - i)) + i;
                assert(i <= key && key < records);
                int temp = narray[i]; narray[i] = narray[key]; narray[key] = temp;
                printf("APPENDREAR %d %d\n", i+1, narray[i]+1);
            }
            free(narray);
        }
    }
    else if (list_type == 2) {
        // ascending addresses
        for (i = 0; i < records; i++) {
            printf("APPENDREAR %d %d\n", i+1, i+records);
        }
    }
    else if (list_type == 3) {
        // descending addresses
        for (i = 0; i < records; i++) {
            printf("APPENDREAR %d %d\n", records - i, 2*records - i);
        }
    }
    else {
        printf("geninput has invalid list type: %d\n", list_type);
        exit(3);
    }
    if (records <= 10)
        printf("Before sorting\n\nPRINTQ\n");
    if (field_type == 1)
        printf("SORTGEN %d\n", sort_type);
    else
        printf("SORTIP %d\n", sort_type);
    if (records <= 20)
        printf("\nAfter sorting\n\nPRINTQ\n");
    printf("QUIT\n");
    exit(0);
}

