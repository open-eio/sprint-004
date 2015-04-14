import urllib2
from random import randint
import time 

# replace these keys with those appropriate to your feed on data.sparkfun.com
#publicKey='9JyZ00pQg7iDyOYaWDrv'
#privateKey='xz4y55b8R1tbm5owZb47'

#publicKey='xR0W4ZM6vpCO4waYNRzA'
#privateKey='ZaevwqAKzrS2WmJoE89n'


# note: can access the phant data associated with the below public and private keys here: https://data.sparkfun.com/streams/AJjpo34AYQc1d401qA41

publicKey='AJjpo34AYQc1d401qA41'
privateKey='rzqrD4bYKMiznp0zqBpz'

while 1:

    # create a random number (in future, could be a measurement)
    r = randint(1,10)/10.

    print r

    # here, we're calling our parameter 'rando':

    temp = randint(1,1000)/10.
    conductivity = randint(1,1000)/10.
    depth = randint(1,1000)/10.

    sentence = 'https://data.sparkfun.com/input/'+str(publicKey)+'?private_key='+str(privateKey)+'&temp='+str(r)+'&conductivity='+str(conductivity)+'&depth='+str(depth)

    print sentence

    url_response = urllib2.urlopen(sentence)

    print url_response

    time.sleep(5) 

