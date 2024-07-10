#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <time.h>
#include <signal.h>

#define MAX_POEM_LENGTH 100
#define FILENAME "poems.txt"
#define NUM_BUNNY_BOYS 4
#define MSG_KEY 1234

volatile sig_atomic_t bunny_boy_pid;

struct msg_buffer {
    long msg_type;
    char msg_text[MAX_POEM_LENGTH];
};

void addPoem();
void listPoems();
void deletePoem();
void modifyPoem();
void wateringGirls();
void handleInterrupt(int signum);
void handleSpecialRequest(int signum);
void sendSignalToBunnyBoy(int signal);
void receiveSpecialRequest();

int main() {
    int choice;
    signal(SIGINT, handleInterrupt);
    signal(SIGUSR1, handleSpecialRequest);
    
    do {
        printf("\nMama Bunny's Watering Poem Recorder\n");
        printf("1. Add a new poem\n");
        printf("2. List all poems\n");
        printf("3. Delete a poem\n");
        printf("4. Modify a poem\n");
        printf("5. Send a bunny boy to water the girls\n");
        printf("6. Bunny boy returns home\n");
        printf("7. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);
        getchar();  
        
        switch (choice) {
            case 1:
                addPoem();
                break;
            case 2:
                listPoems();
                break;
            case 3:
                deletePoem();
                break;
            case 4:
                modifyPoem();
                break;
            case 5:
                wateringGirls();
                break;
            case 6:
                printf("Bunny boy returns home.\n");
                break;
            case 7:
                printf("Exiting program...\n");
                break;
            default:
                printf("Invalid choice! Please enter a number between 1 and 7.\n");
        }
    } while (choice != 7);
    
    return 0;
}

void addPoem() {
    char poem[MAX_POEM_LENGTH];
    
    printf("Enter the new poem: ");
    fgets(poem, MAX_POEM_LENGTH, stdin);
    
    FILE *fp = fopen(FILENAME, "a");
    if (fp == NULL) {
        printf("Error opening file!\n");
        return;
    }
    
    fprintf(fp, "%s", poem);
    fclose(fp);
    
    printf("Poem added successfully!\n");
}

void listPoems() {
    char poem[MAX_POEM_LENGTH];
    
    FILE *fp = fopen(FILENAME, "r");
    if (fp == NULL) {
        printf("No poems found!\n");
        return;
    }
    
    printf("\nList of poems:\n");
    int line = 1;
    while (fgets(poem, MAX_POEM_LENGTH, fp) != NULL) {
        printf("%d. %s", line++, poem);
    }
    
    fclose(fp);
}

void deletePoem() {
    int line, count = 0;
    char c;
    
    printf("Enter the line number of the poem to delete: ");
    scanf("%d", &line);
    
    FILE *fp = fopen(FILENAME, "r");
    FILE *tempFile = fopen("temp.txt", "w");
    if (fp == NULL || tempFile == NULL) {
        printf("Error opening file!\n");
        return;
    }
    
    while ((c = fgetc(fp)) != EOF) {
        if (c == '\n') {
            count++;
        }
        if (count != line) {
            fputc(c, tempFile);
        }
    }
    
    fclose(fp);
    fclose(tempFile);
    
    remove(FILENAME);
    rename("temp.txt", FILENAME);
    
    printf("Poem deleted successfully!\n");
}

void modifyPoem() {
    int line, count = 0;
    char c, newPoem[MAX_POEM_LENGTH];
    
    printf("Enter the line number of the poem to modify: ");
    scanf("%d", &line);
    getchar();  
    
    
    printf("Enter the modified poem: ");
    fgets(newPoem, MAX_POEM_LENGTH, stdin);
    
    FILE *fp = fopen(FILENAME, "r");
    FILE *tempFile = fopen("temp.txt", "w");
    if (fp == NULL || tempFile == NULL) {
        printf("Error opening file!\n");
        return;
    }
    
    while ((c = fgetc(fp)) != EOF) {
        if (c == '\n') {
            count++;
        }
        if (count != line) {
            fputc(c, tempFile);
        } else {
            fprintf(tempFile, "%s", newPoem);
            count++;
        }
    }
    
    fclose(fp);
    fclose(tempFile);
    
    remove(FILENAME);
    rename("temp.txt", FILENAME);
    
    printf("Poem modified successfully!\n");
}

void wateringGirls() {
    srand(time(NULL));
    int chosen_boy = rand() % NUM_BUNNY_BOYS + 1;
    printf("Sending Bunny Boy %d to water the girls.\n", chosen_boy);

    char poems[2][MAX_POEM_LENGTH];
    int num_poems = 0;

 
    FILE *file = fopen(FILENAME, "r");
    if (file == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    
    char all_poems[10][MAX_POEM_LENGTH]; 
    int num_all_poems = 0;
    while (num_all_poems < 10 && fgets(all_poems[num_all_poems], MAX_POEM_LENGTH, file) != NULL) {
        num_all_poems++;
    }
    fclose(file);

   
    int first_poem_index = rand() % num_all_poems;
    int second_poem_index;
    do {
        second_poem_index = rand() % num_all_poems;
    } while (second_poem_index == first_poem_index);

    strcpy(poems[0], all_poems[first_poem_index]);
    strcpy(poems[1], all_poems[second_poem_index]);

   
    printf("Selected poems:\n");
    for (int i = 0; i < 2; i++) {
        printf("%s", poems[i]);
    }

    int chosen_poem_index = rand() % 2;


    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    int msqid;
    key_t key = MSG_KEY;
    struct msg_buffer msg;
    
    if ((msqid = msgget(key, 0666 | IPC_CREAT)) == -1) {
        perror("msgget");
        exit(EXIT_FAILURE);
    }
    
    printf("Waiting for special request from Mama Bunny...\n");
    pause(); 
    
    if (msgrcv(msqid, &msg, sizeof(msg), 1, 0) == -1) {
        perror("msgrcv");
        exit(EXIT_FAILURE);
    }
    
    printf("Received special request: %s\n", msg.msg_text);

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }


    if (pid == 0) { 
        close(pipefd[1]); 

     
        char selected_poem[MAX_POEM_LENGTH];
        read(pipefd[0], selected_poem, MAX_POEM_LENGTH);
        close(pipefd[0]); 

        printf("May I water!\n");
        printf("Bunny Boy %d recites the poem:\n%s\n", chosen_boy, selected_poem);
        printf("Watering the girls...\n");

        exit(EXIT_SUCCESS);
    } else { 
        close(pipefd[0]); 

        write(pipefd[1], poems[chosen_poem_index], MAX_POEM_LENGTH);
        close(pipefd[1]); 

        bunny_boy_pid = pid;

        int status;
        waitpid(pid, &status, 0);

        if (WIFEXITED(status)) {
            printf("Bunny boy process terminated successfully.\n");
        } else {
            printf("Bunny boy process terminated abnormally.\n");
        }
    }
}

void handleInterrupt(int signum) {
    if (signum == SIGINT) {
        if (bunny_boy_pid > 0) {
            printf("Interrupt received. Terminating bunny boy process...\n");
            kill(bunny_boy_pid, SIGTERM);
        }
    }
}

void handleSpecialRequest(int signum) {
    if (signum == SIGUSR1) {
        printf("Bunny boy received a special request from Mama Bunny!\n");
    }
}

void sendSignalToBunnyBoy(int signal) {
    if (bunny_boy_pid > 0) {
        kill(bunny_boy_pid, signal);
    }
}
