import os


fd0 = os.open("/dev/solution_node", os.O_RDWR | os.O_NOCTTY)
fd1 = os.open("/dev/solution_node", os.O_RDWR | os.O_NOCTTY)
#fd2 = os.open("/dev/solution_node", os.O_RDWR | os.O_NOCTTY)
#fd3 = os.open("/dev/solution_node", os.O_RDWR | os.O_NOCTTY)
#fd4 = os.open("/dev/solution_node", os.O_RDWR | os.O_NOCTTY)
f0 = open(fd0, "wb+", buffering=0)
f1 = open(fd1, "wb+", buffering=0)
#f2 = open(fd2, "wb+", buffering=0)
#f3 = open(fd3, "wb+", buffering=0)
#f4 = open(fd4, "wb+", buffering=0)

#print("Session 3 Testing that first read returns valid sid...", f3.read())
#print("Session 4 Testing that first read returns valid sid...", f4.read())
#print("Session 0 Testing that first read returns valid sid...", f0.read())
print("Session 1 Testing that first read returns valid sid...", f1.readline())
#print("Session 2 Testing that first read returns valid sid...", f2.read())

print("Session 1 seek(1,0): 1[]")
f1.seek(1, 0)
print("Session 1 read -> Should be []", f1.readline())

print("Session 1 Writing 'IXCZm': 1IXCZm[]")
f1.write(b'IXCZm')
print("Session 1 seek(0,0): [1]IXCZm")
f1.seek(0, 0)
print("Session 1 read -> should be 1IXCZm", f1.readline())

print("Session 1 seek(-1,2): 1IXCZ[m]")
f1.seek(-1, 2)

print("Should be m", f1.readline())

