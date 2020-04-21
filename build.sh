#COMPILE&LINK
if gcc -g wav_split.c -DDEBUG -I/usr/local/Cellar/libsndfile/1.0.28/include -L/usr/local/Cellar/libsndfile/1.0.28/lib -lsndfile -Wall  -o ex; then
#RUN
./ex -v 2_chan_id.wav 
fi