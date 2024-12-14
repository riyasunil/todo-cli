# Todo CLI Application

A simple, lightweight command-line Todo application written in C.

## Requirements

Before building the application, make sure you have the following libraries installed:

- [`libinih`](https://github.com/benhoyt/inih) (for INI file parsing)
- [`jansson`](https://github.com/akheron/jansson) (for JSON handling)

### installation:
```bash
pacman -S jansson
yay -S libinih
```

## Commands 

- add task : `todo -a "<task_name>" [priority]`
  Add a task with optional priority
  - `"<task_name>"` is the name of the task and should be in quotes 
  - `<priority>` is optional and can be on of the following:
    - `-h` : high priority
    - `-m` : medium priority
    - `-l` : low priority
    If none of these are provided, then no priority is assumed.
- List all tasks : `todo -l`
- Mark tasks as completed : `todo -c <task_number>`
- Delete a task : `todo -d <task_number>`

### to run locally:

```
git clone https://github.com/riyasunil/todo-cli.git
cd todo-cli
gcc -o todo todo.c -ljansson -linih  
```

To let the commands work from any directory do:
```
sudo mv todo /usr/local/bin/todo
```

> In the config.ini file, set the path to the directory to store the file with the tasks. 



