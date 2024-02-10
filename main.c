/*
    ID3 algorighm Implementation in C

    Copyright (c) 2009 Daniele Brunello
    Email: daniele.brunello.dev@gmail.com

    Permission is hereby granted, free of charge, to any person
    obtaining a copy of this software and associated documentation
    files (the "Software"), to deal in the Software without
    restriction, including without limitation the rights to use,
    copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following
    conditions:

    The above copyright notice and this permission notice shall be
    included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
    OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
    HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
    WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
    OTHER DEALINGS IN THE SOFTWARE.
*/

#include <stdio.h>
#include <stdlib.h>

// classified samples
static char *data_set[] =
{
//  attr 1      attr 2      attr 3      attr 4      class
	"SUNNY",  	"HOT",    	"HIGH",    	"WEAK",		"NO",       // day 1
	"SUNNY",	"HOT",    	"HIGH",    	"STRONG",	"NO",       // day 2
	"OVERCAST",	"HOT",    	"HIGH",   	"WEAK",		"YES",      // day 3
	"RAIN",		"MILD",   	"HIGH",   	"WEAK",		"YES",      // day 4
	"RAIN",		"COOL",		"NORMAL",  	"WEAK",		"YES",      // day 5
	"RAIN",		"COOL",  	"NORMAL",  	"STRONG",	"NO",       // day 6
	"OVERCAST",	"COOL",  	"NORMAL",  	"STRONG",	"YES",      // day 7
	"SUNNY", 	"MILD",  	"HIGH",    	"WEAK",		"NO",       // day 8
	"SUNNY", 	"COOL",  	"NORMAL",  	"WEAK",		"YES",      // day 9
	"RAIN",		"MILD",  	"NORMAL",  	"WEAK",		"YES",      // day 10
	"SUNNY", 	"MILD",  	"NORMAL",  	"STRONG",	"YES",      // day 11
	"OVERCAST",	"MILD",  	"HIGH",    	"STRONG",	"YES",      // day 12
	"OVERCAST",	"HOT", 		"NORMAL",  	"WEAK",		"YES",      // day 13
	"RAIN",  	"MILD",  	"HIGH",		"STRONG",	"NO",       // day 14
	NULL
};

int main()
{
    int result = 0;

    // string array for column headers
    // ATTENTION! column name size (5 in this case) MUST BE the sum of total attributes (4) and the final classification column (1)
    char *column_names[ 5 ] = { '\0' };

    column_names[ 0 ] = "OUTLOOK";          // label for attribute 1
    column_names[ 1 ] = "TEMPERATURE";      // label for attribute 2
    column_names[ 2 ] = "HUMIDITY";         // label for attribute 3
    column_names[ 3 ] = "WIND";             // label for attribute 4
    column_names[ 4 ] = "PLAY BALL";        // label for classification value, must be always the last column

    result = id3_get_rules(
        data_set,			// pointer to data set
        5,					// total columns : attributes + last column (classification)
        14,					// total database samples (rows), NULL is excluded
        column_names );

    printf( "Search result (%d) : ", result );
    if( result == 0 ) {
        printf( "OK" );
    } else {
        printf( "Error memory allocation" );
    }
    printf( "\n" );

    return 0;
}
