#!/bin/bash
#
#	binload - move binaries from development machine to a bone
#

target_ip=''
if [ $# == 0 ]
    then
        printf "\n*** usage boneload <prod|dev|last part of the ip address>\n"
        exit
    else
        case "$1" in
            'prod' | '77')
            target_ip=192.168.254.77
        ;;
            'dev' | '34' | 'dev')
            target_ip=192.168.254.34
        ;;
        esac
fi

if [ -z $target_ip ]
	then
		printf "invalid parameter\nscript aborted\n"
	exit
fi

bone_BIN_DIR=/usr/local/sbin

# set directories
CURRENT_OS=$(uname)
if [ $CURRENT_OS == "Linux" ]
	then 
		program_DIR_PREFIX=/home/mam1/Git/Pcon-TNG
	else 
		program_DIR_PREFIX=/Users/mam1/Git/Pcon-TNG
fi

# # # move the bins to the bone
# printf "  copying binaries to device at $target_ip"
# printf "\n *** rsync -> $program_DIR_PREFIX/bin/ root@$target_ip:$bone_BIN_DIR\n"
# rsync -azv      $program_DIR_PREFIX/bin/ root@$target_ip:$bone_BIN_DIR
# printf "\n"

# # fix permissions
# ssh root@$target_ip chown root:root $bone_BIN_DIR/*	# change owner
# ssh root@$target_ip chmod 777 $bone_BIN_DIR/*		# open access