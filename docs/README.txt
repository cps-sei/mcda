DAIG HELP DOCUMENTATION INFORMATION

0. INTRODUCTION

  VERSION: 0.0.1
  CONTACT: James Edmondson <jedmondson@gmail.com>
  WEBSITE: http://mcda.googlecode.com

  This file provides general overviews about the help documentation in this
  directory and where to find additional help documentation where applicable.
  The other files in this directory are input for the Doxygen program to 
  generate the latest documentation for all of the daig tools.

1. OBTAINING PREREQUISITES

  Project: Doxygen (free)
  URL: http://www.doxygen.org

       Click on the downloads link and download the binaries/setup file for
       your operating system. It is unlikely that you will want to download
       the source. On many Linux systems, you can use the system package
       manager of your choice to download Doxygen. Most modern versions
       should work.

  Project: Graphviz (free)
  URL: http://www.graphviz.org

       Click on downloads and navigate to your operating system. Without this
       package, you will not be able to generate helpful documentation.

2. GENERATING DEVELOPER DOCUMENTATION

  2.1. daig

       The (D)istributed (A)pplication (I)ntermediate (G)enerator is a tool
       for interpreting high level models of distributed applications and
       generating C or MADARA from it.

         perl get_version.pl
         doxygen Doxyfile_daig


3 COMMITTING DEVELOPER DOCUMENTATION

Developer documentation is not currently available online, but could be done
so via the following process:

Updating this documentation is done through the SVN repository by enabling
certain auto-props in your client side SVN settings and checking out the
docs repo to the docs/karl directory. The process for getting your machine
configured looks like the following:

  3.1. Configuring SVN

      Open your Subversion Configuration File. On TortoiseSVN (Windows), this
      is done by right clicking on a folder, going to TortoiseSVN->Settings,
      and then pressing the Edit button next to the Subversion Configuration 
      File on the General tab. Linux/Mac users should be able to edit the file
      at ~/.subversion/config. Inside of the file, you need to do the following:

      1) Uncomment or add "enable-auto-props = yes"
      2) Add the following to the config file after the [auto-props] tag. The
         only necessary ones are the *.css, *.html, *.js, *.gif, and *.png for
         Doxygen. The others are to ensure native line endings for your host
         OS during checkout/commit.


*.c = svn:eol-style=native
*.cpp = svn:eol-style=native
*.h = svn:eol-style=native
*.mpb = sv:eol-style=native
*.mpc = sv:eol-style=native
*.mwc = sv:eol-style=native
*.sh = svn:eol-style=native
*.txt = svn:eol-style=native
*.jpg = svn:mime-type=image/jpeg
*.css  = svn:mime-type=text/css
*.html = svn:mime-type=text/html
*.js   = svn:mime-type=text/javascript
*.gif  = svn:mime-type=image/gif
*.png  = svn:mime-type=image/png

  3.2. Checking out the Docs repo

      The doxygen documentation is not included in the trunk to provide a smaller
      checkout for main code development. To checkout or commit docs, you'll need
      to configure your directories to the appropriate svn directory.

      Because the "karl" directory is generated into the current folder, we recommend
      using this structure to your advantage. Create a "karl" directory in this folder
      or delete everything out of the "karl" directory if it exists and do an svn
      checkout to that folder that points to https://madara.googlecode.com/svn/docs/karl.
