# bare-metal-os

## CLI program:
1. Print Welcome Message: print the OS name, BARE METAL OS in the ASCII Art format. 
2. Print MyOS>: print MyOS> while waiting for new commands or after the previous command was executed. 
3. CLI Commands:
help: notify the users about all available commands. 
help <command>: give the users the guide to use a particular command. 
clear: to clear the screen. 
setcolor -t <text-color>: for setting the text color. 
setcolor -b <background-color>: for setting the background color.
setcolor -t <text-color> -b <background-color>: for setting the text and background color at the same time. 
showinfo: for showing the board revision and mac address. 
4. Auto-complete: to support the users to complete their about-to-typed command if the users press the tab button according to the letters in the command name.  
5. Command History Access: to allow the users to browse the command history with _ key as UP arrow and + key as DOWN arrow.
6. Delete Feature: to delete the command.
7. Clear Line Function: to  clear the whole input line without deleting the MyOS>. 

### Welcome to OS:
![Alt text](https://github.com/andtr-2021/cli-printf-bare-metal-os/blob/master/osWelcome.JPG)

### help command:

![Alt text](https://github.com/andtr-2021/cli-printf-bare-metal-os/blob/master/help.JPG)

### help \<command\> command:

![Alt text](https://github.com/andtr-2021/cli-printf-bare-metal-os/blob/master/man.JPG)

### set-text-color command:

![Alt text](https://github.com/andtr-2021/cli-printf-bare-metal-os/blob/master/set%20text%20color.JPG)

### showinfo command: 

![Alt text](https://github.com/andtr-2021/cli-printf-bare-metal-os/blob/master/showinfo.JPG)

### set-bg-color command:

![Alt text](https://github.com/andtr-2021/cli-printf-bare-metal-os/blob/master/set%20text%20color.JPG)

### set-text-bg-color command:

![Alt text](https://github.com/andtr-2021/cli-printf-bare-metal-os/blob/master/settextbackground.JPG)

## printf function:

### printf client code:

![Alt text](https://github.com/andtr-2021/cli-printf-bare-metal-os/blob/master/printf%20code.JPG)

### printf demo:

![Alt text](https://github.com/andtr-2021/cli-printf-bare-metal-os/blob/master/printf%20demo.JPG)

