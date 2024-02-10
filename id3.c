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
#include <string.h>
#include <stdarg.h>
#include <math.h>

#include "id3.h"

// uncomment / comment define to enable / disable fully verbose debug
//#define DO_DEBUG

#ifdef DO_DEBUG
	#define	DEBUG	printf
#else
	#define	DEBUG(...) do{}while(0);
#endif


/*
    string / value data
*/
struct dsinfo_t {
    char            	*name;
    long            	value;
	long				column;
    struct dsinfo_t   	*next;
    struct dsinfo_t   	*prev;
} dsinfo_t;

/*
	node data
*/
typedef struct node_tag {
	long				winvalue;
	long				tot_attrib;
	long				*avail_attrib;
	long				tot_samples;
	long				*samples;
	long				tot_nodes;
	struct node_tag	*nodes;
} node_t;

/*
	first scan of decision tree to gather information about max depth of branches and
	about maximum number of created rules
*/
static void scan_tree( node_t *node, long *max_depth, long *max_rules  )
{
	static int depth = 0;
	int j, i;

	// this is a recursive funcion, if node is null return to upper level
	if( node != NULL ) {
        // increase current depth
		depth += 1;
		// store max branches' depth
		if( depth > *max_depth ) {
            *max_depth = depth;
		}
		// store max number of found rules
		if( node->tot_nodes == 0 ) {
            *max_rules += 1;
		}

		j = 0;
		while( j < node->tot_nodes ) {
            // go deep...
			scan_tree( node->nodes+j, max_depth, max_rules );
            // decrease current depth
			depth -= 1;
			++j;
		}
	}
}

/*
	second scan of decision tree to gather rules for each class
*/
static void scan_rules( node_t *node, long class_id, long *depth, long *path, long maxdepth, long *table, long *tid )
{
	int j, i;

	// this is a recursive funcion, if node is null returns to upper level
	if( node != NULL ) {
        // increase depth
		*depth += 1;

		// update current path
		*( path + ( *depth - 1 ) ) = node->winvalue;

		// check if this is the last node of the branch
		if( node->tot_nodes == 0 && node->winvalue == class_id ) {

			for( i = 0; i < *( depth )-1; i++ ) {
				*( table + ( ( *tid ) * maxdepth ) + i ) = path[ i ];
			}
            *( tid ) +=1;
		}

        // scan every possible branch of this node
		j = 0;
		while( j < node->tot_nodes ) {
            // scan branch
			scan_rules( node->nodes+j, class_id, depth, path, maxdepth, table, tid );
            // decrease current depth
			*depth -= 1;
			// go to next branch
			++j;
		}
	}
}

