#! /bin/bash

function check_command {
local red='\e[0;31m' #red color for echo
local green='\e[0;32m' #green color for echo
local NC='\e[0m' #no color for echo

local passed="${green}passed${NC}"
local failed="${red}failed${NC}"

echo -n "Checking for ${1} ..... "
if ! type $1 &> /dev/null; then
	echo -e $failed
	echo "---Make sure that the expected command is on the evironments path."
else
	echo -e $passed
fi
}

function check_libs {
local red='\e[0;31m' #red color for echo
local green='\e[0;32m' #green color for echo
local NC='\e[0m' #no color for echo

local passed="${green}passed${NC}"
local created="${green}created${NC}"
local failed="${red}failed${NC}"

local lib=$1

echo -n "Checking for ${lib} in path ..... "

local out=`ldconfig -p | grep ${lib}`
if [[ -z "${out}" ]]; then
	echo -e $failed
	echo "---Failed to find '${lib}' sources"
else
	echo -e $passed
fi
}

function check_paths {
local red='\e[0;31m' #red color for echo
local green='\e[0;32m' #green color for echo
local NC='\e[0m' #no color for echo

local passed="${green}passed${NC}"
local created="${green}created${NC}"
local failed="${red}failed${NC}"

local isdir=$2
local name=$1
local create=$3

echo -n "Checking for ${name} ..... "
if [[ $isdir == "yes" ]]; then
	if [[ ! -d $name ]]; then
		if [[ $create == "yes" ]]; then
			echo -n "Creating dir ${name} ..... "
			mkdir $name > /dev/null
			if [[ ! -d $name ]]; then
				echo -e $failed
				echo "---Failed to create dir ${name} pleas create the directory"
			else
				echo -e $created
			fi
		else
			echo "---Cannot find directory ${name} you may have recieved a bad zip file please contact me"
		fi
	else
		echo -e $passed
	fi
else
	if [[ ! -e $name ]]; then
		echo -e $failed
		echo "---Cannot find file ${name} you may have recieved a bad zip file please contact me"
	else
		echo -e $passed
	fi
fi
}

echo "Starting environmental checks ...."
echo "===================================="
echo "Checking for commands ...."

check_command "g++"

echo "===================================="
echo "Checking for source files ...."

check_libs "opencv"

echo "===================================="
echo "Checking file directories .... "

check_paths "Makefile" "no" "no"
check_paths "run.sh" "no" "no"
check_paths "bin" "yes" "yes"
check_paths "Localization_CodeRedizine" "yes" "no"

red='\e[0;31m' #red color for echo
green='\e[0;32m' #green color for echo
NC='\e[0m' #no color for echo

passed="${green}passed${NC}"
created="${green}created${NC}"
failed="${red}failed${NC}"

echo -e "If all checks ${passed} or where ${created} then you can processed to compilation and execution!"
echo -e "If any checks ${failed} please fix them and run this script again or contact me!"