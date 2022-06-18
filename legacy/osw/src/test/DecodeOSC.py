import string
import math
import struct

## Helper functions for dealing w/ various data types

def readString(data):
    length   = string.find(data,"\0")
    nextData = int(math.ceil((length+1) / 4.0) * 4)
    return (data[0:length], data[nextData:])

def readBlob(data):
    length   = struct.unpack(">i", data[0:4])[0]    
    nextData = int(math.ceil((length) / 4.0) * 4) + 4   
    return (data[4:length+4], data[nextData:])

def readInt(data):
    if(len(data)<4):
        print "Error: too few bytes for int", data, len(data)
        rest = data
        integer = 0
    else:
        integer = struct.unpack(">i", data[0:4])[0]
        rest    = data[4:]
        
    return (integer, rest)

def readLong(data):
    """Tries to interpret the next 8 bytes of the data
    as a 64-bit signed integer."""
    high, low = struct.unpack(">ll", data[0:8])
    big = (long(high) << 32) + low
    rest = data[8:]
    return (big, rest)

def readFloat(data):
    if(len(data)<4):
        print "Error: too few bytes for float", data, len(data)
        rest = data
        float = 0
    else:
        float = struct.unpack(">f", data[0:4])[0]
        rest  = data[4:]
    return (float, rest)
    
def readDouble(data):
	if(len(data)<8):
		print "Error: too few bytes for double", data, len(data)
		rest = data
		float = 0
	else:
		float = struct.unpack(">d", data[0:8])[0]
		rest = data[8:]
	return (float, rest)

def readImpulse(data):
	return (1, data)

def readTrue(data):
	return (1, data)
	
def readFalse(data):
	return (0, data)
	
def readNil(data):
	return (None, data)
	
def readFloatVector(data):
	# how many samples???????
	size = struct.unpack(">i", data[0:4])[0] / 4
	data = data[4:]
	#print size, len(data), struct.calcsize(">" + `size` + "f")
	vector = struct.unpack("" + `size` + "f", data[0:4*size])
	return (vector, data[4*size:])
	pass

def readComplex(data):
    if(len(data)<8):
        print "Error: too few bytes for complex", data, len(data)
        rest = data
        real = 0
        imag = 0
    else:
        real = struct.unpack(">f", data[0:4])[0]
        imag = struct.unpack(">f", data[4:8])[0]
        rest  = data[8:]
    return (real + 1j * imag, rest)    

def parseArgs(args):
    """Given a list of strings, produces a list
    where those strings have been parsed (where
    possible) as floats or integers."""
    parsed = []
    for arg in args:
        print arg
        arg = arg.strip()
        interpretation = None
        try:
            interpretation = float(arg)
            if string.find(arg, ".") == -1:
                interpretation = int(interpretation)
        except:
            # Oh - it was a string.
            interpretation = arg
            pass
        parsed.append(interpretation)
    return parsed

unpackargmap = { "i":readInt, "f":readFloat, "d":readDouble, "s":readString, "b":readBlob, "t":readLong, "I":readImpulse, "T": readTrue, "F": readFalse, "N": readNil, "v": readFloatVector, "C":readComplex }

def decodeOSC(data):
    """Converts a typetagged OSC message to a Python list.
    Its not efficient to use this list to reconstruct an OSC message.
    Just use Message.setBinary instead.
    """
    decoded = []
    address,  rest = readString(data)
    typetags = ""

    if address == "#bundle":
        time, rest = readLong(rest)
        decoded.append(address)
        decoded.append(time)
        while len(rest)>0:
            length, rest = readInt(rest)
            decoded.append(decodeOSC(rest[:length]))
            rest = rest[length:]

    elif len(rest)>0:
        typetags, rest = readString(rest)
        decoded.append(address)
        decoded.append(typetags)
        if(typetags[0] == ","):
            for tag in typetags[1:]:
                value, rest = unpackargmap[tag](rest)
                decoded.append(value)
        else:
            print "Oops, typetag lacks the magic ,"

    return decoded