/*
	extract rules from decision tree
*/
static void explain_rules( node_t *node, long cols, struct dsinfo_t *info, char **column_names, long maxdepth, long maxrules )
{
	struct dsinfo_t 	*infoptr 		= info;
	struct dsinfo_t 	*infoptr2 		= NULL;
	long				*rules_table	= NULL;
	long				tableins_id		= 0;
	long				rulestable_sz	= 0;
	long				*temp_path		= NULL;
	long				attrb			= 0;
	long				attrb_id		= 0;
	long				*attrb_name		= 0;
	long				depth			= 0;
	long				i, j, k;

	// allocate memory for rules
	rulestable_sz 	= sizeof( long ) * maxdepth * maxrules;
	rules_table 	= malloc( rulestable_sz );
	temp_path 		= malloc( sizeof( long ) * maxdepth );

	printf( "Found rules:\n\n");
	while( infoptr->next != NULL ) {
		if( infoptr->column == ( cols - 1 ) ) {
			printf( "Class %s\n", infoptr->name );

			i = 0;
			while( i < ( maxdepth * maxrules ) ) {
				*( rules_table + i ) = -1;
				++i;
			}

			for( i = 0; i < maxdepth; i++ )	{
                temp_path[ i ] = -1;
			}
			depth 		= 0;
			tableins_id = 0;

			scan_rules( node, infoptr->value, &depth, temp_path, maxdepth, rules_table, &tableins_id );

			/*
				Class (4): NO
								-1  0  2 -1
								-1  8  5 -1
								-1 -1 -1 -1
								-1 -1 -1 -1
								-1 -1 -1 -1
				Class (7): YES
								-1  0 11 -1
								-1  6 -1 -1
								-1  8  3 -1
								-1 -1 -1 -1
								-1 -1 -1 -1
			*/
			// print found rules for current class
			printf("\t\t");
			for( i = 0; i < maxrules; i++ ) {
				for( j = 0; j < (maxdepth-1); j++ ) {
					attrb = *( rules_table + i * maxdepth + j );
					if( attrb >= 0 ) {
						attrb_id 	= 0;
						infoptr2 	= info;
						while( infoptr2 != NULL ) {
							if( attrb == infoptr2->value ) {
								attrb_id 	= infoptr2->column;
								attrb_name 	= infoptr2->name;
								break;
							}
							infoptr2 = infoptr2->next;
						}

						printf( "if %s = %s ", column_names[ attrb_id ], attrb_name );
						if( *( rules_table + i * maxdepth + j + 1 ) >= 0 ) {
                            printf( "and " );
						} else {
                            printf( "\n\t\t" );
						}
					}
				}
			}
			printf("\n");
		}
		infoptr = infoptr->next;
	}

	free( temp_path );
	free( rules_table );
}

/*
	calculate entropy of sample
	- data: 		pointer to database
	- cols:			number of columns (attributes + class)
	- sample:		sample array
	- totsamples:	total samples
	- info:			class / attributes information
*/
static double calc_entropy_set( long *data, long cols, long *samples, long totsamples, struct dsinfo_t *info )
{
	double 			entropy		= 0;
	double				part		= 0;
	long				total		= 0;
	struct dsinfo_t  	*infoptr	= NULL;
	long				j;

	// search index of classes
	infoptr = info;
	while( infoptr != NULL ) {
		// when we found a class...
		if( infoptr->column == ( cols - 1 ) ) {
            // calculate entropy of database sample
			total = 0;
			for( j = 0; j < totsamples; j++ )
				if( data[ samples[ j ]*cols + cols - 1 ] == infoptr->value ) {
                    ++total;
				}
			// calculate rate
			if( total > 0 && totsamples > 0 ) {
				part	= (double)total / (double)totsamples;
				// sum class entropy to total entropy according to formula
				// Entropy = -p(I) log2( p(I) )
				entropy += ( -part * log2(part) );
			}
		}
		// proceed research
		infoptr = infoptr->next;
	}

	return entropy;
}

