Usage <Radiation Benchmarkxs>

- Compiling
  The compilation of Makefile should generate 3 runnables:
  - datagen (Use this for generate the input)
  - kmeans_gen (Use this to generate your gold file)
  - kmeans_check (Use this to check your gold file with the input. The gold file should be declared as the output parameter)

- Simple Example
  To generate the log file, please run the "install.py" file located on the repository root.
  - First generate your input. On the kmeans root folder, after compiling, generate your input with the following command:

    $ ./datagen <number of elements> <sizeof int> $

    The default sizeof int is 34. 150000 as the number of elements should give an execution of about 2 secs.
  - Run the kmeans_gen from the folder to generate you gold file from the generated input:

    $ ./kmeans_gen -i <input file> -o <output file> $

  - Finally you can execute the kmeans_check to check for errors and generate your log file located in /etc/radiation-benchmarks:

    $ ./kmeans_check -i <input file> -o <gold file> -l <number of interations> $

- Additional Info
  - Some executions have some extra parameters that can be used. They are listed when trying to run a file with no parameters.
