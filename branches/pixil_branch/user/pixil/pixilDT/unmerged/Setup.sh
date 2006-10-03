#!/bin/bash

echo -e "Welcome to PIXIL Desktop Setup, v1.2\n\n"
export PIXIL_BIN=`pwd`
export PIXIL_PORT_DEF="/dev/ttyS0"
export PIXIL_DATA_DEF="$HOME/.pixil"

echo "Where would you like to store your data?"
read -p "[$PIXIL_DATA_DEF] " PIXIL_DATA_TMP

if [ "$PIXIL_DATA_TMP" == "" ]
then
    PIXIL_DATA=$PIXIL_DATA_DEF
else
    PIXIL_DATA=$PIXIL_DATA_TMP
fi

echo -e "Storing data in $PIXIL_DATA\n\n"

echo "Which serial port would you like to use for data transfers?"
read -p "[$PIXIL_PORT_DEF] " PIXIL_PORT_TMP

if [ "$PIXIL_PORT_TMP" == "" ]
then
    PIXIL_PORT=$PIXIL_PORT_DEF
else
    PIXIL_PORT=$PIXIL_PORT_TMP
fi

echo -e "Using $PIXIL_PORT for data transfers.\n"

echo "#!/bin/bash" > $HOME/.pixilrc
echo "export PIXIL_DATA=$PIXIL_DATA" >> $HOME/.pixilrc
echo "export PIXIL_PORT=$PIXIL_PORT" >> $HOME/.pixilrc
echo "export PIXIL_BIN=$PIXIL_BIN" >> $HOME/.pixilrc

mkdir -p $PIXIL_DATA > /dev/null 2>&1
if [ $? -eq 1 ]
then
    echo "Failed to create PIXIL data directory [$PIXIL_DATA]."
    exit
fi
