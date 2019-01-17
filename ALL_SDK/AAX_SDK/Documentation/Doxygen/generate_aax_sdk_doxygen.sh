#!/bin/bash
#
#
# Copyright 2014-2017 by Avid Technology, Inc.
#
# Arguments:
#   --no-pdf: Skip PDF generation
#
#


#
# Parse arguments
#

NO_PDF_OPT="--no-pdf"

GENERATE_PDF=0
if [ $# -gt 0 ]; then
  if [ "$1" == "$NO_PDF_OPT" ]; then
    echo --no-pdf: pdf generation will be skipped
    let GENERATE_PDF=1
  fi
fi


#
# Initialize the shell
echo initializing shell...
#

# Change to the shell's directory
# http://stackoverflow.com/questions/59895/can-a-bash-script-tell-what-directory-its-stored-in
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd)"
#DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd && echo x)"
#DIR="${DIR%x}"
SUBDIR="/."
DOCDIR="$DIR/.."
cd "$DIR$SUBDIR"


#
# Run Doxygen
echo building doxygen...
#

# Remove any previous Doxygen output
echo   cleaning up previous output
if [ -d "output" ]; then
  rm -r output
fi

# Run Doxygen
echo   running doxygen
/Applications/Doxygen.app/Contents/Resources/doxygen AAX_SDK_Doxyfile >> doxygen_log.txt


#
# Fix up the HTML output
echo   fixing html
#

# Doxygen does not copy over images that are included using the <IMG> tag, so we must manually copy the contents of the /img directory to output
if [ -d "output/html" ]; then
  if [ -d "img" ]; then
    cp -r img/ output/html/
  fi
fi

# Put dot images into 'centered' class
perl -e 's#\<img(.*?)src\=\"dot_#\<img class="centered"\1src\=\"dot_#g;' -pi $(find ${DIR}/output/html/*.html -type f)

echo   html output generated at: ${DIR}/output/html/index.html

#
# Generate the PDF output from LaTeX
#

if [ $GENERATE_PDF -eq 0 ]; then
  # See http://stackoverflow.com/questions/592620/check-if-a-program-exists-from-a-bash-script/677212#677212
  if command -v pdflatex >/dev/null 2>&1; then
    echo generating pdf...
    cd "$DIR/output/latex"
    
    # set non-interactive mode
    sed -i "" 's/pdflatex refman/pdflatex -interaction nonstopmode refman/g' Makefile
    
    # build the pdf output
    make pdf >> doxygen_latex_to_pdf_log.txt
    
    # move the pdf to the main documentation folder
    mv refman.pdf ${DOCDIR}/doxygen.pdf
    echo   pdf output copied to: ${DOCDIR}/doxygen.pdf  
    
    cd "$DIR$SUBDIR"
  else
    echo pdf generation skipped due to missing pdflatex
  fi
else
  echo pdf generation skipped due to --no-pdf argument
fi
