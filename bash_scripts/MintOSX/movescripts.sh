#!/bin/bash
#
#	move scripts to /usr/local/myscripts and fix ownership and permissions
#

# set directories for scripts that need to be moved to the bone
# program_DIR := ~/Git/HAS/script_lib
# target_DIR := /usr/local/myscripts


echo moving scripts
rsync -avz ~/Git/HAS/bash_scripts/MintOSX/ /usr/local/myscripts/
	chown mam1 /usr/local/myscripts/*
	chmod 4755 /usr/local/myscripts/*

# @echo $(lib_SRCS)

# # move the scripts to script library 
# # ifeq ($(UNAME_S),Darwin)
# 	rsync -avz $(program_DIR)/OSXMint/ /usr/local/myscripts/
# # else
# # 	rsync -avz $(program_DIR)/Mint/ /usr/local/myscripts/
# # endif

# 	chown mam1 /usr/local/myscripts/*
# 	chmod 4755 /usr/local/myscripts/*

# os:
# 	echo "****************"
# 	echo $(UNAME_S)



