# ID3 algorithm implementation in C

This source code has been around since 2009 previously hosted on http://id3alg.altervista.org/ (link is broken).

This project is a C implementation of brilliant algorithm invented by Ross Quinlan. 
This source generates a decision tree from a dataset and (tries to) extract rules from a categorized dataset.

## Getting Started

### Dependencies

Pure C, only math library required. You can easily build source on any environment, either Windows, Linux or Mac.

### Build & Run

#### Linux

use gcc

\# gcc main.c id3.c -lm -o id3

\# ./id3

#### Windows

Easily build and run in a Code::Blocks project, remember to add link to "m" library in "Build options".

#### Mac

Sorry, never had a Mac in my life but I'm sure code will run on it

## Description

Use this database as a reference:
OUTLOOK, TEMPERATURE, HUMIDITY and WIND are attributes columns, PLAY BALL is classification column

| DAY	| OUTLOOK |	TEMPERATURE	| HUMIDITY | WIND |	PLAY BALL |
| --- | ------- | ----------- | -------- | ---- | --------- |
| D1 | Sunny | Hot | High | Weak | No |
| D2 | Sunny | Hot | High | Strong |	No |  
| D3 | Overcast | Hot | High | Weak | Yes | 
| D4 | Rain | Mild | High | Weak | Yes | 
| D5 | Rain | Cool | Normal | Weak | Yes | 
| D6 | Rain | Cool | Normal | Strong | No | 
| D7 | Overcast | Cool | Normal | Strong | Yes | 
| D8 | Sunny | Mild | High | Weak | No | 
| D9 | Sunny | Cool | Normal | Weak | Yes | 
| D10 | Rain | Mild | Normal | Weak | Yes | 
| D11 | Sunny | Mild | Normal | Strong | Yes | 
| D12 | Overcast | Mild | High | Strong | Yes | 
| D13 | Overcast | Hot | Normal | Weak | Yes | 
| D14 | Rain | Mild | High | Strong | No | 

In file main.c you can see how dataset must be declared for this implementation

```
char *dataset[] =
{
	"SUNNY",  	"HOT",    	"HIGH",    	"WEAK",		"NO",
	"SUNNY",	"HOT",    	"HIGH",    	"STRONG",	"NO",
	"OVERCAST",	"HOT",    	"HIGH",   	"WEAK",		"YES",
	"RAIN",		"MILD",   	"HIGH",   	"WEAK",		"YES",
	"RAIN",		"COOL",		"NORMAL",  	"WEAK",		"YES",
	"RAIN",		"COOL",  	"NORMAL",  	"STRONG",	"NO",
	"OVERCAST",	"COOL",  	"NORMAL",  	"STRONG",	"YES",
	"SUNNY", 	"MILD",  	"HIGH",    	"WEAK",		"NO",
	"SUNNY", 	"COOL",  	"NORMAL",  	"WEAK",		"YES",
	"RAIN",		"MILD",  	"NORMAL",  	"WEAK",		"YES",
	"SUNNY", 	"MILD",  	"NORMAL",  	"STRONG",	"YES",
	"OVERCAST",	"MILD",  	"HIGH",    	"STRONG",	"YES",
	"OVERCAST",	"HOT", 		"NORMAL",  	"WEAK",		"YES",
	"RAIN",  	"MILD",  	"HIGH",		"STRONG",	"NO",
	NULL
};
```

You have to call id3_get_rules() to create the decision tree as you can see in file main.c. This function requires essential parameters about dataset
pointer to dataset
dataset columns (attributes columns + class column)
dataset rows
list of column header strings

id3_get_rules() translates string into unique values; values comparison is faster than string comparison, and is more simple to treat in developing. So we have a conversion table like this

| ... | ... | ... | ... | ... |
| --- | --- | --- | --- | --- |
| Sunny | Hot | High | Weak | No |
| Overcast | Cool | High | Strong | No |
| ... | ... | ... | ... | ... |

translated to something like

| ... | ... | ... | ... | ... |
| --- | --- | --- | --- | --- |
| 1 | 2 | 3 | 4 | 5 | 
| 6 | 7 | 3 | 8 | 5 |
| ... | ... | ... | ... | ... |


Decision tree creation core is the create_leaves() function, that recursevely create new nodes according to the analized samples. 
First of all create_leaves() function calculates the entropy set of dataset samples indexed by values in samples parameter of node struct.
If entropy set has value 1.0 data is totally random and has no rules.
If entropy set has value 0.0 data contained in samples has been totally classified.
If 0.0 < entropy set < 1.0 function search for greatest gain attribute and create new nodes for all kind of values of attribute.
Formula

$Entropy(S) = S -p(I) log2 p(I)$

is solved by calc_entropy_set() function and

$Gain(S, A) = Entropy(S) - S ((|Sv| / |S|) * Entropy(Sv))$

formula is solved by calc_attrib_gain() function. Nodes contains this data types

```
struct node_t {
	long			winvalue;
	long			tot_attrib;
	long			*avail_attrib;
	long			tot_samples;
	long			*samples;
	long			tot_nodes;
	struct node_t		*nodes;
};
```

A short description: winvalue is the attribute value assigned to that node, it must be used in rules extraction, tot_attrib and avail_attrib are used in entropy calculation of samples pointed by samples;tot_nodes and nodes contains info about leaf nodes. 
At the end of create_leaves() function you get a tree like this

![alt text](https://github.com/dannyb79/id3/blob/main/tree.jpg?raw=true)

where
-1 is the root node
- 0 is Sunny value for attribute Outlook
- 6 is Overcast value for attribute Outlook
- 8 is Rain value for attribute Outlook
- 2 is High value for attribute Humidity
- 11 is Normal value for attribute Humidity
- 3 is Weak value for attribute Wind
- 5 is Strong value for attribute Wind
- 4 is value for Class NO
- 7 is value for Class YES

By now greatest part of work has done! We have all information to extract rules, feel free to navigate the tree as you want.
We can now extract the rules

Class YES

	IF outlook = SUNNY AND humidity = NORMAL
	IF outlook = OVERCAST
	IF outlook = RAIN AND Wind = WEAK

Class NO

	IF outlook = SUNNY AND humidity = HIGH
	IF outlook = RAIN AND Wind = STRONG

NOTE! This source code is still an experimental version, many optimizations can be done.

## Credit & License 

The MIT License (MIT)

Copyright (c) 2009 Daniele Brunello

Email: daniele.brunello.dev@gmail.com

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.



