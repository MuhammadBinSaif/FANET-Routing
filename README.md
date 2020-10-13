# A Novel Improved Artiﬁcial Bee Colony and Blockchain-Based Secure Clustering Routing Scheme for FANET 
Flying Ad hoc Network (FANET) has drawn significant consideration from industry and researchers with its rapid advancements and extensive use in civil and military applications. However, due to high mobility, its limited resources and distributed nature have posed a new challenge to develop a secure and efficient routing scheme for FANET. In this paper, a novel cluster based secure routing scheme is proposed, which aims to solve routing data security problem of FANET. To ensure the security, the optimal cluster head selection is studied based on an objective function with objectives, i) maximize residual energy, ii) maximize online time, iii) maximize reputation, iv) maximize transactions, v) minimize mobility, and vi) maximize connectivity, which exploits the fast and efficient improved ABC (IABC). The proposed IABC with two different search equations for employee bee and onlooker bee, retains fast convergence rate and maintains a balance between exploration and exploitation abilities. Further, a lightweight blockchain consensus algorithm, namely, AI-Proof of Witness Consensus Algorithm (AI-PoW), is introduced, in which we utilize IABC for the election of super mining node. The concept of witness for block verification is also involved to make the proposed scheme resource efficient and highly resilient against 51% attack. Extensive experiments are conducted with various network scenarios by comparing the proposed solution with other meta-heuristic based routing schemes. The results show that proposed scheme is superior in terms of packet delivery ratio, end-to-end delay, throughput, and block processing time.
# The Network Simulator, Version 3
1) Overview of NS3

NS-3 is a free open source project aiming to build a discrete-event network simulator targeted for simulation research and education. This is a collaborative project; we hope that the missing pieces of the models we have not yet implemented will be contributed by the community in an open collaboration process. The process of contributing to the ns-3 project varies with the people involved, the amount of time they can invest and the type of model they want to work on, but the current process that the project tries to follow is described here: http://www.nsnam.org/developers/contributing-code/ This README excerpts some details from a more extensive tutorial that is maintained at: http://www.nsnam.org/documentation/latest/

2) Building ns-3

The code for the framework and the default models provided by ns-3 is built as a set of libraries. User simulations are expected to be written as simple programs that make use of these ns-3 libraries. To build the set of default libraries and the example programs included in this package, you need to use the tool 'waf'. Detailed information on how use waf is included in the file doc/build.txt However, the real quick and dirty way to get started is to type the command 

./waf configure --enable-examples

followed by 

./waf

(If errors occurred during the build process, type the following command 

CXXFLAGS="-Wall" ./waf configure --enable-examples 

followed by 

./waf) 

in the the directory which contains this README file. The files built will be copied in the build/ directory. The current codebase is expected to build and run on the set of platforms listed in the RELEASE_NOTES file. Other platforms may or may not work: we welcome patches to improve the portability of the code to these other platforms.

3) Running ns-3

On recent Linux systems, once you have built ns-3 (with examples enabled), it should be easy to run the sample programs with the following command, such as:

./waf --run scratch/FanetSimple

4) Getting access to the ns-3 documentation

Once you have verified that your build of ns-3 works by running the simple-point-to-point example as outlined in 4) above, it is quite likely that you will want to get started on reading some ns-3 documentation. All of that documentation should always be available from the ns-3 website: http:://www.nsnam.org/documentation/.
This documentation includes:

- A Tutorial

- A Reference Manual

- Models In The Ns-3 Model Library

a wiki for user-contributed tips: http://www.nsnam.org/wiki/ API documentation generated using doxygen: this is a reference manual, most likely not very well suited as introductory text: http://www.nsnam.org/doxygen/index.html

5) Working with the development version of ns-3

If you have successfully installed mercurial, you can get a copy of the development version with the following command: "hg clone http://code.nsnam.org/ns-3-dev"
