Author: Andrew Blair
Class: CS 344
Date: 8-11-2021
-----------------------------------------------------------
Assignment 4: Multi-Threaded Merge Sort
-----------------------------------------------------------

Files: uniqify.c, msort_plots.ps, README.txt

Additional Files (located in folder msort_results): msort197.ps, msort595.ps, 
    msort2662.ps, msort6433.ps, msort197.pdf, msort595.pdf, msort2662.pdf, 
    msort6433.pdf, results_210812_144752.xlsx. 


Description: The file uniqify.c contains a program that can be used to sort 
    unique words from a text file. Words will be sorted alphabetically and 
    the number of times each unique word occurs will be counted. The sorted 
    list of words, and their counts, will be output in order with one word per 
    line. Characters other than words from the input will be ignored and all 
    output words will be lowercase. The program can be run with two different 
    variations depending on how it is compiled (more information below). 
    One variation will only use processes to accomplish its goal, while the 
    other will incorporate threading. In both variations, the user must specify 
    a number of sorting tasks for the program to use for sorting operatins (if 
    running the program on the os1 server, it is recommended to not specify a 
    number of sorting tasks greater than 32). 

    Additionally, program performance has been assessed and discussed. Each 
    variation was tested with files of various sizes in kilobytes (197, 595, 2662, 
    6433) and using various numbers of sorting tasks (1 through 32). Execution 
    time was measured and plotted against the number of sorting tasks for each 
    file size. Please see the "Questions" section below for discussion. 
    
    There are four plots displaying results saved in the file named 
    msort_plots.ps. 
    
    Plots displaying results are also saved individually in the files msort197.ps, 
    msort595.ps, msort2662.ps, and msort6433.ps. 

    PDF versions of the plots can be found in the files msort197.pdf, msort595.pdf, 
    msort2662.pdf, and msort6433.pdf.

    Test data and plots are also contained in the file named 
    results_210812_144752.xlsx.


Usage: Once compiled, msort can be run from the command line using the following 
    syntax.

    msort k < input_file > output_file

        k is an integer representing the number of sorting processes the 
        program will run.

        input_file is the file that will be sorted.

        output_file is where the results will be output. 
        (Excluding the right angle bracket and output_file will cause results 
        to be output to stdout, which is the terminal by default.)


Compiling: Compilation of the program can be performed in two ways, which will 
    determine the variation of the program in the resulting executable file. 
    
    Either of the following commands can be used with the gcc compiler to create an 
    executable file named msort. The executable can then be used as instructed 
    above. 

    Threaded Variation:

        gcc -std=c11 -pthread -Wall -Werror -g3 -O0 -o msort uniqify.c

    Process Variation:

        gcc -std=c11 -pthread -Wall -Werror -g3 -O0 -o msort uniqify.c -DPROCESS


Questions:

    What are the benefits of threads over processes, and the benefits of 
    processes over threads? 

            Threads have many benefits over processes, which include faster 
        communication and context switches, a lower "cost" in terms of memory 
        and resource use, and an entire process will not be blocked as a result 
        of a thread becoming blocked. Context switches are less "expensive" 
        between multiple threads in a process because heap, code, and data 
        segments are shared. Fewer resources are spent because the OS does not 
        have to remove the entire process from the CPU in order to run another 
        process. If a thread within a process becomes blocked, other threads 
        may still execute instructions, preventing the entire process from being 
        blocked. Thus, downtime from blocking is reduced and more work can be 
        performed. 

            However, processes do still have some advantages over threads as well. 
        Processes have separate memory allocated, so they do not have to share 
        heap, code, and data segments. They can make calls to execute other programs 
        without interfering with other processes. Processes may also be less prone 
        to race conditions because they are not sharing as many resources. There is 
        also less risk of data corruption because it is not shared as in threads. 

    Which variation runs with higher performance? Was this surprising? 

            Based on the results of testing run times with various file sizes and 
        numbers of sorting processes, it appears that the process variation performed 
        better in most situations. When file sizes were small, as seen with the 
        197 KB file, the thread variation of the program performed significantly better. 
        For all other file sizes, performance was very similar, however the trend line 
        for the process variation executed in faster times than the thread variation. 
        The process version also appeared to have more consistent performance, with a 
        relatively constant rate of execution regardless of how many sorting processes 
        were specified. Results from testing the 595 KB file show that the threaded 
        version performed faster for a small number of sorting processes, and lost 
        performance as the number of sorting processes increased. 

            This result is surprising because it would be expected that threads can 
        execute faster and with greater efficieny than their process equivalent. That 
        was not the case in this experiment. I would attribute this result to the fact 
        we implemented all sorting using child processes. While the number of sorting 
        processes changed, the number of threads created remained constant for every 
        execution of the program. This was necessary as we implemented sorting by 
        having each sorting process execute the "/bin/sort" program, which would not 
        have been possible using only threads. It goes to show that multi-processing 
        is still very useful, even when multi-threading does offer benefits in certain 
        applications. Unsurprisingly, as file sizes increased, the total time of 
	execution also increased for both the process and threaded variations. 


-----------------------------------------------------------

For further questions and comments, please contact me at blairand@oregonstate.edu