/*
	calculate info gain for each attribute
*/
static double calc_attrib_gain( long *data, long cols, long *samples, long totsamples, struct dsinfo_t *info, long attrib )
{
	struct dsinfo_t  	*infoptr 		= NULL;
	long				tot_attribtype 	= 0;
	long				tot_classtype	= 0;
	double 			    gain 			= 0;
	double				vpcgain			= 0;
	double				part			= 0;
	long				size			= 0;
	long				attrvalue		= 0;
	long				*classlist		= NULL;
	long				i = 0, j, k;

	struct vpc_t {
		long			class_id;
		long			tot_found;
	};

	struct gdata_t {
		long			value;
		long			tot_found;
		struct vpc_t	*vpc;
	};
	struct gdata_t		*gdata, *gdataptr;
	struct	vpc_t		*vpcptr;

	// calculate total possible values for attributes and classes
	infoptr = info;
	while( infoptr != NULL ) {
		// sum of all possible values of attribute
		if( infoptr->column == attrib ) {
            ++tot_attribtype;
		}
		// sum of all possible values of class
		if( infoptr->column == ( cols - 1 ) ) {
            ++tot_classtype;
		}
		// proceed research
		infoptr = infoptr->next;
	}

	// classlist array contains all possible classes
	classlist = malloc( sizeof( long ) * tot_classtype );
	infoptr = info , i = 0;
	while( infoptr != NULL ) {
		if( infoptr->column == ( cols - 1 ) ) {
            *( classlist + i++ ) = infoptr->value;
		}
		infoptr = infoptr->next;
	}

	// allocate memory for structure of each type of value of attribute
	size 	= sizeof( struct gdata_t ) * tot_attribtype;
	gdata 	= malloc( size );
	memset( gdata, 0, size );

	// initialize structure for each attribute's value
	i = 0, infoptr = info;
	while( infoptr != NULL ) {
		if( infoptr->column == attrib ) {
			gdataptr 				= gdata + i;
			gdataptr->value 		= infoptr->value;
			gdataptr->tot_found 	= 0;

			size = sizeof( struct vpc_t ) * tot_classtype;
			gdataptr->vpc 			= malloc( size );

			for( j = 0; j < tot_classtype; j++ ) {
				vpcptr 				= gdataptr->vpc + j;
				vpcptr->class_id	= *( classlist + j );
				vpcptr->tot_found	= 0;
			}
			++i;
		}
		// go on with search
		infoptr = infoptr->next;
	}

	// collect sample data about number of values for each attribute; moreover we calculate
	// how many value belong to a class or to another class
	for( i = 0; i < totsamples; i++ ) {
		for( j = 0; j < tot_attribtype; j++ ) {
			gdataptr = gdata + j;
			if( gdataptr->value == data[ samples[ i ]*cols + attrib ] ) {
				gdataptr->tot_found += 1;
				for( k = 0; k < tot_classtype; k++ ) {
					vpcptr = gdataptr->vpc;
					if( data[ samples[ i ]*cols + cols - 1 ] == ( vpcptr+k )->class_id )
							( vpcptr+k )->tot_found += 1;
				}
			}
		}
	}

	// calculate information gain
	for( i = 0; i < tot_attribtype; i++ ) {
		gdataptr 	= gdata + i;
		vpcgain		= 0;

		for( j = 0; j < tot_classtype; j++ ) {
			vpcptr 	= 	gdataptr->vpc + j;
			if( vpcptr->tot_found > 0 && gdataptr->tot_found > 0 ) {
				part	= 	(double)vpcptr->tot_found / (double)gdataptr->tot_found;
				vpcgain +=	( -( part ) * log2( part ) );
			}
 		}
		if( gdataptr->tot_found > 0 && totsamples > 0 ) {
			part	= (double) gdataptr->tot_found / (double) totsamples;
			gain 	+= ( -( part ) * vpcgain );
		}
	}

	// free all allocated memory
	for( i = 0; i < tot_attribtype; i++ ) {
		gdataptr = gdata + i;
		free( gdataptr->vpc );
	}
	free( gdata );
	free( classlist );

	return 	gain;
}

