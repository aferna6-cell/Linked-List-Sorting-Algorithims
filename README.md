# mp3
MP3: Five sorting algorithms and recursion

See the mp3.pdf found on canvas for details about the assignment requirements.

You must first clone the template code.  

To clone and then later submit, review the HW1 details on Canvas about how 
to use gh and git.

You may need to login if you find you get errors about permissions:

    gh auth login 

To check if you are loggged in use 

    gh auth status

To clone

On the github classroom page with the code look for the green "<> Code"
button.  Select the "GitHub CLI" tab.  Use the copy icon to get the line that
starts with (this is incomplete):

   gh repo clone clemson-ece-2230/mp3-more details

In a terminal on your Ubuntu machine, navigate to a mp2 folder and paste 
the clone command.  Now you will have template files for the assignment.

To submit

In the directory with your code do:

    git add lab3.c ids_support.c llist.c 
    git add datatypes.h ids_support.h llist.h
    git add makefile mytestscript.sh

(you can name the mytestscript.sh file anything you like. 
Do not submit other files)

    git commit -m "final mp3 code"

(You can make the message anything you like.)

    git push

If additioanl files are added to the project, you may need to complete
a pull request.

To get them, first complete the Pull requests found on your github classroom 
page.  You need to merge the new files into your respository.  Then in your 
local terminal do:

    git pull

(Completing the Pull requests is not needed if you clone your repository from
the template repository after the files have been added to the template 
repository.)

