#!/bin/awk -f
# input from stdin or a file.
# call as :
# print_line.awk -v LINE=1 <input file>
# to print line 1 of the file
BEGIN {
    lineno=1
}
 {
     if(lineno == LINE )
     {
	 print $0
     }
     lineno = lineno + 1
}
