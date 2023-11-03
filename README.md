# GuiraudStrozecki2023Scheduling
The two folders correspond to the experiment of the sections 6.1 and 6.1 of the paper "Scheduling periodic messages on a shared link".

You need to have gcc and python3 installed on your OS.
To launch an experiment, you can edit the parameters in the DEFINES at the top of the C files, and launch the experiment with a simple "make". A pdf file with the performance of the algorithms will be generated in the folder.
We left a dataset of the computation time of algorithms for Figure 18 in the folder "Fig18". Note that you can recreate the file "time.data" by setting the define FIG18 of "greedyRandStar.c" to 1.