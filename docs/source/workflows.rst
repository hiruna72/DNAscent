.. _workflows:

Workflow
===============================

The following is a full DNAscent workflow, where we'll start off after Dorado has finished running. The recommended Dorado basecalling model for v4.0.3 is ``dna_r10.4.1_e8.2_400bps_fast@v5.0.0``.
In particular, we assume the following:

* You have a directory of R10.4.1 Oxford Nanopore POD5 files (which may be in subdirectories) that you want to use for detection.
* These POD5 files and a reference/genome file have been passed to Dorado (available from Oxford Nanopore) to produce a bam file.

Example Workflow
----------------

Pull the Singularity image:

.. code-block:: console

   singularity pull DNAscent.sif library://mboemo/dnascent/dnascent:4.0.3

Alternatively, you can download and compile DNAscent:

.. code-block:: console

   git clone --recursive https://github.com/MBoemo/DNAscent.git
   cd DNAscent
   git checkout 4.0.3
   make
   cd ..

Let's index the run:

.. code-block:: console

   DNAscent index -f /full/path/to/pod5

This should only take a few seconds to run and will put a file called ``index.dnascent`` in the current directory.  

Suppose we have an output from Dorado called ``alignment.bam`` (which doesn't need to be sorted or indexed). You can run DNAscent detect (on 10 threads, for example) by running:

.. code-block:: console

   DNAscent detect -b alignment.bam -r /full/path/to/reference.fasta -i index.dnascent -o detect_output.bam -t 10

If the system has a CUDA-compatible GPU in it, we can run ``nvidia-smi`` to get an output that looks like the following:

.. code-block:: console

   Thu Aug 20 21:06:57 2020
   +-----------------------------------------------------------------------------+
   | NVIDIA-SMI 450.51.06    Driver Version: 450.51.06    CUDA Version: 11.0     |
   |-------------------------------+----------------------+----------------------+
   | GPU  Name        Persistence-M| Bus-Id        Disp.A | Volatile Uncorr. ECC |
   | Fan  Temp  Perf  Pwr:Usage/Cap|         Memory-Usage | GPU-Util  Compute M. |
   |                               |                      |               MIG M. |
   |===============================+======================+======================|
   |   0  Tesla P100-PCIE...  On   | 00000000:05:00.0 Off |                    0 |
   | N/A   41C    P0    52W / 250W |   2571MiB / 16280MiB |     43%      Default |
   |                               |                      |                  N/A |
   +-------------------------------+----------------------+----------------------+

   +-----------------------------------------------------------------------------+
   | Processes:                                                                  |
   |  GPU   GI   CI        PID   Type   Process name                  GPU Memory | 
   |        ID   ID                                                   Usage      |
   |=============================================================================|
   |    0   N/A  N/A    178943      C   ...DNAscent_dev/bin/DNAscent     2569MiB |
   +-----------------------------------------------------------------------------+

From this, we can see that the GPU's device ID is 0 (just to the left of Tesla) so we can run:

.. code-block:: console

   DNAscent detect -b alignment.bam -r /full/path/to/reference.fasta -i index.dnascent -o detect_output.bam -t 10 --GPU 0

Note that we're assuming the CUDA libraries for the GPU have been set up properly (see :ref:`installation`). If these libraries can't be accessed, DNAscent will splash a warning saying so and default back to using CPUs.

When ``DNAscent detect`` is finished, it will should put a file in modbam format called ``detect_output.bam`` in the current directory. 

Lastly, we can run ``DNAscent forkSense`` on the output of ``DNAscent detect`` to measure replication fork movement.  Suppose that in our experimental protocol, we pulsed BrdU first followed by EdU.  Let's run it on four threads and specify that we want it to keep track of replication origins, forks, and termination sites:

.. code-block:: console

   DNAscent forkSense -d detect_output.bam -o output.forkSense -t 4 --markOrigins --markTerminations --markForks --order BrdU,EdU

This will make the following files: 

* origins_DNAscent_forkSense.bed (with our origin calls),
* terminations_DNAscent_forkSense.bed (with our termination calls), 
* four bed files (leftForks_DNAscent_forkSense.bed, leftForksStressed_DNAscent_forkSense.bed, rightForks_DNAscent_forkSense.bed, rightForksStressed_DNAscent_forkSense.bed) with our fork calls,
* output.forkSense. 

We can load ``detect_output.bam`` as well as the above bed files files directly into IGV to see where origins, forks, and terminiations were called in the genome.

Barcoding
---------

The workflow for a barcoded run is very similar to the workflow above. Rather than using the bam file directly from the ``Dorado basecaller`` executable, this bam file is first passed to the ``Dorado demux`` executable and the resulting bam files are sorted and passed to ``DNAscent detect``.