/*
	create tree nodes
*/
static void create_leaves( node_t *node, long *data, long cols, long rows, struct dsinfo_t *info )
{
	struct dsinfo_t  	*infoptr 		= NULL;
	double 			    entropy_set 	= 0;
	double				*gains			= NULL;
	double				max_gain		= 0;
	long				max_gain_id		= 0;
	long				gbuf_sz			= 0;
	long				max_attr_values	= 0;
	long				tot_new_samples	= 0;
	long				tot_avattrib	= 0;
	long				*sampleptr		= NULL;
	node_t				*node_ptr		= NULL;
	node_t				*new_node		= NULL;
	long				j, i;

	struct smplid_t
	{
		long 				value;
		struct smplid_t 	*next;
		struct smplid_t 	*prev;
	};

	struct smplid_t	*samplelist		= NULL;
	struct smplid_t	*samplelistptr	= NULL;
	struct smplid_t	*samplelistprv	= NULL;


	DEBUG( "Current node @ %p:\n", node );
	DEBUG( "\twinvalue        : %d\n", node->winvalue );
	DEBUG( "\ttot_samples     : %d\n", node->tot_samples );
	DEBUG( "\tsamples         : " );
	for( i = 0; i < node->tot_samples; i++ )
		DEBUG( "%-2d ", node->samples[ i ] );
	DEBUG( "\n\ttot_attrib      : %d (%d %d %d %d )\n", node->tot_attrib, node->avail_attrib[0],node->avail_attrib[1],node->avail_attrib[2],node->avail_attrib[3] );
	DEBUG( "\ttot_nodes       : %d\n", node->tot_nodes );
	DEBUG( "\tnodes           @ %p\n", node->nodes );


	// calulate entropy of samples part
	entropy_set = calc_entropy_set( data, cols, node->samples, node->tot_samples, info );

	DEBUG( "Entropy set = %3.6f\n", entropy_set );

	// value of entropy_set is crucial for deciding to proceed in branches creation:
	// if zero it means that examined samples are perfectly classified, if one samples
	// have no rules (are totally random); if the value is between zero and one we must
	// proceed and calculate the Gain for each available attribute
	if( entropy_set == 0.000f )	{

		node->nodes 				= malloc( sizeof( node_t ) );
		node->tot_nodes				= 1;
		node->nodes->tot_nodes 		= 0;
		node->nodes->winvalue 		= data[ node->samples[ 0 ] * cols + cols - 1 ];

		node->nodes->tot_attrib		= 0;
		node->nodes->avail_attrib	= NULL;
		node->nodes->tot_samples	= 0;
		node->nodes->samples		= NULL;
		node->nodes->nodes			= NULL;

		DEBUG( "\t\t\tTerminal node @ %p:\n", node->nodes );
		DEBUG( "\t\t\twinvalue        : %d\n", node->nodes->winvalue );
		DEBUG( "\t\t\ttot_samples     : %d\n", node->nodes->tot_samples );
		DEBUG( "\t\t\ttot_attrib      : %d\n", node->nodes->tot_attrib );
		DEBUG( "\t\t\ttot_nodes       : %d\n", node->nodes->tot_nodes );
		DEBUG( "\t\t\tnodes           @ %p\n", node->nodes->nodes );
	} else if( entropy_set == 1 ) {
		// totally random data = no rule at all
	} else {
		// calculate total number of available attributes
		tot_avattrib = 0;
		for( j = 0; j < ( cols - 1 ); j++ ) {
			if( node->avail_attrib[ j ] == 1 ) {
                tot_avattrib += 1;
			}
		}

		DEBUG( "\tCalculate entropy for each attribute ( total available %d )\n", tot_avattrib );
		// se c'e' piu' di un attributo disponibile
		if( tot_avattrib > 0 ) {
			// allocate memory for each attribute's gain
			gains = malloc( sizeof( double ) * ( cols - 1 ) );
			for( i = 0; i < ( cols - 1 ); i++ ) {
                gains[ i ] = 0;
			}
            // calculate gain for each attribute
			for( j = 0; j < ( cols - 1 ); j++ )
				if( node->avail_attrib[ j ] == 1 ) {
					gains[ j ] = entropy_set + calc_attrib_gain( data, cols, node->samples, node->tot_samples, info, j );
					DEBUG( "\tInfo Gain for attribute %d = %3.3f\n", j, gains[ j ] );
				}
			// find highest value
			for( j = 0; j < ( cols - 1 ); j++ ) {
				if( gains[ j ] > max_gain ) {
					max_gain	= gains[ j ];
					max_gain_id = j;
				}
			}

			// calcola il numero massimo possibile di valori per l'attributo vincente
			// calculate maximum number of values for winning attribute
			max_attr_values = 0;
			infoptr 		= info;
			while( infoptr != NULL ) {
				if( infoptr->column == max_gain_id ) {
                    ++max_attr_values;
				}
				infoptr = infoptr->next;
			}
			DEBUG( "\tAttribute %d has maximum IG (%3.3f) and %d type of values\n", max_gain_id, max_gain, max_attr_values );

			// create node for each possible attribute value
			// number of nodes is equel to all possible values for this attribute
			node->nodes 	= ( node_t* ) malloc( sizeof( node_t ) * max_attr_values );
			node->tot_nodes = max_attr_values;
			DEBUG( "\tAllocate memory for %d nodes @ %p\n", max_attr_values, node->nodes );

			infoptr 		= info;
			j = 0;
			while( infoptr != NULL ) {
				if( infoptr->column == max_gain_id ) {
					DEBUG( "\t\tSetup node value %d for attribute %d\n", infoptr->value, max_gain_id );

					node_ptr 	= node->nodes;
					node_ptr 	+= j;
					DEBUG( "\t\t\tnode_ptr = %p ( j = %d )\n", node_ptr, j );

					tot_new_samples = 0;

					// search for nodes those matching with infoptr->value, calculate total and store in tot_sample
					for( i = 0; i < node->tot_samples; i++ ) {
						if( data[ node->samples[ i ] * cols + max_gain_id ] == infoptr->value ) {
							if( samplelist == NULL ) {
								samplelist 				= malloc( sizeof( struct smplid_t ) );
								samplelist->value 		= node->samples[ i ];
								samplelist->next		= NULL;
								samplelist->prev		= NULL;
							} else {
								samplelistptr				= samplelist;
								while( samplelistptr->next != NULL ) samplelistptr = samplelistptr->next;
								samplelistptr->next			= malloc( sizeof( struct smplid_t ) );
								samplelistptr->next->prev 	= samplelistptr;
								samplelistptr 				= samplelistptr->next;
								samplelistptr->value 		= node->samples[ i ];
								samplelistptr->next			= NULL;
							}
							tot_new_samples += 1;
						}
					}

					node_ptr->winvalue		= infoptr->value;
					node_ptr->tot_nodes 	= 0;
					node_ptr->tot_samples 	= tot_new_samples;
					node_ptr->samples		= malloc( sizeof( long ) * tot_new_samples );
					sampleptr				= node_ptr->samples;

					samplelistptr			= samplelist;
					while( samplelistptr != NULL ) {
						*( sampleptr++ ) 	= samplelistptr->value;
						samplelistptr 		= samplelistptr->next;
					}

					// we can destroy temporary list once we have inserted index of new sample into array
					samplelistptr			= samplelist;
					samplelistprv			= samplelist;
					while( samplelistptr != NULL ) {
						samplelistprv = samplelistptr->next;
						free( samplelistptr );
						samplelistptr = samplelistprv;
					}
					samplelist = NULL;

					node_ptr->tot_attrib 	= ( cols - 1 );
					node_ptr->avail_attrib	= malloc( sizeof( long ) * ( cols - 1 ) );

					for( i = 0; i < cols-1; i++ ) {
                        node_ptr->avail_attrib[ i ] = node->avail_attrib[ i ];
					}
					node_ptr->avail_attrib[ max_gain_id ] = 0;

					DEBUG( "\t\t\tnode_ptr->winvalue    : %d\n", node_ptr->winvalue );
					DEBUG( "\t\t\tnode_ptr->tot_samples : %d\n", node_ptr->tot_samples );
					DEBUG( "\t\t\tnode_ptr->samples     : %p\n", node_ptr->samples );

					// recursively create child nodes
					if( node_ptr->tot_samples > 0 ) {
                        create_leaves( node_ptr, data, cols, rows, info );
					}
					++j;
				}
				infoptr = infoptr->next;
			}
			free( gains );
		} else {
			node->nodes 				= malloc( sizeof( node_t ) );
			node->tot_nodes				= 1;
			node->nodes->tot_nodes 		= 0;
			node->nodes->winvalue 		= data[ node->samples[ 0 ] * cols + cols - 1 ];

			node->nodes->tot_attrib		= 0;
			node->nodes->avail_attrib	= NULL;
			node->nodes->tot_samples	= 0;
			node->nodes->samples		= NULL;
			node->nodes->nodes			= NULL;

			DEBUG( "\t\t\tTerminal node @ %p:\n", node->nodes );
			DEBUG( "\t\t\twinvalue        : %d\n", node->nodes->winvalue );
			DEBUG( "\t\t\ttot_samples     : %d\n", node->nodes->tot_samples );
			DEBUG( "\t\t\ttot_attrib      : %d\n", node->nodes->tot_attrib );
			DEBUG( "\t\t\ttot_nodes       : %d\n", node->nodes->tot_nodes );
			DEBUG( "\t\t\tnodes           @ %p\n", node->nodes->nodes );
		}
	}
}

