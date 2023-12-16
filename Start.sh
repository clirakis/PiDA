#
# 16-Dec-23 CBL Original
# 
# Kickoff all processes. 
#
#
# Where to store the data taken.
export DATAPATH=/home/clirakis/Data
# start IMU process
cd ICM-20948
./IMU &
cd ../GTOP
./GTOP &
cd ~
. .envFlask/bin/activate
cd PiDA/Flask
export FLASK_APP='PiDA.py'
flask run --host=0.0.0.0 --no-debug --reload >& flask.log &
#flask --app PiDA.py

