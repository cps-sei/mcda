In the Model Checking for Distributed Applications (MCDA) project, we are developing tools and demonstrations for producing verifiably correct distributed applications (i.e., software implementing distributed algorithms) with safety-critical requirements. Our current focus is on applications where nodes are mobile and communicate wirelessly. A canonical example is a team of robots sharing a physical area that must achieve a goal without collisions.


---


**Approach**

We have developed a domain specific language -- called DASL -- for writing distributed applications and their safety properties assuming synchronous network communication. We have also developed a [verifying compiler](https://asimod.informatik.tu-muenchen.de/2004/SLIDES/slides/mod04_hoare_02.pdf) for DASL programs -- called `daslc` -- that first verifies a DASL program against its safety property using Software Model Checking (we are currently using [CBMC](http://www.cprover.org/cbmc/)). If the verification succeeds, `daslc` generates C++ code for the program against the [MADARA](http://madara.googlecode.com) middleware. MADARA ensures the synchronous network abstraction assumed by the application even if the underlying network is asynchronous with unbounded latency. `daslc` can also generate code that can be simulated with the [VREP](http://www.coppeliarobotics.com) simulator. See our [tutorial](https://code.google.com/p/mcda/wiki/Tutorial) for details.


---


**Using**

Download and compile the [source code](https://code.google.com/p/mcda/source/checkout), follow the [installation instructions](https://code.google.com/p/mcda/source/browse/INSTALL.txt), and then follow the [tutorial](https://code.google.com/p/mcda/wiki/Tutorial). Also, check out the [LICENSE](https://code.google.com/p/mcda/wiki/License).


---


**Papers**

  * [Model-Driven Verifying Compilation of Synchronous Distributed Applications](http://www.contrib.andrew.cmu.edu/~schaki/publications/MODELS-2014.html), Sagar Chaki, James Edmondson, Proceedings of the ACM/IEEE 17th International Conference on Model Driven Engineering Languages and Systems ([MODELS](http://modelsconference.org)), Sep 28-Oct 3, 2014, Valencia, Spain.
  * [Toward Parameterized Verification of Synchronous Distributed Applications](http://www.contrib.andrew.cmu.edu/~schaki/publications/SPIN-2014.html), Sagar Chaki, James Edmondson, Proceedings of the 21st International SPIN Symposium on Model Checking of Software ([SPIN](http://spin2014.org)), July 21-23, 2014, San Jose, CA, USA.


---


**Getting Involved**

Contact [James Edmondson](mailto:jedmondson@gmail.com) or [Sagar Chaki](mailto:sagar.chaki@gmail.com).