/*
	try to find dataset rules
*/
int id3_get_rules( char **data, long cols, long rows, char **column_names )
{
    long				*dataset		= NULL;     // pointer to dataset copy with numbers instead of strings
	unsigned long		dataset_size    = 0;        // dataset size (columns * rows)
	struct dsinfo_t  	*infolist		= NULL;     // pointer to dynamic list string/value
	struct dsinfo_t  	*insptr 		= NULL;     // insertion pointer while creating string/value list
	struct dsinfo_t  	*prvptr 		= NULL;
	struct dsinfo_t  	*prvass 		= NULL;
	char				label_found		= 0;        // label found flag
	char				infolisterror	= 0;        // memory error flag
	long				string_id		= 0;        // current string index
	long				assign_id		= 0;
	node_t		        *root			= NULL;     // root node
	long				tree_max_depth	= 0;
	long				tree_max_rules	= 0;
	int					result			= 0;
	int 				i = 0, j = 0, col = 0;

	DEBUG( "ID3 Init: cols = %d rows = %d dataset %p\n", cols, rows, data );

	do {
        // integer values comparison is faster than string comparison,
        // we create a copy of dataset with unique numbers instead of strings
		// calculate size of dataset
		dataset_size = sizeof( long ) * cols * rows;

		// allocate memory for dataset copy
		if( ( dataset = malloc( dataset_size ) ) == NULL ) {
			result = -2;
			break;
		}
		// reset dataset
		memset( dataset, 0, dataset_size );

		// full scan of original dataset (with strings) to gather all information to create a list of unique id for each string
		i = 0, col = 0;
		while( i < ( cols * rows ) ) {
			// se infolist e' NULL significa che l'elemento va ovviamente inserito nella lista
			insptr = NULL;
			// if infolist is NULL the list is empty
			if( infolist == NULL ) {
                // create the 1st element of the list
				infolist = malloc( sizeof( struct dsinfo_t ) );
				// check memory allocation error
				if( infolist == NULL ) {
					infolisterror = 1;
					break;
				}
				// insertion pointer points to the 1st item of the list
				insptr		= infolist;
				prvass		= NULL;
			} else {
                // if list is not empty we must search if any of already found items matches with current string (data[ i ])
				insptr 		= infolist;
				prvptr		= infolist;
				label_found	= 0;
				do {
                    // if we found a match...
					if( !strcmp( insptr->name, data[ i ] ) ) {
                        // set match found flag
						label_found = 1;
						// get value to insert into copy table
						assign_id	= insptr->value;
                        // we must not insert any new item into the list
						insptr		= NULL;
						break;
					}
					// continue with next item
					prvptr	= insptr;
					insptr 	= insptr->next;
				} while( insptr != NULL );

				// if we didn't found any match into the list...
				if( label_found == 0 ) {
                    // create a new item in the list
					prvptr->next 	= malloc( sizeof( struct dsinfo_t ) );
                    // check memory error
					if( prvptr->next == NULL ) {
						infolisterror = 1;
						break;
					}
					// set pointer where we must create a new item
					insptr			= prvptr->next;
					prvass			= prvptr;
				}
			}
			// insptr points to an already allocated memory for the new item of the list
			if( insptr != NULL ) {
                // insert value related to string
				assign_id		= string_id;
				// allocate memory to keep the string
				insptr->name 	= malloc( sizeof( char )*strlen( data[ i ] ) + 1 );
                // check memory error
				if( insptr->name == NULL ) {
					infolisterror = 1;
					break;
				}
                // insert string
				sprintf( insptr->name, data[ i ] );
				// insert current value and update value for next string
				insptr->value	= string_id++;
				insptr->column	= col;
				insptr->next	= NULL;
				insptr->prev	= prvass;
			}

			// update copy table with current value
			dataset[ i ] = assign_id;

			// update current column index of attribute
			if( ++col >= cols ) {
                col = 0;
			}
			// next item in original dataset
			i += 1;
		}
		// exit in case of memory error
		if( infolisterror ) {
			result = -3;
			break;
		}

		// debug string / value list
#ifdef DO_DEBUG
        struct dsinfo_t *p = infolist;
        while( p != NULL ) {
            printf( "name %-12s value %3d column %3d\n", p->name, p->value, p->column );
            p = p->next;
        }
#endif

        // create root node: tree creation starts from here
		if( ( root = ( node_t* ) malloc( sizeof( node_t ) ) ) == NULL ) {
			result = -4;
			break;
		}
		// we must examine full tree, as this is the root node
		root->tot_samples = rows;
		// create an array with indexes ( from 0 to row - 1 ) of all samples to be examined
		if( ( root->samples = malloc( sizeof( long ) * rows ) ) == NULL ) {
			result = -5;
			break;
		}
		// root node contains indexes of all database samples
		for( j = 0; j < rows; j++ ) {
            root->samples[ j ] = j;
		}
        // set all available attributes ( all columns except one, the class column)
		root->tot_attrib = ( cols - 1 );
		// we must evaluate all attributes ( cols -1 )
		if( ( root->avail_attrib = malloc( sizeof( long ) * ( cols - 1 ) ) ) == NULL ) {
			result = -6;
			break;
		}
		// we must check all attributes as we are in the root node
		for( j = 0; j < ( cols - 1 ); j++ )  {
            root->avail_attrib[ j ] = 1;
		}
		// value -1 identifies root node, moreover it has no branches at start
		root->winvalue		= -1;
		root->tot_nodes		= 0;

		DEBUG( "Root node @ %p:\n", root );
		DEBUG( "\twinvalue        : %d\n", root->winvalue );
		DEBUG( "\ttot_samples     : %d\n", root->tot_samples );
		DEBUG( "\tsamples         : " );
		for( i = 0; i < root->tot_samples; i++ )
			DEBUG( "%2d ", root->samples[ i ] );
		DEBUG( "\n\ttot_attrib      : %d (%d %d %d %d )\n", root->tot_attrib, root->avail_attrib[0],root->avail_attrib[1],root->avail_attrib[2],root->avail_attrib[3] );
		DEBUG( "\ttot_nodes       : %d\n", root->tot_nodes );
		DEBUG( "\tnodes           @ %p\n", root->nodes );


		// create tree and children nodes
		create_leaves( root, dataset, cols, rows, infolist );

		// scan tree
		scan_tree( root, &tree_max_depth, &tree_max_rules );

		// rules explanation
		explain_rules( root, cols, infolist, column_names, tree_max_depth, tree_max_rules );

	} while( 0 );

	// TODO free memory allocated for tree

	// free memory allocated for list string / value
	insptr = infolist;
	while( insptr != NULL ) {
		prvass = insptr->next;
		free( insptr );
		insptr = prvass;
	}
	// free memory allocated for copy table
	if( dataset != NULL ) {
        free( dataset );
	}

	return result;
}


