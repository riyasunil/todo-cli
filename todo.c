#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <jansson.h>
#include <ini.h>


#define MAX_TASK_LEN 256
#define FILE_NAME "tasks.json"
#define CONFIG_FILE "config.ini"
#define COLOR_RED     "\x1b[31m"
#define COLOR_YELLOW  "\x1b[33m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_WHITE   "\x1b[37m"
#define COLOR_RESET   "\x1b[0m"


char global_tasks_path[512] = {0};

typedef struct {
    char directory_path[512];  
} configuration;

static int config_handler(void* user, const char* section, const char* name,const char* value) {
  configuration* config = (configuration*)user;

  if (strcmp(section, "") == 0 && strcmp(name, "dir") == 0) {
    strncpy(config->directory_path, value, sizeof(config->directory_path) - 1);
    config->directory_path[sizeof(config->directory_path) - 1] = '\0';
    return 1;
  }

  return 0;
}

int read_config(const char* config_path, configuration* config) {

  config->directory_path[0] = '\0';

  if (ini_parse(config_path, config_handler, config) < 0) {
    fprintf(stderr, "Error: Cannot load configuration file '%s'\n", config_path);
    return 0;
  }

  if (config->directory_path[0] == '\0') {
    fprintf(stderr, "Error: No directory path specified in config file\n");
    return 0;
  }

  return 1;
}



void get_curr_date(char *date_str){
  time_t t = time(NULL);
  struct tm *tm_info = localtime(&t);
  strftime(date_str, 11, "%Y-%m-%d", tm_info);
}

void get_curr_timestamp(char *timestamp_str){
  time_t t = time(NULL);
  struct tm *tm_info = localtime(&t);
  strftime(timestamp_str, 20, "%Y-%m-%dT%H:%M:%S", tm_info);
}
json_t *load_tasks(){
  FILE *f = fopen(global_tasks_path, "r");
  if(f == NULL){
    return json_array();
  }

  json_t *tasks = json_loadf(f, 0, NULL);
  fclose(f);
  if(tasks == NULL){
    fprintf(stderr, "Error loading tasks from file (`;0_0)` \n");
    exit(1);
  }
  return tasks;
}
int save_file(json_t *tasks){
  FILE *f = fopen(global_tasks_path, "w");
  if(f == NULL){
    perror("Error opening file to save tasks :[ \n");
    return(-1);
  }
  if(json_dumpf(tasks, f, JSON_INDENT(4))<0){
    fprintf(stderr, "error while saving tasks :[\n");
    fclose(f);
    return(-1);
  }
  fclose(f);
  return 0;
}

void add_task(const char* task_name, int priority){
  json_t *tasks = load_tasks();

  char timestamp[20];
  get_curr_timestamp(timestamp);

  json_t *task = json_object();
  json_object_set_new(task, "name", json_string(task_name));
  json_object_set_new(task, "priority", json_integer(priority));
  json_object_set_new(task, "completed", json_boolean(0));
  json_object_set_new(task, "date_added", json_string(timestamp));

  if (json_array_append_new(tasks, task) != 0) {
    fprintf(stderr, "Error adding task to task list.\n");
    return; 
  }

  if (save_file(tasks) != 0) {
    fprintf(stderr, "Error saving tasks to file.\n");
    return; 
  }
  printf("Task - %s - added sucessfully!!\n", task_name);
}

void list_tasks() {
  json_t *tasks = load_tasks();

  int task_number = 1;
  size_t index;
  json_t *task;
  json_array_foreach(tasks, index, task) {
    const char *name = json_string_value(json_object_get(task, "name"));
    int completed = json_boolean_value(json_object_get(task, "completed"));
    int priority = json_integer_value(json_object_get(task, "priority"));
    const char *color = COLOR_WHITE;
        if (priority == 3) {
            color = COLOR_RED;
        } else if (priority == 2) {
            color = COLOR_YELLOW;
        } else if (priority == 1) {
            color = COLOR_GREEN;
        }
    printf("%d. %s%s%s, [%d]", task_number++, color, name, COLOR_RESET, priority);
    if (completed) {
      printf(" [Completed]");
    }
    printf("\n");
  }
}

void delete_task(int task_number){
  json_t *tasks = load_tasks();
  if(task_number <=0 || task_number > json_array_size(tasks)){
    fprintf(stderr, "Invalid task number\n");
    return;
  }
  json_t *task = json_array_get(tasks, task_number-1);
  printf(json_string_value((json_object_get(task, "name"))));
  json_array_remove(tasks, task_number-1);
  save_file(tasks);
  printf("- Task deleted :]");

}
void complete_task(int task_number) {
  json_t *tasks = load_tasks();
  if (task_number <= 0 || task_number > json_array_size(tasks)) {
    fprintf(stderr, "Invalid task number\n");
    return;
  }

  json_t *task = json_array_get(tasks, task_number - 1);
  json_object_set_new(task, "completed", json_boolean(1));
  save_file(tasks);

  printf("Task %d marked as completed.\n", task_number);
}


void handle_args(int argc, char *argv[]){
  if(argc <2){
    printf("Usage:\n todo [-a <task-name> <task_priority>] : add task \n [-c <task-number>] : mark task as completed \n [-d <task-number>] : delete task \n [-l] : list tasks\n");
    exit(1);
  }
  if(strcmp(argv[1], "-a") == 0 && argc >=3 && argc <=4){
    int priority = 0;
    if(argc==4){
        if (strcmp(argv[3], "-h") == 0) priority = 3;
        else if (strcmp(argv[3], "-m") == 0) priority = 2;
        else if (strcmp(argv[3], "-l") == 0) priority = 1;
        else{
          fprintf(stderr, "Invalid priority flag. Using default priority (0).\n");
      }
      }
    add_task(argv[2], priority);
  }
  else if(strcmp(argv[1], "-c") == 0 && argc == 3){
    int task_number = atoi(argv[2]);
    if (task_number <= 0) {
      fprintf(stderr, "Invalid task number.\n");
      exit(1);
    }
    complete_task(task_number);
  }
  else if (strcmp(argv[1], "-d") == 0 && argc ==3) {
    int task_number = atoi(argv[2]);
    if(task_number <=0 ){
      fprintf(stderr, "Invalid task number.\n");
      exit(1);
    }
    delete_task(task_number);
  }
  else if(strcmp(argv[1], "-l") == 0){
    list_tasks();
  }
  else{
    printf("Invalid arguments.\n Usage: todo [-a <task>] [-c <task_number>] [-d <task-number>] [-l]\n");
    exit(1);
  }
}
int main(int argc, char *argv[]) {
  configuration config;
  const char* config_path = "/home/ria/todo/config.ini";  //figure out how to get this usr path. 


  if (!read_config(config_path, &config)) {
    return 1;
  }
  snprintf(global_tasks_path, sizeof(global_tasks_path), "%s/tasks.json", config.directory_path);
  handle_args(argc, argv);
  return 0;
}
