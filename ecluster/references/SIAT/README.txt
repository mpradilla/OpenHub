COMPILE PROGRAM

load module openmpi/1.6.5
cd /share/apps/pua/SIAT
make all

REBUILD
svn up
make clean
make all

EXECUTE DAEMON:
cd /share/apps/pua/SIAT

/share/apps/openmpi/1.6.5/bin/mpiexec --mca mtl mxm -n 5 -machinefile machines.txt /share/apps/pua/SIAT/bin/siat_daemon | tee results.txt

REMEMBER TO CHANGE	"-n 2" to "n -6" when all code is completed
master-hpc-mox should be first in machines.txt due socket server handling.
EDIT "machines.txt" and add node-1-amd to node-6-amd