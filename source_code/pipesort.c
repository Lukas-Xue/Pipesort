#include <unistd.h>
#include <getopt.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>

int main( int argc, char *argv[] ){
    int opt;
    int s;
    int l;
    int pipefd[2];  // for parsing to sort
    int pipefd2[2]; // for sorting to suppress

    while((opt=getopt(argc, argv, "n:s:l:")) != -1){
        switch(opt){
            case 'n':
                if (atoi(optarg) != 1){
                    printf("Default sorter number should be one.");
                    exit(1);
                }
                break;
            case 's':
                s = atoi(optarg);
                break;
            case 'l':
                l = atoi(optarg);
            }
    }

    if (pipe(pipefd) == -1){
        fprintf(stderr, "pipe\n");
        exit(EXIT_FAILURE);
    }

    if (fork() == 0){
        // child code
        dup2(pipefd[0], 0); // child dup the reading end to its standard input   
        close(pipefd[1]);   // close both ends of the pipe
        close(pipefd[0]);
        pipe(pipefd2);  // new pipe for merging
        if (fork() == 0){
            // grand child code
            dup2(pipefd2[0], 0);    // dup the input end to sort process's output
            close(pipefd2[0]);
            close(pipefd2[1]);
            char c;
            char new_array[l+1];
            memset(new_array, 0, (l+1)*sizeof(int));
            char old_array[l+1];
            memset(old_array, 0, (l+1)*sizeof(int));
            int length = 0;
            int count = 1;
            int old_length = 0;
            while((c = fgetc(stdin)) != EOF){
                // reading chars
                if (c != '\n'){
                    new_array[length] = c;
                    length += 1;
                // string ends
                }else{
                    // if string is the same as last one: increment count, reset
                    if(strcmp(new_array, old_array) == 0){
                        count += 1;
                        old_length = length;
                        length = 0;
                        memset(new_array, 0x00, l+1);
                        continue;
                    // else not same, copy new_array to old_array, reset
                    }else{
                        old_array[old_length] = '\0';
                        if (old_array[0] != '\0'){
                            printf("%-10d%s\n", count, old_array);
                        }
                        memset(old_array, 0x00, l+1);
                        for (int i = 0; i < length; i++){
                            old_array[i] = new_array[i];
                        }
                        memset(new_array, 0x00, l+1);
                        count = 1;
                        old_length = length;
                        length = 0;
                    }
                }
            }
            if (old_array[0] != 0){
                printf("%-10d%s\n", count, old_array);
            }
        }else{
            // child code
            dup2(pipefd2[1], 1);    // child dup the output to its standard output
            close(pipefd2[0]);
            close(pipefd2[1]);
            if (execlp("sort", "sort", (char *)NULL) == -1){
                fprintf(stderr, "Could not sort file (%s)", strerror(errno));
                exit(1);
            } 
            wait(NULL);
        }
        exit(0);
    }else{
        // parent code (PARSING)
        close(pipefd[0]);   // parent closes the reading end
        FILE *output = fdopen(pipefd[1], "w");  // create a FILE pointer for writing end
        int a[l];
        memset(a, 0, l*sizeof(int));
        char c;
        int count = 0;
        int i;
        while ((c = fgetc(stdin)) != EOF){
            // is alphabetical and array is not filled up
            if ( (tolower(c) >= 'a') && (tolower(c) <= 'z') && (count < l) ){
                a[count] = tolower(c);
                count += 1;
            }
            // if not alphabetical
            else if( (tolower(c) < 'a') || (tolower(c) > 'z') ){
                // check if arrary length is larger than short
                if (count > s){
                    // put everything into pipe
                    for (i = 0; i < count; i++){
                        fputc(a[i], output);
                    }
                    // send a signal
                    fputc('\n', output);
                }
                // reset
                memset(a, 0x00, l);
                count = 0;
            }
        }
        fclose(output);   // close writing end of the pipe
        wait(NULL);
    }
}