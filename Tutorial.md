This is a tutorial on using `daslc`, our verifying compiler for DASL programs. Make sure you have [downloaded and compiled](https://code.google.com/p/mcda/source/checkout) `daslc`. We also assume that environment variables `MCDA_ROOT`, `ACE_ROOT`, `MADARA_ROOT`, `LD_LIBRARY_PATH`, `PATH`, `VREP_MCDA_ROOT` are properly set according to [INSTALL.txt](https://code.google.com/p/mcda/source/browse/INSTALL.txt). Also, download [CBMC](http://www.cprover.org/cbmc) (version 4.7 or later) and ensure that `cbmc` is on your `PATH`. This has been tested on Kubuntu 13.10 and 14.04 64 bit, but should work for other platforms supported by MCDA (e.g., Windows 7) with appropriate change in commands.

### Example 1: Mutual Exclusion ###

**Step 1:** Go to the tutorials folder:
```
$ cd $MCDA_ROOT/docs/tutorial
```

**Step 2:** Verify the first example. The following will verify it for 3 nodes and 100 execution rounds:
```
$ daslc --nodes 3 --seq --rounds 100 --seq-dbl --out tutorial-01.c tutorial-01.dasl; cbmc tutorial-01.c
```

Verification takes about 100 seconds. You should expect an output like the following:
```
Sequentializing with double-buffering and 3 nodes ...
file tutorial-01.c: Parsing
Converting
Type-checking tutorial-01
file tutorial-01.c line 29 function INIT: function `c::nondet_state_0' is not declared
file tutorial-01.c line 30 function INIT: function `c::nondet_lock_i' is not declared
file tutorial-01.c line 34 function INIT: function `c::nondet_state_1' is not declared
file tutorial-01.c line 39 function INIT: function `c::nondet_state_2' is not declared
Generating GOTO Program
Adding CPROVER library
Function Pointer Removal
Partial Inlining
Generic Property Instrumentation
Starting Bounded Model Checking
**** WARNING: no body for function c::print_state
**** WARNING: no body for function c::should_enter
**** WARNING: no body for function c::my_sleep
**** WARNING: no body for function c::in_cs
size of program expression: 29648 steps
simple slicing removed 4 assignments
Generated 303 VCC(s), 303 remaining after simplification
Passing problem to propositional reduction
Running propositional reduction
Post-processing
Solving with MiniSAT 2.2.0 with simplifier
208862 variables, 1158091 clauses
SAT checker: negated claim is UNSATISFIABLE, i.e., holds
Runtime decision procedure: 25.797s
VERIFICATION SUCCESSFUL
```

The last output line shows that verification succeeded.

**Step 3:** Now generate code and compile:
```
$ daslc --nodes 3 --madara --out tutorial-01.cpp tutorial-01.dasl
$ g++ -I$ACE_ROOT -I$MADARA_ROOT/include -o tutorial-01 tutorial-01.cpp $MADARA_ROOT/libMADARA.so $ACE_ROOT/lib/libACE.so
```

This will produce an executable `tutorial-01`. You can also generate code for Windows as:
```
$ daslc --nodes 3 --madara --out tutorial-01.cpp --target WIN_CPP tutorial-01.dasl
```

You will need to compile the resulting `tutorial-01.cpp` using a suitable `C++` compiler for your Windows platform.

**Step 4:** Run the application. Use the following commands, each in a separate terminal:
```
$ ./tutorial-01 --id 0
$ ./tutorial-01 --id 1
$ ./tutorial-01 --id 2
```

You should expect the following output in the three terminals (left is id=0, middle is id=1, and right is id=2):
```
round = 1 : id = 0 : ---------   | round = 1 : id = 1 : ---------   | round = 1 : id = 2 : --------- 
round = 2 : id = 0 : ---------   | round = 2 : id = 1 : ---------   | round = 2 : id = 2 : --------- 
round = 3 : id = 0 : ---------   | round = 3 : id = 1 : ---------   | round = 3 : id = 2 : --------- 
round = 4 : id = 0 : ---------   | round = 4 : id = 1 : ---------   | round = 4 : id = 2 : ------ooo 
round = 5 : id = 0 : ---------   | round = 5 : id = 1 : ---------   | round = 5 : id = 2 : --------- 
round = 6 : id = 0 : ---------   | round = 6 : id = 1 : ---ooo---   | round = 6 : id = 2 : --------- 
round = 7 : id = 0 : ---------   | round = 7 : id = 1 : ---------   | round = 7 : id = 2 : --------- 
round = 8 : id = 0 : ooo------   | round = 8 : id = 1 : ---------   | round = 8 : id = 2 : --------- 
round = 9 : id = 0 : ---------   | round = 9 : id = 1 : ---------   | round = 9 : id = 2 : --------- 
round = 10 : id = 0 : ---------  | round = 10 : id = 1 : ---------  | round = 10 : id = 2 : ---------
round = 11 : id = 0 : ---------  | round = 11 : id = 1 : ---ooo---  | round = 11 : id = 2 : ---------
round = 12 : id = 0 : ---------  | round = 12 : id = 1 : ---------  | round = 12 : id = 2 : ---------
round = 13 : id = 0 : ooo------  | round = 13 : id = 1 : ---------  | round = 13 : id = 2 : ---------
round = 14 : id = 0 : ---------  | round = 14 : id = 1 : ---------  | round = 14 : id = 2 : ---------
round = 15 : id = 0 : ---------  | round = 15 : id = 1 : ---------  | round = 15 : id = 2 : ---------
round = 16 : id = 0 : ---------  | round = 16 : id = 1 : ---------  | round = 16 : id = 2 : ------ooo
round = 17 : id = 0 : ---------  | round = 17 : id = 1 : ---------  | round = 17 : id = 2 : ---------
round = 18 : id = 0 : ---------  | round = 18 : id = 1 : ---ooo---  | round = 18 : id = 2 : ---------
round = 19 : id = 0 : ---------  | round = 19 : id = 1 : ---------  | round = 19 : id = 2 : ---------
round = 20 : id = 0 : ooo------  | round = 20 : id = 1 : ---------  | round = 20 : id = 2 : ---------
round = 21 : id = 0 : ---------  | round = 21 : id = 1 : ---------  | round = 21 : id = 2 : ---------

============ id=0 =============  | ============ id=1 =============  | ============ id=2 =============
```

### Example 2: Collision Avoidance ###

**Step 1:** Go to the tutorials folder:
```
$ cd $MCDA_ROOT/docs/tutorial
```

**Step 2:** Verify the second example. The following will verify it for 3 nodes and 3 execution rounds:
```
$ daslc --nodes 3 --seq --rounds 3 --seq-dbl --out tutorial-02.c tutorial-02.dasl; cbmc tutorial-02.c
```

**Step 3:** Now generate code and compile:
```
$ daslc --nodes 3 --madara --out tutorial-02.cpp tutorial-02.dasl
$ g++ -I$ACE_ROOT -I$MADARA_ROOT/include -o tutorial-02 tutorial-02.cpp $MADARA_ROOT/libMADARA.so $ACE_ROOT/lib/libACE.so
```

This will produce an executable `tutorial-02`.

**Step 4:** Run the application. Use the following commands, each in a separate terminal:
```
$ ./tutorial-02 --id 0 --var_x 0 --var_y 5 --var_xf 9 --var_yf 5
$ ./tutorial-02 --id 1 --var_x 5 --var_y 0 --var_xf 5 --var_yf 9
$ ./tutorial-02 --id 2 --var_x 0 --var_y 0 --var_xf 8 --var_yf 9
```

### Example 3: VREP Simulation of Collision Avoidance ###

**Step 1:** Go to the tutorials folder:
```
$ cd $MCDA_ROOT/docs/tutorial
```

**Step 2:** Verify the second example. The following will verify it for 3 nodes and 3 execution rounds:
```
$ daslc --nodes 3 --seq --rounds 3 --seq-dbl --out tutorial-02.c tutorial-02.dasl; cbmc tutorial-02.c
```

**Step 3:** Now generate VREP code and compile:
```
$ daslc --nodes 3 --madara --vrep --out tutorial-02.cpp tutorial-02.dasl
$ g++ -I$MCDA_ROOT/src -I$VREP_MCDA_ROOT/programming/remoteApi -I$ACE_ROOT -I$MADARA_ROOT/include -o tutorial-02 tutorial-02.cpp $MADARA_ROOT/libMADARA.so $ACE_ROOT/lib/libACE.so $MCDA_ROOT/src/vrep/libDaslVrep.a -lpthread
```

This will produce an executable `tutorial-02`.

**Step 4:** Run the simulation using the supplied script `mcda-vrep.sh` as follows:
```
$ mcda-vrep.sh 3 outdir ./tutorial-02 "--id 0 --var_x 0 --var_y 5 --var_xf 9 --var_yf 5" "--id 1 --var_x 5 --var_y 0 --var_xf 5 --var_yf 9" "--id 2 --var_x 0 --var_y 0 --var_xf 8 --var_yf 9"
```

The script will start VREP and the three nodes. It will print the files where the output from VREP and the three nodes are saved. Press `Enter` on the terminal where `mcda-vrep.sh` was run to terminate the simulation. Here is a [video](https://code.google.com/p/mcda/source/browse/docs/tutorial/collision-avoidance-quadcopter.avi) of what you can expect to see. By default, the simulation uses the Quadrirotor VREP model. You can also use the Ant model using the following command:

```
$ mcda-vrep.sh 3 outdir ./tutorial-02 "-va --id 0 --var_x 1 --var_y 5 --var_xf 9 --var_yf 5" "-va --id 1 --var_x 5 --var_y 1 --var_xf 5 --var_yf 9" "-va --id 2 --var_x 1 --var_y 1 --var_xf 8 --var_yf 9"
```

Here is a [video](https://code.google.com/p/mcda/source/browse/docs/tutorial/collision-avoidance-ant.avi) of what you can expect to see.

### Have fun! ###