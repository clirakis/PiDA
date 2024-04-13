#
# 16-Dec-23 CBL Original
# 
# Kickoff all processes. 
#
#
# Where to store the data taken.
export DATAPATH=/home/clirakis/Data
cd /home/clirakis/PiDA/GTOP
./GTOP &
#
# WAIT A MOMENT to allow the GPS shared memory to start. 
sleep 1
# start IMU process
cd /home/clirakis/PiDA/ICM-20948
./IMU &
cd ~
. .envFlask/bin/activate
cd /home/clirakis/PiDA/Flask
export FLASK_APP='PiDA.py'
flask run --host=0.0.0.0 --no-debug --reload >& flask.log &
#flask --app PiDA.py

